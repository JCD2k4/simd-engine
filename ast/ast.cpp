#include "ast.hpp"
#include "SimpleSQLParser.hpp"
#include "evaluator.hpp"

/*int main(){
    //ASTEvaluator eval = ASTEvaluator();
    std::unique_ptr<SelectStatement> tree = std::make_unique<SelectStatement>();
    SimpleSQLParser parser = SimpleSQLParser();
    auto tokens = parser.tokenize("SELECT name, age FROM employees WHERE age > 18     AND department = 'Engineering' GROUP BY department HAVING COUNT(*) > 10 ORDER BY age DESC;");
    std::unique_ptr<SelectStatement> result = parser.Parse(tokens);
    auto eval = ASTEvaluator();
    eval(result->where.value());
    //eval(tree);
    return 0;
}*/

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <optional>
#include <stdexcept>
#include "ast.hpp"

// Mock Row and Table structures for testing execution
using Value = std::variant<double, std::string>;
using Row = std::unordered_map<std::string, Value>;

struct Table {
    std::string name;
    std::vector<Row> rows;
};

// Evaluates AST expressions against real row data to extract/compute values
struct ASTExecutionEngine {

    // Evaluates a literal or column lookup to a concrete Value
    Value evaluateExpression(const ASTNode& node, const Row& row) const {
        return std::visit([this, &row](auto&& arg) -> Value {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, NumberExpr>) {
                return arg.value;
            } 
            else if constexpr (std::is_same_v<T, StringExpr>) {
                // Check if string is a column name in the row; if not, treat as string literal
                auto it = row.find(arg.value);
                if (it != row.end()) {
                    return it->second;
                }
                return arg.value; 
            } 
            else if constexpr (std::is_same_v<T, std::unique_ptr<BinaryExpr>>) {
                if (!arg) throw std::runtime_error("Null BinaryExpr pointer");
                return evaluateBinary(*arg, row);
            }
            throw std::runtime_error("Unknown AST Node type");
        }, node);
    }

    // Evaluates binary expressions (e.g., age > 18 or salary * 1.1)
    Value evaluateBinary(const BinaryExpr& expr, const Row& row) const {
        Value leftVal = evaluateExpression(expr.left, row);
        Value rightVal = evaluateExpression(expr.right, row);

        // Handle double comparisons/arithmetic
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            double l = std::get<double>(leftVal);
            double r = std::get<double>(rightVal);

            switch (expr.op) {
                case Operator::Add:           return l + r;
                case Operator::Subtract:      return l - r;
                case Operator::Multiply:      return l * r;
                case Operator::Divide:        return l / r;
                case Operator::Greater:       return l > r ? 1.0 : 0.0; // 1.0 = true
                case Operator::Less:          return l < r ? 1.0 : 0.0;
                case Operator::Equal:         return l == r ? 1.0 : 0.0;
                case Operator::Greater_equal: return l >= r ? 1.0 : 0.0;
                case Operator::Less_equal:    return l <= r ? 1.0 : 0.0;
                case Operator::Not_Equal:     return l != r ? 1.0 : 0.0;
                default: break;
            }
        }
        
        // Handle string comparisons
        if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            std::string l = std::get<std::string>(leftVal);
            std::string r = std::get<std::string>(rightVal);

            if (expr.op == Operator::Equal) return l == r ? 1.0 : 0.0;
            if (expr.op == Operator::Not_Equal) return l != r ? 1.0 : 0.0;
        }

        return 0.0; // False / default
    }

    // Executes a SelectStatement against a table and returns matching result rows
    std::vector<Row> executeSelect(const SelectStatement& stmt, const Table& table) const {
        std::vector<Row> result_rows;

        for (const auto& row : table.rows) {
            bool matches_where = true;

            // 1. Evaluate WHERE conditions against the row
            if (stmt.where.has_value()) {
                for (const auto& condition : stmt.where.value()) {
                    Value condResult = evaluateBinary(condition, row);
                    if (std::holds_alternative<double>(condResult) && std::get<double>(condResult) == 0.0) {
                        matches_where = false;
                        break;
                    }
                }
            }

            // 2. If row passes WHERE, select requested columns
            if (matches_where) {
                Row projected_row;
                for (const auto& col_node : stmt.select_list) {
                    Value val = evaluateExpression(*col_node, row);
                    
                    // Use string name if column reference, else auto-label
                    if (std::holds_alternative<StringExpr>(*col_node)) {
                        projected_row[std::get<StringExpr>(*col_node).value] = val;
                    } else {
                        projected_row["expr"] = val;
                    }
                }
                result_rows.push_back(projected_row);
            }
        }

        return result_rows;
    }
};

void run_data_execution_test() {
    std::cout << "========================================\n";
    std::cout << "TEST 8: Direct Statement Data Execution\n";
    std::cout << "========================================\n";

    // 1. Setup mock table data
    Table employees_table{
        "employees",
        {
            { {"name", std::string("Alice")}, {"age", 25.0}, {"department", std::string("Engineering")} },
            { {"name", std::string("Bob")},   {"age", 17.0}, {"department", std::string("Engineering")} },
            { {"name", std::string("Charlie")}, {"age", 30.0}, {"department", std::string("HR")} },
            { {"name", std::string("David")}, {"age", 22.0}, {"department", std::string("Engineering")} }
        }
    };

    // 2. Construct AST: SELECT name, age FROM employees WHERE age > 18 AND department = 'Engineering'
    SelectStatement stmt;
    stmt.select_list.push_back(std::make_unique<ASTNode>(StringExpr{"name"}));
    stmt.select_list.push_back(std::make_unique<ASTNode>(StringExpr{"age"}));
    stmt.table_list.push_back(std::make_unique<ASTNode>(StringExpr{"employees"}));

    std::vector<BinaryExpr> where_clauses;
    where_clauses.emplace_back(Operator::Greater, StringExpr{"age"}, NumberExpr{18.0});
    where_clauses.emplace_back(Operator::Equal, StringExpr{"department"}, StringExpr{"Engineering"});
    stmt.where = std::move(where_clauses);

    // 3. Execute AST against dataset
    ASTExecutionEngine engine;
    std::vector<Row> query_results = engine.executeSelect(stmt, employees_table);

    // 4. Output extracted values directly
    std::cout << "Query Results (" << query_results.size() << " rows returned):\n";
    for (size_t i = 0; i < query_results.size(); ++i) {
        std::cout << "Row " << i + 1 << ": ";
        
        std::string name = std::get<std::string>(query_results[i]["name"]);
        double age = std::get<double>(query_results[i]["age"]);
        
        std::cout << "Name = " << name << ", Age = " << age << "\n";
    }
}

void run_parser_test() {
    std::cout << "========================================\n";
    std::cout << "TEST 9: Parsing Raw SQL String\n";
    std::cout << "========================================\n";

    std::string sql = "SELECT name age FROM employees WHERE age > 18 ORDER BY age ASC";
    
    SimpleSQLParser parser;
    
    // 1. Test Tokenizer
    std::vector<std::string> tokens = parser.tokenize(sql);
    std::cout << "Tokens (" << tokens.size() << "): ";
    for (const auto& t : tokens) {
        std::cout << "[" << t << "] ";
    }
    std::cout << "\n\n";

    // 2. Test Parser AST Generation
    std::unique_ptr<SelectStatement> ast = parser.Parse(tokens);

    if (!ast) {
        std::cout << "Parsing failed: returned nullptr\n";
        return;
    }

    // 3. Print parsed AST using Evaluator
    ASTEvaluator evaluator;
    std::cout << "Reconstructed AST SQL:\n";
    evaluator(*ast);
    std::cout << "\n";
}

void run_complex_parser_test() {
    std::cout << "========================================\n";
    std::cout << "TEST 10: Parsing Aggregated Multi-Clause SQL\n";
    std::cout << "========================================\n";

    std::string sql = "SELECT department name FROM employees departments WHERE age >= 21 GROUP BY department HAVING count > 5 ORDER BY department DESC";
    
    SimpleSQLParser parser;
    
    // 1. Tokenize
    std::vector<std::string> tokens = parser.tokenize(sql);
    std::cout << "Tokens (" << tokens.size() << "): ";
    for (const auto& t : tokens) {
        std::cout << "[" << t << "] ";
    }
    std::cout << "\n\n";

    // 2. Parse into AST
    std::unique_ptr<SelectStatement> ast = parser.Parse(tokens);

    if (!ast) {
        std::cout << "Parsing failed: returned nullptr\n";
        return;
    }

    // 3. Reconstruct SQL using ASTEvaluator
    ASTEvaluator evaluator;
    std::cout << "Reconstructed AST SQL:\n";
    evaluator(*ast);
    std::cout << "\n";
}

void run_stress_parser_test() {
    std::cout << "========================================\n";
    std::cout << "TEST 11: Full Multi-Clause Engine Stress Test\n";
    std::cout << "========================================\n";

    // Clause order: SELECT, FROM, WHERE, GROUP BY, HAVING, ORDER BY
    std::string sql = "SELECT id name salary department FROM employees managers WHERE salary >= 50000 OR age != 30 GROUP BY department level HAVING performance_score <= 4 ORDER BY salary DESC";
    
    SimpleSQLParser parser;
    
    // 1. Tokenize
    std::vector<std::string> tokens = parser.tokenize(sql);
    std::cout << "Tokens (" << tokens.size() << "): ";
    for (const auto& t : tokens) {
        std::cout << "[" << t << "] ";
    }
    std::cout << "\n\n";

    // 2. Parse into AST
    std::unique_ptr<SelectStatement> ast = parser.Parse(tokens);

    if (!ast) {
        std::cout << "Parsing failed: returned nullptr\n";
        return;
    }

    // 3. Reconstruct SQL using ASTEvaluator
    ASTEvaluator evaluator;
    std::cout << "Reconstructed AST SQL:\n";
    evaluator(*ast);
    std::cout << "\n";
}

int main() {
    /*ASTEvaluator eval;

    std::cout << "========================================\n";
    std::cout << "TEST 1: Evaluating Raw AST Expressions\n";
    std::cout << "========================================\n";

    // Build: (age > 18)
    BinaryExpr condition1{
        Operator::Greater,
        StringExpr{"age"},
        NumberExpr{18}
    };

    std::cout << "Expression output: ";
    eval(condition1);
    std::cout << "\n\n";


    std::cout << "========================================\n";
    std::cout << "TEST 2: Full SelectStatement Execution\n";
    std::cout << "========================================\n";

    SelectStatement stmt;

    // SELECT name, age
    stmt.select_list.push_back(std::make_unique<ASTNode>(StringExpr{"name"}));
    stmt.select_list.push_back(std::make_unique<ASTNode>(StringExpr{"age"}));

    // FROM employees
    stmt.table_list.push_back(std::make_unique<ASTNode>(StringExpr{"employees"}));

    // WHERE age > 18 AND department = 'Engineering'
    std::vector<BinaryExpr> where_clauses;

    where_clauses.emplace_back(
        Operator::Greater,
        StringExpr{"age"},
        NumberExpr{18}
    );

    where_clauses.emplace_back(
        Operator::Equal,
        StringExpr{"department"},
        StringExpr{"'Engineering'"}
    );

    stmt.where = std::move(where_clauses);

    // GROUP BY department
    stmt.group_by.push_back(std::make_unique<ASTNode>(StringExpr{"department"}));

    // HAVING count > 10
    stmt.having = std::vector<std::unique_ptr<ASTNode>>{};
    stmt.having->push_back(
        std::make_unique<ASTNode>(
            std::make_unique<BinaryExpr>(
                Operator::Greater,
                StringExpr{"count"},
                NumberExpr{10}
            )
        )
    );

    // ORDER BY age DESC
    stmt.order_by = { StringExpr{"age"}, false };

    // Print full statement
    eval(stmt);

    std::cout << "\n========================================\n";
    std::cout << "TEST 3: Evaluating Where Vector Directly\n";
    std::cout << "========================================\n";

    if (stmt.where.has_value()) {
        std::cout << "WHERE clause list: ";
        eval(stmt.where.value());
        std::cout << "\n";
    }


    std::cout << "========================================\n";
    std::cout << "TEST 4: Deeply Nested Expressions\n";
    std::cout << "========================================\n";
    // Target SQL: ((salary * 1.1) > (bonus + 5000))
    
    // Left side: (salary * 1.1)
    ASTNode salary_mult = std::make_unique<BinaryExpr>(
        Operator::Multiply,
        StringExpr{"salary"},
        NumberExpr{1.1}
    );

    // Right side: (bonus + 5000)
    ASTNode bonus_add = std::make_unique<BinaryExpr>(
        Operator::Add,
        StringExpr{"bonus"},
        NumberExpr{5000}
    );

    // Parent binary node: Left > Right
    BinaryExpr deep_expr{
        Operator::Greater,
        std::move(salary_mult),
        std::move(bonus_add)
    };

    std::cout << "Nested Output: ";
    eval(deep_expr);
    std::cout << "\n\n";


    std::cout << "========================================\n";
    std::cout << "TEST 5: Minimal SELECT Statement\n";
    std::cout << "========================================\n";
    // Target SQL: SELECT 1 FROM users ORDER BY 1 ASC
    // Testing optional fields when they are empty/nullopt

    SelectStatement min_stmt;
    min_stmt.select_list.push_back(std::make_unique<ASTNode>(NumberExpr{1}));
    min_stmt.table_list.push_back(std::make_unique<ASTNode>(StringExpr{"users"}));
    min_stmt.order_by = { NumberExpr{1}, true }; // ASC = true

    eval(min_stmt);
    std::cout << "\n";


    std::cout << "========================================\n";
    std::cout << "TEST 6: Logical AND / OR Operators\n";
    std::cout << "========================================\n";
    // Target SQL: ((status = 'ACTIVE') OR (role = 'ADMIN'))

    ASTNode left_cond = std::make_unique<BinaryExpr>(
        Operator::Equal,
        StringExpr{"status"},
        StringExpr{"'ACTIVE'"}
    );

    ASTNode right_cond = std::make_unique<BinaryExpr>(
        Operator::Equal,
        StringExpr{"role"},
        StringExpr{"'ADMIN'"}
    );

    BinaryExpr logical_expr{
        Operator::Or,
        std::move(left_cond),
        std::move(right_cond)
    };

    std::cout << "Logical Output: ";
    eval(logical_expr);
    std::cout << "\n\n";


    std::cout << "========================================\n";
    std::cout << "TEST 7: Multiple Table Names & Wildcards\n";
    std::cout << "========================================\n";
    // Target SQL: SELECT * FROM orders, customers ORDER BY id ASC

    SelectStatement multi_table_stmt;
    multi_table_stmt.select_list.push_back(std::make_unique<ASTNode>(StringExpr{"*"}));
    multi_table_stmt.table_list.push_back(std::make_unique<ASTNode>(StringExpr{"orders"}));
    multi_table_stmt.table_list.push_back(std::make_unique<ASTNode>(StringExpr{"customers"}));
    multi_table_stmt.order_by = { StringExpr{"id"}, true };

    eval(multi_table_stmt);*/


    run_data_execution_test();
    run_parser_test();
    run_complex_parser_test();
    run_stress_parser_test();
    return 0;
}
