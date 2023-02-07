//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_OBJECT_H
#define CODE_OBJECT_H

#include "element.h"
#include "token_code.h"
#include "type_code.h"

#define CACHE_LINE_SIZE 64

class Identifier;

using IdentP = shared_ptr<Identifier>;

class Object;

using ObjectP = shared_ptr<Object>;

class Object {
public:
    TypeCode type;
    bool is_const = false;
    bool is_global = false;
    IdentP ident_info;

    explicit Object(TypeCode val_type = TypeCode::VOID) : type(val_type) {};

    virtual inline ObjectP copy() const {
        return make_shared<Object>(*this);
    }

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
    int value = 0;

    explicit IntObject(int value = 0) : Object(TypeCode::INT), value(value) {}

    inline ObjectP copy() const override {
        return make_shared<IntObject>(*this);
    }
};

using IntObjectP = shared_ptr<IntObject>;
using Array = vector<ObjectP>;
using ArrayP = shared_ptr<Array>;

class ArrayObject;

using ArrayObjectP = shared_ptr<ArrayObject>;

class ArrayObject : public Object {
public:
    ArrayP data;  // have to use pointer for convenience when passing arguments to functions (cannot use `swap` method)
    vector<long long> dims;
    int dereference_cnt = 0;
    long long base = 0;

    explicit ArrayObject(bool alloc = false) : Object(TypeCode::INT_ARRAY) {
        if (alloc) {
            this->alloc();
        }
    }

    explicit ArrayObject(long long size) : Object(TypeCode::INT_ARRAY) {
        alloc(size);
    }

    inline ObjectP operator[](long long index) const {
        if (dereference_cnt + 1 >= dims.size()) {
            return (*data)[base + index];
        } else {
            ArrayObjectP array = make_shared<ArrayObject>(*this);
            array->dereference_cnt++;
            long long begin = index;
            for (auto it = dims.begin() + array->dereference_cnt; it != dims.end(); ++it) {
                begin *= *it;
            }
            array->base += begin;
            return array;
        }
    }

    inline ObjectP copy() const override {
        return make_shared<ArrayObject>(*this);
    }

    void alloc(long long size = CACHE_LINE_SIZE / sizeof(ObjectP)) {
        data = make_shared<Array>();
        data->reserve(size);
    }
};

class StringObject : public Object {
public:
    string value;

    explicit StringObject(const char *start, long long length) : Object(TypeCode::CHAR_ARRAY) {
        value.assign(start, length);
    }

    inline ObjectP copy() const override { return make_shared<StringObject>(*this); }
};

class FuncObject : public Object {
public:
    TypeCode return_type;
    vector<ObjectP> params;
    long long code_offset = 0;

    explicit FuncObject(TypeCode return_type) : Object(TypeCode::FUNCTION), return_type(return_type) {}

    inline ObjectP copy() const override { return make_shared<FuncObject>(*this); }
};

using FuncObjectP = shared_ptr<FuncObject>;

#endif //CODE_OBJECT_H
