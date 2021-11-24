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

class Token : public Element {
public:
    const int line;
    TokenCode type;

    Token(int line, TokenCode type) : line(line), type(type) {}

    Token(int line, TokenCode type, string &&name) : Element(move(name)), line(line), type(type) {}
};

using TokenP = shared_ptr<Token>;

class Identifier : public Token {
public:
    string name;

    Identifier(int line, const string &name) : Token(line, IDENFR), name(name) {
        this->fullname = "IDENFR " + name;
    }

    Identifier(int line, const char *start, long long length) : Token(line, IDENFR) {
        this->name.assign(start, length);
        this->fullname = "IDENFR " + name;
    }
};

class IntLiteral : public Token, public IntObject {  // it's an r-data
public:
    IntLiteral(int line, int value) : Token(line, INTCON), IntObject(value) {
        this->fullname = "INTCON " + std::to_string(value);
        this->is_const = true;
    }
};

using IntLiteralP = shared_ptr<IntLiteral>;

class FormatString : public Token, public StringObject {  // it's an r-data
public:
    int fmt_char_cnt = 0;
    vector<string> segments;

    FormatString(int line, const char *start, long long length, int fmt_char_cnt, vector<string> segments) :
            Token(line, STRCON), StringObject(start, length), fmt_char_cnt(fmt_char_cnt), segments(move(segments)) {
        this->fullname = "STRCON " + this->value;
    }
};

using FormatStringP = shared_ptr<FormatString>;

class MainToken : public Token, public FuncObject {
public:
    explicit MainToken(int line) : Token(line, NAME(MAINTK)" main"), FuncObject(INT) {
        this->ident_info = make_shared<Identifier>(line, "main");
    }
};

class ConstToken : public Token {
public:
    explicit ConstToken(int line) : Token(line, NAME(CONSTTK)" const") {}
};

class IntToken : public Token {
public:
    explicit IntToken(int line) : Token(line, NAME(INTTK)" int") {}
};

class VoidToken : public Token {
public:
    explicit VoidToken(int line) : Token(line, NAME(VOIDTK)" void") {}
};

class BreakToken : public Token {
public:
    explicit BreakToken(int line) : Token(line, NAME(BREAKTK)" break") {}
};

class ContinueToken : public Token {
public:
    explicit ContinueToken(int line) : Token(line, NAME(CONTINUETK)" continue") {}
};

class IfToken : public Token {
public:
    explicit IfToken(int line) : Token(line, NAME(IFTK)" if") {}
};

class ElseToken : public Token {
public:
    explicit ElseToken(int line) : Token(line, NAME(ELSETK)" else") {}
};

class WhileToken : public Token {
public:
    explicit WhileToken(int line) : Token(line, NAME(WHILETK)" while") {}
};

class GetintToken : public Token {
public:
    explicit GetintToken(int line) : Token(line, NAME(GETINTTK)" getint") {}
};

class PrintfToken : public Token {
public:
    explicit PrintfToken(int line) : Token(line, NAME(PRINTFTK)" printf") {}
};

class ReturnToken : public Token {
public:
    explicit ReturnToken(int line) : Token(line, NAME(RETURNTK)" return") {}
};

class NotToken : public Token {
public:
    explicit NotToken(int line) : Token(line, NAME(NOT)" !") {}
};

class AndToken : public Token {
public:
    explicit AndToken(int line) : Token(line, NAME(AND)" &&") {}
};

class OrToken : public Token {
public:
    explicit OrToken(int line) : Token(line, NAME(OR)" ||") {}
};

class AddToken : public Token {
public:
    explicit AddToken(int line) : Token(line, NAME(PLUS)" +") {}
};

class SubToken : public Token {
public:
    explicit SubToken(int line) : Token(line, NAME(MINU)" -") {}
};

class MulToken : public Token {
public:
    explicit MulToken(int line) : Token(line, NAME(MULT)" *") {}
};

class DivToken : public Token {
public:
    explicit DivToken(int line) : Token(line, NAME(DIV)" /") {}
};

class ModToken : public Token {
public:
    explicit ModToken(int line) : Token(line, NAME(MOD)" %") {}
};

class LtToken : public Token {
public:
    explicit LtToken(int line) : Token(line, NAME(LSS)" <") {}
};

class LeToken : public Token {
public:
    explicit LeToken(int line) : Token(line, NAME(LEQ)" <=") {}
};

class GtToken : public Token {
public:
    explicit GtToken(int line) : Token(line, NAME(GRE)" >") {}
};

class GeToken : public Token {
public:
    explicit GeToken(int line) : Token(line, NAME(GEQ)" >=") {}
};

class EqToken : public Token {
public:
    explicit EqToken(int line) : Token(line, NAME(EQL)" ==") {}
};

class NeToken : public Token {
public:
    explicit NeToken(int line) : Token(line, NAME(NEQ)" !=") {}
};

class AssignToken : public Token {
public:
    explicit AssignToken(int line) : Token(line, NAME(ASSIGN)" =") {}
};

class Semicolon : public Token {
public:
    explicit Semicolon(int line) : Token(line, NAME(SEMICN)" ;") {}
};

class Comma : public Token {
public:
    explicit Comma(int line) : Token(line, NAME(COMMA)" ,") {}
};

class LParen : public Token {
public:
    explicit LParen(int line) : Token(line, NAME(LPARENT)" (") {}
};

class RParen : public Token {
public:
    explicit RParen(int line) : Token(line, NAME(RPARENT)" )") {}
};

class LBracket : public Token {
public:
    explicit LBracket(int line) : Token(line, NAME(LBRACK)" [") {}
};

class RBracket : public Token {
public:
    explicit RBracket(int line) : Token(line, NAME(RBRACK)" ]") {}
};

class LBrace : public Token {
public:
    explicit LBrace(int line) : Token(line, NAME(LBRACE)" {") {}
};

class RBrace : public Token {
public:
    explicit RBrace(int line) : Token(line, NAME(RBRACE)" }") {}
};

#endif //CODE_TOKEN_H
