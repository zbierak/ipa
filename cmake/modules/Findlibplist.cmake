find_package(PkgConfig)
pkg_check_modules(libpl libplist)
set(libplist_DEFINITIONS ${libpl_CFLAGS_OTHER})

find_path(libplist_INCLUDE_DIRS plist/plist.h
    HINTS
        ${libpl_INCLUDEDIR}
        ${libpl_INCLUDE_DIRS}
)

find_library(libplist_LIBRARIES NAMES plist libplist
    HINTS
        ${libpl_LIBDIR}
        ${libpl_LIBRARY_DIRS}
)
