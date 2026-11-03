cmake_minimum_required(VERSION 3.22)

# Принудительная архитектура для вашей GeForce RTX 3060 Laptop (Ampere)
set(CMAKE_CUDA_ARCHITECTURES 86)

project(Gravity3DCuda LANGUAGES CXX CUDA)

# Настройки стандартов C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Смягчаем режим соответствия стандартам компилятора для NVCC
if(MSVC)
    string(APPEND CMAKE_CUDA_FLAGS " -Xcompiler /permissive")
endif()

# Главное приложение
add_executable(gravity3d_run 
    src/main.cpp 
    src/gravity3dCuda.cu
)
target_include_directories(gravity3d_run PRIVATE include)

# ==========================================
# ИНТЕГРАЦИЯ GOOGLETEST (Исправленный Git-метод)
# ==========================================
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0 # Фиксированная стабильная версия, совместимая с v142
)
# Форсируем сборку GTest в виде статических библиотек в памяти
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ==========================================
# СОЗДАНИЕ ТЕСТОВОГО ТАРГЕТА
# ==========================================
enable_testing()

add_executable(gravity3d_tests 
    tests/test_gravity.cpp 
    src/gravity3dCuda.cu
)
target_include_directories(gravity3d_tests PRIVATE include)

# Исправлено: привязываем современное пространство имен GTest
target_link_libraries(gravity3d_tests PRIVATE GTest::gtest_main)

# Регистрируем тест в тестовом движке CMake (CTest)
add_test(NAME Gravity3D_AllTests COMMAND gravity3d_tests)