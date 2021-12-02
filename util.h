//
// Created by Kevin Tan on 2021/11/24.
//

#ifndef CODE_UTIL_H
#define CODE_UTIL_H

#include "opcode.h"
#include "error.h"

class util {
public:
    static inline int binary_operation(BinaryOpCode opcode, int left, int right) {
        switch (opcode) {
            case BINARY_ADD: return left + right;
            case BINARY_SUB: return left - right;
            case BINARY_MUL: return left * right;
            case BINARY_DIV: return left / right;
            case BINARY_MOD: return left % right;
            case BINARY_EQ:  return left == right;
            case BINARY_NE:  return left != right;
            case BINARY_LT:  return left < right;
            case BINARY_LE:  return left <= right;
            case BINARY_GT:  return left > right;
            case BINARY_GE:  return left >= right;
            case BINARY_LOGICAL_AND: return left && right;
            case BINARY_LOGICAL_OR:  return left || right;
            default: ERROR_NOT_SUPPORTED(NOTHING opreator);
        }
    }

    static inline int unary_operation(UnaryOpCode opcode, int value) {
        switch (opcode) {
            case UNARY_POSITIVE: return +value;
            case UNARY_NEGATIVE: return -value;
            case UNARY_NOT: return !value;
            default: ERROR_NOT_SUPPORTED(NOTHING opreator);
        }
    }
};

#endif //CODE_UTIL_H
