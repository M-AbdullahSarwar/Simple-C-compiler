#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <functional>

// Symbol table data structure
static std::map<std::string, double> globalSymbolTable;

// Function table data structure
struct FunctionBody {
    std::map<std::string, double> localSymbolTable;
    std::vector<std::function<void()>> statements;
    double returnValue;
    bool hasReturnValue;
};
static std::map<std::string, FunctionBody> functionTable;

// Current function being defined
static std::string currentFunction = "";

double performBinaryOperation(double lhs, double rhs, int op) {
    switch(op) {
        case '+': return lhs + rhs;
        case '-': return lhs - rhs;
        case '*': return lhs * rhs;
        case '/': return lhs / rhs;
        default: return 0;
    }
}

void print(const char* format, const char* value) {
    printf(format, value);
}

void print(const char* format, double value) {
    printf(format, value);
}

void setValueInSymbolTable(const char* id, double value) {
    std::string name(id);
    if (!currentFunction.empty()) {
        functionTable[currentFunction].localSymbolTable[name] = value;
    } else {
        globalSymbolTable[name] = value;
    }
}

double getValueFromSymbolTable(const char* id) {
    std::string name(id);
    if (!currentFunction.empty() && functionTable[currentFunction].localSymbolTable.find(name) != functionTable[currentFunction].localSymbolTable.end()) {
        return functionTable[currentFunction].localSymbolTable[name];
    } else if (globalSymbolTable.find(name) != globalSymbolTable.end()) {
        return globalSymbolTable[name];
    }
    return 0.0; // Default value for an identifier
}

void startFunctionDefinition(const char* id, bool hasReturnValue) {
    std::string name(id);
    currentFunction = name;
    functionTable[name] = FunctionBody();
    functionTable[name].returnValue = 0.0;
    functionTable[name].hasReturnValue = hasReturnValue;
}

void endFunctionDefinition() {
    currentFunction = "";
}

void addStatementToCurrentFunction(std::function<void()> statement) {
    if (!currentFunction.empty()) {
        functionTable[currentFunction].statements.push_back(statement);
    }
}

double callFunction(const char* id) {
    std::string name(id);
    if (functionTable.find(name) != functionTable.end()) {
        for (const auto& statement : functionTable[name].statements) {
            statement();
        }
        return functionTable[name].hasReturnValue ? functionTable[name].returnValue : 0.0;
    } else {
        printf("Error: Function '%s' not defined.\n", id);
        return 0.0;
    }
}

// Wrapper functions
void wrapReturn(double value) {
    if (!currentFunction.empty()) {
        functionTable[currentFunction].returnValue = value;
    } else {
        setValueInSymbolTable("__global_return", value);
    }
}

void wrapPrints(const char* str) {
    if (currentFunction.empty()) {
        print("%s\n", str);
    } else {
        addStatementToCurrentFunction([str]() { print("%s\n", str); });
    }
}

void wrapPrintd(double value) {
    if (currentFunction.empty()) {
        print("%f\n", value);
    } else {
        addStatementToCurrentFunction([value]() { print("%f\n", value); });
    }
}

void wrapAssignment(const char* id, double value) {
    std::string name(id);
    if (currentFunction.empty()) {
        globalSymbolTable[name] = value;
    } else {
        functionTable[currentFunction].localSymbolTable[name] = value;
        addStatementToCurrentFunction([name, value]() {
            functionTable[currentFunction].localSymbolTable[name] = value;
        });
    }
}

double getValueForIdentifier(const char* id) {
    return getValueFromSymbolTable(id);
}