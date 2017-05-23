var messageKeys = require('message_keys')
var windrose = require('./windrose.js')

var locationOptions = {'timeout': 30000, 'maximumAge': 5 * 60 * 1000, 'enableHighAccuracy': true}

Pebble.addEventListener('ready', function(e) {
  console.log('On ready!', JSON.stringify(e))
  updateWeather();
})

Pebble.addEventListener('appmessage', function(e) {
  console.log('Got AppMessage!', JSON.stringify(e))
  updateWeather()
})

function xhrRequest(url, type, callback) {
  var xhr = new XMLHttpRequest()
  xhr.onload = function() {
    callback(this.responseText)
  }
  xhr.open(type, url)
  xhr.send()
}

function updateWeather() {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions)

  function locationError(error) {
    console.warn('Location error (' + error.code + '): ' + error.message)
  }

  function locationSuccess(position) {
    var lat = position.coords.latitude
    var lon = position.coords.longitude

    console.info('Got location:' + lat + ', ' + lon)

    xhrRequest('https://tuuleeko.fi/fmiproxy/nearest-observations?lat=' + lat + '&lon=' + lon + '&latest=true&marineOnly=true', 'GET', function(responseText) {
      console.log('Got observations:', responseText)

      var result = JSON.parse(responseText)

      var data = {}
      data[messageKeys.Wind] = Math.round(result.observations.windSpeedMs) + ' ' + windrose.getPoint(result.observations.windDir, {depth: 1}).symbol
      data[messageKeys.Temperature] = Math.round(result.observations.temperature) + ''
      data[messageKeys.Pressure] = Math.round(result.observations.pressureMbar) + ''
      data[messageKeys.ObservationStation] = result.station.name

      Pebble.sendAppMessage(data)
    })
  }
}
