cmake_minimum_required(VERSION 3.18)
project(Code)

set(CMAKE_CXX_STANDARD 11)

add_executable(Code main.cpp tokenizer.cpp tokenizer.h token.h parser.cpp parser.h token_code.h type_code.h element.h error.h opcode.h vm.cpp vm.h instruction.h object.h util.h)

# -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Ofast -march=native"
# -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/O2 /favor:INTEL64 /arch:AVX2"