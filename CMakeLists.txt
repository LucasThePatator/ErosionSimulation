# CMakeList.txt : projet CMake pour ErosionSimulation, incluez la source et définissez
# la logique spécifique au projet ici.
#
cmake_minimum_required (VERSION 3.14)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ErosionSimulation")

file(GLOB Imgui_SRC "E:/Libs/imgui-1.90/*.cpp")
file(GLOB Imgui_INCLUDE "E:/Libs/imgui-1.90")

message(INFO ${Imgui_SRC})

set(Imgui_BACKEND_SRC  "E:/Libs/imgui-1.90/backends/imgui_impl_opengl3.cpp" "E:/Libs/imgui-1.90/backends/imgui_impl_glfw.cpp")
set(Imgui_BACKEND_INCLUDE "E:/Libs/imgui-1.90/backends")


add_executable(ErosionSimulation "src/ErosionSimulation.cpp" 
								 "src/ErosionSimulation.h" 
								 "src/ErosionGenerator.cpp" 
								 "src/ErosionGenerator.h"  
								 "src/Hmap3DVisualizer.cpp" 
								 "src/Hmap3DVisualizer.h"
								 "E:/Libs/imgui-1.90/backends/imgui_impl_opengl3.cpp"
								 "E:/Libs/imgui-1.90/backends/imgui_impl_glfw.cpp"
								 ${Imgui_BACKEND_SRC}
								 ${Imgui_SRC}
								"src/Heightmap.h" 
								"src/Heightmap.cpp")

target_compile_features(ErosionSimulation PRIVATE cxx_std_17)

include_directories("E:/Workspace/FastNoise2/out/install/all/include" "E:/Libs/opencv/build/include" ${Imgui_INCLUDE} ${Imgui_BACKEND_INCLUDE} "E:/Libs/glfw-3.3.8.bin.WIN64/include" "E:/Libs/glew-2.1.0/include" "E:/Libs/glm")

target_link_directories(ErosionSimulation PUBLIC "E:/Workspace/FastNoise2/out/install/all/lib" 
												 "E:/Libs/OpenSceneGraph-3.6.5-VC2022-64-Release-2023-01/lib"
												 "E:/Libs/glfw-3.3.8.bin.WIN64/lib-vc2022"
												 "E:/Libs/glew-2.1.0/lib/Release/x64")

target_link_libraries(ErosionSimulation "FastNoise.lib"
										"E:/Libs/opencv/build/x64/vc16/lib/opencv_world480$<IF:$<CONFIG:Debug>,d,>.lib"
									    "glfw3_mt.lib" "glew32.lib" "opengl32.lib")


find_package(OpenMP REQUIRED)
target_link_libraries(ErosionSimulation OpenMP::OpenMP_CXX)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ErosionSimulation PROPERTY CXX_STANDARD 20)
endif()

# TODO: Ajoutez des tests et installez des cibles si nécessaire.
