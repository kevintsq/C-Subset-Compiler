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
    TypeCode type = VOID;
    bool is_const = false;
    IdentP ident_info;

    Object() = default;

    explicit Object(TypeCode val_type) : type(val_type) {};

    virtual ~Object() = default;
};

class IntObject : public Object {
public:
    int value = 0;

    explicit IntObject(int value = 0) : Object(INT), value(value) {}
};

using IntObjectP = shared_ptr<IntObject>;

class ArrayObject : public Object {
public:
    vector<ObjectP> value;
    vector<ObjectP> dims;
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
    unsigned long long code_offset = 0;

    explicit FuncObject(TypeCode return_type) : Object(FUNCTION), return_type(return_type) {}
};

using FuncObjectP = shared_ptr<FuncObject>;

#endif //CODE_OBJECT_H
