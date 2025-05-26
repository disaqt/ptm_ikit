add_executable (lab1  main.cpp imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_impl_dx11.cpp imgui_impl_win32.cpp imgui_widgets.cpp imgui_tables.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lab1 PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(
	lab1 PUBLIC 
	"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")

target_link_directories(
	lab1 PUBLIC 
	"C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

target_link_libraries(
	lab1 PUBLIC
	 d3d11.lib
	 d3dx11.lib
	 dwmapi.lib
	 ${D3D11_LIBRARY}
	 ${D3DX11_LIBRARY}
)

target_include_directories(lab1 PUBLIC "${PROJECT_SOURCE_DIR}/lab1/include/")
