cmake_minimum_required(VERSION 3.16)

project(ProjekAkhirAlpro VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()


qt_add_executable(ProjekAkhirAlpro
    main.cpp

    models/FoodItem.h

    services/expirationchecker.h
    services/expirationchecker.cpp
    services/inventorymanager.h
    services/inventorymanager.cpp

    storage/JsonManager.h
    storage/JsonManager.cpp

    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/mainwindow.ui

    ui/DashboardPage.h
    ui/DashboardPage.cpp
    ui/DashboardPage.ui

    ui/ExpirationPage.ui

    ui/InventoryPage.h
    ui/InventoryPage.cpp
    ui/InventoryPage.ui

    ui/addfooddialog.h
    ui/addfooddialog.cpp
    ui/addfooddialog.ui
)

target_include_directories(ProjekAkhirAlpro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

target_link_libraries(ProjekAkhirAlpro PRIVATE Qt6::Widgets)
