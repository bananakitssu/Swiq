#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file>" << std::endl;
        std::cerr << "       " << argv[0] << " -v   (show version info)" << std::endl;
        std::cerr << "       " << argv[0] << " -c <code>   (runs code)" << std::endl;
        return 1;
    }

    std::string filepath = argv[1];

    // "-v" runs the built-in version script instead of a user file
    if (filepath == "-v") {
        fs::path source_dir = SWIQ_SOURCE_DIR;
        fs::path version_path = source_dir / "private_apis/version.swiq";
        filepath = version_path.string();
    }
    
    if (filepath == "-c") {
        if (argc < 3) {
            std::cerr << "Could not run script. Code is required." << std::endl;
            return 1;
        }
        std::string code = argv[2];
        
        try {
            Lexer lexer(code);
            std::vector<Token> tokens = lexer.tokenize();
            
            Parser parser(tokens);
            std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
            
            Interpreter interpreter;
            interpreter.run(statements);
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
        
        return 0;
    }

    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "Could not open file: " << filepath << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

        Interpreter interpreter;
        interpreter.run(statements);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
