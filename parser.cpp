//
// Created by Kevin Tan on 2021/9/24.
//

#include "parser.h"

#define ERROR(expected, got) do { cerr << "In " << __func__ << " line " << __LINE__ << " source code line " << (*(got))->line << ", expected "#expected", got " << **(got) << endl; exit(-1); } while (0)

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

ObjectP Parser::check_ident_valid_decl(TokenIter &tk, TypeCode type, bool is_const = false, bool is_func = false) {
    IdentP current = dynamic_pointer_cast<Identifier>(*tk);
    ObjectP result;
    auto last_table = sym_table.back();
    if (last_table->find(current->name) == last_table->end()) {
        switch (type) {
            case VOID:
                assert(is_func);
                result = make_shared<FuncObject>(VOID);
                break;
            case INT:
                if (is_func) {
                    result = make_shared<FuncObject>(INT);
                } else {
                    result = make_shared<IntObject>();
                }
                break;
            case INT_ARRAY: {
                assert(!is_func);
                result = make_shared<ArrayObject>();
                break;
            }
            default:
                ERROR(type VOID or INT or INT_ARRAY, tk - 1);
        }
        result->is_const = is_const;
        result->ident_info = current;
        (*last_table)[current->name] = result;
    } else {
        error(IDENT_REDEFINED, current->line);
        result = (*last_table)[current->name];
    }
    elements.push_back(*tk++);
    return result;
}

void Parser::parse_const_def(TokenIter &tk, int nest_level) {
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    ObjectP current;
    ArrayObjectP array;
    if ((*tk)->type == IDENFR) {
        if (has_type(tk + 1, LBRACK)) {
            current = check_ident_valid_decl(tk, INT_ARRAY, true);
            array = cast<ArrayObject>(current);
        } else {
            current = check_ident_valid_decl(tk, INT, true);
        }
    } else {
        ERROR(IDENFR, tk);
    }
    while (has_type(tk, LBRACK)) {
        elements.push_back(*tk++);
        array->dims.push_back(parse_expr<ConstExpr>(tk));
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
    ObjectP current;
    ArrayObjectP array;
    if ((*tk)->type == IDENFR) {
        if (has_type(tk + 1, LBRACK)) {
            current = check_ident_valid_decl(tk, INT_ARRAY);
            array = cast<ArrayObject>(current);
        } else {
            current = check_ident_valid_decl(tk, INT);
        }
    } else {
        ERROR(IDENFR, tk);
    }
    while (has_type(tk, LBRACK)) {
        elements.push_back(*tk++);
        array->dims.push_back(parse_expr<ConstExpr>(tk));
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
    FuncObjectP current;
    if ((*tk)->type == IDENFR) {
        current = cast<FuncObject>(check_ident_valid_decl(tk, current_func_return_type, false, true));
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

void Parser::parse_func_formal_params(TokenIter &tk, FuncObjectP &func) {
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

void Parser::parse_func_formal_param(TokenIter &tk, FuncObjectP &func) {
    // FuncFParam -> BType Ident ['[' ']' { '[' ConstExp ']' }]
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    ObjectP current;
    ArrayObjectP array;
    if ((*tk)->type == IDENFR) {
        if (has_type(tk + 1, LBRACK)) {
            current = check_ident_valid_decl(tk, INT_ARRAY);
            array = cast<ArrayObject>(current);
        } else {
            current = check_ident_valid_decl(tk, INT);
        }
    } else {
        ERROR(IDENFR, tk);
    }
    if ((*tk)->type == LBRACK) {
        elements.push_back(*tk++);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
        while (has_type(tk, LBRACK)) {
            elements.push_back(*tk++);
            // TODO: 对于形参表，第一维不会存进去
            array->dims.push_back(parse_expr<ConstExpr>(tk));
            if ((*tk)->type == RBRACK) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RBRACK, (*(tk - 1))->line);
            }
        }
    }
    func->params.push_back(current);
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
    while (tk < tokens.end() && (starts_with_decl(tk) || starts_with_stmt(tk))) {  // pre-fetch
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
            for (auto p = tk + 1; p < tokens.end() && (*p)->type != SEMICN; ++p) {
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
            while (has_type(tk, COMMA)) {
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
ObjectP Parser::parse_expr(TokenIter &tk) {
    // Exp -> AddExp
    // ConstExp -> AddExp
    ObjectP result = parse_addsub_expr(tk);
    elements.push_back(make_shared<T>());
    return result;
}

ObjectP Parser::parse_cond_expr(TokenIter &tk) {
    // Cond -> LOrExp
    ObjectP result = parse_logical_or_expr(tk);
    elements.push_back(make_shared<ConditionExpr>());
    return result;
}

ObjectP Parser::check_ident_valid_use(TokenIter &tk, bool is_called, bool is_assigned = false) {
    IdentP current = dynamic_pointer_cast<Identifier>(*tk);
    elements.push_back(*tk++);
    ObjectP result;
    for (auto p = sym_table.rbegin(); p != sym_table.rend(); ++p) {
        if ((*p)->find(current->name) != (*p)->end()) {
            result = (**p)[current->name];
            if (is_called && result->type != FUNCTION) {
                cerr << "In source code line " << current->line << ", "
                     << *current << " is not callable" << endl;
            } else if (is_assigned && result->is_const) {
                error(CANNOT_MODIFY_CONST, current->line);
            }  // TODO: check
            return result;
        }
    }
    error(IDENT_UNDEFINED, current->line);
    return make_shared<Object>();
}

ObjectP Parser::parse_lvalue(TokenIter &tk, bool is_assigned = false) {
    // LVal -> Ident { '[' Exp ']' }
    ObjectP result;
    if ((*tk)->type == IDENFR) {
        result = check_ident_valid_use(tk, false, is_assigned);
        instructions.push_back(make_shared<LoadObject>(result));
    } else {
        ERROR(IDENFR, tk);
    }
    int cnt;
    for (cnt = 0; has_type(tk, LBRACK); cnt++) {
        elements.push_back(*tk++);
        parse_expr<NormalExpr>(tk);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    ArrayObjectP array = cast<ArrayObject>(result);
    if (array != nullptr) {
        array->dereference_cnt = cnt > 0 ? cnt : 0;
    }
    elements.push_back(make_shared<LValue>());
    return result;
}

ObjectP Parser::parse_primary_expr(TokenIter &tk) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    ObjectP result;
    switch ((*tk)->type) {
        case LPARENT:
            elements.push_back(*tk++);
            result = parse_expr<NormalExpr>(tk);
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            break;
        case IDENFR:
            result = parse_lvalue(tk);  // pre-fetch
            break;
        case INTCON:
            result = parse_number(tk);  // pre-fetch
            break;
        default:
            ERROR(LPARENT or IDENFR or INTCON, tk);
    }
    elements.push_back(make_shared<PrimaryExpr>());
    return result;
}

ObjectP Parser::parse_number(TokenIter &tk) {
    // Number -> IntLiteral
    auto result = dynamic_pointer_cast<IntLiteral>(*tk);
    elements.push_back(*tk++);
    elements.push_back(make_shared<Number>());
    return result;
}

ObjectP Parser::parse_unary_expr(TokenIter &tk) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    ObjectP result;
    switch ((*tk)->type) {
        case IDENFR:
            if (has_type(tk + 1, LPARENT)) {  // is function call
                int line = (*tk)->line;
                FuncObjectP func = cast<FuncObject>(check_ident_valid_use(tk, true));
                elements.push_back(*tk++);
                if (starts_with_expr(tk)) {
                    parse_func_real_params(tk, func, line);
                } else if (func != nullptr && !func->params.empty()) {
                    error(PARAM_AMOUNT_MISMATCH, line);
                }
                if ((*tk)->type == RPARENT) {
                    elements.push_back(*tk++);
                } else {
                    error(MISSING_RPAREN, (*(tk - 1))->line);
                }
                if (func != nullptr) {
                    instructions.push_back(make_shared<CallFunction>(func));
                    switch (func->return_type) {
                        case VOID:
                            result = make_shared<Object>();
                            break;
                        case INT:
                            result = make_shared<IntObject>();
                            break;
                        default:
                            cerr << "In " << __func__ << " line " << __LINE__ << " source code line "
                                 << func->ident_info->line
                                 << ", compiler only supports VOID or INT function return type" << endl;
                            exit(-1);
                    }
                } else {
                    result = make_shared<Object>();
                }
            } else {  // is not function call
                result = parse_primary_expr(tk);  // pre-fetch LVal
            }
            break;
        case PLUS:
        case MINU:
        case NOT:
            parse_unary_op(tk);  // pre-fetch
            result = parse_unary_expr(tk);
            break;
        default:
            result = parse_primary_expr(tk);  // pre-fetch
    }
    elements.push_back(make_shared<UnaryExpr>());
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

void Parser::parse_func_real_params(TokenIter &tk, FuncObjectP &func, int func_line) {
    // FuncRParams -> Exp { ',' Exp }
    bool first_error = true;
    int cnt;
    for (cnt = 0; tk < tokens.end(); cnt++) {
        ObjectP instance = parse_expr<NormalExpr>(tk);
        if (func != nullptr) {
            if (cnt < func->params.size()) {
                ObjectP func_param = func->params[cnt];
                switch (instance->type) {
                    case INT:
                        if (func_param->type != INT) {
                            error(PARAM_TYPE_MISMATCH, func_line);
                        }
                        break;
                    case INT_ARRAY: {
                        ArrayObjectP array = cast<ArrayObject>(instance);
                        switch (func_param->type) {
                            case INT:
                                if (array->dims.size() - array->dereference_cnt != 0) {
                                    error(PARAM_TYPE_MISMATCH, func_line);
                                }
                                break;
                            case INT_ARRAY: {
                                ArrayObjectP array_param = cast<ArrayObject>(func_param);
                                if (array_param->dims.size() + 1 !=
                                    array->dims.size() - array->dereference_cnt) {
                                    error(PARAM_TYPE_MISMATCH, func_line);
                                }
                                break;
                            }
                            default:
                                cerr << "In " << __func__ << " line " << __LINE__ << " source code line " << func_line
                                     << ", compiler only supports INT or INT_ARRAY function arguments" << endl;
                                exit(-1);
                        }
                        break;
                    }
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
    if (func != nullptr && cnt < func->params.size()) {
        error(PARAM_AMOUNT_MISMATCH, func_line);
    }
    elements.push_back(make_shared<FuncRealParams>());
}

template<typename T>
ObjectP Parser::_parse_expr(TokenIter &tk,
                            ObjectP (Parser::*parse_first)(TokenIter &),
                            const function<bool(TokenCode)> &predicate) {
    ObjectP result, tmp;
    while (tk < tokens.end()) {
        tmp = (this->*parse_first)(tk);
        if (result == nullptr || result->type == INT && tmp->type != INT) {
            result = tmp;
        }
        if (predicate((*tk)->type)) {
            elements.push_back(make_shared<T>());
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(make_shared<T>());
    return result;
}

ObjectP Parser::parse_muldiv_expr(TokenIter &tk) {
    // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    return _parse_expr<MulDivExpr>(tk, &Parser::parse_unary_expr,
                                   [](TokenCode next) { return next == MULT || next == DIV || next == MOD; });
}

ObjectP Parser::parse_addsub_expr(TokenIter &tk) {
    // AddExp -> MulExp | AddExp ('+' | '-') MulExp
    // AddExp -> MulExp { ('+' | '-') MulExp }
    return _parse_expr<AddSubExpr>(tk, &Parser::parse_muldiv_expr,
                                   [](TokenCode next) { return next == PLUS || next == MINU; });
}

ObjectP Parser::parse_rel_expr(TokenIter &tk) {
    // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    return _parse_expr<RelationalExpr>(tk, &Parser::parse_addsub_expr,
                                       [](TokenCode next) {
                                           return next == LSS || next == LEQ || next == GRE || next == GEQ;
                                       });
}

ObjectP Parser::parse_eq_expr(TokenIter &tk) {
    // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    // EqExp -> RelExp { ('==' | '!=') RelExp }
    return _parse_expr<EqualExpr>(tk, &Parser::parse_rel_expr,
                                  [](TokenCode next) { return next == EQL || next == NEQ; });
}

ObjectP Parser::parse_logical_and_expr(TokenIter &tk) {
    // LAndExp -> EqExp | LAndExp '&&' EqExp
    // LAndExp -> EqExp { '&&' EqExp }
    return _parse_expr<LogicalAndExpr>(tk, &Parser::parse_eq_expr,
                                       [](TokenCode next) { return next == AND; });
}

ObjectP Parser::parse_logical_or_expr(TokenIter &tk) {
    // LOrExp -> LAndExp | LOrExp '||' LAndExp
    // LOrExp -> LAndExp { '||' LAndExp }
    return _parse_expr<LogicalOrExpr>(tk, &Parser::parse_logical_and_expr,
                                      [](TokenCode next) { return next == OR; });
}
