cmake_minimum_required(VERSION 3.16)

project(LottoBaseSynchro VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Sql Network HttpServer Test)

qt_standard_project_setup()

# Application target
qt_add_executable(appLottoBaseSynchro
    src/main.cpp
    src/databasemanager.h
    src/databasemanager.cpp
    src/syncengine.h
    src/syncengine.cpp
    src/lottomodel.h
    src/lottomodel.cpp
)

qt_add_qml_module(appLottoBaseSynchro
    URI LottoSynchro
    VERSION 1.0
    QML_FILES
        src/Main.qml
)

target_link_libraries(appLottoBaseSynchro PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::Network
)

# Test target
enable_testing()

qt_add_executable(test_lotto_sync
    tests/test_sync.cpp
    src/databasemanager.h
    src/databasemanager.cpp
    src/syncengine.h
    src/syncengine.cpp
    src/lottomodel.h
    src/lottomodel.cpp
)

target_link_libraries(test_lotto_sync PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Network
    Qt6::HttpServer
    Qt6::Test
)

add_test(NAME test_lotto_sync COMMAND test_lotto_sync)
