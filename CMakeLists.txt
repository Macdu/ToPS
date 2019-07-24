cmake_minimum_required(VERSION 3.8)

project(ToPS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Vulkan REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(glm)

SET(SOURCES 
	src/cpu/CPU.cpp
	src/cpu/interpreter/Disassembler.cpp
	src/cpu/interpreter/Interpreter.cpp
	src/gpu/renderer/SceneRendering.cpp
	src/gpu/renderer/VulkanRenderer.cpp
	src/gpu/GPU.cpp
	src/qt/MainWindow.cpp
	src/qt/RenderWindow.cpp
	src/Bios.cpp
	src/DMA.cpp
	src/Emulator.cpp
	src/Memory.cpp
	src/RAM.cpp
	main.cpp
)

SET(HEADERS 
	src/cpu/CPU.h
	src/cpu/CPUState.h
	src/cpu/interpreter/Disassembler.h
	src/cpu/interpreter/Interpreter.h
	src/gpu/renderer/SceneRendering.h
	src/gpu/renderer/VulkanRenderer.h
	src/gpu/renderer/VulkanVertex.h
	src/gpu/GPU.h
	src/gpu/GPUProperties.h
	src/qt/MainWindow.h
	src/qt/RenderWindow.h
	src/Bios.h
	src/definitions.h
	src/DMA.h
	src/Emulator.h
	src/Memory.cpp
	src/RAM.h
)

add_executable(ToPS ${SOURCES} ${HEADERS})
target_link_libraries(ToPS Qt5::Widgets Vulkan::Vulkan glm)