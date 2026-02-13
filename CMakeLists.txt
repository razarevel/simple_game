cmake_minimum_required(VERSION 3.23)

project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
add_subdirectory(thirdParty/imgui)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(game ${MY_SOURCES})

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")

target_include_directories(game
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/imgui"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(game
    PUBLIC
		glfw
		Vulkan::Vulkan
		GPUOpen::VulkanMemoryAllocator
		glslang::glslang 
		glslang::SPIRV 
		assimp::assimp
		imgui
)
