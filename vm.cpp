//
// Created by Kevin Tan on 2021/11/20.
//

#include "vm.h"

void StackMachine::run() {
    auto pc = instructions.begin();
    while (pc < instructions.end()) {
        switch ((*pc)->opcode) {
            case LOAD_FAST:
                stack->push_back(cast<LoadFast>(*pc)->object);
                ++pc;
                break;
            case LOAD_NAME: {
                ObjectP info = cast<LoadName>(*pc)->object;
                if (info->is_global) {
                    stack->push_back(globals[info->ident_info->name]);
                } else {
                    auto &objects = frames.back()->objects;
                    auto it = objects.find(info->ident_info);
                    if (it != objects.end()) {
                        stack->push_back((*it).second);
                    } else {
                        cerr << "WARNING: use of unbound name " << info->ident_info->name << " (declared in line "
                             << info->ident_info->line << "), has bound its value to 0" << endl;
                        IntObjectP o = make_shared<IntObject>();
                        objects[info->ident_info] = o;
                        stack->push_back(o);
                    }
                }
                ++pc;
                break;
            }
            case STORE_NAME: {
                ObjectP o = cast<StoreName>(*pc)->object;
                auto &name = o->ident_info->name;
                auto &locals = frames.back()->objects;
                if (o->is_const) {
                    ERROR_NOT_SUPPORTED(modifing const);
                } else if (o->is_global) {
                    if (o->type == INT_ARRAY) {
                        cast<ArrayObject>(globals[name])->data = cast<ArrayObject>(stack->back())->data;
                    } else {
                        globals[name] = stack->back()->copy();  // replace store, must be copied
                    }
                } else {
                    if (o->type == INT_ARRAY) {
                        cast<ArrayObject>(stack->back())->dims = cast<ArrayObject>(o)->dims;
                    }  // dim of array on stack may be unknown, so we need to copy them
                    locals[o->ident_info] = stack->back()->copy();  // replace store, must be copied
                }
                stack->pop_back();
                ++pc;
                break;
            }
            case POP_TOP:
                stack->pop_back();
                ++pc;
                break;
            case BUILD_ARRAY: {
                long long size = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                ArrayObjectP array = make_shared<ArrayObject>(size);
                for (long long i = 0; i < size; i++) {
                    array->data->push_back(make_shared<IntObject>());  // zeroed for safety
                }
                stack->push_back(array);
                ++pc;
                break;
            }
            case INIT_ARRAY: {
                ArrayObjectP array = make_shared<ArrayObject>((long long) stack->size());
                // not safe to use `*array->data = *stack` because stack size may be smaller than the array size
                // shouldn't use `insert` because zeros are not overwritten
                // the operand of the instruction is not used because we don't have the `ArrayLiteral` class to
                // record the actual size of the array literal when VLA is supported
                for (auto &i: *stack) {
                    array->data->push_back(i->copy());  // must be copied
                }
                stack->clear();
                stack->push_back(array);
                ++pc;
                break;
            }
            case STORE_SUBSCR: {
                auto value = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                cast<IntObject>(stack->back())->value = value;  // in-place store, must not be copied
                stack->pop_back();
                ++pc;
                break;
            }
            case SUBSCR_ARRAY: {
                long long index = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                ArrayObjectP array = cast<ArrayObject>(stack->back());
                stack->pop_back();
                stack->push_back((*array)[index]);
                ++pc;
                break;
            }
            case CALL_GETINT: {
                int n;
                cin >> n;
                stack->push_back(make_shared<IntObject>(n));
                ++pc;
                break;
            }
            case JUMP_ABSOLUTE:
                pc = instructions.begin() + cast<JumpAbsolute>(*pc)->get_offset();
                break;
            case POP_JUMP_IF_FALSE: {
                long long value = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                if (!value) {
                    pc = instructions.begin() + cast<PopJumpIfFalse>(*pc)->get_offset();
                } else {
                    ++pc;
                }
                break;
            }
            case POP_JUMP_IF_TRUE: {
                long long value = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                if (value) {
                    pc = instructions.begin() + cast<PopJumpIfTrue>(*pc)->get_offset();
                } else {
                    ++pc;
                }
                break;
            }
            case CALL_PRINTF: {
                FormatStringP fmt_str = cast<PrintF>(*pc)->format_string;
                auto seg_it = fmt_str->segments.begin();
                outs << *seg_it;
                for (auto it = stack->end() - fmt_str->fmt_char_cnt; it != stack->end(); ++it) {
                    outs << cast<IntObject>(*it)->value << *++seg_it;
                }
                for (int i = 0; i < fmt_str->fmt_char_cnt; i++) {
                    stack->pop_back();
                }
                ++pc;
                break;
            }
            case EXIT_INTERP:
                return;
            case CALL_FUNCTION: {
                FrameP new_frame = make_shared<Frame>();
                new_frame->return_offset = pc - instructions.begin() + 1;
                FuncObjectP func = cast<CallFunction>(*pc)->get_func();
                auto &params = func->params;
                for (auto i = (long long) params.size() - 1; i >= 0; i--) {
                    ObjectP o = stack->back();
                    switch (o->type) {
                        case INT:
                            new_frame->objects[params[i]->ident_info] = cast<IntObject>(o)->copy();
                            break;
                        case INT_ARRAY: {
                            ArrayObjectP array = cast<ArrayObject>(o->copy());
                            new_frame->objects[params[i]->ident_info] = array;
                            array->dims = cast<ArrayObject>(params[i])->dims;
                            break;  // note the dimension difference when addressing
                        }
                        default:
                            ERROR_LIMITED_SUPPORT(INT or INT_ARRAY function arguments);
                    }
                    stack->pop_back();
                }
                stack = new_frame->stack;
                frames.push_back(new_frame);
                pc = instructions.begin() + func->code_offset;
                break;
            }
            case RETURN_VALUE: {
                if (!stack->empty()) {
                    assert(stack->size() == 1);
                    ObjectP value = stack->back();
                    stack->pop_back();
                    pc = instructions.begin() + frames.back()->return_offset;
                    frames.pop_back();
                    stack = frames.back()->stack;
                    stack->push_back(value);
                } else {
                    pc = instructions.begin() + frames.back()->return_offset;
                    frames.pop_back();
                    stack = frames.back()->stack;
                    stack->push_back(make_shared<Object>());
                    // void object is returned for void function because the POP_TOP instruction is always added
                    // after the function call when the return value is not used
                }
                break;
            }
            case UNARY_OP: {
                auto value = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                stack->push_back(make_shared<IntObject>(util::unary_operation(cast<UnaryOperation>(*pc)->unary_opcode, value)));
                ++pc;
                break;
            }
            case BINARY_OP: {
                auto right = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                auto left = cast<IntObject>(stack->back())->value;
                stack->pop_back();
                stack->push_back(make_shared<IntObject>(util::binary_operation(cast<BinaryOperation>(*pc)->binary_opcode, left, right)));
                ++pc;
                break;
            }
            default:
                ++pc;
        }
    }
}
