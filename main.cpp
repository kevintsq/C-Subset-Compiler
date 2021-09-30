#include "tokenizer.h"
#include "parser.h"

int main() {
    freopen("output.txt", "w", stdout);
    Tokenizer tokenizer("testfile.txt");
    Parser parser(tokenizer.tokens);
    tokenizer.print();
    return 0;
}
