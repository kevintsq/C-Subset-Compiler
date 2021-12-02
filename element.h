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
using std::move;

struct Element {
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

struct CompUnit : public Element {
    explicit CompUnit() : Element("<CompUnit>") {}
};

struct ConstDecl : public Element {
    explicit ConstDecl() : Element("<ConstDecl>") {}
};

struct ConstDef : public Element {
    explicit ConstDef() : Element("<ConstDef>") {}
};

struct ConstInitVal : public Element {
    explicit ConstInitVal() : Element("<ConstInitVal>") {}
};

struct VarDecl : public Element {
    explicit VarDecl() : Element("<VarDecl>") {}
};

struct VarDef : public Element {
    explicit VarDef() : Element("<VarDef>") {}
};

struct InitVal : public Element {
    explicit InitVal() : Element("<InitVal>") {}
};

struct FuncDef : public Element {
    explicit FuncDef() : Element("<FuncDef>") {}
};

struct MainFuncDef : public Element {
    explicit MainFuncDef() : Element("<MainFuncDef>") {}
};

struct FuncType : public Element {
    explicit FuncType() : Element("<FuncType>") {}
};

struct FuncFormalParams : public Element {
    explicit FuncFormalParams() : Element("<FuncFParams>") {}
};

struct FuncFormalParam : public Element {
    explicit FuncFormalParam() : Element("<FuncFParam>") {}
};

struct Block : public Element {
    explicit Block() : Element("<Block>") {}
};

struct Statement : public Element {
    explicit Statement() : Element("<Stmt>") {}
};

struct NormalExpr : public Element {
    explicit NormalExpr() : Element("<Exp>") {}
};

struct ConditionExpr : public Element {
    explicit ConditionExpr() : Element("<Cond>") {}
};

struct LValue : public Element {
    explicit LValue() : Element("<LVal>") {}
};

struct PrimaryExpr : public Element {
    explicit PrimaryExpr() : Element("<PrimaryExp>") {}
};

struct Number : public Element {
    explicit Number() : Element("<Number>") {}
};

struct UnaryExpr : public Element {
    explicit UnaryExpr() : Element("<UnaryExp>") {}
};

struct UnaryOp : public Element {
    explicit UnaryOp() : Element("<UnaryOp>") {}
};

struct FuncRealParams : public Element {
    explicit FuncRealParams() : Element("<FuncRParams>") {}
};

struct MulDivExpr : public Element {
    explicit MulDivExpr() : Element("<MulExp>") {}
};

struct AddSubExpr : public Element {
    explicit AddSubExpr() : Element("<AddExp>") {}
};

struct RelationalExpr : public Element {
    explicit RelationalExpr() : Element("<RelExp>") {}
};

struct EqualExpr : public Element {
    explicit EqualExpr() : Element("<EqExp>") {}
};

struct LogicalAndExpr : public Element {
    explicit LogicalAndExpr() : Element("<LAndExp>") {}
};

struct LogicalOrExpr : public Element {
    explicit LogicalOrExpr() : Element("<LOrExp>") {}
};

struct ConstExpr : public Element {
    explicit ConstExpr() : Element("<ConstExp>") {}
};

#endif //CODE_ELEMENT_H
