#include "tokenizer.h"
#include "parser.h"

int main() {
    if (freopen("output.txt", "w", stdout) == nullptr) {
        perror("freopen");
        return EXIT_FAILURE;
    }
    Tokenizer tokenizer("testfile.txt");
    Parser parser(tokenizer.tokens);
    parser.print();
    return EXIT_SUCCESS;
}
