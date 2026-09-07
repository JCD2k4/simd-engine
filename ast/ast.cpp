#include <iostream>
#include <variant>
#include <memory>
#include <vector>
#include <optional>
#include <unordered_set>





struct NumberExpr;
struct StringExpr;
struct BinaryExpr; // +, -, *, /, >, <, etc
struct ASTEvaluator;
struct SelectStatement;
using ASTNode = std::variant<NumberExpr, StringExpr, std::unique_ptr<BinaryExpr>>;


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

struct StringExpr{
    std::string value;
};



struct BinaryExpr{
    Operator op;
    ASTNode left;
    ASTNode right;
};

struct SelectStatement{
    //General SQL statement
    std::vector<std::unique_ptr<ASTNode>> select_list; //SELECT
    std::vector<std::unique_ptr<ASTNode>> table_list; //FROM
    std::optional<std::vector<BinaryExpr>> where; //Where clause
    std::vector<ASTNode> group_by;
    std::optional<ASTNode> having;
    std::vector<std::pair<ASTNode, bool>> order_by; //Expression + asc/desc
};

struct ASTEvaluator{
    void operator()(const NumberExpr& n) {
        std::cout << "Number: " << n.value << "\n";
    }

    void operator()(const StringExpr& s) {
        std::cout << "String: " << s.value << "\n";
    }

    void operator()(const std::unique_ptr<BinaryExpr>& b) {
        //std::cout << "Binary op: " << b->op << "\n";
        //std::visit(*this, *b->left);
        //std::visit(*this, *b->right);
    }


    void evaluateList(const std::vector<std::unique_ptr<ASTNode>>& list) {
        for (const auto& nodePtr : list) {
            std::visit(*this, *nodePtr);
        }
    }
    

    void parser(std::vector<std::string>& tokens, std::unique_ptr<SelectStatement>& tree){
        
    }
};




struct SimpleSQLParser{

    //Not an exhaustive list but this will do
    std::unordered_set<std::string> SQLClauses = {"SELECT", "FROM", "WHERE", "GROUP BY", "HAVING", "ORDER BY"};
    int max_clause_count = 6;

    std::string stringLowerCase(std::string s){
        std::string new_string = "";
        for (auto it = s.cbegin(); it != s.cend(); ++it){
            new_string.push_back(std::tolower(*it));
        }

        return new_string;
    }



    std::vector<std::string> tokenize(std::string query){ //Prob just gonna change to operator overloading in the future

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
        /*for (auto it = tokens.cbegin(); it != tokens.cend(); ++it){
            std::cout << *it << '\n';
        }*/

        return tokens;

    }
    
    void SelectParse(std::unique_ptr<SelectStatement>& statement, std::vector<std::string>& tokens){

        auto it = tokens.cbegin() + 1;
        std::unique_ptr<ASTNode> current_column;
        for (; it != tokens.cend(); ++it){
            current_column = std::make_unique<ASTNode>(StringExpr{*it});
            statement->select_list.push_back(std::move(current_column));
        }

    }

    void FromParse(std::unique_ptr<SelectStatement>& statement, std::vector<std::string>& tokens){

        auto it = tokens.cbegin() + 1;
        std::unique_ptr<ASTNode> current_table;
        for (; it != tokens.cend(); ++it){
            current_table = std::make_unique<ASTNode>(StringExpr{*it});
            statement->table_list.push_back(std::move(current_table));
        }

    }

    void WhereParse(SelectStatement* statement, std::vector<std::string>& tokens){

        //This one will be a little different
        /*auto it = tokens.cbegin() + 1;
        std::unique_ptr<ASTNode> current_table;
        for (it; it != tokens.cend(); ++it){
            current_table = std::make_unique<ASTNode>(StringExpr{*it});
            statement->table_list.push_back(current_table);
        }*/
        
    }

    void GroupByParse(){}
    void HavingParse(){}
    void OrderByParse(){}


    std::unique_ptr<SelectStatement> Parse(std::vector<std::string>& tokens){
        //Needs to be finished
        //UPDATE 9-7-2026: SO FAR 0 Error handling or testing and i'm assuming user input is correct
        auto result = std::make_unique<SelectStatement>(); //final result
        if (stringLowerCase(tokens[0]) != "select"){
            return nullptr; //error (add error message later)
        }

        //Groups each token to their each clause, so it is easier to construct an AST;
        std::vector<std::vector<std::string>> groups(max_clause_count);
        std::vector<std::string> current_group = {"SELECT"};
        u_int8_t group_index = 0;
        for (auto it = tokens.begin() + 1; it != tokens.end(); ++it){
            if (SQLClauses.find(*it) == SQLClauses.end()){
                //This if statement below is for GROUP BY and ORDER BY since the two words get tokenized seperately
                if (((*it == "GROUP" || *it == "ORDER") && *(it + 1) == "BY")){ 
                    groups[group_index++] = current_group;
                    current_group = {*it + " " + *(it + 1)};
                    it++;
                }else{
                    current_group.push_back(*it);
                }
            }else{
                groups[group_index++] = current_group;
                //groups.push_back(current_group);
                current_group = {*it};
            }
        }
        groups[group_index++] = current_group;

        //Test Groups:
        /*int i = 1;
        for (auto group : groups){
            std::cout << "GROUP " << i << '\n';
            for (std::string token : group){
                std::cout << token << std::endl;
            }
            std::cout << "\n";
            i++;
        }*/


        //Testing evaluator
        //ASTEvaluator evaluator;

        //SELECT CLAUSE
        SelectParse(result, groups[0]);
        //evaluator.evaluateList(result->select_list);

        //FROM CLAUSE
        FromParse(result, groups[1]);
        
        //evaluator.evaluateList(result->table_list);
        //WHERE CLAUSE CHECK
        
        return result;
    }
};



int main(){
    //ASTEvaluator eval = ASTEvaluator();
    std::unique_ptr<SelectStatement> tree = std::make_unique<SelectStatement>();
    SimpleSQLParser parser = SimpleSQLParser();
    auto tokens = parser.tokenize("SELECT name, age FROM employees WHERE age > 18     AND department = 'Engineering' GROUP BY department HAVING COUNT(*) > 10 ORDER BY age DESC;");
    parser.Parse(tokens);
    //eval(tree);
    return 0;
}
