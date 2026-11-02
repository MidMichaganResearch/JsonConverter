cmake_minimum_required(VERSION 3.16)
project(PlotConverter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# List of source files for the PlotConverter library.
set(PLOTCONV_SOURCES
    src/converter.cpp
    src/core/data_row.cpp
    src/core/file_header.cpp
    src/core/json_writer.cpp
    src/core/number_token.cpp
    src/core/text_utils.cpp
    src/handlers/matrix_frames_handler.cpp
    src/handlers/matrix_frames_types.cpp
    src/handlers/single_matrix_handler.cpp
    src/handlers/single_matrix_types.cpp
    src/handlers/type_registry.cpp
    src/plotly/surface_figure.cpp
    src/handlers/line_series_handler.cpp
    src/handlers/line_series_types.cpp
    src/handlers/point_frames_handler.cpp
    src/handlers/point_frames_types.cpp
    src/plotly/figure_parts.cpp
    src/plotly/line_figure.cpp
    src/plotly/point_figure.cpp
)

# (To be implemented) Used when constructing the converter as a library.
add_library(plotconv STATIC ${PLOTCONV_SOURCES})
target_include_directories(plotconv PUBLIC src)

# Generates a CLI window for the converter.
add_executable(converter src/cli/main.cpp)
target_link_libraries(converter PRIVATE plotconv)

if(MSVC)
    target_compile_options(plotconv PRIVATE /W4)
    target_compile_options(converter PRIVATE /W4)
else()
    target_compile_options(plotconv PRIVATE -Wall -Wextra)
    target_compile_options(converter PRIVATE -Wall -Wextra)
endif()