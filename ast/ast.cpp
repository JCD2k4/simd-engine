#include <iostream>
#include <variant>
#include <memory>
#include <vector>
#include <optional>

struct BinaryExpr; // +, -, *, /, >, <, etc
struct NumberExpr;
struct ColumnExpr;

using ASTNode = std::variant<NumberExpr, std::unique_ptr<BinaryExpr>>;

enum class Operator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Less,
    Greater,
    Less_equal,
    Greater_equal,
    And,
    Or
};




struct NumberExpr {
    double value;
};


struct BinaryExpr{
    Operator op;
    ASTNode left;
    ASTNode right;
};

struct SelectStatement{
    //General SQL statement
    std::vector<ASTNode> select_list; //SELECT
    std::string table_name; //FROM
    std::optional<BinaryExpr> where; //Where clause
    std::vector<ASTNode> group_by;
    std::optional<ASTNode> having;
    std::vector<std::pair<ASTNode, bool>> order_by; //Expression + asc/desc
};



struct ASTEvaluator{
    void operator()(const std::unique_ptr<SelectStatement>& query){
        std::cout << "HELLO WORLD\n";
    }

    void tokenize(std::string query){ //Prob just gonna change to operator overloading in the future

        //IMPORTANT: Add support for punctuation(i.e.(), ;, commas, etc. ). I'm just super lazy to do it right now
        std::vector<std::string> tokens;
        
        std::string current = "";
        for (auto it = query.cbegin(); it != query.cend(); it++){

            char c_char = *it;
            if (c_char == ' ' || c_char == '\n' || c_char == ',' || c_char == '\0'){
                if (!current.empty()){
                    tokens.push_back(current);
                    current = "";
                }
            }else{
                current.push_back(c_char);
            }

        }

        if (!current.empty()){
            tokens.push_back(current);
        }

        //test tokens;
        for (auto it = tokens.cbegin(); it != tokens.cend(); ++it){
            std::cout << *it << '\n';
        }

    }

    void parser(std::vector<std::string>& tokens, std::unique_ptr<SelectStatement>& tree){
        
    }
};

int main(){
    ASTEvaluator eval = ASTEvaluator();
    std::unique_ptr<SelectStatement> tree = std::make_unique<SelectStatement>();
    eval.tokenize("SELECT name, age FROM employees WHERE age > 18     AND department = 'Engineering' GROUP BY department HAVING COUNT(*) > 10 ORDER BY age DESC;");
    //eval(tree);
    return 0;
}
