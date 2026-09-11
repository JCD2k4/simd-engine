#pragma once

#include <iostream>
#include <variant>
#include <memory>
#include <vector>
#include <optional>
#include <unordered_set>
#include <unordered_map>


enum class Operator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Less,
    Greater,
    Less_equal,
    Greater_equal,
    Equal,
    Not_Equal,
    And,
    Or
};

struct NumberExpr {
    double value;
};

struct StringExpr {
    std::string value;
};

// Forward declare BinaryExpr so ASTNode can reference it via unique_ptr
struct BinaryExpr;

using ASTNode = std::variant<NumberExpr, StringExpr, std::unique_ptr<BinaryExpr>>;

struct BinaryExpr {
    Operator op;
    ASTNode left;
    ASTNode right;

    BinaryExpr(Operator op, ASTNode left, ASTNode right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
};

struct SelectStatement {
    std::vector<std::unique_ptr<ASTNode>> select_list;
    std::vector<std::unique_ptr<ASTNode>> table_list;
    std::optional<std::vector<BinaryExpr>> where; //should be an ASTNode vector
    std::vector<std::unique_ptr<ASTNode>> group_by;
    std::optional<std::vector<std::unique_ptr<ASTNode>>> having;
    std::optional<std::pair<ASTNode, bool>> order_by; //just one ORDER BY for now, might make it a vector of multiple conditions in the future
};