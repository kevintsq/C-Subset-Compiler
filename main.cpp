#include "tokenizer.h"
#include "parser.h"

int main() {
    freopen("output.txt", "w", stdout);
    Tokenizer tokenizer("testfile.txt");
    Parser parser(tokenizer.tokens);
    parser.print();
    return 0;
}
