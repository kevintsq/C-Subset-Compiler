//
// Created by Kevin Tan on 2021/11/18.
//

#ifndef CODE_OPCODE_H
#define CODE_OPCODE_H

enum OpCode {  // cannot use enum class because of macro string printing
    LOAD_NAME,
    LOAD_FAST,
    STORE_NAME,
    POP_TOP,
    BUILD_ARRAY,
    INIT_ARRAY,
    SUBSCR_ARRAY,

    STORE_SUBSCR,
    CALL_PRINTF,
    CALL_GETINT,
    EXIT_INTERP,
    JUMP_ABSOLUTE,
    POP_JUMP_IF_FALSE,
    POP_JUMP_IF_TRUE,
    CALL_FUNCTION,

    RETURN_VALUE,
    UNARY_OP,

    BINARY_OP,
    NOP
};

enum UnaryOpCode {  // cannot use enum class because of macro string printing
    UNARY_POSITIVE,
    UNARY_NEGATIVE,
    UNARY_NOT
};

enum BinaryOpCode {  // cannot use enum class because of macro string printing
    NOTHING = 0,
    BINARY_ADD,
    BINARY_SUB,
    BINARY_MUL,
    BINARY_DIV,
    BINARY_MOD,
    BINARY_EQ,
    BINARY_NE,
    BINARY_LT,
    BINARY_LE,
    BINARY_GT,
    BINARY_GE,
    BINARY_LOGICAL_AND,
    BINARY_LOGICAL_OR
};

#endif //CODE_OPCODE_H
