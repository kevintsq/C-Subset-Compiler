//
// Created by Kevin Tan on 2021/10/23.
//

#ifndef CODE_ERROR_H
#define CODE_ERROR_H

#include <iostream>
#include <queue>

#define ERROR_EXPECTED_GOT(expected, got) do { cerr << "In " << __func__ << " line " << __LINE__ << " source code line " << (*(got))->line << ", expected "#expected", got " << **(got) << endl; exit(-1); } while (0)
#define ERROR_NOT_SUPPORTED(got) do { cerr << "In " << __func__ << " line " << __LINE__ << ", "#got" is not supported." << endl; exit(-1); } while (0)
#define ERROR_LIMITED_SUPPORT_WITH_LINE(line, support) do { cerr << "In " << __func__ << " line " << __LINE__ << " source code line " << (line) << ", only supports "#support << endl; exit(-1); } while (0)
#define ERROR_LIMITED_SUPPORT(support) do { cerr << "In " << __func__ << " line " << __LINE__ << ", only supports "#support << endl; exit(-1); } while (0)

using namespace std;

enum class ErrorCode {
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
using heapq = priority_queue<Pair, vector<Pair>, bool (*)(Pair &, Pair &)>;

class Error {
    static bool cmp(Pair &a, Pair &b) {
        return a.first == b.first ? a.second > b.second : a.first > b.first;
    }

public:
    heapq errors = heapq(cmp);

    inline void operator()(ErrorCode err, int line) {
        errors.emplace(line, 'a' + (char) err);
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
