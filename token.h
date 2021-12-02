//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_TOKEN_H
#define CODE_TOKEN_H

#include <cstring>
#include <string>
#include <regex>
#include <utility>
#include "object.h"
#include "util.h"
#include "error.h"

#define NAME(name) name, #name

struct Token : public Element {
    const int line;
    TokenCode token_type;

    Token(int line, TokenCode type) : line(line), token_type(type) {}

    Token(int line, TokenCode type, string &&name) : Element(move(name)), line(line), token_type(type) {}
};

using TokenP = shared_ptr<Token>;

struct Identifier : public Token {
    string name;

    Identifier(int line, const string &name) : Token(line, IDENFR), name(name) {
        this->fullname = "IDENFR " + name;
    }

    Identifier(int line, const char *start, long long length) : Token(line, IDENFR) {
        this->name.assign(start, length);
        this->fullname = "IDENFR " + name;
    }
};

struct IntLiteral : public Token, public IntObject {  // it's an r-data
    IntLiteral(int line, int value) : Token(line, INTCON), IntObject(value) {
        this->fullname = "INTCON " + std::to_string(value);
        this->is_const = true;
    }
};

struct FormatString : public Token, public StringObject {  // it's an r-data
    int fmt_char_cnt = 0;
    vector<string> segments;

    FormatString(int line, const char *start, long long length, int fmt_char_cnt, vector<string> segments) :
            Token(line, STRCON), StringObject(start, length), fmt_char_cnt(fmt_char_cnt), segments(move(segments)) {
        this->fullname = "STRCON " + this->value;
    }
};

using FormatStringP = shared_ptr<FormatString>;

struct MainToken : public Token, public FuncObject {
    explicit MainToken(int line) : Token(line, NAME(MAINTK)" main"), FuncObject(INT) {
        this->ident_info = make_shared<Identifier>(line, "main");
    }
};

struct ConstToken : public Token {
    explicit ConstToken(int line) : Token(line, NAME(CONSTTK)" const") {}
};

struct IntToken : public Token {
    explicit IntToken(int line) : Token(line, NAME(INTTK)" int") {}
};

struct VoidToken : public Token {
    explicit VoidToken(int line) : Token(line, NAME(VOIDTK)" void") {}
};

struct BreakToken : public Token {
    explicit BreakToken(int line) : Token(line, NAME(BREAKTK)" break") {}
};

struct ContinueToken : public Token {
    explicit ContinueToken(int line) : Token(line, NAME(CONTINUETK)" continue") {}
};

struct IfToken : public Token {
    explicit IfToken(int line) : Token(line, NAME(IFTK)" if") {}
};

struct ElseToken : public Token {
    explicit ElseToken(int line) : Token(line, NAME(ELSETK)" else") {}
};

struct WhileToken : public Token {
    explicit WhileToken(int line) : Token(line, NAME(WHILETK)" while") {}
};

struct GetintToken : public Token {
    explicit GetintToken(int line) : Token(line, NAME(GETINTTK)" getint") {}
};

struct PrintfToken : public Token {
    explicit PrintfToken(int line) : Token(line, NAME(PRINTFTK)" printf") {}
};

struct ReturnToken : public Token {
    explicit ReturnToken(int line) : Token(line, NAME(RETURNTK)" return") {}
};

struct NotToken : public Token {
    explicit NotToken(int line) : Token(line, NAME(NOT)" !") {}
};

struct AndToken : public Token {
    explicit AndToken(int line) : Token(line, NAME(AND)" &&") {}
};

struct OrToken : public Token {
    explicit OrToken(int line) : Token(line, NAME(OR)" ||") {}
};

struct AddToken : public Token {
    explicit AddToken(int line) : Token(line, NAME(PLUS)" +") {}
};

struct SubToken : public Token {
    explicit SubToken(int line) : Token(line, NAME(MINU)" -") {}
};

struct MulToken : public Token {
    explicit MulToken(int line) : Token(line, NAME(MULT)" *") {}
};

struct DivToken : public Token {
    explicit DivToken(int line) : Token(line, NAME(DIV)" /") {}
};

struct ModToken : public Token {
    explicit ModToken(int line) : Token(line, NAME(MOD)" %") {}
};

struct LtToken : public Token {
    explicit LtToken(int line) : Token(line, NAME(LSS)" <") {}
};

struct LeToken : public Token {
    explicit LeToken(int line) : Token(line, NAME(LEQ)" <=") {}
};

struct GtToken : public Token {
    explicit GtToken(int line) : Token(line, NAME(GRE)" >") {}
};

struct GeToken : public Token {
    explicit GeToken(int line) : Token(line, NAME(GEQ)" >=") {}
};

struct EqToken : public Token {
    explicit EqToken(int line) : Token(line, NAME(EQL)" ==") {}
};

struct NeToken : public Token {
    explicit NeToken(int line) : Token(line, NAME(NEQ)" !=") {}
};

struct AssignToken : public Token {
    explicit AssignToken(int line) : Token(line, NAME(ASSIGN)" =") {}
};

struct Semicolon : public Token {
    explicit Semicolon(int line) : Token(line, NAME(SEMICN)" ;") {}
};

struct Comma : public Token {
    explicit Comma(int line) : Token(line, NAME(COMMA)" ,") {}
};

struct LParen : public Token {
    explicit LParen(int line) : Token(line, NAME(LPARENT)" (") {}
};

struct RParen : public Token {
    explicit RParen(int line) : Token(line, NAME(RPARENT)" )") {}
};

struct LBracket : public Token {
    explicit LBracket(int line) : Token(line, NAME(LBRACK)" [") {}
};

struct RBracket : public Token {
    explicit RBracket(int line) : Token(line, NAME(RBRACK)" ]") {}
};

struct LBrace : public Token {
    explicit LBrace(int line) : Token(line, NAME(LBRACE)" {") {}
};

struct RBrace : public Token {
    explicit RBrace(int line) : Token(line, NAME(RBRACE)" }") {}
};

#endif //CODE_TOKEN_H
