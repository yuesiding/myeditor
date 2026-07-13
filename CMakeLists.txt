cmake_minimum_required(VERSION 3.16)

project(CodeEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

add_executable(CodeEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    editorwidget.cpp
    editorwidget.h
    highlighter.cpp
    highlighter.h
    cpphighlighter.cpp
    cpphighlighter.h
    findreplacedialog.cpp
    findreplacedialog.h
    theme.cpp
    theme.h
    thememanager.cpp
    thememanager.h
    syntaxdefinition.cpp
    syntaxdefinition.h
    generichighlighter.cpp
    generichighlighter.h
    syntaxmanager.cpp
    syntaxmanager.h
    preferencesdialog.cpp        
    preferencesdialog.h  
    task.h                   
    tasklistwidget.cpp       
    tasklistwidget.h 
    aiassistantwidget.cpp        
    aiassistantwidget.h
)

target_link_libraries(CodeEditor PRIVATE Qt5::Widgets Qt5::Network)


add_custom_command(TARGET CodeEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/syntax
        $<TARGET_FILE_DIR:CodeEditor>/syntax
)