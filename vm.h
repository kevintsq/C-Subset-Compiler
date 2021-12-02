//
// Created by Kevin Tan on 2021/11/20.
//

#ifndef CODE_VM_H
#define CODE_VM_H

#include "instruction.h"

using Stack = vector<ObjectP>;
using StackP = shared_ptr<Stack>;

struct Frame {
    unordered_map<IdentP, ObjectP> objects{CACHE_LINE_SIZE / sizeof(ObjectP)};
    StackP stack = make_shared<Stack>();
    long long return_offset = 0;
};

using FrameP = shared_ptr<Frame>;

class StackMachine {
public:
    vector<InstructionP> &instructions;
    unordered_map<string, ObjectP> &globals;
    ostream &outs;
    vector<FrameP> frames{make_shared<Frame>()};  // dummy frame
    StackP stack = frames.back()->stack;

    explicit StackMachine(vector<InstructionP> &instructions, unordered_map<string, ObjectP> &globals,
                          ostream &outs) : instructions(instructions), globals(globals), outs(outs) {};

    void run();

    friend ostream &operator<<(ostream &out, const StackMachine &self) {
        for (int i = 0; i < self.instructions.size(); i++) {
            out << i << '\t' << *self.instructions[i] << endl;
        }
        return out;
    }
};


#endif //CODE_VM_H
