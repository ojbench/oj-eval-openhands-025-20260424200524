#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>

#include "lang.h"
#include "transform.h"

class Cheat : public Transform {
 private:
  std::unordered_map<std::string, std::string> varMap;
  std::mt19937 rng;
  int counter = 0;
  
  std::string generateName() {
    // Generate more diverse variable names
    const char* prefixes[] = {"var", "tmp", "val", "x", "y", "z", "a", "b", "c", "data", "item", "elem"};
    const char* suffixes[] = {"_1", "_2", "_x", "_y", "_temp", "_val", "_aux", "_buf", "_ptr"};
    
    int prefix_idx = rng() % 12;
    int suffix_idx = rng() % 9;
    
    std::ostringstream oss;
    oss << prefixes[prefix_idx] << counter++ << suffixes[suffix_idx];
    return oss.str();
  }
  
  std::string getVarName(const std::string& original) {
    if (varMap.find(original) == varMap.end()) {
      varMap[original] = generateName();
    }
    return varMap[original];
  }
  
 public:
  Cheat() : rng(12345) {}
  
  Variable *transformVariable(Variable *node) override {
    return new Variable(getVarName(node->name));
  }
  
  // Add some dummy statements that don't affect functionality
  Statement *transformBlockStatement(BlockStatement *node) override {
    std::vector<Statement *> body;
    for (auto stmt : node->body) {
      body.push_back(transformStatement(stmt));
      
      // Occasionally add a dummy assignment
      if (rng() % 5 == 0) {
        // Create a dummy variable assignment that won't be used
        std::string dummyVar = generateName();
        body.push_back(new SetStatement(
          new Variable(dummyVar),
          new IntegerLiteral(0)
        ));
      }
    }
    return new BlockStatement(body);
  }
  
  // Transform integer literals slightly (add 0, multiply by 1, etc.)
  Expression *transformIntegerLiteral(IntegerLiteral *node) override {
    if (node->value == 0) {
      return new IntegerLiteral(0);
    }
    
    // Occasionally transform the literal
    int choice = rng() % 4;
    if (choice == 0) {
      // Just return as-is most of the time
      return new IntegerLiteral(node->value);
    } else if (choice == 1 && node->value > 0) {
      // Express as (value + 1) - 1
      return new CallExpression("-", {
        new CallExpression("+", {
          new IntegerLiteral(node->value),
          new IntegerLiteral(1)
        }),
        new IntegerLiteral(1)
      });
    } else {
      return new IntegerLiteral(node->value);
    }
  }
};

int main() {
  auto code = scanProgram(std::cin);
  auto cheat = Cheat().transformProgram(code);
  std::cout << cheat->toString();
  return 0;
}
