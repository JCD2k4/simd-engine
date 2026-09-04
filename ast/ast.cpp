#include <iostream>
#include <variant>
#include <memory>
#include <vector>

struct BinaryExpr; // +, -, *, /, >, <, etc
struct NumberExpr;
struct ClauseExpr;
using ASTNode = std::variant<NumberExpr, std::unique_ptr<BinaryExpr>, std::unique_ptr<ClauseExpr>>;

struct NumberExpr {
    double value;
};


struct BinaryExpr{
    char op;
    ASTNode left;
    ASTNode right;
};


struct ClauseExpr{
    const char* clause; //For example: WHERE clause in SQL
    const char* table_name; //In my implementation I use an array. IN SQL this would be a table
    BinaryExpr condition; //RHS
};

struct ASTEvaluator{
    void operator()(const NumberExpr& n) const {
        std::cout << n.value;
    }
    void operator()(const std::unique_ptr<BinaryExpr>& bin) const{
        std::cout << "(";
        std::visit(*this, bin->left);
        std::cout << " " << bin->op << " ";
        std::visit(*this, bin->right);
        std::cout << ")";
    }

    void operator()(const std::unique_ptr<ClauseExpr>& clause) const {

    }
};

int main(){

    //Some basic testing
    ASTNode tree = std::make_unique<BinaryExpr>(BinaryExpr{
        '+',
        NumberExpr{10},
        NumberExpr{5}
    });

    /*ASTNode tree2 = std::make_unique<ClauseExpr>(ClauseExpr{
        ""
    });*/

    std::visit(ASTEvaluator{}, tree);
    return 0;
}
