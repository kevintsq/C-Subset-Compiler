//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include <vector>
#include "token.h"


class Parser {
public:
    explicit Parser(std::vector<TokenP> &tokens);

    void print();

    void parse_comp_unit();

    void parse_decl();

    void parse_const_decl();

    void parse_basic_type();

    void parse_const_def();

    void parse_const_init_val();

    void parse_var_decl();

    void parse_var_def();

    void parse_var_init();

    void parse_func_def();

    void parse_main_func_def();

    void parse_func_type();

    void parse_func_formal_params();

    void parse_func_formal_param();

    void parse_block();

    void parse_block_item();

    void parse_stmt();

    void parse_expr();

    void parse_cond_expr();

    void parse_lvalue_expr();

    void parse_prim_expr();

    void parse_number();

    void parse_unary_expr();

    void parse_unary_op();

    void parse_func_real_expr();

    void parse_muldiv_expr();

    void parse_addsub_expr();

    void parse_rel_expr();

    void parse_eq_expr();

    void parse_logical_and_expr();

    void parse_logical_or_expr();

    void parse_const_expr();
};


#endif //CODE_PARSER_H
