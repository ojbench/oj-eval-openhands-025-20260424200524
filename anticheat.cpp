#include <cstdlib>
#include <iostream>
#include <sstream>
#include <set>
#include <unordered_map>
#include <cmath>

#include "lang.h"
#include "visitor.h"

// Calculate the "length" (complexity) of a program
class Length : public Visitor<int> {
 public:
  int visitProgram(Program *node) override {
    int l = 0;
    for (auto func : node->body) {
      l += visitFunctionDeclaration(func);
    }
    return l;
  }
  int visitFunctionDeclaration(FunctionDeclaration *node) override {
    return visitStatement(node->body);
  }

  int visitExpressionStatement(ExpressionStatement *node) override {
    return visitExpression(node->expr) + 1;
  }
  int visitSetStatement(SetStatement *node) override {
    return visitExpression(node->value) + 1;
  }
  int visitIfStatement(IfStatement *node) override {
    return visitExpression(node->condition) + visitStatement(node->body) + 1;
  }
  int visitForStatement(ForStatement *node) override {
    return visitStatement(node->init) + visitExpression(node->test) + 
           visitStatement(node->update) + visitStatement(node->body) + 1;
  }
  int visitBlockStatement(BlockStatement *node) override {
    int l = 0;
    for (auto stmt : node->body) {
      l += visitStatement(stmt);
    }
    return l;
  }
  int visitReturnStatement(ReturnStatement *node) override { return 1; }

  int visitIntegerLiteral(IntegerLiteral *node) override { return 1; }
  int visitVariable(Variable *node) override { return 1; }
  int visitCallExpression(CallExpression *node) override {
    int l = 1;
    for (auto expr : node->args) {
      l += visitExpression(expr);
    }
    return l;
  }
};

// Extract structural features (function call patterns, control flow)
class StructureExtractor : public Visitor<std::string> {
 public:
  std::string visitProgram(Program *node) override {
    std::string s = "P[";
    for (auto func : node->body) {
      s += visitFunctionDeclaration(func) + ";";
    }
    s += "]";
    return s;
  }
  
  std::string visitFunctionDeclaration(FunctionDeclaration *node) override {
    return "F(" + std::to_string(node->params.size()) + ")" + visitStatement(node->body);
  }

  std::string visitExpressionStatement(ExpressionStatement *node) override {
    return "E" + visitExpression(node->expr);
  }
  
  std::string visitSetStatement(SetStatement *node) override {
    return "S";
  }
  
  std::string visitIfStatement(IfStatement *node) override {
    return "I";
  }
  
  std::string visitForStatement(ForStatement *node) override {
    return "L";
  }
  
  std::string visitBlockStatement(BlockStatement *node) override {
    std::string s = "B{";
    for (auto stmt : node->body) {
      s += visitStatement(stmt) + ",";
    }
    s += "}";
    return s;
  }
  
  std::string visitReturnStatement(ReturnStatement *node) override {
    return "R";
  }

  std::string visitIntegerLiteral(IntegerLiteral *node) override {
    return "N";
  }
  
  std::string visitVariable(Variable *node) override {
    return "V";
  }
  
  std::string visitCallExpression(CallExpression *node) override {
    return "C(" + node->func + "," + std::to_string(node->args.size()) + ")";
  }
};

// Count different types of constructs
class ConstructCounter : public Visitor<std::unordered_map<std::string, int>> {
 private:
  std::unordered_map<std::string, int> counts;
  
 public:
  std::unordered_map<std::string, int> getCounts(Program *node) {
    counts.clear();
    visitProgram(node);
    return counts;
  }
  
  std::unordered_map<std::string, int> visitProgram(Program *node) override {
    for (auto func : node->body) {
      visitFunctionDeclaration(func);
    }
    return counts;
  }
  
  std::unordered_map<std::string, int> visitFunctionDeclaration(FunctionDeclaration *node) override {
    counts["functions"]++;
    visitStatement(node->body);
    return counts;
  }

  std::unordered_map<std::string, int> visitSetStatement(SetStatement *node) override {
    counts["sets"]++;
    visitExpression(node->value);
    return counts;
  }
  
  std::unordered_map<std::string, int> visitIfStatement(IfStatement *node) override {
    counts["ifs"]++;
    visitExpression(node->condition);
    visitStatement(node->body);
    return counts;
  }
  
  std::unordered_map<std::string, int> visitForStatement(ForStatement *node) override {
    counts["fors"]++;
    visitStatement(node->init);
    visitExpression(node->test);
    visitStatement(node->update);
    visitStatement(node->body);
    return counts;
  }
  
  std::unordered_map<std::string, int> visitBlockStatement(BlockStatement *node) override {
    counts["blocks"]++;
    for (auto stmt : node->body) {
      visitStatement(stmt);
    }
    return counts;
  }
  
  std::unordered_map<std::string, int> visitReturnStatement(ReturnStatement *node) override {
    counts["returns"]++;
    visitExpression(node->value);
    return counts;
  }

  std::unordered_map<std::string, int> visitCallExpression(CallExpression *node) override {
    counts["calls"]++;
    counts["call_" + node->func]++;
    for (auto expr : node->args) {
      visitExpression(expr);
    }
    return counts;
  }
  
  std::unordered_map<std::string, int> visitExpressionStatement(ExpressionStatement *node) override {
    visitExpression(node->expr);
    return counts;
  }
  
  std::unordered_map<std::string, int> visitIntegerLiteral(IntegerLiteral *node) override {
    return counts;
  }
  
  std::unordered_map<std::string, int> visitVariable(Variable *node) override {
    return counts;
  }
};

int getLength(Program *p) {
  return Length().visitProgram(p);
}

double calculateSimilarity(Program *prog1, Program *prog2) {
  // Multiple metrics for similarity
  
  // 1. Length-based similarity
  int len1 = getLength(prog1);
  int len2 = getLength(prog2);
  int lenDiff = abs(len1 - len2);
  double lenSim = 1.0 - std::min(1.0, lenDiff / 50.0);
  
  // 2. Structure-based similarity
  std::string struct1 = StructureExtractor().visitProgram(prog1);
  std::string struct2 = StructureExtractor().visitProgram(prog2);
  
  // Simple string similarity (Jaccard on bigrams)
  std::set<std::string> bigrams1, bigrams2;
  for (size_t i = 0; i + 1 < struct1.size(); i++) {
    bigrams1.insert(struct1.substr(i, 2));
  }
  for (size_t i = 0; i + 1 < struct2.size(); i++) {
    bigrams2.insert(struct2.substr(i, 2));
  }
  
  int intersection = 0;
  for (const auto& bg : bigrams1) {
    if (bigrams2.count(bg)) intersection++;
  }
  
  double structSim = 0.5;
  if (!bigrams1.empty() || !bigrams2.empty()) {
    int unionSize = bigrams1.size() + bigrams2.size() - intersection;
    structSim = (double)intersection / unionSize;
  }
  
  // 3. Construct count similarity
  auto counts1 = ConstructCounter().getCounts(prog1);
  auto counts2 = ConstructCounter().getCounts(prog2);
  
  std::set<std::string> allKeys;
  for (const auto& p : counts1) allKeys.insert(p.first);
  for (const auto& p : counts2) allKeys.insert(p.first);
  
  double countSim = 0.0;
  int keyCount = 0;
  for (const auto& key : allKeys) {
    int c1 = counts1[key];
    int c2 = counts2[key];
    if (c1 + c2 > 0) {
      countSim += 1.0 - (double)abs(c1 - c2) / (c1 + c2);
      keyCount++;
    }
  }
  if (keyCount > 0) countSim /= keyCount;
  
  // Combine metrics
  double finalSim = 0.3 * lenSim + 0.4 * structSim + 0.3 * countSim;
  
  // Map to [0, 1] range with 0.5 as neutral
  return 0.5 + 0.5 * finalSim;
}

int main() {
  // Read two programs
  Program *prog1 = scanProgram(std::cin);
  Program *prog2 = scanProgram(std::cin);
  
  // Read sample input
  std::string input;
  int c;
  while ((c = std::cin.get()) != EOF) {
    input += c;
  }

  // Optionally run the programs to compare outputs
  // (Not using this for now as it's expensive)
  
  // Calculate similarity
  double similarity = calculateSimilarity(prog1, prog2);
  
  // Clamp to [0, 1]
  if (similarity < 0) similarity = 0;
  if (similarity > 1) similarity = 1;
  
  std::cout << similarity << std::endl;
  return 0;
}
