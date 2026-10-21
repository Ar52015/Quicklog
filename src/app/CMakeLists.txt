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
        PhasePopup.qml
        RosePine.qml
)

target_link_libraries(quicklog PRIVATE Qt6::Quick quicklogcore)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(quicklog PRIVATE QUICKLOG_DEV)
    set(bundle_suffix -dev)
endif()

if(APPLE)
    target_sources(quicklog PRIVATE macwindow.mm macwindow.h)
    set_target_properties(quicklog PROPERTIES
        OUTPUT_NAME QuickLog
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME QuickLog
        MACOSX_BUNDLE_GUI_IDENTIFIER io.github.ar52015.quicklog${bundle_suffix}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

install(TARGETS quicklog
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(APPLE)
    qt_generate_deploy_qml_app_script(
        TARGET quicklog
        OUTPUT_SCRIPT deploy_script
    )
    install(SCRIPT ${deploy_script})
else()
    configure_file(quicklog.desktop.in quicklog.desktop @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/quicklog.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()
