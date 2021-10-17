//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include <vector>
#include <unordered_map>
#include <functional>
#include "token.h"

using TokenIter = std::vector<TokenP>::iterator;

class Parser {
public:
    std::vector<TokenP> &tokens;
    std::vector<ElementP> elements;
    std::unordered_map<std::string, IdentP> sym_table;

    explicit Parser(std::vector<TokenP> &tokens);

    void print() {
        for (auto &e : elements) {
            e->print();
        }
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

    template <typename T>
    void _parse_expr(TokenIter &tk,
                     void (Parser::*parse_first)(TokenIter &),
                     const std::function<bool(TokenCode)> &predicate);

    void parse_comp_unit(TokenIter &tk);

    void parse_decl(TokenIter &tk);

    void parse_const_decl(TokenIter &tk);

    void parse_const_def(TokenIter &tk);

    void parse_var_decl(TokenIter &tk);

    void parse_var_def(TokenIter &tk);

    template <typename ExprT, typename ElementT>
    void parse_init_val(TokenIter &tk);

    void parse_func_def(TokenIter &tk);

    void parse_main_func_def(TokenIter &tk);

    void parse_func_type(TokenIter &tk);

    void parse_func_formal_params(TokenIter &tk);

    void parse_func_formal_param(TokenIter &tk);

    void parse_block(TokenIter &tk);

    void parse_block_item(TokenIter &tk);

    void parse_stmt(TokenIter &tk);

    template <typename T>
    void parse_expr(TokenIter &tk);

    void parse_cond_expr(TokenIter &tk);

    void parse_lvalue(TokenIter &tk);

    void parse_primary_expr(TokenIter &tk);

    void parse_number(TokenIter &tk);

    void parse_unary_op(TokenIter &tk);

    void parse_unary_expr(TokenIter &tk);

    void parse_func_real_params(TokenIter &tk);

    void parse_muldiv_expr(TokenIter &tk);

    void parse_addsub_expr(TokenIter &tk);

    void parse_rel_expr(TokenIter &tk);

    void parse_eq_expr(TokenIter &tk);

    void parse_logical_and_expr(TokenIter &tk);

    void parse_logical_or_expr(TokenIter &tk);
};


#endif //CODE_PARSER_H
