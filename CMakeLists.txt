cmake_minimum_required(VERSION 3.16)

project(ProjekAkhirAlpro VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    services/expirationchecker.h
    services/expirationchecker.cpp
    services/inventorymanager.h
    services/inventorymanager.cpp
    storage/JsonManager.h
    storage/JsonManager.cpp
    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/mainwindow.ui
    ui/DashboardPage.ui
    ui/ExpirationPage.ui
    ui/InventoryPage.ui
)

qt_add_executable(ProjekAkhirAlpro
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_include_directories(ProjekAkhirAlpro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

target_link_libraries(ProjekAkhirAlpro PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

qt_finalize_executable(ProjekAkhirAlpro)