#include "ast.hpp"
#include "parser.hpp"
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

int main() {
    ASTEvaluator eval;

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

    eval(multi_table_stmt);

    return 0;
}
