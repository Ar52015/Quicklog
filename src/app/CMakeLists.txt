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
