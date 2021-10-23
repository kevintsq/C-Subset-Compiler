//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKEN_H
#define CODE_TOKEN_H

#include <cstring>
#include <string>
#include "element.h"
#include "token_code.h"
#include "type_code.h"
#include "error.h"

class Token : public Element {
public:
    const int line;
    TokenCode type;

    Token(int line, TokenCode type) : line(line), type(type) {}
};

using TokenP = shared_ptr<Token>;

class Identifier;

using IdentP = shared_ptr<Identifier>;

class Instance : public Token {
public:
    bool is_const = false;
    TypeCode val_type;
    vector<ElementP> array_dims;
    int dereference_cnt = 0;
    bool is_func = false;
    bool is_called = false;
    bool is_defined = false;
    vector<IdentP> func_params;
    int nest_level = 0;

    Instance(int line, TokenCode type) : Token(line, type) {}

    Instance(const Instance &other) :
            Token(other),
            is_const(other.is_const),
            val_type(other.val_type),
            array_dims(other.array_dims),
            is_func(other.is_func),
            is_defined(other.is_defined),
            func_params(other.func_params) {}

    Instance &operator=(const Instance &other) {
        if (&other == this) {
            return *this;
        }
        is_const = other.is_const;
        val_type = other.val_type;
        array_dims = other.array_dims;
        dereference_cnt = 0;
        is_func = other.is_func;
        is_called = false;
        func_params = other.func_params;
        nest_level = 0;
        return *this;
    }
};

using InstanceP = shared_ptr<Instance>;

class Identifier : public Instance {  // it's actually an l-value
public:
    string value;

    Identifier(int line, const char *start, long long length) : Instance(line, IDENFR) {
        this->value.assign(start, length);
        this->name = "IDENFR " + value;
    }
};

class IntConst : public Instance {  // it's actually an r-value
public:
    long value;

    IntConst(int line, int value) : Instance(line, INTCON), value(value) {
        this->name = "INTCON " + std::to_string(this->value);
        this->val_type = INT;
        this->is_const = true;
    }
};

class FormatString : public Token {
public:
    string value;
    int fmt_char_cnt = 0;

    FormatString(int line, const char *start, long long length, int fmt_char_cnt) :
            Token(line, STRCON), fmt_char_cnt(fmt_char_cnt) {
        this->value.assign(start, length);
        this->name = "STRCON " + this->value;
    }
};

class MainToken : public Token {
public:
    explicit MainToken(int line) : Token(line, MAINTK) {
        this->name = "MAINTK main";
    }
};

class ConstToken : public Token {
public:
    explicit ConstToken(int line) : Token(line, CONSTTK) {
        this->name = "CONSTTK const";
    }
};

class IntToken : public Token {
public:
    explicit IntToken(int line) : Token(line, INTTK) {
        this->name = "INTTK int";
    }
};

class VoidToken : public Token {
public:
    explicit VoidToken(int line) : Token(line, VOIDTK) {
        this->name = "VOIDTK void";
    }
};

class BreakToken : public Token {
public:
    explicit BreakToken(int line) : Token(line, BREAKTK) {
        this->name = "BREAKTK break";
    }
};

class ContinueToken : public Token {
public:
    explicit ContinueToken(int line) : Token(line, CONTINUETK) {
        this->name = "CONTINUETK continue";
    }
};

class IfToken : public Token {
public:
    explicit IfToken(int line) : Token(line, IFTK) {
        this->name = "IFTK if";
    }
};

class ElseToken : public Token {
public:
    explicit ElseToken(int line) : Token(line, ELSETK) {
        this->name = "ELSETK else";
    }
};

class WhileToken : public Token {
public:
    explicit WhileToken(int line) : Token(line, WHILETK) {
        this->name = "WHILETK while";
    }
};

class GetintToken : public Token {
public:
    explicit GetintToken(int line) : Token(line, GETINTTK) {
        this->name = "GETINTTK getint";
    }
};

class PrintfToken : public Token {
public:
    explicit PrintfToken(int line) : Token(line, PRINTFTK) {
        this->name = "PRINTFTK printf";
    }
};

class ReturnToken : public Token {
public:
    explicit ReturnToken(int line) : Token(line, RETURNTK) {
        this->name = "RETURNTK return";
    }
};

class NotToken : public Token {
public:
    explicit NotToken(int line) : Token(line, NOT) {
        this->name = "NOT !";
    }
};

class AndToken : public Token {
public:
    explicit AndToken(int line) : Token(line, AND) {
        this->name = "AND &&";
    }
};

class OrToken : public Token {
public:
    explicit OrToken(int line) : Token(line, OR) {
        this->name = "OR ||";
    }
};

class AddToken : public Token {
public:
    explicit AddToken(int line) : Token(line, PLUS) {
        this->name = "PLUS +";
    }
};

class SubToken : public Token {
public:
    explicit SubToken(int line) : Token(line, MINU) {
        this->name = "MINU -";
    }
};

class MulToken : public Token {
public:
    explicit MulToken(int line) : Token(line, MULT) {
        this->name = "MULT *";
    }
};

class DivToken : public Token {
public:
    explicit DivToken(int line) : Token(line, DIV) {
        this->name = "DIV /";
    }
};

class ModToken : public Token {
public:
    explicit ModToken(int line) : Token(line, MOD) {
        this->name = "MOD %";
    }
};

class LtToken : public Token {
public:
    explicit LtToken(int line) : Token(line, LSS) {
        this->name = "LSS <";
    }
};

class LeToken : public Token {
public:
    explicit LeToken(int line) : Token(line, LEQ) {
        this->name = "LEQ <=";
    }
};

class GtToken : public Token {
public:
    explicit GtToken(int line) : Token(line, GRE) {
        this->name = "GRE >";
    }
};

class GeToken : public Token {
public:
    explicit GeToken(int line) : Token(line, GEQ) {
        this->name = "GEQ >=";
    }
};

class EqToken : public Token {
public:
    explicit EqToken(int line) : Token(line, EQL) {
        this->name = "EQL ==";
    }
};

class NeToken : public Token {
public:
    explicit NeToken(int line) : Token(line, NEQ) {
        this->name = "NEQ !=";
    }
};

class AssignToken : public Token {
public:
    explicit AssignToken(int line) : Token(line, ASSIGN) {
        this->name = "ASSIGN =";
    }
};

class Semicolon : public Token {
public:
    explicit Semicolon(int line) : Token(line, SEMICN) {
        this->name = "SEMICN ;";
    }
};

class Comma : public Token {
public:
    explicit Comma(int line) : Token(line, COMMA) {
        this->name = "COMMA ,";
    }
};

class LParen : public Token {
public:
    explicit LParen(int line) : Token(line, LPARENT) {
        this->name = "LPARENT (";
    }
};

class RParen : public Token {
public:
    explicit RParen(int line) : Token(line, RPARENT) {
        this->name = "RPARENT )";
    }
};

class LBracket : public Token {
public:
    explicit LBracket(int line) : Token(line, LBRACK) {
        this->name = "LBRACK [";
    }
};

class RBracket : public Token {
public:
    explicit RBracket(int line) : Token(line, RBRACK) {
        this->name = "RBRACK ]";
    }
};

class LBrace : public Token {
public:
    explicit LBrace(int line) : Token(line, LBRACE) {
        this->name = "LBRACE {";
    }
};

class RBrace : public Token {
public:
    explicit RBrace(int line) : Token(line, RBRACE) {
        this->name = "RBRACE }";
    }
};

#endif //CODE_TOKEN_H
