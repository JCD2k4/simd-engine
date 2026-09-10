#include "ast.hpp"
#include "parser.hpp"
#include "evaluator.hpp"

int main(){
    //ASTEvaluator eval = ASTEvaluator();
    std::unique_ptr<SelectStatement> tree = std::make_unique<SelectStatement>();
    SimpleSQLParser parser = SimpleSQLParser();
    auto tokens = parser.tokenize("SELECT name, age FROM employees WHERE age > 18     AND department = 'Engineering' GROUP BY department HAVING COUNT(*) > 10 ORDER BY age DESC;");
    std::unique_ptr<SelectStatement> result = parser.Parse(tokens);
    auto eval = ASTEvaluator();
    //eval(result->where.value());
    //eval(tree);
    return 0;
}
