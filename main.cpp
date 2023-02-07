#include <ratio>
#include <chrono>

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
        if (!result.is_open()) {
            perror("Failed to create result file");
            exit(EXIT_FAILURE);
        }
        StackMachine machine(parser.instructions, parser.sym_table.back(), result);
        cout << machine;
        auto start = chrono::high_resolution_clock::now();
        machine.run();
        chrono::duration<double, ratio<1, 1>> duration_s(chrono::high_resolution_clock::now() - start);
        cout << "Process finished in " << duration_s.count() << " seconds" << endl;
    } else {
        ofstream error_stream("error.txt");
        if (!error_stream.is_open()) {
            perror("Failed to create error file");
            exit(EXIT_FAILURE);
        }
        error_stream << error;
    }
    return EXIT_SUCCESS;
}
