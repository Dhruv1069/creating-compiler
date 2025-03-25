#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cctype>
#include <stack>

using namespace std;

// Token Types
enum class TokenType { TYPE, IDENTIFIER, NUMBER, OPERATOR, PAREN_OPEN, PAREN_CLOSE, COMMA, SEMICOLON, RETURN, BRACE_OPEN, BRACE_CLOSE, END, ERROR };

// Token Structure
struct Token {
    TokenType type;
    string value;
};

// Function Storage (Function Name -> Function Definition)
unordered_map<string, function<int(vector<int>)>> functionTable;

// Operator precedence map
unordered_map<string, int> precedence = {{"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"%", 2}};

// Tokenize Input
vector<Token> tokenize(const string &input) {
    vector<Token> tokens;
    unordered_map<string, TokenType> keywords = {{"int", TokenType::TYPE}, {"return", TokenType::RETURN}};
    size_t i = 0;

    while (i < input.size()) {
        char ch = input[i];

        if (isspace(ch)) {
            i++;
            continue;
        }

        if (isalpha(ch)) {  // Identifier or Keyword
            string word;
            while (i < input.size() && (isalnum(input[i]) || input[i] == '_')) {
                word += input[i++];
            }
            tokens.push_back({keywords.count(word) ? keywords[word] : TokenType::IDENTIFIER, word});
        } 
        else if (isdigit(ch)) {  // Number
            string number;
            while (i < input.size() && isdigit(input[i])) {
                number += input[i++];
            }
            tokens.push_back({TokenType::NUMBER, number});
        } 
        else {  // Symbols
            string sym(1, ch);
            if (ch == '(') tokens.push_back({TokenType::PAREN_OPEN, sym});
            else if (ch == ')') tokens.push_back({TokenType::PAREN_CLOSE, sym});
            else if (ch == '{') tokens.push_back({TokenType::BRACE_OPEN, sym});
            else if (ch == '}') tokens.push_back({TokenType::BRACE_CLOSE, sym});
            else if (ch == ',') tokens.push_back({TokenType::COMMA, sym});
            else if (ch == ';') tokens.push_back({TokenType::SEMICOLON, sym});
            else if (string("+-*/%").find(ch) != string::npos) tokens.push_back({TokenType::OPERATOR, sym});
            else tokens.push_back({TokenType::ERROR, sym});
            i++;
        }
    }
    tokens.push_back({TokenType::END, "$"});
    return tokens;
}

// Evaluate expressions using operator precedence
int evaluateExpression(const string &expr, unordered_map<string, int> &values) {
    istringstream ss(expr);
    stack<int> operands;
    stack<string> operators;
    string token;

    while (ss >> token) {
        if (isdigit(token[0])) {  // Number
            operands.push(stoi(token));
        } 
        else if (values.count(token)) {  // Variable
            operands.push(values[token]);
        } 
        else if (precedence.count(token)) {  // Operator
            while (!operators.empty() && precedence[operators.top()] >= precedence[token]) {
                int b = operands.top(); operands.pop();
                int a = operands.top(); operands.pop();
                string op = operators.top(); operators.pop();

                if (op == "+") operands.push(a + b);
                else if (op == "-") operands.push(a - b);
                else if (op == "*") operands.push(a * b);
                else if (op == "/") {
                    if (b == 0) {
                        cerr << "Runtime Error: Division by zero\n";
                        return 0;
                    }
                    operands.push(a / b);
                }
                else if (op == "%") {
                    if (b == 0) {
                        cerr << "Runtime Error: Modulo by zero\n";
                        return 0;
                    }
                    operands.push(a % b);
                }
            }
            operators.push(token);
        }
    }

    while (!operators.empty()) {  // Final computation
        int b = operands.top(); operands.pop();
        int a = operands.top(); operands.pop();
        string op = operators.top(); operators.pop();

        if (op == "+") operands.push(a + b);
        else if (op == "-") operands.push(a - b);
        else if (op == "*") operands.push(a * b);
        else if (op == "/") operands.push(a / b);
        else if (op == "%") operands.push(a % b);
    }

    return operands.top();
}

// Parse Function Definition and Store It
bool parseFunction(vector<Token> &tokens) {
    if (tokens.size() < 8) {
        cerr << "Syntax Error: Function definition is too short.\n";
        return false;
    }

    int i = 0;
    
    if (tokens[i].type != TokenType::TYPE || tokens[i].value != "int") {
        cerr << "Syntax Error: Function must start with 'int'.\n";
        return false;
    }
    i++;

    if (tokens[i].type != TokenType::IDENTIFIER) {
        cerr << "Syntax Error: Function must have a valid name.\n";
        return false;
    }
    string funcName = tokens[i].value;
    i++;

    if (tokens[i].type != TokenType::PAREN_OPEN) {
        cerr << "Syntax Error: Expected '(' after function name.\n";
        return false;
    }
    i++;

    vector<string> paramNames;
    while (tokens[i].type != TokenType::PAREN_CLOSE) {
        if (tokens[i].type != TokenType::TYPE) {
            cerr << "Syntax Error: Expected parameter type before name.\n";
            return false;
        }
        i++;
        if (tokens[i].type != TokenType::IDENTIFIER) {
            cerr << "Syntax Error: Expected parameter name after type.\n";
            return false;
        }
        paramNames.push_back(tokens[i].value);
        i++;

        if (tokens[i].type == TokenType::COMMA) i++;
    }
    i++;

    if (tokens[i].type != TokenType::BRACE_OPEN) {
        cerr << "Syntax Error: Expected '{' before function body.\n";
        return false;
    }
    i++;

    if (tokens[i].type != TokenType::RETURN) {
        cerr << "Syntax Error: Expected 'return' inside function body.\n";
        return false;
    }
    i++;

    string expr;
    while (tokens[i].type != TokenType::SEMICOLON) {
        expr += tokens[i].value + " ";
        i++;
    }
    i++;

    functionTable[funcName] = [expr, paramNames](vector<int> args) -> int {
        unordered_map<string, int> values;
        for (size_t j = 0; j < paramNames.size(); j++) {
            values[paramNames[j]] = args[j];
        }
        return evaluateExpression(expr, values);
    };

    cout << "Function '" << funcName << "' defined successfully.\n";
    return true;
}

// Execute Function Call
bool executeFunction(vector<Token> &tokens) {
    if (tokens.size() < 4 || tokens[1].type != TokenType::PAREN_OPEN) {
        cerr << "Syntax Error: Invalid function call format.\n";
        return false;
    }

    string funcName = tokens[0].value;
    if (functionTable.find(funcName) == functionTable.end()) {
        cerr << "Error: Function '" << funcName << "' not defined.\n";
        return false;
    }

    vector<int> args;
    int i = 2;  // Skip function name and '('

    while (tokens[i].type != TokenType::PAREN_CLOSE) {
        if (tokens[i].type == TokenType::NUMBER) {
            args.push_back(stoi(tokens[i].value));
            i++;
            if (tokens[i].type == TokenType::COMMA) i++; // Skip comma if present
        } else {
            cerr << "Syntax Error: Expected numeric argument.\n";
            return false;
        }
    }

    int result = functionTable[funcName](args);
    cout << "Output: " << result << endl;
    return true;
}


// Main Function
int main() {
    string input;
    cout << "Custom Compiler Interactive Mode\n";

    while (true) {
        cout << "\n>> ";
        getline(cin, input);
        if (input == "exit") break;
        vector<Token> tokens = tokenize(input);
        if (tokens.size() > 2 && tokens[1].type == TokenType::IDENTIFIER) parseFunction(tokens);
        else executeFunction(tokens);
    }
    return 0;
}
