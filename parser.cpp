//
// Created by Kevin Tan on 2021/9/24.
//

#include "parser.h"

#define ERROR(expected, got) do { cerr << "In " << __func__ << " line " << __LINE__ << " source code line " << (*got)->line << ", expected "#expected", got " << **got << endl; exit(-1); } while (0)

Parser::Parser(vector<TokenP> &tokens, Error &error) : tokens(tokens), error(error) {
    if (!tokens.empty()) {
        auto tk = tokens.begin();
        parse_comp_unit(tk);
    }
}

void Parser::parse_comp_unit(TokenIter &tk) {
    // CompUnit -> {Decl} {FuncDef} MainFuncDef
    sym_table.push_back(make_shared<HashMap>());
    while (starts_with_decl(tk)) {
        parse_decl(tk, 0);
    }
    while (starts_with_func_def(tk)) {
        parse_func_def(tk);
    }
    if ((*tk)->type == INTTK) {
        parse_main_func_def(tk);
    }
    elements.push_back(make_shared<CompUnit>());
}

void Parser::parse_decl(TokenIter &tk, int nest_level) {
    // Decl -> ConstDecl | VarDecl
    switch ((*tk)->type) {
        case CONSTTK:
            parse_const_decl(tk, nest_level);
            break;
        case INTTK:
            parse_var_decl(tk, nest_level);
            break;
        default:
            ERROR(CONSTTK or INTTK, tk);
    }
}

void Parser::parse_const_decl(TokenIter &tk, int nest_level) {
    // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    if ((*tk)->type == CONSTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(CONSTTK, tk);
    }
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    while (tk < tokens.end()) {
        parse_const_def(tk, nest_level);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else if ((*tk)->type == SEMICN) {
            elements.push_back(*tk++);
            break;
        } else {
            error(MISSING_SEMICN, (*(tk - 1))->line);
            break;
        }
    }
    elements.push_back(make_shared<ConstDecl>());
}

IdentP Parser::check_ident_valid_decl(TokenIter &tk, TypeCode type, int nest_level,
                                      bool is_const = false, bool is_func = false) {
    IdentP current = dynamic_pointer_cast<Identifier>(*tk);
    auto last_table = sym_table.back();
    if (last_table->find(current->value) == last_table->end()) {
        (*last_table)[current->value] = current;
        current->val_type = type;
        current->nest_level = nest_level;
        current->is_const = is_const;
        current->is_func = is_func;
        current->is_defined = true;
    } else {
        error(IDENT_REDEFINED, current->line);
    }
    elements.push_back(*tk++);
    return current;
}

void Parser::parse_const_def(TokenIter &tk, int nest_level) {
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    IdentP current;
    if ((*tk)->type == IDENFR) {
        current = check_ident_valid_decl(tk, INT, nest_level, true);
    } else {
        ERROR(IDENFR, tk);
    }
    while ((*tk)->type == LBRACK && tk < tokens.end()) {
        elements.push_back(*tk++);
        current->val_type = ARRAY;
        current->array_dims.push_back(parse_expr<ConstExpr>(tk));
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    if ((*tk)->type == ASSIGN) {
        elements.push_back(*tk++);
    } else {
        ERROR(ASSIGN, tk);
    }
    parse_init_val<ConstExpr, ConstInitVal>(tk);
    elements.push_back(make_shared<ConstDef>());
}

void Parser::parse_var_decl(TokenIter &tk, int nest_level) {
    // VarDecl -> BType VarDef { ',' VarDef } ';'
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    while (tk < tokens.end()) {
        parse_var_def(tk, nest_level);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else if ((*tk)->type == SEMICN) {
            elements.push_back(*tk++);
            break;
        } else {
            error(MISSING_SEMICN, (*(tk - 1))->line);
            break;
        }
    }
    elements.push_back(make_shared<VarDecl>());
}

void Parser::parse_var_def(TokenIter &tk, int nest_level) {
    // VarDef -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal
    IdentP current;
    if ((*tk)->type == IDENFR) {
        current = check_ident_valid_decl(tk, INT, nest_level);
    } else {
        ERROR(IDENFR, tk);
    }
    while ((*tk)->type == LBRACK && tk < tokens.end()) {
        elements.push_back(*tk++);
        current->val_type = ARRAY;
        current->array_dims.push_back(parse_expr<ConstExpr>(tk));
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    if ((*tk)->type == ASSIGN) {
        elements.push_back(*tk++);
        parse_init_val<NormalExpr, InitVal>(tk);
    }
    elements.push_back(make_shared<VarDef>());
}

template<typename ExprT, typename ElementT>
void Parser::parse_init_val(TokenIter &tk) {
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    if ((*tk)->type == LBRACE) {
        elements.push_back(*tk++);
        if ((*tk)->type != RBRACE) {
            while (tk < tokens.end()) {
                parse_init_val<ExprT, ElementT>(tk);
                if ((*tk)->type == COMMA) {
                    elements.push_back(*tk++);
                } else {
                    break;
                }
            }
        }
        if ((*tk)->type == RBRACE) {
            elements.push_back(*tk++);
        } else {
            ERROR(COMMA or RBRACE, tk);
        }
    } else {
        parse_expr<ExprT>(tk);
    }
    elements.push_back(make_shared<ElementT>());
}

void Parser::parse_func_def(TokenIter &tk) {
    // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    current_func_return_type = parse_func_type(tk);
    IdentP current;
    if ((*tk)->type == IDENFR) {
        current = check_ident_valid_decl(tk, current_func_return_type, 0, false, true);
    } else {
        ERROR(IDENFR, tk);
    }
    if ((*tk)->type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR(LPARENT, tk);
    }
    sym_table.push_back(make_shared<HashMap>());
    if ((*tk)->type == INTTK) {  // pre-fetch
        parse_func_formal_params(tk, current);
    }
    if ((*tk)->type == RPARENT) {
        elements.push_back(*tk++);
    } else {
        error(MISSING_RPAREN, (*(tk - 1))->line);
    }
    parse_block(tk, 1, true);
    if (current_func_return_type == INT && !is_return_at_end) {
        error(MISSING_RETURN, (*(tk - 1))->line);
    }
    is_return_at_end = false;
    elements.push_back(make_shared<FuncDef>());
}

void Parser::parse_main_func_def(TokenIter &tk) {
    // MainFuncDef -> 'int' 'main' '(' ')' Block
    if ((*tk)->type == INTTK) {
        current_func_return_type = INT;
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    if ((*tk)->type == MAINTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(MAINTK, tk);
    }
    if ((*tk)->type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR(LPARENT, tk);
    }
    if ((*tk)->type == RPARENT) {
        elements.push_back(*tk++);
    } else {
        error(MISSING_RPAREN, (*(tk - 1))->line);
    }
    parse_block(tk, 1);
    if (!is_return_at_end) {
        error(MISSING_RETURN, (*(tk - 1))->line);
    }
    elements.push_back(make_shared<MainFuncDef>());
}

TypeCode Parser::parse_func_type(TokenIter &tk) {
    // FuncType -> 'void' | 'int'
    TypeCode result;
    if ((*tk)->type == INTTK) {
        result = INT;
    } else if ((*tk)->type == VOIDTK) {
        result = VOID;
    } else {
        ERROR(INTTK or VOIDTK, tk);
    }
    elements.push_back(*tk++);
    elements.push_back(make_shared<FuncType>());
    return result;
}

void Parser::parse_func_formal_params(TokenIter &tk, IdentP &func) {
    // FuncFParams -> FuncFParam { ',' FuncFParam }
    while (tk < tokens.end()) {
        parse_func_formal_param(tk, func);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(make_shared<FuncFormalParams>());
}

void Parser::parse_func_formal_param(TokenIter &tk, IdentP &func) {
    // FuncFParam -> BType Ident ['[' ']' { '[' ConstExp ']' }]
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    IdentP current;
    if ((*tk)->type == IDENFR) {
        current = check_ident_valid_decl(tk, INT, 1);
        func->func_params.push_back(current);
    } else {
        ERROR(IDENFR, tk);
    }
    if ((*tk)->type == LBRACK) {
        elements.push_back(*tk++);
        current->val_type = ARRAY;
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
        while ((*tk)->type == LBRACK && tk < tokens.end()) {
            elements.push_back(*tk++);
            // TODO: 对于形参表，第一维不会存进去
            current->array_dims.push_back(parse_expr<ConstExpr>(tk));
            if ((*tk)->type == RBRACK) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RBRACK, (*(tk - 1))->line);
            }
        }
    }
    elements.push_back(make_shared<FuncFormalParam>());
}

void Parser::parse_block(TokenIter &tk, int nest_level, bool from_func_def) {
    // Block -> '{' { BlockItem } '}'
    if ((*tk)->type == LBRACE) {
        elements.push_back(*tk++);
    } else {
        ERROR(LBRACE, tk);
    }
    if (!from_func_def) {
        sym_table.push_back(make_shared<HashMap>());
    }
    while ((starts_with_decl(tk) || starts_with_stmt(tk)) && tk < tokens.end()) {  // pre-fetch
        parse_block_item(tk, nest_level);
    }
    if ((*tk)->type == RBRACE) {
        elements.push_back(*tk++);
    } else {
        ERROR(RBRACE, tk);
    }
    sym_table.pop_back();
    elements.push_back(make_shared<Block>());
}

void Parser::parse_block_item(TokenIter &tk, int nest_level) {
    // BlockItem -> Decl | Stmt
    if (starts_with_decl(tk)) {
        parse_decl(tk, nest_level);
    } else if (starts_with_stmt(tk)) {
        parse_stmt(tk, nest_level);
    } else {
        ERROR(Decl or Stmt, tk);
    }
}

void Parser::parse_stmt(TokenIter &tk, int nest_level) {
    // Stmt -> LVal '=' Exp ';'
    // | [Exp] ';'
    // | Block
    // | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    // | 'while' '(' Cond ')' Stmt
    // | 'break' ';' | 'continue' ';'
    // | 'return' [Exp] ';'
    // | LVal = 'getint' '(' ')' ';'
    // | 'printf' '(' FormatString { "," Exp } ')' ';'
    is_return_at_end = false;
    switch ((*tk)->type) {
        case IDENFR: {
            bool no_assign = true;
            for (auto p = tk + 1; (*p)->type != SEMICN && p < tokens.end(); ++p) {
                if ((*p)->type == ASSIGN) {
                    no_assign = false;
                    parse_lvalue(tk, true);  // pre-fetch
                    if ((*tk)->type == ASSIGN) {
                        elements.push_back(*tk++);
                    } else {
                        // error(MISSING_SEMICN, (*(tk - 1))->line);
                        break;
                    }
                    if ((*tk)->type == GETINTTK) {
                        elements.push_back(*tk++);
                        if ((*tk)->type == LPARENT) {
                            elements.push_back(*tk++);
                        } else {
                            ERROR(LPARENT, tk);
                        }
                        if ((*tk)->type == RPARENT) {
                            elements.push_back(*tk++);
                        } else {
                            error(MISSING_RPAREN, (*(tk - 1))->line);
                        }
                    } else {
                        parse_expr<NormalExpr>(tk);  // pre-fetch
                    }
                    break;
                }
            }
            if (no_assign) {
                parse_expr<NormalExpr>(tk);  // pre-fetch
            }
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            break;
        }
        case SEMICN:
            elements.push_back(*tk++);
            break;
        case LBRACE:
            parse_block(tk, nest_level + 1);  // pre-fetch
            break;
        case IFTK:
            elements.push_back(*tk++);
            if ((*tk)->type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR(LPARENT, tk);
            }
            parse_cond_expr(tk);
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            parse_stmt(tk, nest_level + 1);
            if ((*tk)->type == ELSETK) {
                elements.push_back(*tk++);
                parse_stmt(tk, nest_level + 1);
            }
            break;
        case WHILETK:
            while_cnt++;
            elements.push_back(*tk++);
            if ((*tk)->type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR(LPARENT, tk);
            }
            parse_cond_expr(tk);
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            parse_stmt(tk, nest_level + 1);
            while_cnt--;
            break;
        case BREAKTK:
        case CONTINUETK:
            if (while_cnt == 0) {
                error(BREAK_CONTINUE_NOT_IN_LOOP, (*tk)->line);
            }
            elements.push_back(*tk++);
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            break;
        case RETURNTK:
            elements.push_back(*tk++);
            if (starts_with_expr(tk)) {
                if (current_func_return_type == VOID) {
                    error(RETURN_TYPE_MISMATCH, (*(tk - 1))->line);
                }
                parse_expr<NormalExpr>(tk);
            }
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            if (nest_level == 1 && (*tk)->type == RBRACE) {
                is_return_at_end = true;
            }
            break;
        case PRINTFTK: {
            TokenP p = *tk;
            elements.push_back(*tk++);
            if ((*tk)->type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR(LPARENT, tk);
            }
            int fmt_char_cnt;
            auto fmt_str = dynamic_pointer_cast<FormatString>(*tk);
            if ((*tk)->type == STRCON) {
                fmt_char_cnt = fmt_str->fmt_char_cnt;
                elements.push_back(*tk++);
            } else {
                ERROR(STRCON, tk);
            }
            int cnt = 0;
            while ((*tk)->type == COMMA && tk < tokens.end()) {
                elements.push_back(*tk++);
                parse_expr<NormalExpr>(tk);
                cnt++;
            }
            if (cnt != fmt_char_cnt) {
                error(FORMAT_STRING_ARGUMENT_MISMATCH, p->line);
            }
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            break;
        }
        default:
            parse_expr<NormalExpr>(tk);  // pre-fetch
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
    }
    elements.push_back(make_shared<Statement>());
}

template<typename T>
ElementP Parser::parse_expr(TokenIter &tk, InstanceP *out_instance) {
    // Exp -> AddExp
    // ConstExp -> AddExp
    ElementP result = parse_addsub_expr(tk, out_instance);
    elements.push_back(make_shared<T>());
    return result;
}

ElementP Parser::parse_cond_expr(TokenIter &tk, InstanceP *out_instance) {
    // Cond -> LOrExp
    ElementP result = parse_logical_or_expr(tk, out_instance);
    elements.push_back(make_shared<ConditionExpr>());
    return result;
}

IdentP Parser::check_ident_valid_use(TokenIter &tk, bool is_assigned = false) {
    IdentP current = dynamic_pointer_cast<Identifier>(*tk);
    elements.push_back(*tk++);
    for (auto p = sym_table.rbegin(); p != sym_table.rend(); ++p) {
        if ((*p)->find(current->value) != (*p)->end()) {
            current->is_defined = true;
            IdentP result = (**p)[current->value];
            if (is_assigned && result->is_const) {
                error(CANNOT_MODIFY_CONST, current->line);
            } else {
                *current = *result;
            }
            return current;
        }
    }
    error(IDENT_UNDEFINED, current->line);
    return current;
}

IdentP Parser::parse_lvalue(TokenIter &tk, bool is_assigned = false) {
    // LVal -> Ident { '[' Exp ']' }
    IdentP result;
    if ((*tk)->type == IDENFR) {
        result = check_ident_valid_use(tk, is_assigned);
    } else {
        ERROR(IDENFR, tk);
    }
    int cnt;
    for (cnt = 0; (*tk)->type == LBRACK && tk < tokens.end(); cnt++) {
        elements.push_back(*tk++);
        parse_expr<NormalExpr>(tk);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    result->dereference_cnt = cnt;
    elements.push_back(make_shared<LValue>());
    return result;
}

ElementP Parser::parse_primary_expr(TokenIter &tk, InstanceP *out_instance) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    ElementP result;
    switch ((*tk)->type) {
        case LPARENT:
            elements.push_back(*tk++);
            result = parse_expr<NormalExpr>(tk, out_instance);
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            break;
        case IDENFR:
            if (out_instance == nullptr) {
                result = parse_lvalue(tk);  // pre-fetch
            } else {
                result = *out_instance = parse_lvalue(tk);  // pre-fetch
            }
            break;
        case INTCON:
            if (out_instance == nullptr) {
                result = parse_number(tk);  // pre-fetch
            } else {
                result = *out_instance = parse_number(tk);  // pre-fetch
            }
            break;
        default:
            ERROR(LPARENT or IDENFR or INTCON, tk);
    }
    elements.push_back(make_shared<PrimaryExpr>());
    return result;
}

InstanceP Parser::parse_number(TokenIter &tk) {
    // Number -> IntConst
    auto result = dynamic_pointer_cast<IntConst>(*tk);
    elements.push_back(*tk++);
    elements.push_back(make_shared<Number>());
    return result;
}

ElementP Parser::parse_unary_expr(TokenIter &tk, InstanceP *out_instance) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    switch ((*tk)->type) {
        case IDENFR:
            if ((*(tk + 1))->type == LPARENT) {
                int line = (*tk)->line;
                IdentP func = check_ident_valid_use(tk);
                elements.push_back(*tk++);
                func->is_called = true;
                if (starts_with_expr(tk)) {
                    parse_func_real_params(tk, func, line, func->is_defined);
                } else if (!func->func_params.empty()) {
                    error(PARAM_AMOUNT_MISMATCH, line);
                }
                if ((*tk)->type == RPARENT) {
                    elements.push_back(*tk++);
                } else {
                    error(MISSING_RPAREN, (*(tk - 1))->line);
                }
                if (out_instance != nullptr) {
                    *out_instance = func;
                }
            } else {
                parse_primary_expr(tk, out_instance);  // pre-fetch LVal
            }
            break;
        case PLUS:
        case MINU:
        case NOT:
            parse_unary_op(tk);  // pre-fetch
            parse_unary_expr(tk, out_instance);
            break;
        default:
            parse_primary_expr(tk, out_instance);  // pre-fetch
    }
    elements.push_back(make_shared<UnaryExpr>());
    ElementP result;  // FIXME
    return result;
}

TokenCode Parser::parse_unary_op(TokenIter &tk) {
    TokenCode result = (*tk)->type;
    switch ((*tk)->type) {
        case PLUS:
        case MINU:
        case NOT:
            elements.push_back(*tk++);
            break;
        default:
            ERROR(PLUS or MINU or NOT, tk);
    }
    elements.push_back(make_shared<UnaryOp>());
    return result;
}

void Parser::parse_func_real_params(TokenIter &tk, IdentP &func, int func_line, bool report_error=true) {
    // FuncRParams -> Exp { ',' Exp }
    bool first_error = true;
    int cnt;
    for (cnt = 0; tk < tokens.end(); cnt++) {
        InstanceP instance;
        parse_expr<NormalExpr>(tk, &instance);
        if (report_error) {
            if (cnt < func->func_params.size()) {
                IdentP func_param = func->func_params[cnt];
                if (instance->is_func && !instance->is_called) {
                    error(PARAM_TYPE_MISMATCH, func_line);
                }
                instance->is_called = false;
                switch (instance->val_type) {
                    case INT:
                        if (func_param->val_type != INT) {
                            error(PARAM_TYPE_MISMATCH, func_line);
                        }
                        break;
                    case ARRAY:
                        if (func_param->val_type == INT) {
                            if (instance->array_dims.size() - instance->dereference_cnt != 0) {
                                error(PARAM_TYPE_MISMATCH, func_line);
                            }
                        } else if (func_param->array_dims.size() + 1 !=
                                   instance->array_dims.size() - instance->dereference_cnt) {
                            error(PARAM_TYPE_MISMATCH, func_line);
                        }
                        break;
                    default:
                        error(PARAM_TYPE_MISMATCH, func_line);
                }
            } else if (first_error) {
                error(PARAM_AMOUNT_MISMATCH, func_line);
                first_error = false;
            }
        }
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else {
            cnt++;
            break;
        }
    }
    if (cnt < func->func_params.size()) {
        error(PARAM_AMOUNT_MISMATCH, func_line);
    }
    elements.push_back(make_shared<FuncRealParams>());
}

template<typename T>
ElementP Parser::_parse_expr(TokenIter &tk, InstanceP *out_instance,
                             ElementP (Parser::*parse_first)(TokenIter &, InstanceP *),
                             const function<bool(TokenCode)> &predicate) {
    while (tk < tokens.end()) {
        (this->*parse_first)(tk, out_instance);
        if (predicate((*tk)->type)) {
            elements.push_back(make_shared<T>());
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(make_shared<T>());
    ElementP result;  // FIXME
    return result;
}

ElementP Parser::parse_muldiv_expr(TokenIter &tk, InstanceP *out_instance) {
    // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    return _parse_expr<MulDivExpr>(tk, out_instance, &Parser::parse_unary_expr,
                                   [](TokenCode next) { return next == MULT || next == DIV || next == MOD; });
}

ElementP Parser::parse_addsub_expr(TokenIter &tk, InstanceP *out_instance) {
    // AddExp -> MulExp | AddExp ('+' | '-') MulExp
    // AddExp -> MulExp { ('+' | '-') MulExp }
    return _parse_expr<AddSubExpr>(tk, out_instance, &Parser::parse_muldiv_expr,
                                   [](TokenCode next) { return next == PLUS || next == MINU; });
}

ElementP Parser::parse_rel_expr(TokenIter &tk, InstanceP *out_instance) {
    // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    return _parse_expr<RelationalExpr>(tk, out_instance, &Parser::parse_addsub_expr,
                                       [](TokenCode next) {
                                           return next == LSS || next == LEQ || next == GRE || next == GEQ;
                                       });
}

ElementP Parser::parse_eq_expr(TokenIter &tk, InstanceP *out_instance) {
    // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    // EqExp -> RelExp { ('==' | '!=') RelExp }
    return _parse_expr<EqualExpr>(tk, out_instance, &Parser::parse_rel_expr,
                                  [](TokenCode next) { return next == EQL || next == NEQ; });
}

ElementP Parser::parse_logical_and_expr(TokenIter &tk, InstanceP *out_instance) {
    // LAndExp -> EqExp | LAndExp '&&' EqExp
    // LAndExp -> EqExp { '&&' EqExp }
    return _parse_expr<LogicalAndExpr>(tk, out_instance, &Parser::parse_eq_expr,
                                       [](TokenCode next) { return next == AND; });
}

ElementP Parser::parse_logical_or_expr(TokenIter &tk, InstanceP *out_instance) {
    // LOrExp -> LAndExp | LOrExp '||' LAndExp
    // LOrExp -> LAndExp { '||' LAndExp }
    return _parse_expr<LogicalOrExpr>(tk, out_instance, &Parser::parse_logical_and_expr,
                                      [](TokenCode next) { return next == OR; });
}
