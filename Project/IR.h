#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>

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


class CodeGenerator {
private:
    std::ofstream outputFile;
    std::stringstream functionDefinitions;
    std::stringstream mainFunctionBody;
    std::vector<std::string> functionNames;
    bool hasMainFunction;

public:
    CodeGenerator() : hasMainFunction(false) {}

    void startGeneration(const std::string& filename = "output.c") {
        outputFile.open(filename);
        if (!outputFile.is_open()) {
            fprintf(stderr, "Error: Could not open output file %s\n", filename.c_str());
            exit(EXIT_FAILURE);
        }

        // Standard header
        outputFile << "#include <stdio.h>\n\n";
        
        // Ensure main function is created
        startMainFunction();
    }

    void addFunctionDefinition(const std::string& functionName, bool hasReturnValue) {
        // Avoid duplicate function definitions
        if (std::find(functionNames.begin(), functionNames.end(), functionName) != functionNames.end()) {
            return;
        }
        functionNames.push_back(functionName);

        // Store function definition header
        if (hasReturnValue) {
            functionDefinitions << "double " << functionName << "() {\n";
        } else {
            functionDefinitions << "void " << functionName << "() {\n";
        }
    }

    void addFunctionStatement(const std::string& statement) {
        functionDefinitions << "    " << statement << "\n";
        fprintf(stderr, "addFunctionStatement : %s\n", statement.c_str());
    }

    void closeFunctionDefinition() {
        functionDefinitions << "}\n\n";
    }

    void startMainFunction() {
        if (!hasMainFunction) {
            mainFunctionBody.str(""); // Clear any existing content
            mainFunctionBody << "int main() {\n";
            hasMainFunction = true;
        }
    }

    void addMainStatement(const std::string& statement) {
        mainFunctionBody << "    " << statement << "\n";
        fprintf(stderr, "addMainStatement : %s\n", statement.c_str());
    }

    void finalize() {
        if (hasMainFunction) {
            mainFunctionBody << "    return 0;\n";
            mainFunctionBody << "}\n";
        }

        // Write function definitions first
        outputFile << functionDefinitions.str();

        // Write main function
        outputFile << mainFunctionBody.str();

        outputFile.close();
        
        // Print success message
        printf("Code generation completed. Output written to output.c\n");
    }

    void translatePrints(const std::string& str) {
        std::string statement = "printf(" + str + ");\n";
        if (!currentFunction.empty()) {
            addFunctionStatement(statement);
        } else {
            addMainStatement(statement);
        }
    }

    void translatePrintd(const std::string& expr) {
        std::string statement = "printf(\"%f\\n\", " + expr + ");\n";
        if (!currentFunction.empty()) {
            addFunctionStatement(statement);
        } else {
            addMainStatement(statement);
        }
    }

    void translateAssignment(const std::string& identifier, const std::string& expr) {
        std::string statement = "double " + identifier + " = " + expr + ";\n";
        if (!currentFunction.empty()) {
            addFunctionStatement(statement);
        } else {
            addMainStatement(statement);
        }
    }

    void translateReturn(const std::string& expr) {
        std::string statement = "return " + expr + ";\n";
        addFunctionStatement(statement);
    }

    void translateFunctionCall(const std::string& funcName) {
        std::string statement = funcName + "();\n";
        if (!currentFunction.empty()) {
            addFunctionStatement(statement);
        } else {
            addMainStatement(statement);
        }
    }
};

// Global code generator instance
static CodeGenerator codeGenerator;



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