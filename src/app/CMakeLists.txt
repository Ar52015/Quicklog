qt_add_executable(quicklog
    main.cpp
    backend.cpp backend.h
)

set_source_files_properties(RosePine.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(quicklog
    URI Quicklog
    QML_FILES
        Main.qml
        EnterButton.qml
        InputFrame.qml
        NamePopup.qml
        RosePine.qml
)

target_link_libraries(quicklog PRIVATE Qt6::Quick quicklogcore)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(quicklog PRIVATE QUICKLOG_DEV)
endif()

install(TARGETS quicklog
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(quicklog.desktop.in quicklog.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/quicklog.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
