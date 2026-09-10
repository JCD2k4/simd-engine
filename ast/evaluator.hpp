#pragma once
#include "ast.hpp"

struct ASTEvaluator {
    void operator()(const NumberExpr& n) {
        std::cout << "Number: " << n.value << "\n";
    }

    void operator()(const StringExpr& s) {
        std::cout << "String: " << s.value << "\n";
    }

    std::string OperatorToString(Operator op) {
        switch (op) {
            case Operator::Greater:       return ">";
            case Operator::Less:          return "<";
            case Operator::Equal:         return "=";
            case Operator::Greater_equal: return ">=";
            case Operator::Less_equal:    return "<=";
            case Operator::Not_Equal:     return "!=";
        }
        return "?"; // unreachable if all enumerators are handled
    }

    void operator()(const BinaryExpr& b) {
        std::cout << "(";
        std::visit(*this, b.left);
        std::cout << " " << OperatorToString(b.op) << " ";
        std::visit(*this, b.right);
        std::cout << ")";
    }

    void operator()(const std::unique_ptr<BinaryExpr>& b) {
        if (b) {
            (*this)(*b);
        }
    }

    void operator()(const std::vector<BinaryExpr>& w) {
        for (const auto& expr : w) {
            (*this)(expr);
        }
    }

    void evaluateList(const std::vector<std::unique_ptr<ASTNode>>& list) {
        for (const auto& nodePtr : list) {
            std::visit(*this, *nodePtr);
        }
    }

};