cmake_minimum_required(VERSION 3.15)

# Project name and language
project(pixlscan LANGUAGES CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Widgets Core Gui Svg PrintSupport REQUIRED)

# Enable clangd compilation database generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source files
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/doc_snapper.cpp
    src/mainwindow.cpp
    src/export_dialog.cpp
)

# Include OpenCV headers and link libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include/pixlscan")
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Svg Qt5::PrintSupport ${OpenCV_LIBS})
  
## Auto-generate Qt resource file for FontAwesome SVG icons
set(ICON_QRC "${CMAKE_BINARY_DIR}/icons_generated.qrc")
file(WRITE ${ICON_QRC} "<RCC>\n")
foreach(category IN ITEMS solid regular brands)
  file(APPEND ${ICON_QRC} "  <qresource prefix=\"/icons/${category}\">\n")
  file(GLOB ICON_FILES
       RELATIVE ${CMAKE_SOURCE_DIR}
       ${CMAKE_SOURCE_DIR}/fontawesome-free-6.7.2-web/svgs/${category}/*.svg)
  foreach(svg IN LISTS ICON_FILES)
    # Register resource under its base filename for simpler lookup
    get_filename_component(iconName ${svg} NAME)
    file(APPEND ${ICON_QRC} "    <file alias=\"${iconName}\">${CMAKE_SOURCE_DIR}/${svg}</file>\n")
  endforeach()
  file(APPEND ${ICON_QRC} "  </qresource>\n")
endforeach()
file(APPEND ${ICON_QRC} "</RCC>\n")
qt5_add_resources(ICON_RESOURCES ${ICON_QRC})
target_sources(${PROJECT_NAME} PRIVATE ${ICON_RESOURCES})

# Copy QDarkStyleSheet QSS into build directory
configure_file(${CMAKE_SOURCE_DIR}/qdarkstyle/dark/darkstyle.qss ${CMAKE_BINARY_DIR}/darkstyle.qss COPYONLY)
