# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_compile_options(-Wno-missing-field-initializers)

project(hal32cam)

idf_build_set_property(COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)
