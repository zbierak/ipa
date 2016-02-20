find_package(PkgConfig)
pkg_check_modules(sql3 sqlite3)
set(sqlite3_DEFINITIONS ${sql3_CFLAGS_OTHER})

find_path(sqlite3_INCLUDE_DIRS sqlite3.h
    HINTS
        ${sql3_INCLUDEDIR}
        ${sql3_INCLUDE_DIRS}
)

find_library(sqlite3_LIBRARIES NAMES sqlite sqlite3
    HINTS
        ${sql3_LIBDIR}
        ${sql3_LIBRARY_DIRS}
)
