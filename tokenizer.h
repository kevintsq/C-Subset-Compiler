//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKENIZER_H
#define CODE_TOKENIZER_H


#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <cctype>
#include "token.h"

class Tokenizer {
public:
    std::vector<Token *> tokens;

    explicit Tokenizer(const char *filename);

    static inline bool isalnum_(char c) {
        return isalnum(c) || c == '_';
    }

    static inline bool startswith(char **buffer, const char *keyword) {
        if (strstr(*buffer, keyword) == *buffer) {
            (*buffer) += strlen(keyword);
            return true;
        } else {
            return false;
        }
    }

    template <typename KeywordTokenClass>
    static inline Token *get_ident_or_keyword(char **buffer, const char *current, int line) {
        if (Tokenizer::isalnum_(*(*buffer)++)) {
            while (Tokenizer::isalnum_(*(*buffer)++));
            return new Identifier(line, current, --(*buffer) - current);
        } else {
            (*buffer)--;
            return new KeywordTokenClass(line);
        }
    }

    void print() {
        for (auto &token: this->tokens) {
            token->print();
        }
    }
};


#endif //CODE_TOKENIZER_H
