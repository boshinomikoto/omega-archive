cmake_minimum_required(VERSION 3.31)
project(POSINAPI)

include(FetchContent)

add_subdirectory(mylib)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

add_executable(POSINAPI main.cpp)

target_link_libraries(POSINAPI PRIVATE
        mylib
        ftxui::screen
        ftxui::dom
        ftxui::component
)