//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_VM_H
#define CODE_VM_H

#include "instruction.h"

using Stack = vector<ObjectP>;
using StackP = shared_ptr<Stack>;

class StackMachine {
public:
    vector<InstructionP> &instructions;
    StackP stack = make_shared<Stack>();
    vector<StackP> frames{stack};
    ostream &outs;

    explicit StackMachine(vector<InstructionP> &instructions, ostream &outs) : instructions(instructions),
                                                                               outs(outs) {};

    void run();

    friend ostream &operator<<(ostream &out, const StackMachine &self) {
        for (int i = 0; i < self.instructions.size(); i++) {
            out << i << '\t' << *self.instructions[i] << endl;
        }
        return out;
    }
};


#endif //CODE_VM_H
