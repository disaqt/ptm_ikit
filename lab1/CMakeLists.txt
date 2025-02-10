add_executable (lab1  "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lab1 PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(lab1 PUBLIC "${PROJECT_SOURCE_DIR}/lab1/include/")
