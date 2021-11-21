//
// Created by Kevin Tan on 2021/10/14.
//

#ifndef CODE_ELEMENT_H
#define CODE_ELEMENT_H

#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cassert>

#define cast dynamic_pointer_cast

using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::string;
using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;
using std::vector;
using std::unordered_map;
using std::function;
using std::move;

class Element {
public:
    string fullname;

    Element() = default;

    explicit Element(string &&name) : fullname(move(name)) {}

    friend ostream &operator<<(ostream &out, const Element &self) {
        out << self.fullname;
        return out;
    }

    virtual ~Element() = default;
};

using ElementP = shared_ptr<Element>;

class CompUnit : public Element {
public:
    explicit CompUnit() : Element("<CompUnit>") {}
};

class ConstDecl : public Element {
public:
    explicit ConstDecl() : Element("<ConstDecl>") {}
};

class ConstDef : public Element {
public:
    explicit ConstDef() : Element("<ConstDef>") {}
};

class ConstInitVal : public Element {
public:
    explicit ConstInitVal() : Element("<ConstInitVal>") {}
};

class VarDecl : public Element {
public:
    explicit VarDecl() : Element("<VarDecl>") {}
};

class VarDef : public Element {
public:
    explicit VarDef() : Element("<VarDef>") {}
};

class InitVal : public Element {
public:
    explicit InitVal() : Element("<InitVal>") {}
};

class FuncDef : public Element {
public:
    explicit FuncDef() : Element("<FuncDef>") {}
};

class MainFuncDef : public Element {
public:
    explicit MainFuncDef() : Element("<MainFuncDef>") {}
};

class FuncType : public Element {
public:
    explicit FuncType() : Element("<FuncType>") {}
};

class FuncFormalParams : public Element {
public:
    explicit FuncFormalParams() : Element("<FuncFParams>") {}
};

class FuncFormalParam : public Element {
public:
    explicit FuncFormalParam() : Element("<FuncFParam>") {}
};

class Block : public Element {
public:
    explicit Block() : Element("<Block>") {}
};

class BlockItem : public Element {
public:
    explicit BlockItem() : Element("<BlockItem>") {}
};

class Statement : public Element {
public:
    explicit Statement() : Element("<Stmt>") {}
};

class NormalExpr : public Element {
public:
    explicit NormalExpr() : Element("<Exp>") {}
};

class ConditionExpr : public Element {
public:
    explicit ConditionExpr() : Element("<Cond>") {}
};

class LValue : public Element {
public:
    explicit LValue() : Element("<LVal>") {}
};

class PrimaryExpr : public Element {
public:
    explicit PrimaryExpr() : Element("<PrimaryExp>") {}
};

class Number : public Element {
public:
    explicit Number() : Element("<Number>") {}
};

class UnaryExpr : public Element {
public:
    explicit UnaryExpr() : Element("<UnaryExp>") {}
};

class UnaryOp : public Element {
public:
    explicit UnaryOp() : Element("<UnaryOp>") {}
};

class FuncRealParams : public Element {
public:
    explicit FuncRealParams() : Element("<FuncRParams>") {}
};

class MulDivExpr : public Element {
public:
    explicit MulDivExpr() : Element("<MulExp>") {}
};

class AddSubExpr : public Element {
public:
    explicit AddSubExpr() : Element("<AddExp>") {}
};

class RelationalExpr : public Element {
public:
    explicit RelationalExpr() : Element("<RelExp>") {}
};

class EqualExpr : public Element {
public:
    explicit EqualExpr() : Element("<EqExp>") {}
};

class LogicalAndExpr : public Element {
public:
    explicit LogicalAndExpr() : Element("<LAndExp>") {}
};

class LogicalOrExpr : public Element {
public:
    explicit LogicalOrExpr() : Element("<LOrExp>") {}
};

class ConstExpr : public Element {
public:
    explicit ConstExpr() : Element("<ConstExp>") {}
};

#endif //CODE_ELEMENT_H
