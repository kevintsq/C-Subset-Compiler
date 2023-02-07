//
// Created by Kevin Tan on 2021/9/24.
//

#ifndef CODE_PARSER_H
#define CODE_PARSER_H

#include "token.h"
#include "instruction.h"

using TokenIter = vector<TokenP>::iterator;
using HashMap = unordered_map<string, ObjectP>;

struct LoopInfo {
    long long start;
    vector<JumpInstructionP> break_instructions;

    explicit LoopInfo(long long start) : start(start) {}
};

enum EmitMode {
    NO_EMIT_IN_CONST_DEF,
    NO_EMIT_IN_FPARAMS,
    EMIT_IN_VAR_DEF,
    EMIT_IN_NORM_STMT,
    EMIT_IN_COND_STMT
};

class Parser {
    Error &error;
    vector<TokenP> &tokens;
    vector<ArrayObjectP> arrays;
    InstructionP entry_inst;

    vector<LoopInfo> loop_info;
    TypeCode current_func_return_type = TypeCode::INT;
    bool has_return_at_end = false;
public:
    vector<ElementP> elements;
    vector<HashMap> sym_table;
    vector<InstructionP> instructions;

    explicit Parser(vector<TokenP> &tokens, Error &error);

    friend ostream &operator<<(ostream &out, const Parser &self) {
        for (const auto &element: self.elements) {
            out << *element << endl;
        }
        return out;
    }

    inline void relocate_jump_instructions(vector<JumpInstructionP> &is) const {
        for (auto &i: is) {
            i->set_offset((long long) instructions.size());
        }
    }

    inline bool has_type(const TokenIter &tk, TokenCode type) {
        if (tk == tokens.end()) {
            cerr << "unexpected EOF while parsing" << endl;
            exit(-1);
        } else {
            return (*tk)->token_type == type;
        }
    }

    static inline bool starts_with_decl(const TokenIter &tk) {
        auto next = (*tk)->token_type;
        return next == TokenCode::CONSTTK ||
               (next == TokenCode::INTTK && (*(tk + 2))->token_type != TokenCode::LPARENT);
    }

    static inline bool starts_with_func_def(const TokenIter &tk) {
        auto next = (*tk)->token_type;
        return next == TokenCode::VOIDTK ||
               (next == TokenCode::INTTK &&
                (*(tk + 1))->token_type != TokenCode::MAINTK &&
                (*(tk + 2))->token_type == TokenCode::LPARENT);
    }

    static inline bool starts_with_expr(const TokenIter &tk) {
        switch ((*tk)->token_type) {
            case TokenCode::LPARENT:
            case TokenCode::IDENFR:
            case TokenCode::INTCON:
            case TokenCode::PLUS:
            case TokenCode::MINU:
            case TokenCode::NOT:
                return true;
            default:
                return false;
        }
    }

    static inline bool starts_with_stmt(const TokenIter &tk) {
        switch ((*tk)->token_type) {
            case TokenCode::LBRACE:
            case TokenCode::SEMICN:
            case TokenCode::IFTK:
            case TokenCode::WHILETK:
            case TokenCode::BREAKTK:
            case TokenCode::CONTINUETK:
            case TokenCode::RETURNTK:
            case TokenCode::PRINTFTK:
                return true;
            default:
                return starts_with_expr(tk);
        }
    }

    static BinaryOpCode determine_addsub(TokenCode type) {
        switch (type) {
            case TokenCode::PLUS:
                return BinaryOpCode::BINARY_ADD;
            case TokenCode::MINU:
                return BinaryOpCode::BINARY_SUB;
            default:
                return BinaryOpCode::NOTHING;
        }
    }

    static BinaryOpCode determine_muldiv(TokenCode type) {
        switch (type) {
            case TokenCode::MULT:
                return BinaryOpCode::BINARY_MUL;
            case TokenCode::DIV:
                return BinaryOpCode::BINARY_DIV;
            case TokenCode::MOD:
                return BinaryOpCode::BINARY_MOD;
            default:
                return BinaryOpCode::NOTHING;
        }
    }

    static BinaryOpCode determine_relation(TokenCode type) {
        switch (type) {
            case TokenCode::LSS:
                return BinaryOpCode::BINARY_LT;
            case TokenCode::LEQ:
                return BinaryOpCode::BINARY_LE;
            case TokenCode::GRE:
                return BinaryOpCode::BINARY_GT;
            case TokenCode::GEQ:
                return BinaryOpCode::BINARY_GE;
            default:
                return BinaryOpCode::NOTHING;
        }
    }

    static BinaryOpCode determine_equality(TokenCode type) {
        switch (type) {
            case TokenCode::EQL:
                return BinaryOpCode::BINARY_EQ;
            case TokenCode::NEQ:
                return BinaryOpCode::BINARY_NE;
            default:
                return BinaryOpCode::NOTHING;
        }
    }

    static BinaryOpCode determine_or(TokenCode type) {
        return type == TokenCode::OR ? BinaryOpCode::BINARY_LOGICAL_OR : BinaryOpCode::NOTHING;
    }

    static BinaryOpCode determine_and(TokenCode type) {
        return type == TokenCode::AND ? BinaryOpCode::BINARY_LOGICAL_AND : BinaryOpCode::NOTHING;
    }

    template<typename T>
    ObjectP _parse_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op,
                        ObjectP (Parser::*parse_first)(TokenIter &, EmitMode, BinaryOpCode),
                        BinaryOpCode (*predicate)(TokenCode));

    void parse_comp_unit(TokenIter &tk);

    ObjectP check_ident_valid_use(TokenIter &tk, bool is_called, bool is_assigned);

    ObjectP check_ident_valid_decl(TokenIter &tk, TypeCode type, bool is_global, bool is_const, bool is_func);

    void parse_decl(TokenIter &tk, int nest_level);

    void parse_const_decl(TokenIter &tk, int nest_level);

    void parse_const_def(TokenIter &tk, int nest_level);

    void parse_var_decl(TokenIter &tk, int nest_level);

    void parse_var_def(TokenIter &tk, int nest_level);

    template<typename ExprT, typename ElementT>
    ObjectP parse_init_val(TokenIter &tk, EmitMode emit_mode);

    void parse_func_def(TokenIter &tk);

    void parse_main_func_def(TokenIter &tk);

    TypeCode parse_func_type(TokenIter &tk);

    void parse_func_formal_params(TokenIter &tk, FuncObjectP &func);

    void parse_func_formal_param(TokenIter &tk, FuncObjectP &func);

    void parse_block(TokenIter &tk, int nest_level, bool from_func_def = false);

    void parse_block_item(TokenIter &tk, int nest_level);

    void parse_stmt(TokenIter &tk, int nest_level);

    template<typename T>
    ObjectP parse_expr(TokenIter &tk, EmitMode emit_mode);

    ObjectP parse_cond_expr(TokenIter &tk, EmitMode emit_mode, vector<JumpInstructionP> &eval_jump_instructions,
                            vector<JumpInstructionP> *control_jump_instructions);

    ObjectP parse_lvalue(TokenIter &tk, EmitMode emit_mode, bool is_assigned);

    ObjectP parse_primary_expr(TokenIter &tk, EmitMode emit_mode);

    IntObjectP parse_number(TokenIter &tk);

    UnaryOpCode parse_unary_op(TokenIter &tk);

    ObjectP parse_unary_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode dummy);

    void parse_func_real_params(TokenIter &tk, FuncObjectP &func, int func_line);

    ObjectP parse_muldiv_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op);

    ObjectP parse_addsub_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op);

    ObjectP parse_rel_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op);

    ObjectP parse_eq_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op);

    ObjectP parse_logical_and_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op,
                                   vector<JumpInstructionP> &eval_jump_instructions);

    ObjectP parse_logical_or_expr(TokenIter &tk, EmitMode emit_mode, BinaryOpCode last_op,
                                  vector<JumpInstructionP> &eval_jump_instructions,
                                  vector<JumpInstructionP> *control_jump_instructions);
};


#endif //CODE_PARSER_H
