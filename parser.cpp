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
    sym_table.emplace_back();
    while (starts_with_decl(tk)) {
        parse_decl(tk, 0);
    }
    instructions.push_back(make_shared<CallFunction>());
    entry_inst = instructions.back();
    instructions.push_back(make_shared<Exit>());
    while (starts_with_func_def(tk)) {
        parse_func_def(tk);
    }
    if ((*tk)->token_type == INTTK) {
        parse_main_func_def(tk);
    }
    elements.push_back(make_shared<CompUnit>());
}

void Parser::parse_decl(TokenIter &tk, int nest_level) {
    // Decl -> ConstDecl | VarDecl
    switch ((*tk)->token_type) {
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
    if ((*tk)->token_type == CONSTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(CONSTTK, tk);
    }
    if ((*tk)->token_type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
    }
    while (tk < tokens.end()) {
        parse_const_def(tk, nest_level);
        if ((*tk)->token_type == COMMA) {
            elements.push_back(*tk++);
        } else if ((*tk)->token_type == SEMICN) {
            elements.push_back(*tk++);
            break;
        } else {
            error(MISSING_SEMICN, (*(tk - 1))->line);
            break;
        }
    }
    elements.push_back(make_shared<ConstDecl>());
}

ObjectP Parser::check_ident_valid_decl(TokenIter &tk, TypeCode type,
                                       bool is_global = false, bool is_const = false, bool is_func = false) {
    ObjectP result;
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
    IdentP current = cast<Identifier>(*tk);
    auto &last_table = sym_table.back();
    if (last_table.find(current->name) == last_table.end()) {
        result->is_const = is_const;
        result->is_global = is_global;
        result->ident_info = current;
        last_table[current->name] = result;
    } else {
        error(IDENT_REDEFINED, current->line);
    }
    elements.push_back(*tk++);
    return result;
}

void Parser::parse_const_def(TokenIter &tk, int nest_level) {
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    ObjectP current;
    ArrayObjectP array;
    bool is_array;
    if ((*tk)->token_type == IDENFR) {
        is_array = has_type(tk + 1, LBRACK);
        if (is_array) {
            current = check_ident_valid_decl(tk, INT_ARRAY, nest_level == 0, true);
            array = cast<ArrayObject>(current);
        } else {
            current = check_ident_valid_decl(tk, INT, nest_level == 0, true);
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    while (has_type(tk, LBRACK)) {
        elements.push_back(*tk++);
        array->dims.push_back(cast<IntObject>(parse_expr<ConstExpr>(tk, NO_EMIT_IN_CONST_DEF))->value);
        if ((*tk)->token_type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
    }
    if ((*tk)->token_type == ASSIGN) {
        elements.push_back(*tk++);
        ObjectP init_val = parse_init_val<ConstExpr, ConstInitVal>(tk, NO_EMIT_IN_CONST_DEF);
        if (is_array) {
            array->data = cast<ArrayObject>(init_val)->data;
            // array is flattened for convenience,
            // and need to be considered well when passed to functions
        } else {
            cast<IntObject>(current)->value = cast<IntObject>(init_val)->value;
        }
    } else {
        ERROR_EXPECTED_GOT(ASSIGN, tk);
    }
    elements.push_back(make_shared<ConstDef>());
}

void Parser::parse_var_decl(TokenIter &tk, int nest_level) {
    // VarDecl -> BType VarDef { ',' VarDef } ';'
    if ((*tk)->token_type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
    }
    while (tk < tokens.end()) {
        parse_var_def(tk, nest_level);
        if ((*tk)->token_type == COMMA) {
            elements.push_back(*tk++);
        } else if ((*tk)->token_type == SEMICN) {
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
    if ((*tk)->token_type == IDENFR) {
        is_array = has_type(tk + 1, LBRACK);
        if (is_array) {
            current = check_ident_valid_decl(tk, INT_ARRAY, nest_level == 0);
            array = cast<ArrayObject>(current);
        } else {
            current = check_ident_valid_decl(tk, INT, nest_level == 0);
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    while (has_type(tk, LBRACK)) {
        elements.push_back(*tk++);
        array->dims.push_back(cast<IntObject>(parse_expr<ConstExpr>(tk, EMIT_IN_VAR_DEF))->value);
        if ((*tk)->token_type == RBRACK) {
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
        instructions.push_back(make_shared<StoreName>(array));
    }
    if ((*tk)->token_type == ASSIGN) {
        elements.push_back(*tk++);
        parse_init_val<NormalExpr, InitVal>(tk, EMIT_IN_NORM_STMT);
        if (is_array) {
            instructions.push_back(make_shared<InitArray>(array));
            // array is flattened for convenience,
            // and need to be considered well when passed to functions
        }
        instructions.push_back(make_shared<StoreName>(current));
    }
    elements.push_back(make_shared<VarDef>());
}

template<typename ExprT, typename ElementT>
ObjectP Parser::parse_init_val(TokenIter &tk, EmitMode emit_mode) {
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    ObjectP result;
    if ((*tk)->token_type == LBRACE) {
        elements.push_back(*tk++);
        if ((*tk)->token_type != RBRACE) {
            ArrayObjectP array = make_shared<ArrayObject>(true);
            while (tk < tokens.end()) {
                ObjectP o = parse_init_val<ExprT, ElementT>(tk, emit_mode);
                if (emit_mode == NO_EMIT_IN_CONST_DEF && o->type == INT_ARRAY) {
                    ArrayObjectP tmp = cast<ArrayObject>(o);
                    array->data->insert(array->data->end(), tmp->data->begin(), tmp->data->end());
                } else {
                    array->data->push_back(o);
                }
                if ((*tk)->token_type == COMMA) {
                    elements.push_back(*tk++);
                } else {
                    break;
                }
            }
            result = array;
        }
        if ((*tk)->token_type == RBRACE) {
            elements.push_back(*tk++);
        } else {
            ERROR_EXPECTED_GOT(COMMA or RBRACE, tk);
        }
    } else {
        result = parse_expr<ExprT>(tk, emit_mode)->copy();  // must be copied
    }
    elements.push_back(make_shared<ElementT>());
    return result;
}

void Parser::parse_func_def(TokenIter &tk) {
    // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    current_func_return_type = parse_func_type(tk);
    FuncObjectP current;
    if ((*tk)->token_type == IDENFR) {
        current = cast<FuncObject>(check_ident_valid_decl(tk, current_func_return_type, true, false, true));
        current->code_offset = (long long) instructions.size();
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    if ((*tk)->token_type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(LPARENT, tk);
    }
    sym_table.emplace_back();
    if ((*tk)->token_type == INTTK) {  // pre-fetch
        parse_func_formal_params(tk, current);
    }
    if ((*tk)->token_type == RPARENT) {
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
    if ((*tk)->token_type == INTTK) {
        current_func_return_type = INT;
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
    }
    if ((*tk)->token_type == MAINTK) {
        FuncObjectP main = cast<FuncObject>(*tk);
        main->code_offset = (long long) instructions.size();
        cast<CallFunction>(entry_inst)->set_func(main);
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(MAINTK, tk);
    }
    if ((*tk)->token_type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(LPARENT, tk);
    }
    if ((*tk)->token_type == RPARENT) {
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
    if ((*tk)->token_type == INTTK) {
        result = INT;
    } else if ((*tk)->token_type == VOIDTK) {
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
        if ((*tk)->token_type == COMMA) {
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(make_shared<FuncFormalParams>());
}

void Parser::parse_func_formal_param(TokenIter &tk, FuncObjectP &func) {
    // FuncFParam -> BType Ident ['[' ']' { '[' ConstExp ']' }]
    if ((*tk)->token_type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(INTTK, tk);
    }
    ObjectP current;
    ArrayObjectP array;
    if ((*tk)->token_type == IDENFR) {
        if (has_type(tk + 1, LBRACK)) {
            current = check_ident_valid_decl(tk, INT_ARRAY);
            array = cast<ArrayObject>(current);
        } else {
            current = check_ident_valid_decl(tk, INT);
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    if ((*tk)->token_type == LBRACK) {
        elements.push_back(*tk++);
        if ((*tk)->token_type == RBRACK) {
            elements.push_back(*tk++);
            array->dims.push_back(0);  // dummy dimension for int a[]
        } else {
            error(MISSING_RBRACK, (*(tk - 1))->line);
        }
        while (has_type(tk, LBRACK)) {
            elements.push_back(*tk++);
            array->dims.push_back(cast<IntObject>(parse_expr<ConstExpr>(tk, NO_EMIT_IN_FPARAMS))->value);
            if ((*tk)->token_type == RBRACK) {
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
    if ((*tk)->token_type == LBRACE) {
        elements.push_back(*tk++);
    } else {
        ERROR_EXPECTED_GOT(LBRACE, tk);
    }
    if (!from_func_def) {
        sym_table.emplace_back();
    }
    while (tk < tokens.end() && (starts_with_decl(tk) || starts_with_stmt(tk))) {  // pre-fetch
        parse_block_item(tk, nest_level);
    }
    if ((*tk)->token_type == RBRACE) {
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

void Parser::parse_stmt(TokenIter &tk, int nest_level) {  // TODO: 返回值报语句结束的指令
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
    switch ((*tk)->token_type) {
        case IDENFR: {
            bool no_assign = true, is_indexed = has_type(tk + 1, LBRACK);
            for (auto p = tk + 1; p < tokens.end() && (*p)->token_type != SEMICN; ++p) {
                if ((*p)->token_type == ASSIGN) {
                    no_assign = false;
                    ObjectP lvalue = parse_lvalue(tk, EMIT_IN_NORM_STMT, true);  // pre-fetch
                    if ((*tk)->token_type == ASSIGN) {
                        elements.push_back(*tk++);
                    } else {
                        // error(MISSING_SEMICN, (*(tk - 1))->line);
                        break;
                    }
                    if ((*tk)->token_type == GETINTTK) {
                        elements.push_back(*tk++);
                        if ((*tk)->token_type == LPARENT) {
                            elements.push_back(*tk++);
                        } else {
                            ERROR_EXPECTED_GOT(LPARENT, tk);
                        }
                        if ((*tk)->token_type == RPARENT) {
                            elements.push_back(*tk++);
                            instructions.push_back(make_shared<GetInt>());
                        } else {
                            error(MISSING_RPAREN, (*(tk - 1))->line);
                        }
                    } else {
                        parse_expr<NormalExpr>(tk, EMIT_IN_NORM_STMT);  // pre-fetch
                    }
                    if (is_indexed) {
                        instructions.push_back(make_shared<StoreSubscript>());
                    } else {
                        instructions.push_back(make_shared<StoreName>(lvalue));
                    }
                    break;
                }
            }
            if (no_assign) {
                parse_expr<NormalExpr>(tk, EMIT_IN_NORM_STMT);  // pre-fetch
                instructions.push_back(make_shared<PopTop>());
            }
            if ((*tk)->token_type == SEMICN) {
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
        case IFTK: {
            elements.push_back(*tk++);
            if ((*tk)->token_type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR_EXPECTED_GOT(LPARENT, tk);
            }
            vector<JumpInstructionP> eval_jump_instructions;
            vector<JumpInstructionP> control_jump_instructions;
            parse_cond_expr(tk, EMIT_IN_COND_STMT, eval_jump_instructions, &control_jump_instructions);
            if ((*tk)->token_type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            relocate_jump_instructions(eval_jump_instructions);
            parse_stmt(tk, nest_level + 1);
            if ((*tk)->token_type == ELSETK) {
                elements.push_back(*tk++);
                auto ja = make_shared<JumpAbsolute>();
                instructions.push_back(ja);
                relocate_jump_instructions(control_jump_instructions);
                parse_stmt(tk, nest_level + 1);
                ja->set_offset((long long) instructions.size());
            } else {
                relocate_jump_instructions(control_jump_instructions);
            }
            break;
        }
        case WHILETK: {
            loop_info.emplace_back((long long) instructions.size());
            elements.push_back(*tk++);
            if ((*tk)->token_type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR_EXPECTED_GOT(LPARENT, tk);
            }
            auto offset = (long long) instructions.size();
            vector<JumpInstructionP> eval_jump_instructions;
            vector<JumpInstructionP> control_jump_instructions;
            parse_cond_expr(tk, EMIT_IN_COND_STMT, eval_jump_instructions, &control_jump_instructions);
            if ((*tk)->token_type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            relocate_jump_instructions(eval_jump_instructions);
            parse_stmt(tk, nest_level + 1);
            instructions.push_back(make_shared<JumpAbsolute>(offset));
            relocate_jump_instructions(control_jump_instructions);
            relocate_jump_instructions(loop_info.back().break_instructions);
            loop_info.pop_back();
            break;
        }
        case BREAKTK:
            if (loop_info.empty()) {
                error(BREAK_CONTINUE_NOT_IN_LOOP, (*tk)->line);
            } else {
                auto ja = make_shared<JumpAbsolute>();
                instructions.push_back(ja);
                loop_info.back().break_instructions.push_back(ja);
            }
            elements.push_back(*tk++);
            if ((*tk)->token_type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            break;
        case CONTINUETK:
            if (loop_info.empty()) {
                error(BREAK_CONTINUE_NOT_IN_LOOP, (*tk)->line);
            } else {
                instructions.push_back(make_shared<JumpAbsolute>(loop_info.back().start));
            }
            elements.push_back(*tk++);
            if ((*tk)->token_type == SEMICN) {
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
                parse_expr<NormalExpr>(tk, EMIT_IN_NORM_STMT);
            }
            if ((*tk)->token_type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            if (nest_level == 1 && (*tk)->token_type == RBRACE) {
                has_return_at_end = true;
            }
            instructions.push_back(make_shared<ReturnValue>());
            break;
        case PRINTFTK: {
            TokenP p = *tk;
            elements.push_back(*tk++);
            if ((*tk)->token_type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR_EXPECTED_GOT(LPARENT, tk);
            }
            int fmt_char_cnt;
            FormatStringP fmt_str = cast<FormatString>(*tk);
            if ((*tk)->token_type == STRCON) {
                fmt_char_cnt = fmt_str->fmt_char_cnt;
                elements.push_back(*tk++);
            } else {
                ERROR_EXPECTED_GOT(STRCON, tk);
            }
            int cnt;
            for (cnt = 0; has_type(tk, COMMA); cnt++) {
                elements.push_back(*tk++);
                parse_expr<NormalExpr>(tk, EMIT_IN_NORM_STMT);
            }
            if (cnt != fmt_char_cnt) {
                error(FORMAT_STRING_ARGUMENT_MISMATCH, p->line);
            }
            if ((*tk)->token_type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            if ((*tk)->token_type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
            instructions.push_back(make_shared<PrintF>(fmt_str));
            break;
        }
        default:
            parse_expr<NormalExpr>(tk, EMIT_IN_NORM_STMT);  // pre-fetch
            instructions.push_back(make_shared<PopTop>());
            if ((*tk)->token_type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_SEMICN, (*(tk - 1))->line);
            }
    }
    elements.push_back(make_shared<Statement>());
}

template<typename T>
ObjectP Parser::parse_expr(TokenIter &tk, EmitMode emit_mode) {
    // Exp -> AddExp
    // ConstExp -> AddExp
    ObjectP result = parse_addsub_expr(tk, emit_mode, NOTHING);
    elements.push_back(make_shared<T>());
    return result;
}

ObjectP Parser::parse_cond_expr(TokenIter &tk, EmitMode emit_mode, vector<JumpInstructionP> &eval_jump_instructions,
                                vector<JumpInstructionP> *control_jump_instructions) {
    // Cond -> LOrExp
    // the pointer argument is preserved for condition expression outside control flow
    ObjectP result = parse_logical_or_expr(tk, emit_mode, NOTHING, eval_jump_instructions,
                                           control_jump_instructions);
    elements.push_back(make_shared<ConditionExpr>());
    return result;
}

ObjectP Parser::check_ident_valid_use(TokenIter &tk, bool is_called, bool is_assigned = false) {
    IdentP current = dynamic_pointer_cast<Identifier>(*tk);
    elements.push_back(*tk++);
    ObjectP result;
    for (auto p = sym_table.rbegin(); p != sym_table.rend(); ++p) {
        if (p->find(current->name) != p->end()) {
            result = (*p)[current->name];
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

ObjectP Parser::parse_lvalue(TokenIter &tk, EmitMode emit_mode, bool is_assigned = false) {
    // LVal -> Ident { '[' Exp ']' }
    ObjectP result;
    if ((*tk)->token_type == IDENFR) {
        result = check_ident_valid_use(tk, false, is_assigned);
        if ((emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) &&
            (!is_assigned || has_type(tk, LBRACK))) {
            if (result->is_const) {
                instructions.push_back(make_shared<LoadFast>(result));
            } else {
                instructions.push_back(make_shared<LoadName>(result));
            }
        }
    } else {
        ERROR_EXPECTED_GOT(IDENFR, tk);
    }
    vector<long long> indexes;
    int cnt;
    for (cnt = 0; has_type(tk, LBRACK); cnt++) {
        elements.push_back(*tk++);
        ObjectP index = parse_expr<NormalExpr>(tk, emit_mode);
        if (index->type == INT) {
            indexes.push_back(cast<IntObject>(index)->value);
        }
        if ((*tk)->token_type == RBRACK) {
            elements.push_back(*tk++);
            if (emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) {
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

ObjectP Parser::parse_primary_expr(TokenIter &tk, EmitMode emit_mode) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    ObjectP result;
    switch ((*tk)->token_type) {
        case LPARENT:
            elements.push_back(*tk++);
            result = parse_expr<NormalExpr>(tk, emit_mode);
            if ((*tk)->token_type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                error(MISSING_RPAREN, (*(tk - 1))->line);
            }
            break;
        case IDENFR:
            result = parse_lvalue(tk, emit_mode);  // pre-fetch
            break;
        case INTCON:
            result = parse_number(tk);  // pre-fetch
            if (emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) {
                instructions.push_back(make_shared<LoadFast>(result));
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

ObjectP Parser::parse_unary_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode = NOTHING) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    ObjectP result;
    switch ((*tk)->token_type) {
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
                if ((*tk)->token_type == RPARENT) {
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
                result = parse_primary_expr(tk, emit_mode);  // pre-fetch LVal
            }
            break;
        case PLUS:
        case MINU:
        case NOT: {
            UnaryOpCode opcode = parse_unary_op(tk);  // pre-fetch
            result = parse_unary_expr(tk, emit_mode);
            if (emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) {
                instructions.push_back(make_shared<UnaryOperation>(opcode));
            } else if (result->type == INT) {
                result = make_shared<IntObject>(util::unary_operation(opcode, cast<IntObject>(result)->value));
            }
            break;
        }
        default:
            result = parse_primary_expr(tk, emit_mode);  // pre-fetch
    }
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    elements.push_back(make_shared<UnaryExpr>());
    return result;
}

UnaryOpCode Parser::parse_unary_op(TokenIter &tk) {
    TokenCode type = (*tk)->token_type;
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
        ObjectP instance = parse_expr<NormalExpr>(tk, EMIT_IN_NORM_STMT);
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
        if ((*tk)->token_type == COMMA) {
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
ObjectP Parser::_parse_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op,
                            ObjectP (Parser::*parse_first)(TokenIter &, EmitMode, BinaryOpCode),
                            BinaryOpCode (*predicate)(TokenCode)) {
    ObjectP result, tmp;
    BinaryOpCode next_op = NOTHING;
    while (tk < tokens.end()) {
        tmp = (this->*parse_first)(tk, emit_mode, last_op);  // TODO: check
        if (result == nullptr || result->type == INT && tmp->type != INT) {
            result = tmp;
        } else if ((emit_mode == NO_EMIT_IN_CONST_DEF || emit_mode == NO_EMIT_IN_FPARAMS || emit_mode == EMIT_IN_VAR_DEF) &&
                   result->type == INT && tmp->type == INT) {
            result = make_shared<IntObject>(
                    util::binary_operation(last_op, cast<IntObject>(result)->value, cast<IntObject>(tmp)->value));
        }
        if ((emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) &&
            next_op != NOTHING && next_op == last_op) {
            instructions.push_back(make_shared<BinaryOperation>(last_op));
        }
        next_op = predicate((*tk)->token_type);
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

ObjectP Parser::parse_muldiv_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op) {
    // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    ObjectP result = _parse_expr<MulDivExpr>(tk, emit_mode, last_op, &Parser::parse_unary_expr, determine_muldiv);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_addsub_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op) {
    // AddExp -> MulExp | AddExp ('+' | '-') MulExp
    // AddExp -> MulExp { ('+' | '-') MulExp }
    ObjectP result = _parse_expr<AddSubExpr>(tk, emit_mode, last_op, &Parser::parse_muldiv_expr, determine_addsub);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_rel_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op) {
    // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    ObjectP result = _parse_expr<RelationalExpr>(tk, emit_mode, last_op, &Parser::parse_addsub_expr,
                                                 determine_relation);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_eq_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op) {
    // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    // EqExp -> RelExp { ('==' | '!=') RelExp }
    ObjectP result = _parse_expr<EqualExpr>(tk, emit_mode, last_op, &Parser::parse_rel_expr, determine_equality);
//    if (last_op != NOTHING) {
//        instructions.push_back(make_shared<BinaryOperation>(last_op));
//    }
    return result;
}

ObjectP Parser::parse_logical_and_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op,
                                       vector<JumpInstructionP> &eval_jump_instructions) {
    // LAndExp -> EqExp | LAndExp '&&' EqExp
    // LAndExp -> EqExp { '&&' EqExp }
    ObjectP result, tmp;
    BinaryOpCode next_op = NOTHING;
    while (tk < tokens.end()) {
        tmp = parse_eq_expr(tk, emit_mode, last_op);
        if (result == nullptr || result->type == INT && tmp->type != INT) {
            result = tmp;
        } /*else if ((emit_mode == NO_EMIT_IN_CONST_DEF || emit_mode == NO_EMIT_IN_FPARAMS || emit_mode == EMIT_IN_VAR_DEF) &&
                   result->type == INT && tmp->type == INT) {
            result = make_shared<IntObject>(
                    util::binary_operation(last_op, cast<IntObject>(result)->value, cast<IntObject>(tmp)->value));
        }
        if ((emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) &&
            next_op != NOTHING && next_op == last_op) {
            instructions.push_back(make_shared<BinaryOperation>(last_op));
        }*/
        next_op = determine_and((*tk)->token_type);
        if (next_op != NOTHING) {
            last_op = next_op;
            auto bgz = make_shared<PopJumpIfFalse>();
            instructions.push_back(bgz);
            eval_jump_instructions.push_back(bgz);
            elements.push_back(make_shared<LogicalAndExpr>());
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(make_shared<LogicalAndExpr>());
    return result;
}

ObjectP Parser::parse_logical_or_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op,
                                      vector<JumpInstructionP> &eval_jump_instructions,
                                      vector<JumpInstructionP> *control_jump_instructions) {
    // LOrExp -> LAndExp | LOrExp '||' LAndExp
    // LOrExp -> LAndExp { '||' LAndExp }
    ObjectP result, tmp;
    BinaryOpCode next_op = NOTHING;
    while (tk < tokens.end()) {
        vector<JumpInstructionP> and_eval_jump_instructions;
        tmp = parse_logical_and_expr(tk, emit_mode, last_op, and_eval_jump_instructions);
        if (result == nullptr || result->type == INT && tmp->type != INT) {
            result = tmp;
        } /*else if ((emit_mode == NO_EMIT_IN_CONST_DEF || emit_mode == NO_EMIT_IN_FPARAMS || emit_mode == EMIT_IN_VAR_DEF) &&
                   result->type == INT && tmp->type == INT) {
            result = make_shared<IntObject>(
                    util::binary_operation(last_op, cast<IntObject>(result)->value, cast<IntObject>(tmp)->value));
        }
        if ((emit_mode == EMIT_IN_VAR_DEF || emit_mode == EMIT_IN_NORM_STMT || emit_mode == EMIT_IN_COND_STMT) &&
            next_op != NOTHING && next_op == last_op) {
            instructions.push_back(make_shared<BinaryOperation>(last_op));
        }*/
        next_op = determine_or((*tk)->token_type);
        if (next_op != NOTHING) {
            last_op = next_op;
            auto bgz = make_shared<PopJumpIfTrue>();
            instructions.push_back(bgz);
            eval_jump_instructions.push_back(bgz);
            relocate_jump_instructions(and_eval_jump_instructions);
            elements.push_back(make_shared<LogicalOrExpr>());
            elements.push_back(*tk++);
        } else {
            if (control_jump_instructions != nullptr) {
                auto bez = make_shared<PopJumpIfFalse>();
                instructions.push_back(bez);
                control_jump_instructions->push_back(bez);
                control_jump_instructions->insert(control_jump_instructions->end(),
                                                  and_eval_jump_instructions.begin(), and_eval_jump_instructions.end());
            }
            break;
        }
    }
    elements.push_back(make_shared<LogicalOrExpr>());
    return result;
}
