//
// Created by Kevin Tan on 2021/10/23.
//

#ifndef CODE_ERROR_H
#define CODE_ERROR_H

#include <iostream>
#include <queue>

using namespace std;

enum ErrorCode {
    ILLEGAL_CHAR,
    IDENT_REDEFINED,
    IDENT_UNDEFINED,
    PARAM_AMOUNT_MISMATCH,
    PARAM_TYPE_MISMATCH,
    RETURN_TYPE_MISMATCH,
    MISSING_RETURN,
    CANNOT_MODIFY_CONST,
    MISSING_SEMICN,
    MISSING_RPAREN,
    MISSING_RBRACK,
    FORMAT_STRING_ARGUMENT_MISMATCH,
    BREAK_CONTINUE_NOT_IN_LOOP
};

using Pair = pair<int, char>;
auto cmp = [](Pair &a, Pair &b) -> bool {
    return a.first == b.first ? a.second > b.second : a.first > b.first;
};
using heapq = priority_queue<Pair, vector<Pair>, decltype(cmp)>;

class Error {
public:
    heapq errors = heapq(cmp);

    inline void operator()(ErrorCode err, int line) {
        errors.push({line, 'a' + err});
    }

    friend ostream &operator<<(ostream &out, Error &e) {
        while (!e.errors.empty()) {
            auto top = e.errors.top();
            out << top.first << ' ' << top.second << endl;
            e.errors.pop();
        }
        return out;
    }
};

#endif //CODE_ERROR_H
