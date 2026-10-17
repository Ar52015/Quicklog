cmake_minimum_required(VERSION 3.16)

project(QuickLog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.11 REQUIRED COMPONENTS Quick Test)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_library(quicklogcore STATIC
    fronts.cpp fronts.h
    logedit.cpp logedit.h
    logfile.cpp logfile.h
)

target_link_libraries(quicklogcore PUBLIC Qt6::Core)

qt_add_executable(quicklog
    main.cpp
    backend.cpp backend.h
)

qt_add_qml_module(quicklog
    URI Quicklog
    QML_FILES
        Main.qml
)

target_link_libraries(quicklog PRIVATE Qt6::Quick quicklogcore)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(quicklog PRIVATE QUICKLOG_DEV)
endif()

enable_testing()

qt_add_executable(tst_fronts tst_fronts.cpp)
target_link_libraries(tst_fronts PRIVATE Qt6::Test quicklogcore)
add_test(NAME tst_fronts COMMAND tst_fronts)

qt_add_executable(tst_logedit tst_logedit.cpp)
target_link_libraries(tst_logedit PRIVATE Qt6::Test quicklogcore)
add_test(NAME tst_logedit COMMAND tst_logedit)

qt_add_executable(tst_logfile tst_logfile.cpp)
target_link_libraries(tst_logfile PRIVATE Qt6::Test quicklogcore)
add_test(NAME tst_logfile COMMAND tst_logfile)
