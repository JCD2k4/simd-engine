#pragma once
#include "ast.hpp"
#include "evaluator.hpp"


struct SimpleSQLParser {
    std::unordered_set<std::string> SQLClauses = {"SELECT", "FROM", "WHERE", "GROUP BY", "HAVING", "ORDER BY"};
    int MAX_CLAUSE_COUNT = 6;

    std::string stringLowerCase(std::string s);
    std::vector<std::string> tokenize(std::string query);

    void SelectParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens);
    void FromParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens);
    void WhereParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens);
    void GroupByParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens);
    void HavingParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens);
    void OrderByParse(std::unique_ptr<SelectStatement>& statement, const std::vector<std::string>& tokens);

    std::unique_ptr<SelectStatement> Parse(std::vector<std::string>& tokens);
};