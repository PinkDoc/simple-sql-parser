#pragma once

#include <map>
#include <iostream>

#include "sql_select.hpp"

// support select , insert , create, update

// select [attr_name_list] from [table_list] | where [cand_list] ;
// insert into [table_name] values (....);
// create table [table_name] ([attr_list])

enum class TokenType {
    SELECT_T,
    INSERT_T,
    CREATE_T,
    DELETE_T,
    UPDATE_T,
    FROM_T,
    WHERE_T,
    INTO_T,
    VALUES_T,
    TABLE_T,
    ID_T,

    PLUS_T,
    MINUS_T,
    FLOAT_T,
    INTEGER_T,
    DOT_T,

    ASSIGN_T,           // ‘=’
    EQ_T,
    NOT_EQ_T,
    GREATER_T,
    SMALLER_T,
    LBRACE_T,
    RBRACE_T,
    BAD_EXPR        // fail!
};

enum class ResultCode {
    SUCCESS,

};

class Parser {
private:
    using string = std::string;

    std::map<string , TokenType> tokens_;

    TokenType current_token_;
    Value current_value_;
    ResultCode rc_;

    char to_upper(char ch) {
        if (ch >= 'a' && ch <= 'z') {
            return ch - 'a' + 'A';
        }
        return ch;
    }

    bool is_blank(char ch) {
        return ch == ' ';
    }

    void skip_blank_space() {
        while (p_ < input_.size() && is_blank(input_[p_])) {
            p_++;
        }
    }

    std::string str_to_upper(const std::string& s) {
        std::string res{s};
        for (auto & ch : res) {
            if (ch >= 'a' && ch <= 'z' ) {
                ch = ch - 'a' + 'A';
            }
        }
        return res;
    }

    std::string str_to_upper(const std::string_view& s) {
        std::string res{s};
        for (auto & ch : res) {
            if (ch >= 'a' && ch <= 'z' ) {
                ch = ch - 'a' + 'A';
            }
        }
        return res;
    }

    // get tokens
    void next();
    std::string_view input_;
    size_t p_ = 0;
public:
    Parser();
    ~Parser() = default;

    void init(std::string_view input);
    ResultCode parse();

    // for test
    std::pair<std::any, TokenType> current_token() {
        return {current_value_, current_token_};
    }

};

Parser::Parser() {
    tokens_.emplace("SELECT", TokenType::SELECT_T);
    tokens_.emplace("INSERT", TokenType::INSERT_T);
    tokens_.emplace("UPDATE", TokenType::UPDATE_T);
    tokens_.emplace("CREATE", TokenType::CREATE_T);
    tokens_.emplace("DELETE", TokenType::DELETE_T);
}

void Parser::init(std::string_view input) {
    input_ = input;
}

ResultCode Parser::parse() {
    next();
}

// just like lex...
void Parser::next() {
    skip_blank_space();
    char ch = input_[p_];
    while (p_ < input_.size()) {
        switch (ch) {
            // id
            case 'a'...'z':
            case 'A'...'Z':
            case '_': {
                size_t end = p_;
                ch = input_[end];
                for (; (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' ; end++) {
                    ch = input_[end];
                }
                auto id = std::string_view{input_.data() + p_, end - 1 - p_};
                auto to_upper_id = str_to_upper(id);
                p_ = end;

                for (auto& token : tokens_) {
                    if (token.first == to_upper_id) {
                        current_token_ = token.second;
                        return;
                    }
                }

                current_value_ = id;
                current_token_ = TokenType::ID_T;
                return;
            } break;

            case '+': {
                current_token_ = TokenType::PLUS_T;
                p_++;
                return;
            } break;

            case '-': {
                current_token_ = TokenType::MINUS_T;
                p_++;
                return;
            } break;

            case '0'...'9': {
                bool minus = current_token_ == TokenType::MINUS_T;
                long num = 0;
                size_t end = p_;
                for (; input_[end] >= '0' && input_[end] <= '9' ; end++) {
                    num = input_[end] - '0' + num * 10;
                }
                p_ = end;
                current_token_ = TokenType::INTEGER_T;
                current_value_ = minus ? -num : num;
                return;
            } break;

            case '>': {
                current_token_ = TokenType::GREATER_T;
                p_++;
                return;
            } break;

            case '<': {
                if (p_ + 1 < input_.size() && input_[p_ + 1] == '>') {
                    current_token_ = TokenType::NOT_EQ_T;
                    p_ += 2;
                    return;
                }
                current_token_ = TokenType::SMALLER_T;
                p_++;
                return;
            } break;

            case '=': {
                current_token_ = TokenType::ASSIGN_T;
                p_++;
                return;
            } break;

        }
    }
}

