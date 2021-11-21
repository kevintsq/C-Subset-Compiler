//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_VM_H
#define CODE_VM_H

#include "instruction.h"

class StackMachine {
public:
    vector<InstructionP> &instructions;
    vector<ObjectP> stack;

    explicit StackMachine(vector<InstructionP> &instructions);

    friend ostream &operator<<(ostream &out, const StackMachine &self) {
        for (int i = 0; i < self.instructions.size(); i++) {
            out << i << '\t' << *self.instructions[i] << endl;
        }
        return out;
    }
};


#endif //CODE_VM_H
