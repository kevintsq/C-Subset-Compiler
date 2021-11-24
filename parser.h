//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "token.h"
#include "instruction.h"

using TokenIter = vector<TokenP>::iterator;
using HashMap = unordered_map<string, ObjectP>;
using HashMapP = shared_ptr<HashMap>;

class Parser {
public:
    Error &error;
    vector<TokenP> &tokens;
    vector<ElementP> elements;
    vector<HashMapP> sym_table;
    vector<InstructionP> instructions;
    InstructionP entry_inst;

    int while_cnt = 0;
    TypeCode current_func_return_type = INT;
    bool has_return_at_end = false;

    explicit Parser(vector<TokenP> &tokens, Error &error);

    friend ostream &operator<<(ostream &out, const Parser &self) {
        for (const auto &element: self.elements) {
            out << *element << endl;
        }
        return out;
    }

    inline bool has_type(const TokenIter &tk, TokenCode type) {
        return tk < tokens.end() && (*tk)->type == type;
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

    static BinaryOpCode determine_addsub(TokenCode type) {
        switch (type) {
            case PLUS:
                return BINARY_ADD;
            case MINU:
                return BINARY_SUB;
            default:
                return NOTHING;
        }
    }

    static BinaryOpCode determine_muldiv(TokenCode type) {
        switch (type) {
            case MULT:
                return BINARY_MUL;
            case DIV:
                return BINARY_DIV;
            case MOD:
                return BINARY_MOD;
            default:
                return NOTHING;
        }
    }

    static BinaryOpCode determine_relation(TokenCode type) {
        switch (type) {
            case LSS:
                return BINARY_LT;
            case LEQ:
                return BINARY_LE;
            case GRE:
                return BINARY_GT;
            case GEQ:
                return BINARY_GE;
            default:
                return NOTHING;
        }
    }

    static BinaryOpCode determine_equality(TokenCode type) {
        switch (type) {
            case EQL:
                return BINARY_EQ;
            case NEQ:
                return BINARY_NE;
            default:
                return NOTHING;
        }
    }

    static BinaryOpCode determine_or(TokenCode type) {
        return type == OR ? BINARY_LOGICAL_OR : NOTHING;
    }

    static BinaryOpCode determine_and(TokenCode type) {
        return type == AND ? BINARY_LOGICAL_AND : NOTHING;
    }

    template<typename T>
    ObjectP _parse_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op,
                        ObjectP (Parser::*parse_first)(TokenIter &, bool, BinaryOpCode),
                        BinaryOpCode (*predicate)(TokenCode));

    void parse_comp_unit(TokenIter &tk);

    ObjectP check_ident_valid_use(TokenIter &tk, bool is_called, bool is_assigned);

    ObjectP check_ident_valid_decl(TokenIter &tk, TypeCode type, bool is_const, bool is_func);

    void parse_decl(TokenIter &tk, int nest_level);

    void parse_const_decl(TokenIter &tk, int nest_level);

    void parse_const_def(TokenIter &tk, int nest_level);

    void parse_var_decl(TokenIter &tk, int nest_level);

    void parse_var_def(TokenIter &tk, int nest_level);

    template<typename ExprT, typename ElementT>
    ObjectP parse_init_val(TokenIter &tk, bool generate_inst = true);

    void parse_func_def(TokenIter &tk);

    void parse_main_func_def(TokenIter &tk);

    TypeCode parse_func_type(TokenIter &tk);

    void parse_func_formal_params(TokenIter &tk, FuncObjectP &func);

    void parse_func_formal_param(TokenIter &tk, FuncObjectP &func);

    void parse_block(TokenIter &tk, int nest_level, bool from_func_def = false);

    void parse_block_item(TokenIter &tk, int nest_level);

    void parse_stmt(TokenIter &tk, int nest_level);

    template<typename T>
    ObjectP parse_expr(TokenIter &tk, bool generate_inst = true);

    ObjectP parse_cond_expr(TokenIter &tk);

    ObjectP parse_lvalue(TokenIter &tk, bool generate_inst, bool is_assigned);

    ObjectP parse_primary_expr(TokenIter &tk, bool generate_inst);

    IntObjectP parse_number(TokenIter &tk);

    UnaryOpCode parse_unary_op(TokenIter &tk);

    ObjectP parse_unary_expr(TokenIter &tk, bool generate_inst, BinaryOpCode dummy);

    void parse_func_real_params(TokenIter &tk, FuncObjectP &func, int func_line);

    ObjectP parse_muldiv_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op);

    ObjectP parse_addsub_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op);

    ObjectP parse_rel_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op);

    ObjectP parse_eq_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op);

    ObjectP parse_logical_and_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op);

    ObjectP parse_logical_or_expr(TokenIter &tk, bool generate_inst, BinaryOpCode last_op);
};


#endif //CODE_PARSER_H
