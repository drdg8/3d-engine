cmake_minimum_required(VERSION 3.10)

project(project0)

set(THIRD_PARTY_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/external)


aux_source_directory(. PROJECT_SRC)

set(PROJECT_HDR ./Scene.h)

set(BASE_HDR ../base/application.h
             ../base/frame_rate_indicator.h
             ../base/input.h
             ../base/glsl_program.h
             ../base/camera.h
             ../base/frustum.h
             ../base/plane.h
             ../base/transform.h
             ../base/model.h
             ../base/bounding_box.h
             ../base/vertex.h
             ../base/cube.h
             ../base/cone.h
             ../base/cylinder.h
             ../base/sphere.h
             ../base/light.h)

set(BASE_SRC ../base/application.cpp 
             ../base/glsl_program.cpp 
             ../base/camera.cpp 
             ../base/transform.cpp
             ../base/cube.cpp
             ../base/cone.cpp
             ../base/cylinder.cpp
             ../base/sphere.cpp
             ../base/model.cpp)

add_executable(project0 ${PROJECT_SRC} ${PROJECT_HDR} ${BASE_SRC} ${BASE_HDR} )

if(WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
elseif(UNIX)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Debug")
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Release")
endif()
endif()

include_directories(${THIRD_PARTY_LIBRARY_PATH}/glm)
include_directories(${THIRD_PARTY_LIBRARY_PATH}/glad/include)
include_directories(${THIRD_PARTY_LIBRARY_PATH}/glfw/include)
include_directories(${THIRD_PARTY_LIBRARY_PATH}/tinyobjloader)
include_directories(${THIRD_PARTY_LIBRARY_PATH}/imgui)
include_directories(${THIRD_PARTY_LIBRARY_PATH}/tinygltf)

target_link_libraries(project0 glm)
target_link_libraries(project0 glad)
target_link_libraries(project0 glfw)
target_link_libraries(project0 tinyobjloader)
target_link_libraries(project0 imgui)
target_link_libraries(project0 tinygltf)