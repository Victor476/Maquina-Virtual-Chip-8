# 1. VERSÃO E NOME DO PROJETO
# Define a versão mínima do CMake necessária
cmake_minimum_required(VERSION 3.10) 

# Define o nome do projeto (o nome do seu executável será 'chip8_emulator')
project(CHIP8_EMULATOR CXX)

# 2. CONFIGURAÇÃO DO C++
# Define o padrão C++ a ser usado (C++17 atende bem aos requisitos de C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define o diretório de saída do executável
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

# Define o diretório de saída do executável
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

# 3. ADICIONAR SDL3 COMO SUBDIRETÓRIO E VINCULAR
# Este comando faz com que o CMake processe o CMakeLists.txt da SDL3.
# Isso irá definir targets de linkagem como SDL3::SDL3.
add_subdirectory(vendor/SDL3)

# 4. CONFIGURAÇÃO DE FONTES E TARGETS
file(GLOB SOURCE_FILES
    "src/*.cpp"
    "src/components/*.cpp"
)



# Cria o target executável
add_executable(chip8_emulator ${SOURCE_FILES})

# 5. VINCULAÇÃO DE BIBLIOTECAS E INCLUDES
# Vincula o executável ao target da SDL3 que acabamos de construir.
# Esta é a sintaxe moderna do CMake para linkar SDL3 a partir do código-fonte.
target_link_libraries(chip8_emulator PUBLIC 
    SDL3::SDL3 
)

# 6. CONFIGURAÇÕES ADICIONAIS
if(CMAKE_COMPILER_IS_GNUtoRClang)
    target_compile_options(chip8_emulator PUBLIC -Wall -Wextra -pedantic)
endif()

# Adicionado no final do CMakeLists.txt
add_custom_target(rebuild 
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/build
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${PROJECT_SOURCE_DIR} -B ${PROJECT_SOURCE_DIR}/build
    COMMAND ${CMAKE_COMMAND} --build ${PROJECT_SOURCE_DIR}/build -- -j
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    BYPRODUCTS chip8_emulator
)