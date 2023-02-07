//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKENIZER_H
#define CODE_TOKENIZER_H

#include <cctype>
#include "token.h"

class Tokenizer {
public:
    vector<TokenP> tokens;

    explicit Tokenizer(const string &filename, Error &error);

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

    template<typename KeywordTokenClass>
    static inline TokenP get_ident_or_keyword(char **buffer, const char *current, int line) {
        if (isalnum_(*(*buffer)++)) {
            while (isalnum_(*(*buffer)++));
            return make_shared<Identifier>(line, current, --(*buffer) - current);
        } else {
            (*buffer)--;
            return make_shared<KeywordTokenClass>(line);
        }
    }

    friend ostream &operator<<(ostream &out, const Tokenizer &self) {
        for (const auto &token: self.tokens) {
            out << *token << endl;
        }
        return out;
    }
};


#endif //CODE_TOKENIZER_H
