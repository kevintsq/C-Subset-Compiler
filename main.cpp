#include "tokenizer.h"
#include "parser.h"
#include "vm.h"

int main() {
    Error error;

    Tokenizer tokenizer("testfile.txt", error);

//    ofstream output("output.txt");
    Parser parser(tokenizer.tokens, error);
//    output << parser;

    if (error.errors.empty()) {
        ofstream result("pcoderesult.txt");
        StackMachine machine(parser.instructions, result);
        cout << machine;
        machine.run();
    } else {
//        ofstream error_stream("error.txt");
//        error_stream << error;
    }
    return EXIT_SUCCESS;
}
