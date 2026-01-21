cmake_minimum_required(VERSION 3.16)

project(YtDlpGui VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 关键修正：开启 Qt 的自动代码生成 ===
set(CMAKE_AUTOMOC ON) # 解决 undefined reference to vtable/staticMetaObject
set(CMAKE_AUTORCC ON) # 自动处理资源文件
set(CMAKE_AUTOUIC ON) # 自动处理 .ui 文件

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        ParamData.h
)

add_executable(YtDlpGui ${PROJECT_SOURCES})

target_link_libraries(YtDlpGui PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

# Windows 下发布时隐藏控制台窗口
if(WIN32)
    set_target_properties(YtDlpGui PROPERTIES WIN32_EXECUTABLE ON)
endif()
