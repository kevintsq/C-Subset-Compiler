//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include "element.h"
#include "token_code.h"
#include "type_code.h"

class Identifier;

using IdentP = shared_ptr<Identifier>;

class Object;

using ObjectP = shared_ptr<Object>;

class Object {
public:
    TypeCode type;
    bool is_const = false;
    IdentP ident_info;

    explicit Object(TypeCode val_type = VOID) : type(val_type) {};

    virtual ~Object() = default;

    Object &operator=(const Object &other) {
        if (this != &other) {
            this->type = other.type;
            this->is_const = other.is_const;
        }
        return *this;
    }
};

class IntObject : public Object {
public:
    long long value = 0;

    explicit IntObject(long long value = 0) : Object(INT), value(value) {}
};

using IntObjectP = shared_ptr<IntObject>;
using Array = vector<ObjectP>;
using ArrayP = shared_ptr<Array>;

class ArrayObject : public Object {
public:
    ArrayP data;
    vector<long long> dims;
    int dereference_cnt = 0;

    ArrayObject() : Object(INT_ARRAY) {}
};

using ArrayObjectP = shared_ptr<ArrayObject>;

class StringObject : public Object {
public:
    string value;

    explicit StringObject(const char *start, long long length) : Object(CHAR_ARRAY) {
        value.assign(start, length);
    }
};

class FuncObject : public Object {
public:
    TypeCode return_type;
    vector<ObjectP> params;
    long long code_offset = 0;

    explicit FuncObject(TypeCode return_type) : Object(FUNCTION), return_type(return_type) {}
};

using FuncObjectP = shared_ptr<FuncObject>;

#endif //CODE_OBJECT_H
