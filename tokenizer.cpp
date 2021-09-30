//
// Created by Kevin Tan on 2021/9/24.
//

#include "tokenizer.h"

Tokenizer::Tokenizer(const char *filename) {
//    FILE *fp = fopen("testfile.txt", "rb");
//    fseek(fp, 0, SEEK_END);
//    long size = ftell(fp);
//    rewind(fp);
//    char *buffer = new char[size + 1];
//    fread(buffer, 1, size, fp);
//    buffer[size] = '\0';
//    fclose(fp);
    std::ifstream file(filename, std::ios::binary);
    file.seekg(0, std::ifstream::end);
    long long size = file.tellg();
    file.seekg(std::ifstream::beg);

    char *buffer = new char[size + 1];
    char *original = buffer;
    buffer[size] = '\0';
    file.read(buffer, size);
    file.close();

    int line = 1;
    long number;
    for (char *current = buffer; *buffer != '\0'; current = buffer) {
        if (*buffer == '\n') {
            line++;
            buffer++;
        } else if (isspace(*buffer)) {  // cannot omit this
            buffer++;
        } else if (*buffer == '+') {
            this->tokens.push_back(std::make_shared<Token *>(new AddToken(line)));
            buffer++;
        } else if (*buffer == '-') {
            this->tokens.push_back(std::make_shared<Token *>(new SubToken(line)));
            buffer++;
        } else if ((number = strtol(current, &buffer, 10)) || current != buffer) {
            // Must be after the leading 3!
            this->tokens.push_back(std::make_shared<Token *>(new IntConst(line, number)));
        } else if (*buffer == '"') {
            while (*++buffer != '"') {
                if (*buffer == '\0') { return; }  // Bad FormatString
            }
            this->tokens.push_back(std::make_shared<Token *>(new FormatString(line, current, ++buffer - current)));
        } else if (startswith(&buffer, "//")) {
            while (*buffer++ != '\n') {
                if (*buffer == '\0') { return; }
            }
        } else if (startswith(&buffer, "/*")) {
            while (true) {
                while (*buffer++ != '*') {
                    if (*buffer == '\n') { line++; }
                    else if (*buffer == '\0') { return; }  // Bad Comment
                }
                if (*buffer == '/') { buffer++; break; }
                else if (*buffer == '\n') { buffer++; line++; }
                else if (*buffer == '\0') { return; }  // Bad Comment
            }
        } else if (startswith(&buffer, "&&")) {
            this->tokens.push_back(std::make_shared<Token *>(new AndToken(line)));
        } else if (startswith(&buffer, "||")) {
            this->tokens.push_back(std::make_shared<Token *>(new OrToken(line)));
        } else if (startswith(&buffer, "<=")) {
            this->tokens.push_back(std::make_shared<Token *>(new LeToken(line)));
        } else if (startswith(&buffer, ">=")) {
            this->tokens.push_back(std::make_shared<Token *>(new GeToken(line)));
        } else if (startswith(&buffer, "==")) {
            this->tokens.push_back(std::make_shared<Token *>(new EqToken(line)));
        } else if (startswith(&buffer, "!=")) {
            this->tokens.push_back(std::make_shared<Token *>(new NeToken(line)));
        } else if (startswith(&buffer, "main")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<MainToken>(&buffer, current, line));
        } else if (startswith(&buffer, "const")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<ConstToken>(&buffer, current, line));
        } else if (startswith(&buffer, "int")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<IntToken>(&buffer, current, line));
        } else if (startswith(&buffer, "void")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<VoidToken>(&buffer, current, line));
        } else if (startswith(&buffer, "break")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<BreakToken>(&buffer, current, line));
        } else if (startswith(&buffer, "continue")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<ContinueToken>(&buffer, current, line));
        } else if (startswith(&buffer, "if")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<IfToken>(&buffer, current, line));
        } else if (startswith(&buffer, "else")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<ElseToken>(&buffer, current, line));
        } else if (startswith(&buffer, "while")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<WhileToken>(&buffer, current, line));
        } else if (startswith(&buffer, "getint")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<GetintToken>(&buffer, current, line));
        } else if (startswith(&buffer, "printf")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<PrintfToken>(&buffer, current, line));
        } else if (startswith(&buffer, "return")) {
            this->tokens.push_back(Tokenizer::get_ident_or_keyword<ReturnToken>(&buffer, current, line));
        } else if (isalpha(*buffer) || *buffer == '_') {
            while (Tokenizer::isalnum_(*++buffer));
            this->tokens.push_back(std::make_shared<Token *>(new Identifier(line, current, buffer - current)));
        } else {
            switch (*buffer++) {
                case '*': this->tokens.push_back(std::make_shared<Token *>(new MulToken(line))); break;
                case '/': this->tokens.push_back(std::make_shared<Token *>(new DivToken(line))); break;
                case '%': this->tokens.push_back(std::make_shared<Token *>(new ModToken(line))); break;
                case '!': this->tokens.push_back(std::make_shared<Token *>(new NotToken(line))); break;
                case ',': this->tokens.push_back(std::make_shared<Token *>(new Comma(line))); break;
                case ';': this->tokens.push_back(std::make_shared<Token *>(new Semicolon(line))); break;
                case '=': this->tokens.push_back(std::make_shared<Token *>(new AssignToken(line))); break;
                case '<': this->tokens.push_back(std::make_shared<Token *>(new LtToken(line))); break;
                case '>': this->tokens.push_back(std::make_shared<Token *>(new GtToken(line))); break;
                case '(': this->tokens.push_back(std::make_shared<Token *>(new LParen(line))); break;
                case ')': this->tokens.push_back(std::make_shared<Token *>(new RParen(line))); break;
                case '[': this->tokens.push_back(std::make_shared<Token *>(new LBracket(line))); break;
                case ']': this->tokens.push_back(std::make_shared<Token *>(new RBracket(line))); break;
                case '{': this->tokens.push_back(std::make_shared<Token *>(new LBrace(line))); break;
                case '}': this->tokens.push_back(std::make_shared<Token *>(new RBrace(line))); break;
            }
        }
    }
    delete[] original;
}
