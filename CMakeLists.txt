CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(marine_watchface)
SET(CMAKE_CXX_STANDARD 11)
SET(PEBBLE_FLAVOUR aplite)
SET(PEBBLE_SDK_VER 3.14)

SET(PEBBLE_SDK_INCLUDE_DIR "~/Library/Application Support/Pebble SDK/SDKs/${PEBBLE_SDK_VER}/sdk-core/pebble/${PEBBLE_FLAVOUR}/include")

INCLUDE_DIRECTORIES("${PEBBLE_SDK_INCLUDE_DIR}")
# The generated files such as resource_ids are picked up from here:
INCLUDE_DIRECTORIES("build/${PEBBLE_FLAVOUR}")
INCLUDE_DIRECTORIES("build/include")

ADD_SUBDIRECTORY(src)