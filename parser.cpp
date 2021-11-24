//
// Created by Kevin Tan on 2021/9/24.
//

#include "parser.h"

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
    instructions.push_back(make_shared<CallFunction>());
    entry_inst = instructions.back();
    instructions.push_back(make_shared<Exit>());
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
            ERROR_EXPECTED_GOT(CONSTTK or INTTK, tk);
    }
}

void Parser::parse_const_decl(TokenIter &tk, int nest_level) {
    // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    if ((*tk)->type == CONSTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(CONSTTK, tk);
    }
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
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
                ERROR_EXPECTED_GOT(type VOID or INT or INT_ARRAY, tk - 1);
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
    bool is_array;
    if ((*tk)->type == IDENFR) {
        is_array = has_type(tk + 1, LBRACK);
        if (is_array) {
            current = check_ident_valid_decl(tk, INT_ARRAY, true);
            array = cast<ArrayObject>(current);
//            instructions.push_back(make_shared<LoadObject>(array));
        } else {
            current = check_ident_valid_decl(tk, INT, true);
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    while (has_type(tk, LBRACK)) {
        elements.push_back(*tk++);
        array->dims.push_back(cast<IntObject>(parse_expr<ConstExpr>(tk, false))->value);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
            if (array->dims.size() > 1) {
//                instructions.push_back(make_shared<BinaryOperation>(BINARY_MUL));
            }
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    if (is_array) {
//        instructions.push_back(make_shared<BuildArray>());
    }
    if ((*tk)->type == ASSIGN) {
        elements.push_back(*tk++);
        ObjectP init_val = parse_init_val<ConstExpr, ConstInitVal>(tk, false);
        if (is_array) {
//            instructions.push_back(make_shared<InitArray>(array));
            array->data = cast<ArrayObject>(init_val)->data;
            // array is flattened for convenience,
            // and need to be considered well when passed to functions
        } else {
//            instructions.push_back(make_shared<StoreObject>(current));
            cast<IntObject>(current)->value = cast<IntObject>(init_val)->value;
        }
    } else {
        ERROR_EXPECTED_GOT(ASSIGN, tk);
    }
    elements.push_back(make_shared<ConstDef>());
}

void Parser::parse_var_decl(TokenIter &tk, int nest_level) {
    // VarDecl -> BType VarDef { ',' VarDef } ';'
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
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
    bool is_array;
    if ((*tk)->type == IDENFR) {
        is_array = has_type(tk + 1, LBRACK);
        if (is_array) {
            current = check_ident_valid_decl(tk, INT_ARRAY);
            array = cast<ArrayObject>(current);
            instructions.push_back(make_shared<LoadObject>(array));
        } else {
            current = check_ident_valid_decl(tk, INT);
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    while (has_type(tk, LBRACK)) {
        elements.push_back(*tk++);
        array->dims.push_back(cast<IntObject>(parse_expr<ConstExpr>(tk))->value);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
            if (array->dims.size() > 1) {
                instructions.push_back(make_shared<BinaryOperation>(BINARY_MUL));
            }
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    if (is_array) {
        instructions.push_back(make_shared<BuildArray>());
    }
    if ((*tk)->type == ASSIGN) {
        elements.push_back(*tk++);
        parse_init_val<NormalExpr, InitVal>(tk);
        if (is_array) {
            instructions.push_back(make_shared<InitArray>(array));
            // array is flattened for convenience,
            // and need to be considered well when passed to functions
        } else {
            instructions.push_back(make_shared<StoreObject>(current));
        }
    }
    elements.push_back(make_shared<VarDef>());
}

template<typename ExprT, typename ElementT>
ObjectP Parser::parse_init_val(TokenIter &tk, bool generate_inst) {
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    ObjectP result;
    if ((*tk)->type == LBRACE) {
        elements.push_back(*tk++);
        if ((*tk)->type != RBRACE) {
            ArrayObjectP array = make_shared<ArrayObject>();
            array->data = make_shared<Array>();
            while (tk < tokens.end()) {
                ObjectP o = parse_init_val<ExprT, ElementT>(tk, generate_inst);
                if (o->type == INT_ARRAY) {
                    ArrayObjectP tmp = cast<ArrayObject>(o);
                    array->data->insert(array->data->end(), tmp->data->begin(), tmp->data->end());
                } else {
                    array->data->push_back(o);
                }
                if ((*tk)->type == COMMA) {
                    elements.push_back(*tk++);
                } else {
                    break;
                }
            }
            result = array;
        }
        if ((*tk)->type == RBRACE) {
            elements.push_back(*tk++);
        } else {
            ERROR_EXPECTED_GOT(COMMA or RBRACE, tk);
        }
    } else {
        result = parse_expr<ExprT>(tk, generate_inst);
    }
    elements.push_back(make_shared<ElementT>());
    return result;
}

void Parser::parse_func_def(TokenIter &tk) {
    // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    current_func_return_type = parse_func_type(tk);
    FuncObjectP current;
    if ((*tk)->type == IDENFR) {
        current = cast<FuncObject>(check_ident_valid_decl(tk, current_func_return_type, false, true));
        current->code_offset = (long long) instructions.size();
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    if ((*tk)->type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(LPARENT, tk);
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
    if (!has_return_at_end) {
        if (current_func_return_type == VOID) {
            instructions.push_back(make_shared<ReturnValue>());
        } else {
            error(MISSING_RETURN, (*(tk - 1))->line);
        }
    }
    has_return_at_end = false;
    elements.push_back(make_shared<FuncDef>());
}

void Parser::parse_main_func_def(TokenIter &tk) {
    // MainFuncDef -> 'int' 'main' '(' ')' Block
    if ((*tk)->type == INTTK) {
        current_func_return_type = INT;
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
    }
    if ((*tk)->type == MAINTK) {
        FuncObjectP main = cast<FuncObject>(*tk);
        main->code_offset = (long long) instructions.size();
        cast<CallFunction>(entry_inst)->set_func(main);
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(MAINTK, tk);
    }
    if ((*tk)->type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(LPARENT, tk);
    }
    if ((*tk)->type == RPARENT) {
        elements.push_back(*tk++);
    } else {
        error(MISSING_RPAREN, (*(tk - 1))->line);
    }
    parse_block(tk, 1);
    if (!has_return_at_end) {
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
        ERROR_EXPECTED_GOT(INTTK or VOIDTK, tk);
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
        ERROR_EXPECTED_GOT(INTTK, tk);
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
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    if ((*tk)->type == LBRACK) {
        elements.push_back(*tk++);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
            array->dims.push_back(0);  // dummy dimension for int a[]
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
        while (has_type(tk, LBRACK)) {
            elements.push_back(*tk++);
            array->dims.push_back(cast<IntObject>(parse_expr<ConstExpr>(tk, false))->value);
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
        ERROR_EXPECTED_GOT(LBRACE, tk);
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
        ERROR_EXPECTED_GOT(RBRACE, tk);
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
        ERROR_EXPECTED_GOT(Decl or Stmt, tk);
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
    has_return_at_end = false;
    switch ((*tk)->type) {
        case IDENFR: {
            bool no_assign = true, is_indexed = has_type(tk + 1, LBRACK);
            for (auto p = tk + 1; p < tokens.end() && (*p)->type != SEMICN; ++p) {
                if ((*p)->type == ASSIGN) {
                    no_assign = false;
                    ObjectP lvalue = parse_lvalue(tk, true, true);  // pre-fetch
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
                            ERROR_EXPECTED_GOT(LPARENT, tk);
                        }
                        if ((*tk)->type == RPARENT) {
                            elements.push_back(*tk++);
                            instructions.push_back(make_shared<GetInt>());
                        } else {
                            error(MISSING_RPAREN, (*(tk - 1))->line);
                        }
                    } else {
                        parse_expr<NormalExpr>(tk);  // pre-fetch
                    }
                    if (is_indexed) {
                        instructions.push_back(make_shared<StoreSubscript>());
                    } else {
                        instructions.push_back(make_shared<StoreObject>(lvalue));
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
                ERROR_EXPECTED_GOT(LPARENT, tk);
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
                ERROR_EXPECTED_GOT(LPARENT, tk);
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
                has_return_at_end = true;
            }
            instructions.push_back(make_shared<ReturnValue>());
            break;
        case PRINTFTK: {
            TokenP p = *tk;
            elements.push_back(*tk++);
            if ((*tk)->type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR_EXPECTED_GOT(LPARENT, tk);
            }
            int fmt_char_cnt;
            FormatStringP fmt_str = cast<FormatString>(*tk);
            if ((*tk)->type == STRCON) {
                fmt_char_cnt = fmt_str->fmt_char_cnt;
                elements.push_back(*tk++);
            } else {
                ERROR_EXPECTED_GOT(STRCON, tk);
            }
            int cnt;
            for (cnt = 0; has_type(tk, COMMA); cnt++) {
                elements.push_back(*tk++);
                parse_expr<NormalExpr>(tk);
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
            instructions.push_back(make_shared<PrintF>(fmt_str));
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
ObjectP Parser::parse_expr(TokenIter &tk, bool generate_inst) {
    // Exp -> AddExp
    // ConstExp -> AddExp
    ObjectP result = parse_addsub_expr(tk, generate_inst, NOTHING);
    elements.push_back(make_shared<T>());
    return result;
}

ObjectP Parser::parse_cond_expr(TokenIter &tk) {
    // Cond -> LOrExp
    ObjectP result = parse_logical_or_expr(tk, true, NOTHING);
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

ObjectP Parser::parse_lvalue(TokenIter &tk, bool generate_inst, bool is_assigned = false) {
    // LVal -> Ident { '[' Exp ']' }
    ObjectP result;
    if ((*tk)->type == IDENFR) {
        result = check_ident_valid_use(tk, false, is_assigned);
        if (generate_inst && (!is_assigned || has_type(tk, LBRACK))) {
            instructions.push_back(make_shared<LoadObject>(result));
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    vector<long long> indexes;
    int cnt;
    for (cnt = 0; has_type(tk, LBRACK); cnt++) {
        elements.push_back(*tk++);
        ObjectP index = parse_expr<NormalExpr>(tk, generate_inst);
        if (index->type == INT) {
            indexes.push_back(cast<IntObject>(index)->value);
        }
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
            if (generate_inst) {
                instructions.push_back(make_shared<SubscriptArray>());
            }
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    ArrayObjectP array = cast<ArrayObject>(result);
    if (array != nullptr) {
        array->dereference_cnt = cnt > 0 ? cnt : 0;
        if (array->is_const && array->dims.size() == indexes.size()) {
            long long index = indexes.back();
            for (int i = 1; i < array->dims.size(); i++) {
                index += indexes[i - 1] * array->dims[i];
            }
            result = (*array->data)[index];
        }
    }
    elements.push_back(make_shared<LValue>());
    return result;
}

ObjectP Parser::parse_primary_expr(TokenIter &tk, bool generate_inst) {
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
            result = parse_lvalue(tk, generate_inst);  // pre-fetch
            break;
        case INTCON:
            result = parse_number(tk);  // pre-fetch
            if (generate_inst) {
                instructions.push_back(make_shared<LoadObject>(result));
            }
            break;
        default:
            ERROR_EXPECTED_GOT(LPARENT or IDENFR or INTCON, tk);
    }
    elements.push_back(make_shared<PrimaryExpr>());
    return result;
}

IntObjectP Parser::parse_number(TokenIter &tk) {
    // Number -> IntLiteral
    IntObjectP result = cast<IntObject>(*tk);
    elements.push_back(*tk++);
    elements.push_back(make_shared<Number>());
    return result;
}

ObjectP Parser::parse_unary_expr(TokenIter &tk, bool generate_inst, BinaryOpCode dummy = NOTHING) {
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
                            ERROR_LIMITED_SUPPORT_WITH_LINE(func->ident_info->line, VOID or INT function return type);
                    }
                } else {
                    result = make_shared<Object>();
                }
            } else {  // is not function call
                result = parse_primary_expr(tk, generate_inst);  // pre-fetch LVal
            }
            break;
        case PLUS:
        case MINU:
        case NOT: {
            UnaryOpCode opcode = parse_unary_op(tk);  // pre-fetch
            result = parse_unary_expr(tk, generate_inst);
            if (generate_inst) {
                instructions.push_back(make_shared<UnaryOperation>(opcode));
            } else if (result->type == INT) {  // TODO: not generate_inst
                result = make_shared<IntObject>(util::unary_operation(opcode, cast<IntObject>(result)->value));
            }
            break;
        }
        default:
            result = parse_primary_expr(tk, generate_inst);  // pre-fetch
    }
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    elements.push_back(make_shared<UnaryExpr>());
    return result;
}

UnaryOpCode Parser::parse_unary_op(TokenIter &tk) {
    TokenCode type = (*tk)->type;
    elements.push_back(*tk++);
    elements.push_back(make_shared<UnaryOp>());
    switch (type) {
        case PLUS: return UNARY_POSITIVE;
        case MINU: return UNARY_NEGATIVE;
        case NOT: return UNARY_NOT;
        default: ERROR_EXPECTED_GOT(PLUS or MINU or NOT, tk);
    }
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
                                if (array_param->dims.size() !=
                                    array->dims.size() - array->dereference_cnt) {
                                    error(PARAM_TYPE_MISMATCH, func_line);
                                }
                                break;
                            }
                            default:
                                ERROR_LIMITED_SUPPORT_WITH_LINE(func_line, INT or INT_ARRAY function arguments);
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
ObjectP Parser::_parse_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op,
                            ObjectP (Parser::*parse_first)(TokenIter &, bool, BinaryOpCode),
                            BinaryOpCode (*predicate)(TokenCode)) {
    ObjectP result, tmp;
    BinaryOpCode next_op = NOTHING;
    while (tk < tokens.end()) {
        tmp = (this->*parse_first)(tk, generate_inst, last_op);  // TODO: check
        if (result == nullptr || result->type == INT && tmp->type != INT) {
            result = tmp;
        } else if (!generate_inst && result->type == INT && tmp->type == INT) {
            result = make_shared<IntObject>(util::binary_operation(last_op, cast<IntObject>(result)->value, cast<IntObject>(tmp)->value));
        }
        if (generate_inst && next_op != NOTHING && next_op == last_op) {
            instructions.push_back(make_shared<BinaryOperation>(last_op));
        }
        next_op = predicate((*tk)->type);
        if (next_op != NOTHING) {
            last_op = next_op;
            elements.push_back(make_shared<T>());
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(make_shared<T>());
    return result;
}

ObjectP Parser::parse_muldiv_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op) {
    // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    ObjectP result = _parse_expr<MulDivExpr>(tk, generate_inst, last_op, &Parser::parse_unary_expr, determine_muldiv);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_addsub_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op) {
    // AddExp -> MulExp | AddExp ('+' | '-') MulExp
    // AddExp -> MulExp { ('+' | '-') MulExp }
    ObjectP result = _parse_expr<AddSubExpr>(tk, generate_inst, last_op, &Parser::parse_muldiv_expr, determine_addsub);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_rel_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op) {
    // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    ObjectP result = _parse_expr<RelationalExpr>(tk, generate_inst, last_op, &Parser::parse_addsub_expr, determine_relation);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_eq_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op) {
    // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    // EqExp -> RelExp { ('==' | '!=') RelExp }
    ObjectP result = _parse_expr<EqualExpr>(tk, generate_inst, last_op, &Parser::parse_rel_expr, determine_equality);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_logical_and_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op) {
    // LAndExp -> EqExp | LAndExp '&&' EqExp
    // LAndExp -> EqExp { '&&' EqExp }
    ObjectP result = _parse_expr<LogicalAndExpr>(tk, generate_inst, last_op, &Parser::parse_eq_expr, determine_and);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_logical_or_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op) {
    // LOrExp -> LAndExp | LOrExp '||' LAndExp
    // LOrExp -> LAndExp { '||' LAndExp }
    ObjectP result = _parse_expr<LogicalOrExpr>(tk, generate_inst, last_op, &Parser::parse_logical_and_expr, determine_or);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}
