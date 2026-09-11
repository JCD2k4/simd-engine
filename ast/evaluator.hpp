#pragma once
#include "ast.hpp"

struct ASTEvaluator {
    // Basic Expressions
    void operator()(const NumberExpr& n) const {
        std::cout << n.value;
    }

    void operator()(const StringExpr& s) const {
        std::cout << s.value;
    }

    void operator()(const BinaryExpr& b) const {
        std::cout << "(";
        std::visit(*this, b.left);
        std::cout << " " << OperatorToString(b.op) << " ";
        std::visit(*this, b.right);
        std::cout << ")";
    }

    void operator()(const std::unique_ptr<BinaryExpr>& b) const {
        if (b) {
            (*this)(*b);
        }
    }

    void operator()(const std::vector<BinaryExpr>& exprs) const {
        for (size_t i = 0; i < exprs.size(); ++i) {
            (*this)(exprs[i]);
            if (i + 1 < exprs.size()) {
                std::cout << " AND ";
            }
        }
    }

    // Helper functions for collection visitors
    void evaluateList(const std::vector<std::unique_ptr<ASTNode>>& list) const {
        for (size_t i = 0; i < list.size(); ++i) {
            if (list[i]) {
                std::visit(*this, *list[i]);
                if (i + 1 < list.size()) std::cout << ", ";
            }
        }
    }

    void evaluateNode(const ASTNode& node) const {
        std::visit(*this, node);
    }

    // Full Statement Evaluation
    void operator()(const SelectStatement& stmt) const {
        std::cout << "SELECT ";
        evaluateList(stmt.select_list);

        if (!stmt.table_list.empty()) {
            std::cout << "\nFROM ";
            evaluateList(stmt.table_list);
        }

        if (stmt.where.has_value() && !stmt.where->empty()) {
            std::cout << "\nWHERE ";
            for (size_t i = 0; i < stmt.where->size(); ++i) {
                (*this)((*stmt.where)[i]);
                if (i + 1 < stmt.where->size()) std::cout << " AND ";
            }
        }

        if (!stmt.group_by.empty()) {
            std::cout << "\nGROUP BY ";
            evaluateList(stmt.group_by);
        }

        if (stmt.having.has_value() && !stmt.having->empty()) {
            std::cout << "\nHAVING ";
            evaluateList(*stmt.having);
        }

        // Output ORDER BY if a target expression exists inside the variant
        std::cout << "\nORDER BY ";
        evaluateNode(stmt.order_by.first);
        std::cout << (stmt.order_by.second ? " ASC" : " DESC");
        std::cout << "\n";
    }

    static std::string OperatorToString(Operator op) {
        switch (op) {
            case Operator::Greater:       return ">";
            case Operator::Less:          return "<";
            case Operator::Equal:         return "=";
            case Operator::Greater_equal: return ">=";
            case Operator::Less_equal:    return "<=";
            case Operator::Not_Equal:     return "!=";
            case Operator::Add:           return "+";
            case Operator::Subtract:      return "-";
            case Operator::Multiply:      return "*";
            case Operator::Divide:        return "/";
            case Operator::And:           return "AND";
            case Operator::Or:            return "OR";
        }
        return "?";
    }
};