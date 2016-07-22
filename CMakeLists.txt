cmake_minimum_required (VERSION 2.8.11)
project(ipa)

option (IPA_RELEASE "Build a release version of ipa" ON)
if (IPA_RELEASE)
	add_definitions(-DLOG_LEVEL=LOG_LEVEL_ERROR)
	set(CMAKE_BUILD_TYPE Release)
else(IPA_RELEASE)
	add_definitions(-DLOG_LEVEL=LOG_LEVEL_ALL)
	add_definitions(-DENABLE_DEBUG_ENVIRONMENT)
endif(IPA_RELEASE)

add_subdirectory(src)
