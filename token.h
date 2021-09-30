//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKEN_H
#define CODE_TOKEN_H

#include <iostream>
#include <cstring>
#include <memory>
#include "token_code.h"


class Token {
public:
    TokenCode type;
    int line;

    explicit Token(int line) : line(line) {}

    Token(const Token &token) = delete;

    virtual void print() = 0;
};


using TokenP = std::shared_ptr<Token *>;


class Identifier : public Token {
public:
    std::string value;

    Identifier(int line, const char *start, long long length) : Token(line) {
        this->type = IDENFR;
        this->value.assign(start, length);
    }

    Identifier(const Identifier &token) = delete;

    void print() override {
        std::cout << "IDENFR " << this->value << std::endl;
    }
};

class IntConst : public Token {
public:
    long value;

    IntConst(int line, int value) : Token(line), value(value) {
        this->type = INTCON;
    }

    IntConst(const IntConst &token) = delete;

    void print() override {
        std::cout << "INTCON " << this->value << std::endl;
    }
};

class FormatString : public Token {
public:
    std::string value;

    FormatString(int line, const char *start, long long length) : Token(line) {
        this->type = STRCON;
        this->value.assign(start, length);
    }

    FormatString(const FormatString &token) = delete;

    void print() override {
        std::cout << "STRCON " << this->value << std::endl;
    }
};

class MainToken : public Token {
public:
    explicit MainToken(int line) : Token(line) {
        this->type = MAINTK;
    }

    MainToken(const MainToken &token) = delete;

    void print() override {
        std::cout << "MAINTK main" << std::endl;
    }
};

class ConstToken : public Token {
public:
    explicit ConstToken(int line) : Token(line) {
        this->type = CONSTTK;
    }

    ConstToken(const ConstToken &token) = delete;

    void print() override {
        std::cout << "CONSTTK const" << std::endl;
    }
};

class IntToken : public Token {
public:
    explicit IntToken(int line) : Token(line) {
        this->type = INTTK;
    }

    IntToken(const IntToken &token) = delete;

    void print() override {
        std::cout << "INTTK int" << std::endl;
    }
};

class VoidToken : public Token {
public:
    explicit VoidToken(int line) : Token(line) {
        this->type = VOIDTK;
    }

    VoidToken(const VoidToken &token) = delete;

    void print() override {
        std::cout << "VOIDTK void" << std::endl;
    }
};

class BreakToken : public Token {
public:
    explicit BreakToken(int line) : Token(line) {
        this->type = BREAKTK;
    }

    BreakToken(const BreakToken &token) = delete;

    void print() override {
        std::cout << "BREAKTK break" << std::endl;
    }
};

class ContinueToken : public Token {
public:
    explicit ContinueToken(int line) : Token(line) {
        this->type = CONTINUETK;
    }

    ContinueToken(const ContinueToken &token) = delete;

    void print() override {
        std::cout << "CONTINUETK continue" << std::endl;
    }
};

class IfToken : public Token {
public:
    explicit IfToken(int line) : Token(line) {
        this->type = IFTK;
    }

    IfToken(const IfToken &token) = delete;

    void print() override {
        std::cout << "IFTK if" << std::endl;
    }
};

class ElseToken : public Token {
public:
    explicit ElseToken(int line) : Token(line) {
        this->type = ELSETK;
    }

    ElseToken(const ElseToken &token) = delete;

    void print() override {
        std::cout << "ELSETK else" << std::endl;
    }
};

class WhileToken : public Token {
public:
    explicit WhileToken(int line) : Token(line) {
        this->type = WHILETK;
    }

    WhileToken(const WhileToken &token) = delete;

    void print() override {
        std::cout << "WHILETK while" << std::endl;
    }
};

class GetintToken : public Token {
public:
    explicit GetintToken(int line) : Token(line) {
        this->type = GETINTTK;
    }

    GetintToken(const GetintToken &token) = delete;

    void print() override {
        std::cout << "GETINTTK getint" << std::endl;
    }
};

class PrintfToken : public Token {
public:
    explicit PrintfToken(int line) : Token(line) {
        this->type = PRINTFTK;
    }

    PrintfToken(const PrintfToken &token) = delete;

    void print() override {
        std::cout << "PRINTFTK printf" << std::endl;
    }
};

class ReturnToken : public Token {
public:
    explicit ReturnToken(int line) : Token(line) {
        this->type = RETURNTK;
    }

    ReturnToken(const ReturnToken &token) = delete;

    void print() override {
        std::cout << "RETURNTK return" << std::endl;
    }
};

class NotToken : public Token {
public:
    explicit NotToken(int line) : Token(line) {
        this->type = NOT;
    }

    NotToken(const NotToken &token) = delete;

    void print() override {
        std::cout << "NOT !" << std::endl;
    }
};

class AndToken : public Token {
public:
    explicit AndToken(int line) : Token(line) {
        this->type = AND;
    }

    AndToken(const AndToken &token) = delete;

    void print() override {
        std::cout << "AND &&" << std::endl;
    }
};

class OrToken : public Token {
public:
    explicit OrToken(int line) : Token(line) {
        this->type = OR;
    }

    OrToken(const OrToken &token) = delete;

    void print() override {
        std::cout << "OR ||" << std::endl;
    }
};

class AddToken : public Token {
public:
    explicit AddToken(int line) : Token(line) {
        this->type = PLUS;
    }

    AddToken(const AddToken &token) = delete;

    void print() override {
        std::cout << "PLUS +" << std::endl;
    }
};

class SubToken : public Token {
public:
    explicit SubToken(int line) : Token(line) {
        this->type = MINU;
    }

    SubToken(const SubToken &token) = delete;

    void print() override {
        std::cout << "MINU -" << std::endl;
    }
};

class MulToken : public Token {
public:
    explicit MulToken(int line) : Token(line) {
        this->type = MULT;
    }

    MulToken(const MulToken &token) = delete;

    void print() override {
        std::cout << "MULT *" << std::endl;
    }
};

class DivToken : public Token {
public:
    explicit DivToken(int line) : Token(line) {
        this->type = DIV;
    }

    DivToken(const DivToken &token) = delete;

    void print() override {
        std::cout << "DIV /" << std::endl;
    }
};

class ModToken : public Token {
public:
    explicit ModToken(int line) : Token(line) {
        this->type = MOD;
    }

    ModToken(const ModToken &token) = delete;

    void print() override {
        std::cout << "MOD %" << std::endl;
    }
};

class LtToken : public Token {
public:
    explicit LtToken(int line) : Token(line) {
        this->type = LSS;
    }

    LtToken(const LtToken &token) = delete;

    void print() override {
        std::cout << "LSS <" << std::endl;
    }
};

class LeToken : public Token {
public:
    explicit LeToken(int line) : Token(line) {
        this->type = LEQ;
    }

    LeToken(const LeToken &token) = delete;

    void print() override {
        std::cout << "LEQ <=" << std::endl;
    }
};

class GtToken : public Token {
public:
    explicit GtToken(int line) : Token(line) {
        this->type = GRE;
    }

    GtToken(const GtToken &token) = delete;

    void print() override {
        std::cout << "GRE >" << std::endl;
    }
};

class GeToken : public Token {
public:
    explicit GeToken(int line) : Token(line) {
        this->type = GEQ;
    }

    GeToken(const GeToken &token) = delete;

    void print() override {
        std::cout << "GEQ >=" << std::endl;
    }
};

class EqToken : public Token {
public:
    explicit EqToken(int line) : Token(line) {
        this->type = EQL;
    }

    EqToken(const EqToken &token) = delete;

    void print() override {
        std::cout << "EQL ==" << std::endl;
    }
};

class NeToken : public Token {
public:
    explicit NeToken(int line) : Token(line) {
        this->type = NEQ;
    }

    NeToken(const NeToken &token) = delete;

    void print() override {
        std::cout << "NEQ !=" << std::endl;
    }
};

class AssignToken : public Token {
public:
    explicit AssignToken(int line) : Token(line) {
        this->type = ASSIGN;
    }

    AssignToken(const AssignToken &token) = delete;

    void print() override {
        std::cout << "ASSIGN =" << std::endl;
    }
};

class Semicolon : public Token {
public:
    explicit Semicolon(int line) : Token(line) {
        this->type = SEMICN;
    }

    Semicolon(const Semicolon &token) = delete;

    void print() override {
        std::cout << "SEMICN ;" << std::endl;
    }
};

class Comma : public Token {
public:
    explicit Comma(int line) : Token(line) {
        this->type = COMMA;
    }

    Comma(const Comma &token) = delete;

    void print() override {
        std::cout << "COMMA ," << std::endl;
    }
};

class LParen : public Token {
public:
    explicit LParen(int line) : Token(line) {
        this->type = LPARENT;
    }

    LParen(const LParen &token) = delete;

    void print() override {
        std::cout << "LPARENT (" << std::endl;
    }
};

class RParen : public Token {
public:
    explicit RParen(int line) : Token(line) {
        this->type = RPARENT;
    }

    RParen(const RParen &token) = delete;

    void print() override {
        std::cout << "RPARENT )" << std::endl;
    }
};

class LBracket : public Token {
public:
    explicit LBracket(int line) : Token(line) {
        this->type = LBRACK;
    }

    LBracket(const LBracket &token) = delete;

    void print() override {
        std::cout << "LBRACK [" << std::endl;
    }
};

class RBracket : public Token {
public:
    explicit RBracket(int line) : Token(line) {
        this->type = RBRACK;
    }

    RBracket(const RBracket &token) = delete;

    void print() override {
        std::cout << "RBRACK ]" << std::endl;
    }
};

class LBrace : public Token {
public:
    explicit LBrace(int line) : Token(line) {
        this->type = LBRACE;
    }

    LBrace(const LBrace &token) = delete;

    void print() override {
        std::cout << "LBRACE {" << std::endl;
    }
};

class RBrace : public Token {
public:
    explicit RBrace(int line) : Token(line) {
        this->type = RBRACE;
    }

    RBrace(const RBrace &token) = delete;

    void print() override {
        std::cout << "RBRACE }" << std::endl;
    }
};

#endif //CODE_TOKEN_H
