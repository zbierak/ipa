add_definitions(-D_GNU_SOURCE -std=c11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

file(GLOB sources "*.c")
add_executable(${CMAKE_PROJECT_NAME} ${sources})

find_package(PkgConfig)
pkg_check_modules(external REQUIRED
	glib-2.0
	sqlite3
	libimobiledevice-1.0
	libplist
	fuse
)

include_directories(${external_INCLUDE_DIRS})
add_definitions(${external_CFLAGS_OTHER})

target_link_libraries(${CMAKE_PROJECT_NAME}
	${external_LIBRARIES}
)