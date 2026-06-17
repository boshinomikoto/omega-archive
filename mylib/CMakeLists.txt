include(FetchContent)

FetchContent_Declare(sqlite3
        URL https://www.sqlite.org/2026/sqlite-amalgamation-3530200.zip
)
FetchContent_MakeAvailable(sqlite3)

add_library(mylib STATIC
        omegadb.c
        omegadb.h
        auth.c
        auth.h
        ${sqlite3_SOURCE_DIR}/sqlite3.c
)

target_include_directories(mylib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${sqlite3_SOURCE_DIR}
)
