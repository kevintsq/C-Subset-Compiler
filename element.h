//
// Created by Kevin Tan on 2021/10/14.
//

#ifndef CODE_ELEMENT_H
#define CODE_ELEMENT_H

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

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

class Element {
public:
    string name;

    friend std::ostream &operator<<(std::ostream &out, const Element &self) {
        out << self.name << endl;
        return out;
    }

    virtual ~Element() = default;
};

using ElementP = shared_ptr<Element>;

class CompUnit : public Element {
public:
    explicit CompUnit() {
        this->name = "<CompUnit>";
    }
};

class ConstDecl : public Element {
public:
    explicit ConstDecl() {
        this->name = "<ConstDecl>";
    }
};

class ConstDef : public Element {
public:
    explicit ConstDef() {
        this->name = "<ConstDef>";
    }
};

class ConstInitVal : public Element {
public:
    explicit ConstInitVal() {
        this->name = "<ConstInitVal>";
    }
};

class VarDecl : public Element {
public:
    explicit VarDecl() {
        this->name = "<VarDecl>";
    }
};

class VarDef : public Element {
public:
    explicit VarDef() {
        this->name = "<VarDef>";
    }
};

class InitVal : public Element {
public:
    explicit InitVal() {
        this->name = "<InitVal>";
    }
};

class FuncDef : public Element {
public:
    explicit FuncDef() {
        this->name = "<FuncDef>";
    }
};

class MainFuncDef : public Element {
public:
    explicit MainFuncDef() {
        this->name = "<MainFuncDef>";
    }
};

class FuncType : public Element {
public:
    explicit FuncType() {
        this->name = "<FuncType>";
    }
};

class FuncFormalParams : public Element {
public:
    explicit FuncFormalParams() {
        this->name = "<FuncFParams>";
    }
};

class FuncFormalParam : public Element {
public:
    explicit FuncFormalParam() {
        this->name = "<FuncFParam>";
    }
};

class Block : public Element {
public:
    explicit Block() {
        this->name = "<Block>";
    }
};

class BlockItem : public Element {
public:
    explicit BlockItem() {
        this->name = "<BlockItem>";
    }
};

class Statement : public Element {
public:
    explicit Statement() {
        this->name = "<Stmt>";
    }
};

class NormalExpr : public Element {
public:
    explicit NormalExpr() {
        this->name = "<Exp>";
    }
};

class ConditionExpr : public Element {
public:
    explicit ConditionExpr() {
        this->name = "<Cond>";
    }
};

class LValue : public Element {
public:
    explicit LValue() {
        this->name = "<LVal>";
    }
};

class PrimaryExpr : public Element {
public:
    explicit PrimaryExpr() {
        this->name = "<PrimaryExp>";
    }
};

class Number : public Element {
public:
    explicit Number() {
        this->name = "<Number>";
    }
};

class UnaryExpr : public Element {
public:
    explicit UnaryExpr() {
        this->name = "<UnaryExp>";
    }
};

class UnaryOp : public Element {
public:
    explicit UnaryOp() {
        this->name = "<UnaryOp>";
    }
};

class FuncRealParams : public Element {
public:
    explicit FuncRealParams() {
        this->name = "<FuncRParams>";
    }
};

class MulDivExpr : public Element {
public:
    explicit MulDivExpr() {
        this->name = "<MulExp>";
    }
};

class AddSubExpr : public Element {
public:
    explicit AddSubExpr() {
        this->name = "<AddExp>";
    }
};

class RelationalExpr : public Element {
public:
    explicit RelationalExpr() {
        this->name = "<RelExp>";
    }
};

class EqualExpr : public Element {
public:
    explicit EqualExpr() {
        this->name = "<EqExp>";
    }
};

class LogicalAndExpr : public Element {
public:
    explicit LogicalAndExpr() {
        this->name = "<LAndExp>";
    }
};

class LogicalOrExpr : public Element {
public:
    explicit LogicalOrExpr() {
        this->name = "<LOrExp>";
    }
};

class ConstExpr : public Element {
public:
    explicit ConstExpr() {
        this->name = "<ConstExp>";
    }
};

#endif //CODE_ELEMENT_H
