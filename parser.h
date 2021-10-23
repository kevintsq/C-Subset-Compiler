//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "token.h"

using TokenIter = vector<TokenP>::iterator;
using HashMap = unordered_map<string, IdentP>;
using HashMapP = shared_ptr<HashMap>;

class Parser {
public:
    vector<TokenP> &tokens;
    Error &error;
    vector<ElementP> elements;
    vector<HashMapP> sym_table;
    int while_cnt = 0;
    TypeCode current_func_return_type = INT;
    bool is_return_at_end = false;

    explicit Parser(vector<TokenP> &tokens, Error &error);

    friend ostream &operator<<(ostream &out, const Parser &self) {
        for (const auto &element: self.elements) {
            out << *element;
        }
        return out;
    }

    static inline bool starts_with_decl(const TokenIter &tk) {
        auto next = (*tk)->type;
        return next == CONSTTK || (next == INTTK && (*(tk + 2))->type != LPARENT);
    }

    static inline bool starts_with_func_def(const TokenIter &tk) {
        auto next = (*tk)->type;
        return next == VOIDTK || (next == INTTK && (*(tk + 1))->type != MAINTK && (*(tk + 2))->type == LPARENT);
    }

    static inline bool starts_with_expr(const TokenIter &tk) {
        switch ((*tk)->type) {
            case LPARENT:
            case IDENFR:
            case INTCON:
            case PLUS:
            case MINU:
            case NOT:
                return true;
            default:
                return false;
        }
    }

    static inline bool starts_with_stmt(const TokenIter &tk) {
        switch ((*tk)->type) {
            case LBRACE:
            case SEMICN:
            case IFTK:
            case WHILETK:
            case BREAKTK:
            case CONTINUETK:
            case RETURNTK:
            case PRINTFTK:
                return true;
            default:
                return starts_with_expr(tk);
        }
    }

    template<typename T>
    ElementP _parse_expr(TokenIter &tk, InstanceP *out_instance,
                         ElementP (Parser::*parse_first)(TokenIter &, InstanceP *),
                         const function<bool(TokenCode)> &predicate);

    void parse_comp_unit(TokenIter &tk);

    IdentP check_ident_valid_use(TokenIter &tk, bool is_assigned);

    IdentP check_ident_valid_decl(TokenIter &tk, TypeCode type, int nest_level, bool is_const, bool is_func);

    void parse_decl(TokenIter &tk, int nest_level);

    void parse_const_decl(TokenIter &tk, int nest_level);

    void parse_const_def(TokenIter &tk, int nest_level);

    void parse_var_decl(TokenIter &tk, int nest_level);

    void parse_var_def(TokenIter &tk, int nest_level);

    template<typename ExprT, typename ElementT>
    void parse_init_val(TokenIter &tk);

    void parse_func_def(TokenIter &tk);

    void parse_main_func_def(TokenIter &tk);

    TypeCode parse_func_type(TokenIter &tk);

    void parse_func_formal_params(TokenIter &tk, IdentP &func);

    void parse_func_formal_param(TokenIter &tk, IdentP &func);

    void parse_block(TokenIter &tk, int nest_level, bool from_func_def = false);

    void parse_block_item(TokenIter &tk, int nest_level);

    void parse_stmt(TokenIter &tk, int nest_level);

    template<typename T>
    ElementP parse_expr(TokenIter &tk, InstanceP *out_instance = nullptr);

    ElementP parse_cond_expr(TokenIter &tk, InstanceP *out_instance = nullptr);

    IdentP parse_lvalue(TokenIter &tk, bool is_assigned);

    ElementP parse_primary_expr(TokenIter &tk, InstanceP *out_instance);

    InstanceP parse_number(TokenIter &tk);

    TokenCode parse_unary_op(TokenIter &tk);

    ElementP parse_unary_expr(TokenIter &tk, InstanceP *out_instance);

    void parse_func_real_params(TokenIter &tk, IdentP &func, int func_line, bool report_error);

    ElementP parse_muldiv_expr(TokenIter &tk, InstanceP *out_instance);

    ElementP parse_addsub_expr(TokenIter &tk, InstanceP *out_instance);

    ElementP parse_rel_expr(TokenIter &tk, InstanceP *out_instance);

    ElementP parse_eq_expr(TokenIter &tk, InstanceP *out_instance);

    ElementP parse_logical_and_expr(TokenIter &tk, InstanceP *out_instance);

    ElementP parse_logical_or_expr(TokenIter &tk, InstanceP *out_instance);
};


#endif //CODE_PARSER_H
