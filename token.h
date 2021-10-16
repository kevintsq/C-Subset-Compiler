//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKEN_H
#define CODE_TOKEN_H

#include <iostream>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include "element.h"
#include "token_code.h"
#include "type_code.h"


class Token : public Element {
public:
    TokenCode type;
    const int line;

    Token(int line, TokenCode type) : line(line), type(type) {}

    Token(const Token &token) = delete;
};


using TokenP = std::shared_ptr<Token>;


class Identifier : public Token {
public:
    std::string value;
    bool is_const = false;
    TypeCode val_type;
    std::vector<int> array_dims;
    std::vector<std::shared_ptr<Identifier>> func_params;
    std::vector<std::pair<int, int>> nest_info;

    Identifier(int line, const char *start, long long length) : Token(line, IDENFR) {
        this->value.assign(start, length);
        this->name = "IDENFR " + value;
    }

    Identifier(const Identifier &token) = delete;
};

using IdentP = std::shared_ptr<Identifier>;

class IntConst : public Token {
public:
    long value;

    IntConst(int line, int value) : Token(line, INTCON), value(value) {
        this->name = "INTCON " + std::to_string(this->value);
    }

    IntConst(const IntConst &token) = delete;
};

class FormatString : public Token {
public:
    std::string value;

    FormatString(int line, const char *start, long long length) : Token(line, STRCON) {
        this->value.assign(start, length);
        this->name = "STRCON " + this->value;
    }

    FormatString(const FormatString &token) = delete;
};

class MainToken : public Token {
public:
    explicit MainToken(int line) : Token(line, MAINTK) {
        this->name = "MAINTK main";
    }

    MainToken(const MainToken &token) = delete;
};

class ConstToken : public Token {
public:
    explicit ConstToken(int line) : Token(line, CONSTTK) {
        this->name = "CONSTTK const";
    }

    ConstToken(const ConstToken &token) = delete;
};

class IntToken : public Token {
public:
    explicit IntToken(int line) : Token(line, INTTK) {
        this->name = "INTTK int";
    }

    IntToken(const IntToken &token) = delete;
};

class VoidToken : public Token {
public:
    explicit VoidToken(int line) : Token(line, VOIDTK) {
        this->name = "VOIDTK void";
    }

    VoidToken(const VoidToken &token) = delete;
};

class BreakToken : public Token {
public:
    explicit BreakToken(int line) : Token(line, BREAKTK) {
        this->name = "BREAKTK break";
    }

    BreakToken(const BreakToken &token) = delete;
};

class ContinueToken : public Token {
public:
    explicit ContinueToken(int line) : Token(line, CONTINUETK) {
        this->name = "CONTINUETK continue";
    }

    ContinueToken(const ContinueToken &token) = delete;
};

class IfToken : public Token {
public:
    explicit IfToken(int line) : Token(line, IFTK) {
        this->name = "IFTK if";
    }

    IfToken(const IfToken &token) = delete;
};

class ElseToken : public Token {
public:
    explicit ElseToken(int line) : Token(line, ELSETK) {
        this->name = "ELSETK else";
    }

    ElseToken(const ElseToken &token) = delete;
};

class WhileToken : public Token {
public:
    explicit WhileToken(int line) : Token(line, WHILETK) {
        this->name = "WHILETK while";
    }

    WhileToken(const WhileToken &token) = delete;
};

class GetintToken : public Token {
public:
    explicit GetintToken(int line) : Token(line, GETINTTK) {
        this->name = "GETINTTK getint";
    }

    GetintToken(const GetintToken &token) = delete;
};

class PrintfToken : public Token {
public:
    explicit PrintfToken(int line) : Token(line, PRINTFTK) {
        this->name = "PRINTFTK printf";
    }

    PrintfToken(const PrintfToken &token) = delete;
};

class ReturnToken : public Token {
public:
    explicit ReturnToken(int line) : Token(line, RETURNTK) {
        this->name = "RETURNTK return";
    }

    ReturnToken(const ReturnToken &token) = delete;
};

class NotToken : public Token {
public:
    explicit NotToken(int line) : Token(line, NOT) {
        this->name = "NOT !";
    }

    NotToken(const NotToken &token) = delete;
};

class AndToken : public Token {
public:
    explicit AndToken(int line) : Token(line, AND) {
        this->name = "AND &&";
    }

    AndToken(const AndToken &token) = delete;
};

class OrToken : public Token {
public:
    explicit OrToken(int line) : Token(line, OR) {
        this->name = "OR ||";
    }

    OrToken(const OrToken &token) = delete;
};

class AddToken : public Token {
public:
    explicit AddToken(int line) : Token(line, PLUS) {
        this->name = "PLUS +";
    }

    AddToken(const AddToken &token) = delete;
};

class SubToken : public Token {
public:
    explicit SubToken(int line) : Token(line, MINU) {
        this->name = "MINU -";
    }

    SubToken(const SubToken &token) = delete;
};

class MulToken : public Token {
public:
    explicit MulToken(int line) : Token(line, MULT) {
        this->name = "MULT *";
    }

    MulToken(const MulToken &token) = delete;
};

class DivToken : public Token {
public:
    explicit DivToken(int line) : Token(line, DIV) {
        this->name = "DIV /";
    }

    DivToken(const DivToken &token) = delete;
};

class ModToken : public Token {
public:
    explicit ModToken(int line) : Token(line, MOD) {
        this->name = "MOD %";
    }

    ModToken(const ModToken &token) = delete;
};

class LtToken : public Token {
public:
    explicit LtToken(int line) : Token(line, LSS) {
        this->name = "LSS <";
    }

    LtToken(const LtToken &token) = delete;
};

class LeToken : public Token {
public:
    explicit LeToken(int line) : Token(line, LEQ) {
        this->name = "LEQ <=";
    }

    LeToken(const LeToken &token) = delete;
};

class GtToken : public Token {
public:
    explicit GtToken(int line) : Token(line, GRE) {
        this->name = "GRE >";
    }

    GtToken(const GtToken &token) = delete;
};

class GeToken : public Token {
public:
    explicit GeToken(int line) : Token(line, GEQ) {
        this->name = "GEQ >=";
    }

    GeToken(const GeToken &token) = delete;
};

class EqToken : public Token {
public:
    explicit EqToken(int line) : Token(line, EQL) {
        this->name = "EQL ==";
    }

    EqToken(const EqToken &token) = delete;
};

class NeToken : public Token {
public:
    explicit NeToken(int line) : Token(line, NEQ) {
        this->name = "NEQ !=";
    }

    NeToken(const NeToken &token) = delete;
};

class AssignToken : public Token {
public:
    explicit AssignToken(int line) : Token(line, ASSIGN) {
        this->name = "ASSIGN =";
    }

    AssignToken(const AssignToken &token) = delete;
};

class Semicolon : public Token {
public:
    explicit Semicolon(int line) : Token(line, SEMICN) {
        this->name = "SEMICN ;";
    }

    Semicolon(const Semicolon &token) = delete;
};

class Comma : public Token {
public:
    explicit Comma(int line) : Token(line, COMMA) {
        this->name = "COMMA ,";
    }

    Comma(const Comma &token) = delete;
};

class LParen : public Token {
public:
    explicit LParen(int line) : Token(line, LPARENT) {
        this->name = "LPARENT (";
    }

    LParen(const LParen &token) = delete;
};

class RParen : public Token {
public:
    explicit RParen(int line) : Token(line, RPARENT) {
        this->name = "RPARENT )";
    }

    RParen(const RParen &token) = delete;
};

class LBracket : public Token {
public:
    explicit LBracket(int line) : Token(line, LBRACK) {
        this->name = "LBRACK [";
    }

    LBracket(const LBracket &token) = delete;
};

class RBracket : public Token {
public:
    explicit RBracket(int line) : Token(line, RBRACK) {
        this->name = "RBRACK ]";
    }

    RBracket(const RBracket &token) = delete;
};

class LBrace : public Token {
public:
    explicit LBrace(int line) : Token(line, LBRACE) {
        this->name = "LBRACE {";
    }

    LBrace(const LBrace &token) = delete;
};

class RBrace : public Token {
public:
    explicit RBrace(int line) : Token(line, RBRACE) {
        this->name = "RBRACE }";
    }

    RBrace(const RBrace &token) = delete;
};

#endif //CODE_TOKEN_H
