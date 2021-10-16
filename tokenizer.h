//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKENIZER_H
#define CODE_TOKENIZER_H


#include <fstream>
#include <vector>
#include <cctype>
#include "token.h"


class Tokenizer {
public:
    std::vector<TokenP> tokens;

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
    static inline TokenP get_ident_or_keyword(char **buffer, const char *current, int line) {
        if (Tokenizer::isalnum_(*(*buffer)++)) {
            while (Tokenizer::isalnum_(*(*buffer)++));
            return std::make_shared<Identifier>(line, current, --(*buffer) - current);
        } else {
            (*buffer)--;
            return std::make_shared<KeywordTokenClass>(line);
        }
    }

    void print() {
        for (const auto& token: this->tokens) {
            token->print();
        }
    }
};


#endif //CODE_TOKENIZER_H
