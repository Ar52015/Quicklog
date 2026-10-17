qt_add_library(quicklogcore STATIC
    fronts.cpp fronts.h
    logedit.cpp logedit.h
    logfile.cpp logfile.h
    obsidian.cpp obsidian.h
)

target_include_directories(quicklogcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(quicklogcore PUBLIC Qt6::Core)
