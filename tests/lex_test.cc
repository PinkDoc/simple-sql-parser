#include "sql_parser.hpp"

#include <cassert>

// test lex
class LexTester   {
private:
    std::string input_;
    Parser parser_;
public:
    LexTester(std::string input) {
        input_ = std::move(input);
        parser_.init(std::string_view{input.data(), input.size()});
    }

    void do_test() {
        input_ = "SELECT <> -1234";
        parser_.init(std::string_view{input_.data(), input_.size()});

        // select
        parser_.parse();
        auto res = parser_.current_token();
        assert(res.second == TokenType::SELECT_T);

        // <>
        parser_.parse();
        res = parser_.current_token();
        assert(res.second == TokenType::NOT_EQ_T);

        // -
        parser_.parse();
        res = parser_.current_token();
        assert(res.second == TokenType::MINUS_T);

        // 1234
        parser_.parse();
        res = parser_.current_token();
        assert(res.second == TokenType::INTEGER_T);
        assert(std::any_cast<long>(res.first) == -1234);
    }
};

int main() {
    LexTester tester{"fuck"};
    tester.do_test();
}