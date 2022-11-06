#include "../sql_parser.hpp"

#include <cassert>

class LexTester {
private:
  std::string input_;
  Parser parser_;

public:
  LexTester(std::string input)
  {
    input_ = std::move(input);
    parser_.init(std::string_view{input.data(), input.size()});
  }

  void do_test()
  {
    input_ = "SELECT <> -1234 fuck = , , , 4213.11 23.100 hello.ide hello hello.world";
    parser_.init(std::string_view{input_.data(), input_.size()});

    // select
    parser_.next();
    auto res = parser_.current_token();
    assert(res.second == TokenType::SELECT_T);

    // <>
    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::NOT_EQ_T);

    // -
    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::MINUS_T);

    // 1234
    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::INTEGER_T);
    assert(std::any_cast<long>(res.first) == -1234);

    // fuck
    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ID_T);
    assert(std::any_cast<std::string>(res.first) == std::string_view{"fuck"});

    // =
    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ASSIGN_T);

    // ,
    for (auto i = 0; i < 3; ++i) {
      parser_.next();
      res = parser_.current_token();
      assert(res.second == TokenType::COMMAS_T);
    }

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::FLOAT_T);
    //        assert(std::any_cast<float>(res.first) == 4213.11);

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::FLOAT_T);
    //        assert(std::any_cast<float>(res.first) == 23.100);

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ID_T);
    assert(std::any_cast<std::string>(res.first) == "hello");

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::DOT_T);

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ID_T);
    assert(std::any_cast<std::string>(res.first) == "ide");

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ID_T);
    assert(std::any_cast<std::string>(res.first) == "hello");

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ID_T);
    assert(std::any_cast<std::string>(res.first) == "hello");

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::DOT_T);

    parser_.next();
    res = parser_.current_token();
    assert(res.second == TokenType::ID_T);
    assert(std::any_cast<std::string>(res.first) == "world");
  }
};

int main()
{
  LexTester tester{"fuck"};
  tester.do_test();
}