find_package(PkgConfig)
pkg_check_modules(libi libimobiledevice-1.0)
set(libimobiledevice_DEFINITIONS ${libi_CFLAGS_OTHER})

find_path(libimobiledevice_INCLUDE_DIRS libimobiledevice/libimobiledevice.h
    HINTS
        ${libi_INCLUDEDIR}
        ${libi_INCLUDE_DIRS}
)

find_library(libimobiledevice_LIBRARIES NAMES imobiledevice libimobiledevice
    HINTS
        ${libi_LIBDIR}
        ${libi_LIBRARY_DIRS}
)
