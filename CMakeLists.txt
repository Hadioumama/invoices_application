cmake_minimum_required(VERSION 3.16)
project(FacturationApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt auto-generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Trouver Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Network)

# Sources
set(SOURCES
    main.cpp
    mainwindow.cpp
    database/database.cpp
    models/client.cpp
    dialogs/registerdialog.cpp
    utils/emailsender.cpp
)

# Exécutable
add_executable(FacturationApp ${SOURCES})

# Lier Qt
target_link_libraries(FacturationApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# Includes
target_include_directories(FacturationApp PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/models
    ${CMAKE_SOURCE_DIR}/dialogs
    ${CMAKE_SOURCE_DIR}/utils
)
