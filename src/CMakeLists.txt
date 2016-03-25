add_definitions(-D_GNU_SOURCE -std=c11)

file(GLOB sources "*.c")
add_executable(${CMAKE_PROJECT_NAME} ${sources})

find_package(libimobiledevice REQUIRED)
find_package(libplist REQUIRED)
find_package(sqlite3 REQUIRED)

include_directories(
	${libimobiledevice_INCLUDE_DIRS}
	${libplist_INCLUDE_DIRS}
	${sqlite3_INCLUDE_DIRS}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
	${libimobiledevice_LIBRARIES}
	${libplist_LIBRARIES}
	${sqlite3_LIBRARIES}
)

add_definitions(
	${libimobiledevice_DEFINITIONS}
	${libplist_DEFINITIONS}
	${sqlite3_DEFINITIONS}
)
