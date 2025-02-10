# CMakeList.txt: проект CMake для ptm_ikit; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (ptm_ikit "ptm_ikit.cpp" "ptm_ikit.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ptm_ikit PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
