cmake_minimum_required(VERSION 3.16)

project(QuickLog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.11 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(quicklog
    main.cpp
)

qt_add_qml_module(quicklog
    URI Quicklog
    QML_FILES
        Main.qml
)

target_link_libraries(quicklog PRIVATE Qt6::Quick)
