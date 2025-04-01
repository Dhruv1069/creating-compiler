#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cctype>
#include <stack>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
#include <algorithm>
#include <cmath> 

using namespace std;

// Token Types (unchanged)
enum class TokenType { TYPE, IDENTIFIER, NUMBER, OPERATOR, PAREN_OPEN, PAREN_CLOSE, COMMA, SEMICOLON, RETURN, BRACE_OPEN, BRACE_CLOSE, END, ERROR };

// Token Structure (unchanged)
struct Token {
    TokenType type;
    string value;
};

// Function Storage (now supports variant return types via string)
unordered_map<string, function<string(vector<string>)>> functionTable;

// Operator precedence map (unchanged)
unordered_map<string, int> precedence = {{"+", 1}, {"-", 1}, {"*", 2}, {"/", 2}, {"%", 2}};

// Tokenize Input (updated for new keywords)
vector<Token> tokenize(const string &input) {
    vector<Token> tokens;
    // In the tokenize function, update the keywords map:
unordered_map<string, TokenType> keywords = {
    {"int", TokenType::TYPE}, {"return", TokenType::RETURN},
    {"print", TokenType::IDENTIFIER}, {"println", TokenType::IDENTIFIER}, {"read", TokenType::IDENTIFIER},
    {"readInt", TokenType::IDENTIFIER}, {"readFloat", TokenType::IDENTIFIER}, {"readLine", TokenType::IDENTIFIER},
    {"isNull", TokenType::IDENTIFIER}, {"typeOf", TokenType::IDENTIFIER}, {"getCurrentTime", TokenType::IDENTIFIER},
    {"getCurrentDate", TokenType::IDENTIFIER}, {"getDayOfWeek", TokenType::IDENTIFIER}, {"getTimestamp", TokenType::IDENTIFIER},
    {"sleep", TokenType::IDENTIFIER}, {"length", TokenType::IDENTIFIER}, {"concat", TokenType::IDENTIFIER},
    {"substring", TokenType::IDENTIFIER}, {"touppercase", TokenType::IDENTIFIER}, {"tolowercase", TokenType::IDENTIFIER},
    {"charAt", TokenType::IDENTIFIER}, {"indexOf", TokenType::IDENTIFIER}, {"replace", TokenType::IDENTIFIER},
    {"split", TokenType::IDENTIFIER}, {"trim", TokenType::IDENTIFIER},
    // New functions added
    {"equals", TokenType::IDENTIFIER}, {"notequals", TokenType::IDENTIFIER}, {"greaterThan", TokenType::IDENTIFIER},
    {"lessThan", TokenType::IDENTIFIER}, {"min", TokenType::IDENTIFIER}, {"max", TokenType::IDENTIFIER},
    {"swap", TokenType::IDENTIFIER}, {"iseven", TokenType::IDENTIFIER}, {"isodd", TokenType::IDENTIFIER},
    {"startsWith", TokenType::IDENTIFIER}, {"endsWith", TokenType::IDENTIFIER}, {"countOccurrences", TokenType::IDENTIFIER},
    {"reverseNumber", TokenType::IDENTIFIER}, {"reverseString", TokenType::IDENTIFIER}, {"factorial", TokenType::IDENTIFIER},
    {"fibonacci", TokenType::IDENTIFIER},
    // New mathematical functions
    {"mod", TokenType::IDENTIFIER}, {"pow", TokenType::IDENTIFIER}, {"sqrt", TokenType::IDENTIFIER},
    {"cos", TokenType::IDENTIFIER}, {"sin", TokenType::IDENTIFIER}, {"ceil", TokenType::IDENTIFIER},
    {"floor", TokenType::IDENTIFIER}
};
    size_t i = 0;

    while (i < input.size()) {
        char ch = input[i];
        if (isspace(ch)) { i++; continue; }
        if (isalpha(ch)) {
            string word;
            while (i < input.size() && (isalnum(input[i]) || input[i] == '_')) word += input[i++];
            tokens.push_back({keywords.count(word) ? keywords[word] : TokenType::IDENTIFIER, word});
        } else if (isdigit(ch)) {
            string number;
            while (i < input.size() && (isdigit(input[i]) || input[i] == '.')) number += input[i++]; // Support floats
            tokens.push_back({TokenType::NUMBER, number});
        } else {
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

// Evaluate expressions (updated to return string for flexibility)
string evaluateExpression(const string &expr, unordered_map<string, string> &values) {
    istringstream ss(expr);
    stack<string> operands;
    stack<string> operators;
    string token;

    while (ss >> token) {
        if (isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
            operands.push(token);
        } else if (values.count(token)) {
            operands.push(values[token]);
        } else if (precedence.count(token)) {
            while (!operators.empty() && precedence[operators.top()] >= precedence[token]) {
                string b = operands.top(); operands.pop();
                string a = operands.top(); operands.pop();
                string op = operators.top(); operators.pop();
                if (op == "+") operands.push(to_string(stoi(a) + stoi(b)));
                else if (op == "-") operands.push(to_string(stoi(a) - stoi(b)));
                else if (op == "*") operands.push(to_string(stoi(a) * stoi(b)));
                else if (op == "/") operands.push(to_string(stoi(a) / stoi(b)));
                else if (op == "%") operands.push(to_string(stoi(a) % stoi(b)));
            }
            operators.push(token);
        }
    }

    while (!operators.empty()) {
        string b = operands.top(); operands.pop();
        string a = operands.top(); operands.pop();
        string op = operators.top(); operators.pop();
        if (op == "+") operands.push(to_string(stoi(a) + stoi(b)));
        else if (op == "-") operands.push(to_string(stoi(a) - stoi(b)));
        else if (op == "*") operands.push(to_string(stoi(a) * stoi(b)));
        else if (op == "/") operands.push(to_string(stoi(a) / stoi(b)));
        else if (op == "%") operands.push(to_string(stoi(a) % stoi(b)));
    }

    return operands.top();
}

// Parse Function Definition (updated for string return)
bool parseFunction(vector<Token> &tokens) {
    int i = 0;
    if (tokens[i].type != TokenType::TYPE || tokens[i].value != "int") return false;
    i++;
    if (tokens[i].type != TokenType::IDENTIFIER) return false;
    string funcName = tokens[i].value;
    i++;
    if (tokens[i].type != TokenType::PAREN_OPEN) return false;
    i++;

    vector<string> paramNames;
    while (tokens[i].type != TokenType::PAREN_CLOSE) {
        if (tokens[i].type != TokenType::TYPE) return false;
        i++;
        if (tokens[i].type != TokenType::IDENTIFIER) return false;
        paramNames.push_back(tokens[i].value);
        i++;
        if (tokens[i].type == TokenType::COMMA) i++;
    }
    i++;
    if (tokens[i].type != TokenType::BRACE_OPEN) return false;
    i++;
    if (tokens[i].type != TokenType::RETURN) return false;
    i++;

    string expr;
    while (tokens[i].type != TokenType::SEMICOLON) expr += tokens[i].value + " ", i++;
    i++;

    functionTable[funcName] = [expr, paramNames](vector<string> args) -> string {
        unordered_map<string, string> values;
        for (size_t j = 0; j < paramNames.size(); j++) values[paramNames[j]] = args[j];
        return evaluateExpression(expr, values);
    };
    cout << "Function '" << funcName << "' defined successfully.\n";
    return true;
}

// Initialize Built-in Functions
void initializeBuiltins() {
    // Existing built-in functions (keeping them as they are)
    functionTable["print"] = [](vector<string> args) { cout << args[0]; return ""; };
    functionTable["println"] = [](vector<string> args) { cout << args[0] << endl; return ""; };
    functionTable["read"] = [](vector<string> args) { string s; getline(cin, s); return s; };
    functionTable["readInt"] = [](vector<string> args) { string s; getline(cin, s); return to_string(stoi(s)); };
    functionTable["readFloat"] = [](vector<string> args) { string s; getline(cin, s); return s; };
    functionTable["readLine"] = [](vector<string> args) { string s; getline(cin, s); return s; };
    functionTable["isNull"] = [](vector<string> args) { return args[0].empty() ? "1" : "0"; };
    functionTable["typeOf"] = [](vector<string> args) {
        if (args[0].find('.') != string::npos) return "float";
        else if (isdigit(args[0][0]) || (args[0][0] == '-' && isdigit(args[0][1]))) return "int";
        return "string";
    };
    functionTable["getCurrentTime"] = [](vector<string> args) {
        auto now = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(now);
        stringstream ss; ss << put_time(localtime(&tt), "%H:%M:%S");
        return ss.str();
    };
    functionTable["getCurrentDate"] = [](vector<string> args) {
        auto now = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(now);
        stringstream ss; ss << put_time(localtime(&tt), "%Y-%m-%d");
        return ss.str();
    };
    functionTable["getDayOfWeek"] = [](vector<string> args) {
        tm tm = {};
        istringstream ss(args[0]); ss >> get_time(&tm, "%Y-%m-%d");
        mktime(&tm);
        return to_string(tm.tm_wday);
    };
    functionTable["getTimestamp"] = [](vector<string> args) {
        auto now = chrono::system_clock::now();
        return to_string(chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count());
    };
    functionTable["length"] = [](vector<string> args) { return to_string(args[0].size()); };
    functionTable["concat"] = [](vector<string> args) { return args[0] + args[1]; };
    functionTable["substring"] = [](vector<string> args) { return args[0].substr(stoi(args[1]), stoi(args[2])); };
    functionTable["touppercase"] = [](vector<string> args) {
        string s = args[0];
        for (char &c : s) c = toupper(c);
        return s;
    };
    functionTable["tolowercase"] = [](vector<string> args) {
        string s = args[0];
        for (char &c : s) c = tolower(c);
        return s;
    };
    functionTable["charAt"] = [](vector<string> args) { return string(1, args[0][stoi(args[1])]); };
    functionTable["indexOf"] = [](vector<string> args) { return to_string(args[0].find(args[1])); };
    functionTable["replace"] = [](vector<string> args) {
        string s = args[0];
        size_t pos = s.find(args[1]);
        if (pos != string::npos) s.replace(pos, args[1].size(), args[2]);
        return s;
    };
    functionTable["split"] = [](vector<string> args) {
        string s = args[0], delim = args[1], result;
        size_t pos = 0;
        while ((pos = s.find(delim)) != string::npos) {
            result += s.substr(0, pos) + " ";
            s.erase(0, pos + delim.length());
        }
        result += s;
        return result;
    };
    functionTable["trim"] = [](vector<string> args) {
        string s = args[0];
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        return s;
    };

    // New functions added below
    functionTable["equals"] = [](vector<string> args) { return (args[0] == args[1]) ? "1" : "0"; };
    functionTable["notequals"] = [](vector<string> args) { return (args[0] != args[1]) ? "1" : "0"; };
    functionTable["greaterThan"] = [](vector<string> args) { return (stoi(args[0]) > stoi(args[1])) ? "1" : "0"; };
    functionTable["lessThan"] = [](vector<string> args) { return (stoi(args[0]) < stoi(args[1])) ? "1" : "0"; };
    functionTable["min"] = [](vector<string> args) { return to_string(min(stoi(args[0]), stoi(args[1]))); };
    functionTable["max"] = [](vector<string> args) { return to_string(max(stoi(args[0]), stoi(args[1]))); };
    functionTable["swap"] = [](vector<string> args) { return args[1] + " " + args[0]; }; // Returns swapped values as space-separated string
    functionTable["iseven"] = [](vector<string> args) { return (stoi(args[0]) % 2 == 0) ? "1" : "0"; };
    functionTable["isodd"] = [](vector<string> args) { return (stoi(args[0]) % 2 != 0) ? "1" : "0"; };
    functionTable["startsWith"] = [](vector<string> args) { return (args[0].rfind(args[1], 0) == 0) ? "1" : "0"; };
    functionTable["endsWith"] = [](vector<string> args) {
        return (args[0].size() >= args[1].size() && 
                args[0].compare(args[0].size() - args[1].size(), args[1].size(), args[1]) == 0) ? "1" : "0";
    };
    functionTable["countOccurrences"] = [](vector<string> args) {
        int count = 0;
        size_t pos = 0;
        while ((pos = args[0].find(args[1], pos)) != string::npos) {
            count++;
            pos += args[1].length();
        }
        return to_string(count);
    };
    functionTable["reverseNumber"] = [](vector<string> args) {
        string s = args[0];
        reverse(s.begin(), s.end());
        return s;
    };
    functionTable["reverseString"] = [](vector<string> args) {
        string s = args[0];
        reverse(s.begin(), s.end());
        return s;
    };
    functionTable["factorial"] = [](vector<string> args) {
        int n = stoi(args[0]);
        long long result = 1;
        for (int i = 2; i <= n; i++) result *= i;
        return to_string(result);
    };
    functionTable["fibonacci"] = [](vector<string> args) {
        int n = stoi(args[0]);
        if (n <= 1) return to_string(n);
        long long a = 0, b = 1;
        for (int i = 2; i <= n; i++) {
            long long temp = a + b;
            a = b;
            b = temp;
        }
        return to_string(b);
    };
    functionTable["mod"] = [](vector<string> args) { return to_string(stoi(args[0]) % stoi(args[1])); };
    functionTable["pow"] = [](vector<string> args) { return to_string(static_cast<int>(pow(stoi(args[0]), stoi(args[1])))); };
    functionTable["sqrt"] = [](vector<string> args) { return to_string(static_cast<int>(sqrt(stof(args[0])))); };
    functionTable["cos"] = [](vector<string> args) { return to_string(cos(stof(args[0]))); };
    functionTable["sin"] = [](vector<string> args) { return to_string(sin(stof(args[0]))); };
    functionTable["ceil"] = [](vector<string> args) { return to_string(static_cast<int>(ceil(stof(args[0])))); };
    functionTable["floor"] = [](vector<string> args) { return to_string(static_cast<int>(floor(stof(args[0])))); };

}

// Execute Function Call (updated)
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

    vector<string> args;
    int i = 2;
    string currentArg;
    while (tokens[i].type != TokenType::PAREN_CLOSE) {
        if (tokens[i].type == TokenType::NUMBER || tokens[i].type == TokenType::IDENTIFIER) {
            currentArg += tokens[i].value;
            i++;
            if (tokens[i].type == TokenType::COMMA || tokens[i].type == TokenType::PAREN_CLOSE) {
                args.push_back(currentArg);
                currentArg = "";
            }
        } else {
            cerr << "Syntax Error: Expected argument.\n";
            return false;
        }
        if (tokens[i].type == TokenType::COMMA) i++;
    }

    string result = functionTable[funcName](args);
    if (!result.empty() && funcName != "print" && funcName != "println") cout << "Output: " << result << endl;
    return true;
}

// Main Function
int main() {
    initializeBuiltins();
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
