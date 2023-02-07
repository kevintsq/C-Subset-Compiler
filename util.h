//
// Created by Kevin Tan on 2021/11/24.
//

#ifndef CODE_UTIL_H
#define CODE_UTIL_H

#include "opcode.h"
#include "error.h"

namespace util {
    static inline int binary_operation(BinaryOpCode opcode, int left, int right) {
        switch (opcode) {
            case BinaryOpCode::BINARY_ADD: return left + right;
            case BinaryOpCode::BINARY_SUB: return left - right;
            case BinaryOpCode::BINARY_MUL: return left * right;
            case BinaryOpCode::BINARY_DIV: return left / right;
            case BinaryOpCode::BINARY_MOD: return left % right;
            case BinaryOpCode::BINARY_EQ:  return left == right;
            case BinaryOpCode::BINARY_NE:  return left != right;
            case BinaryOpCode::BINARY_LT:  return left < right;
            case BinaryOpCode::BINARY_LE:  return left <= right;
            case BinaryOpCode::BINARY_GT:  return left > right;
            case BinaryOpCode::BINARY_GE:  return left >= right;
            case BinaryOpCode::BINARY_LOGICAL_AND: return left && right;
            case BinaryOpCode::BINARY_LOGICAL_OR:  return left || right;
            default: ERROR_NOT_SUPPORTED(NOTHING opreator);
        }
    }

    static inline int unary_operation(UnaryOpCode opcode, int value) {
        switch (opcode) {
            case UnaryOpCode::UNARY_POSITIVE: return +value;
            case UnaryOpCode::UNARY_NEGATIVE: return -value;
            case UnaryOpCode::UNARY_NOT: return !value;
            default: ERROR_NOT_SUPPORTED(NOTHING opreator);
        }
    }
}

#endif //CODE_UTIL_H
