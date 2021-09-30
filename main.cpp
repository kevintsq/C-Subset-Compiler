#include "tokenizer.h"

int main() {
    freopen("output.txt", "w", stdout);
    Tokenizer tokenizer("testfile.txt");
    tokenizer.print();
    return 0;
}
