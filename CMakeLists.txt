cmake_minimum_required(VERSION 3.14)
project(JsonView VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(JsonView
    main.cpp
    MainWindow.cpp
    JsonFile.cpp
    JsonTableModel.cpp
    JsonTreeItem.cpp
    JsonTreeModel.cpp
    JsonCellEditorDelegate.cpp
    json.cpp
    Locale.cpp
    treeViewUtil.cpp
)
target_link_libraries(JsonView PRIVATE Qt6::Widgets)
