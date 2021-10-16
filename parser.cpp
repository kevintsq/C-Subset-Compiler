//
// Created by Kevin Tan on 2021/9/24.
//

#include "parser.h"

#define ERROR(expected, got) do { std::cout << "In " << __func__ << " line " << __LINE__ << " source code line " << (*got)->line << ", expected "#expected", got "; (*got)->print(); exit(-1); } while (0)

Parser::Parser(std::vector<TokenP> &tokens) : tokens(tokens) {
    auto tk = tokens.begin();
    parse_comp_unit(tk);
}

void Parser::parse_comp_unit(TokenIter &tk) {
    // CompUnit -> {Decl} {FuncDef} MainFuncDef
    while (starts_with_decl(tk)) {
        parse_decl(tk);
    }
    while (starts_with_func_def(tk)) {
        parse_func_def(tk);
    }
    parse_main_func_def(tk);
    elements.push_back(std::make_shared<CompUnit>());
}

void Parser::parse_decl(TokenIter &tk) {
    // Decl -> ConstDecl | VarDecl
    switch ((*tk)->type) {
        case CONSTTK:
            parse_const_decl(tk);
            break;
        case INTTK:
            parse_var_decl(tk);
            break;
        default:
            ERROR(CONSTTK or INTTK, tk);
    }
}

void Parser::parse_const_decl(TokenIter &tk) {
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
        parse_const_def(tk);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else if ((*tk)->type == SEMICN) {
            elements.push_back(*tk++);
            break;
        } else {
            ERROR(COMMA or SEMICN, tk);
        }
    }
    elements.push_back(std::make_shared<ConstDecl>());
}

void Parser::parse_const_def(TokenIter &tk) {
    // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    if ((*tk)->type == IDENFR) {
        elements.push_back(*tk++);
    } else {
        ERROR(IDENFR, tk);
    }
    while ((*tk)->type == LBRACK && tk < tokens.end()) {
        elements.push_back(*tk++);
        parse_const_expr(tk);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            ERROR(RBRACK, tk);
        }
    }
    if ((*tk)->type == ASSIGN) {
        elements.push_back(*tk++);
    } else {
        ERROR(ASSIGN, tk);
    }
    parse_const_init_val(tk);
    elements.push_back(std::make_shared<ConstDef>());
}

void Parser::parse_const_init_val(TokenIter &tk) {
    // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    if ((*tk)->type == LBRACE) {
        elements.push_back(*tk++);
        while (tk < tokens.end()) {
            parse_const_init_val(tk);
            if ((*tk)->type == COMMA) {
                elements.push_back(*tk++);
            } else {
                break;
            }
        }
        if ((*tk)->type == RBRACE) {
            elements.push_back(*tk++);
        } else {
            ERROR(COMMA or RBRACE, tk);
        }
    } else {
        parse_const_expr(tk);
    }
    elements.push_back(std::make_shared<ConstInitVal>());
}

void Parser::parse_var_decl(TokenIter &tk) {
    // VarDecl -> BType VarDef { ',' VarDef } ';'
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    while (tk < tokens.end()) {
        parse_var_def(tk);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else if ((*tk)->type == SEMICN) {
            elements.push_back(*tk++);
            break;
        } else {
            ERROR(COMMA or SEMICN, tk);
        }
    }
    elements.push_back(std::make_shared<VarDecl>());
}

void Parser::parse_var_def(TokenIter &tk) {
    // VarDef -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal
    if ((*tk)->type == IDENFR) {
        elements.push_back(*tk++);
    } else {
        ERROR(IDENFR, tk);
    }
    while ((*tk)->type == LBRACK && tk < tokens.end()) {
        elements.push_back(*tk++);
        parse_const_expr(tk);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            ERROR(RBRACK, tk);
        }
    }
    if ((*tk)->type == ASSIGN) {
        elements.push_back(*tk++);
        parse_init_val(tk);
    }
    elements.push_back(std::make_shared<VarDef>());
}

void Parser::parse_init_val(TokenIter &tk) {
    // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    if ((*tk)->type == LBRACE) {
        elements.push_back(*tk++);
        while (tk < tokens.end()) {
            parse_init_val(tk);
            if ((*tk)->type == COMMA) {
                elements.push_back(*tk++);
            } else {
                break;
            }
        }
        if ((*tk)->type == RBRACE) {
            elements.push_back(*tk++);
        } else {
            ERROR(COMMA or RBRACE, tk);
        }
    } else {
        parse_expr(tk);
    }
    elements.push_back(std::make_shared<InitVar>());
}

void Parser::parse_func_def(TokenIter &tk) {
    // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    parse_func_type(tk);
    if ((*tk)->type == IDENFR) {
        elements.push_back(*tk++);
    } else {
        ERROR(IDENFR, tk);
    }
    if ((*tk)->type == LPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR(LPARENT, tk);
    }
    if ((*tk)->type == INTTK) {  // pre-fetch
        parse_func_formal_params(tk);
    }
    if ((*tk)->type == RPARENT) {
        elements.push_back(*tk++);
    } else {
        ERROR(RPARENT, tk);
    }
    parse_block(tk);
    elements.push_back(std::make_shared<FuncDef>());
}

void Parser::parse_main_func_def(TokenIter &tk) {
    // MainFuncDef -> 'int' 'main' '(' ')' Block
    if ((*tk)->type == INTTK) {
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
        ERROR(RPARENT, tk);
    }
    parse_block(tk);
    elements.push_back(std::make_shared<MainFuncDef>());
}

inline void Parser::parse_func_type(TokenIter &tk) {
    // FuncType -> 'void' | 'int'
    switch ((*tk)->type) {
        case INTTK:
        case VOIDTK:
            elements.push_back(*tk++);
            break;
        default:
            ERROR(INTTK or VOIDTK, tk);
    }
    elements.push_back(std::make_shared<FuncType>());
}

void Parser::parse_func_formal_params(TokenIter &tk) {
    // FuncFParams -> FuncFParam { ',' FuncFParam }
    while (tk < tokens.end()) {
        parse_func_formal_param(tk);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(std::make_shared<FuncFormalParams>());
}

void Parser::parse_func_formal_param(TokenIter &tk) {
    // FuncFParam -> BType Ident ['[' ']' { '[' ConstExp ']' }]
    if ((*tk)->type == INTTK) {
        elements.push_back(*tk++);
    } else {
        ERROR(INTTK, tk);
    }
    if ((*tk)->type == IDENFR) {
        elements.push_back(*tk++);
    } else {
        ERROR(IDENFR, tk);
    }
    if ((*tk)->type == LBRACK) {
        elements.push_back(*tk++);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            ERROR(RBRACK, tk);
        }
        while ((*tk)->type == LBRACK && tk < tokens.end()) {
            elements.push_back(*tk++);
            parse_const_expr(tk);
            if ((*tk)->type == RBRACK) {
                elements.push_back(*tk++);
            } else {
                ERROR(RBRACK, tk);
            }
        }
    }
    elements.push_back(std::make_shared<FuncFormalParam>());
}

void Parser::parse_block(TokenIter &tk) {
    // Block -> '{' { BlockItem } '}'
    if ((*tk)->type == LBRACE) {
        elements.push_back(*tk++);
    } else {
        ERROR(LBRACE, tk);
    }
    while ((starts_with_decl(tk) || starts_with_stmt(tk)) && tk < tokens.end()) {  // pre-fetch
        parse_block_item(tk);
    }
    if ((*tk)->type == RBRACE) {
        elements.push_back(*tk++);
    } else {
        ERROR(RBRACE, tk);
    }
    elements.push_back(std::make_shared<Block>());
}

void Parser::parse_block_item(TokenIter &tk) {
    // BlockItem -> Decl | Stmt
    if (starts_with_decl(tk)) {
        parse_decl(tk);
    } else if (starts_with_stmt(tk)) {
        parse_stmt(tk);
    } else {
        ERROR(Decl or Stmt, tk);
    }
}

void Parser::parse_stmt(TokenIter &tk) {
    // Stmt -> LVal '=' Exp ';'
    // | [Exp] ';'
    // | Block
    // | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    // | 'while' '(' Cond ')' Stmt
    // | 'break' ';' | 'continue' ';'
    // | 'return' [Exp] ';'
    // | LVal = 'getint' '(' ')' ';'
    // | 'printf' '(' FormatString { "," Exp } ')' ';'
    switch ((*tk)->type) {
        case IDENFR: {
            bool no_assign = true;
            for (auto p = tk + 1; (*p)->type != SEMICN && p < tokens.end(); p++) {
                if ((*p)->type == ASSIGN) {
                    no_assign = false;
                    parse_lvalue(tk);  // pre-fetch
                    if ((*tk)->type == ASSIGN) {
                        elements.push_back(*tk++);
                    } else {
                        ERROR(ASSIGN, tk);
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
                            ERROR(RPARENT, tk);
                        }
                    } else {
                        parse_expr(tk);  // pre-fetch
                    }
                    break;
                }
            }
            if (no_assign) {
                parse_expr(tk);  // pre-fetch
            }
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                ERROR(SEMICN, tk);
            }
            break;
        }
        case LBRACE:
            parse_block(tk);  // pre-fetch
            break;
        case SEMICN:
            elements.push_back(*tk++);
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
                ERROR(RPARENT, tk);
            }
            parse_stmt(tk);
            if ((*tk)->type == ELSETK) {
                elements.push_back(*tk++);
            }
            break;
        case WHILETK:
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
                ERROR(RPARENT, tk);
            }
            parse_stmt(tk);
            break;
        case BREAKTK:
        case CONTINUETK:
            elements.push_back(*tk++);
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                ERROR(SEMICN, tk);
            }
            break;
        case RETURNTK:
            elements.push_back(*tk++);
            if (starts_with_expr(tk)) {
                parse_expr(tk);
            }
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                ERROR(SEMICN, tk);
            }
            break;
        case PRINTFTK:
            elements.push_back(*tk++);
            if ((*tk)->type == LPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR(LPARENT, tk);
            }
            if ((*tk)->type == STRCON) {
                elements.push_back(*tk++);
            } else {
                ERROR(STRCON, tk);
            }
            while ((*tk)->type == COMMA && tk < tokens.end()) {
                elements.push_back(*tk++);
                parse_expr(tk);
            }
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR(RPARENT, tk);
            }
            if ((*tk)->type == SEMICN) {
                elements.push_back(*tk++);
            } else {
                ERROR(SEMICN, tk);
            }
            break;
        default:
            parse_expr(tk);  // pre-fetch
    }
    elements.push_back(std::make_shared<Statement>());
}

void Parser::parse_expr(TokenIter &tk) {
    // Exp -> AddExp
    parse_addsub_expr(tk);
    elements.push_back(std::make_shared<Expression>());
}

void Parser::parse_cond_expr(TokenIter &tk) {
    // Cond -> LOrExp
    parse_logical_or_expr(tk);
    elements.push_back(std::make_shared<ConditionExpr>());
}

void Parser::parse_lvalue(TokenIter &tk) {
    // LVal -> Ident { '[' Exp ']' }
    if ((*tk)->type == IDENFR) {
        elements.push_back(*tk++);
    } else {
        ERROR(IDENFR, tk);
    }
    while ((*tk)->type == LBRACK && tk < tokens.end()) {
        elements.push_back(*tk++);
        parse_expr(tk);
        if ((*tk)->type == RBRACK) {
            elements.push_back(*tk++);
        } else {
            ERROR(RBRACK, tk);
        }
    }
    elements.push_back(std::make_shared<LValue>());
}

void Parser::parse_primary_expr(TokenIter &tk) {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    switch ((*tk)->type) {
        case LPARENT:
            elements.push_back(*tk++);
            parse_expr(tk);
            if ((*tk)->type == RPARENT) {
                elements.push_back(*tk++);
            } else {
                ERROR(RPARENT, tk);
            }
            break;
        case IDENFR:
            parse_lvalue(tk);  // pre-fetch
            break;
        case INTCON:
            parse_number(tk);  // pre-fetch
            break;
        default:
            ERROR(LPARENT or IDENFR or INTCON, tk);
    }
    elements.push_back(std::make_shared<PrimaryExpr>());
}

inline void Parser::parse_number(TokenIter &tk) {
    // Number -> IntConst
    elements.push_back(*tk++);
    elements.push_back(std::make_shared<Number>());
}

void Parser::parse_unary_expr(TokenIter &tk) {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    switch ((*tk)->type) {
        case IDENFR:
            if ((*(tk + 1))->type == LPARENT) {
                elements.push_back(*tk++);
                elements.push_back(*tk++);
                if (starts_with_expr(tk)) {
                    parse_func_real_params(tk);
                }
                if ((*tk)->type == RPARENT) {
                    elements.push_back(*tk++);
                } else {
                    ERROR(RPARENT, tk);
                }
            } else {
                parse_primary_expr(tk);  // pre-fetch LVal
            }
            break;
        case PLUS:
        case MINU:
        case NOT:
            elements.push_back(*tk++);
            parse_unary_expr(tk);
            break;
        default:
            parse_primary_expr(tk);  // pre-fetch
    }
    elements.push_back(std::make_shared<UnaryExpr>());
}

void Parser::parse_func_real_params(TokenIter &tk) {
    // FuncRParams -> Exp { ',' Exp }
    while (tk < tokens.end()) {
        parse_expr(tk);
        if ((*tk)->type == COMMA) {
            elements.push_back(*tk++);
        } else {
            break;
        }
    }
    elements.push_back(std::make_shared<FuncRealParams>());
}

template<typename T>
void Parser::_parse_expr(TokenIter &tk,
                         void (Parser::*parse_first)(TokenIter &),
                         const std::function<bool(TokenCode)> &predicate) {
    while (tk < tokens.end()) {
        (this->*parse_first)(tk);
        auto next = (*tk)->type;
        if (predicate(next)) {
            elements.pop_back();
            elements.push_back(std::make_shared<T>());
            tk++;
        } else {
            break;
        }
    }
    elements.push_back(std::make_shared<T>());
}

void Parser::parse_muldiv_expr(TokenIter &tk) {
    // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
    _parse_expr<MulDivExpr>(tk, &Parser::parse_unary_expr,
                            [](TokenCode next) { return next == MULT || next == DIV || next == MOD; });
}

void Parser::parse_addsub_expr(TokenIter &tk) {
    // AddExp -> MulExp | AddExp ('+' | '-') MulExp
    // AddExp -> MulExp { ('+' | '-') MulExp }
    _parse_expr<AddSubExpr>(tk, &Parser::parse_muldiv_expr,
                            [](TokenCode next) { return next == PLUS || next == MINU; });
}

void Parser::parse_rel_expr(TokenIter &tk) {
    // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
    _parse_expr<RelationalExpr>(tk, &Parser::parse_addsub_expr,
                                [](TokenCode next) {
                                    return next == LSS || next == LEQ || next == GRE || next == GEQ;
                                });
}

void Parser::parse_eq_expr(TokenIter &tk) {
    // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    // EqExp -> RelExp { ('==' | '!=') RelExp }
    _parse_expr<EqualExpr>(tk, &Parser::parse_rel_expr,
                           [](TokenCode next) { return next == EQL || next == NEQ; });
}

void Parser::parse_logical_and_expr(TokenIter &tk) {
    // LAndExp -> EqExp | LAndExp '&&' EqExp
    // LAndExp -> EqExp { '&&' EqExp }
    _parse_expr<LogicalAndExpr>(tk, &Parser::parse_eq_expr,
                                [](TokenCode next) { return next == AND; });
}

void Parser::parse_logical_or_expr(TokenIter &tk) {
    // LOrExp -> LAndExp | LOrExp '||' LAndExp
    // LOrExp -> LAndExp { '||' LAndExp }
    _parse_expr<LogicalOrExpr>(tk, &Parser::parse_logical_and_expr,
                               [](TokenCode next) { return next == OR; });
}

void Parser::parse_const_expr(TokenIter &tk) {
    // ConstExp -> AddExp
    parse_addsub_expr(tk);
    elements.push_back(std::make_shared<ConstExpr>());
}
