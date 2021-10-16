cmake_minimum_required(VERSION 3.18)
project(Code)

set(CMAKE_CXX_STANDARD 11)

add_executable(Code main.cpp tokenizer.cpp tokenizer.h token.cpp token.h parser.cpp parser.h token_code.h type_code.h element.cpp element.h)
