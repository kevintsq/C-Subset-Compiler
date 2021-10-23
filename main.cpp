#include "tokenizer.h"
#include "parser.h"

int main() {
    Error error;
    Tokenizer tokenizer("testfile.txt", error);
    Parser parser(tokenizer.tokens, error);
    ofstream output("output.txt");
    output << parser;
    ofstream error_stream("error.txt");
    error_stream << error;
    return EXIT_SUCCESS;
}
