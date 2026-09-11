#include "ast.hpp"
#include "evaluator.hpp"
#include "SimpleSQLParser.hpp"
#include <algorithm>


//Not an exhaustive list but this will do
std::unordered_set<std::string> SQLClauses = {"SELECT", "FROM", "WHERE", "GROUP BY", "HAVING", "ORDER BY"};
int MAX_CLAUSE_COUNT = 6;

std::string SimpleSQLParser::stringLowerCase(std::string s){
    std::string new_string = "";
    for (auto it = s.cbegin(); it != s.cend(); ++it){
        new_string.push_back(std::tolower(*it));
    }

    return new_string;
}


/**
 * @brief Turns a simple SQL query into a vector of tokens
 * @param query The SQL query
 * @return Vector of tokens
 */
std::vector<std::string> SimpleSQLParser::tokenize(std::string query){

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

    return tokens;

}

/**
 * @brief Helper function to parse the SELECT clause in an SQL query and store it into a SelectStatement struct
 * @param statement Unique pointer pointing to the SelectStatement struct
 * @param tokens Tokenized query
 * @return Void
 */
void SimpleSQLParser::SelectParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens){

    std::unique_ptr<ASTNode> current_column;
    for (auto it = tokens.cbegin() + 1; it != tokens.cend(); ++it){
        current_column = std::make_unique<ASTNode>(StringExpr{*it});
        statement->select_list.push_back(std::move(current_column));
    }

}

/**
 * @brief Helper function to parse the FROM clause in an SQL query and store it into a SelectStatement struct
 * @param statement Unique pointer pointing to the SelectStatement struct
 * @param tokens Tokenized query
 * @return Void
 */
void SimpleSQLParser::FromParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens){

    auto it = tokens.cbegin() + 1;
    std::unique_ptr<ASTNode> current_table;
    for (; it != tokens.cend(); ++it){
        current_table = std::make_unique<ASTNode>(StringExpr{*it});
        statement->table_list.push_back(std::move(current_table));
    }

}

/**
 * @brief Helper function to parse the WHERE clause in an SQL query and store it into a SelectStatement struct
 * @param statement Unique pointer pointing to the SelectStatement struct
 * @param tokens Tokenized query
 * @return Void
 */
void SimpleSQLParser::WhereParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens){
    //AS OF 9-10 THIS IS NOT DONE
    //This one will be a little different

    statement->where.emplace(); //Emplace since I now know it must exist
    const std::unordered_map<std::string, Operator> c_op = {
        {">", Operator::Greater}, 
        {"<", Operator::Less}, 
        {"=", Operator::Equal}, 
        {">=", Operator::Greater_equal}, 
        {"<=", Operator::Less_equal}, 
        {"!=", Operator::Not_Equal}
    };
    std::vector<BinaryExpr> expressions;

    //This loop looks for operations and then gets the left and right side of them.
    //It then creates a new BinaryExpr and stores it into the expression array
    for (auto it = tokens.cbegin(); it != tokens.cend(); ++it){
        if (c_op.find(*it) != c_op.end()){
            expressions.push_back(BinaryExpr{
                c_op.at(*it),
                ASTNode{StringExpr{*(it - 1)}},
                ASTNode{StringExpr{*(it + 1)}},
            });
        }
    }
    statement->where = std::move(expressions); //store expressions into statement

    
}


/**
 * @brief Helper function to parse the GROUP BY clause in an SQL query and store it into a SelectStatement struct
 * @param statement Unique pointer pointing to the SelectStatement struct
 * @param tokens Tokenized query
 * @return Void
 */
void SimpleSQLParser::GroupByParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens){
    
    if (tokens.empty()) return;

    size_t start_idx = 1;
    if (tokens[0] == "GROUP BY" || tokens[0] == "group by") {
        start_idx = 1;
    } else if (tokens.size() >= 2 && stringLowerCase(tokens[0]) == "group") {
        start_idx = 2;
    }

    for (size_t i = start_idx; i < tokens.size(); ++i) {
        statement->group_by.push_back(std::make_unique<ASTNode>(StringExpr{tokens[i]}));
    }
}

/**
 * @brief Helper function to parse the HAVING clause in an SQL query and store it into a SelectStatement struct
 * @param statement Unique pointer pointing to the SelectStatement struct
 * @param tokens Tokenized query
 * @return Void
 */
void SimpleSQLParser::HavingParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens){
    
    //All boolean operations
    const std::unordered_map<std::string, Operator> c_op = {
        {">", Operator::Greater}, 
        {"<", Operator::Less}, 
        {"=", Operator::Equal}, 
        {">=", Operator::Greater_equal}, 
        {"<=", Operator::Less_equal}, 
        {"!=", Operator::Not_Equal}
    };
    std::vector<std::unique_ptr<ASTNode>> expressions;

    //This loop looks for operations and then gets the left and right side of them.
    //It then creates a new BinaryExpr and stores it into the expression array
    for (auto it = tokens.cbegin() + 1; it != tokens.cend(); ++it){
        if (c_op.find(*it) != c_op.end()){
            std::unique_ptr<ASTNode> current_expression = std::make_unique<ASTNode>(
                std::make_unique<BinaryExpr>(
                    c_op.at(*it),
                    ASTNode{StringExpr{*(it - 1)}},
                    ASTNode{StringExpr{*(it + 1)}}
                )
            );
            expressions.push_back(std::move(current_expression));
        }
    }
    // store them into the statment struct
    statement->having = std::move(expressions);       
}

/**
 * @brief Helper function to parse the ORDER BY clause in an SQL query and store it into a SelectStatement struct
 * @param statement Unique pointer pointing to the SelectStatement struct
 * @param tokens Tokenized query
 * @return Void
 */
void SimpleSQLParser::OrderByParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens){
    if (tokens.empty()) return;

    size_t col_idx = 0;
    size_t dir_idx = 0;

    // Case 1: Merged token ["ORDER BY", "age", "ASC"]
    if (tokens[0] == "ORDER BY" || tokens[0] == "order by") {
        col_idx = 1;
        dir_idx = 2;
    } 
    // Case 2: Separate tokens ["ORDER", "BY", "age", "ASC"]
    else if (tokens.size() >= 2 && stringLowerCase(tokens[0]) == "order") {
        col_idx = 2;
        dir_idx = 3;
    } 
    else {
        return;
    }

    if (tokens.size() <= col_idx) return; // Out of bounds guard

    ASTNode column = StringExpr{tokens[col_idx]};
    bool asc = true;

    if (tokens.size() > dir_idx) {
        if (stringLowerCase(tokens[dir_idx]) == "desc") {
            asc = false;
        }
    }

    statement->order_by = std::make_pair(std::move(column), asc);
}

/**
 * @brief Function that parses the query tokens
 * @param tokens Tokenized query
 * @return The finalized SelectStatement struct object
 */
std::unique_ptr<SelectStatement> SimpleSQLParser::Parse(std::vector<std::string>& tokens){
    //Needs to be finished
    //UPDATE 9-7-2026: SO FAR 0 Error handling or testing and i'm assuming user input is correct
    auto result = std::make_unique<SelectStatement>(); //final result
    if (stringLowerCase(tokens[0]) != "select"){
        return nullptr; //error (add error message later)
    }

    //Groups each token to their each clause, so it is easier to construct an AST;
    std::vector<std::vector<std::string>> groups(MAX_CLAUSE_COUNT);
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

    // Dispatch clauses dynamically based on header token
    for (const auto& group : groups) {
        if (group.empty()) continue;
        std::string header = stringLowerCase(group[0]);

        if (header == "select") {
            SelectParse(result, group);
        } else if (header == "from") {
            FromParse(result, group);
        } else if (header == "where") {
            WhereParse(result, group);
        } else if (header == "group" || header == "group by") {
            GroupByParse(result, group);
        } else if (header == "having") {
            HavingParse(result, group);
        } else if (header == "order" || header == "order by") {
            OrderByParse(result, group);
        }
    }


    return result;
}
