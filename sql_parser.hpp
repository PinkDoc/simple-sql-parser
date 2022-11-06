#pragma once

#include <iostream>
#include <map>

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
  STRING_T,

  COLON_T,
  DOT_T,     // '.'
  COMMAS_T,  // ','
  STAR_T,    // '*'
  ASSIGN_T,  // ‘=’
  NOT_EQ_T,
  GREATER_T,
  SMALLER_T,
  LBRACE_T,
  RBRACE_T,

  BAD_EXPR  // fail!
};

enum class QueryType { SELECT, CREATE_TABLE, INSERT, DELETE, UPDATE, NONE };

enum class RC { SUCCESS, SYNTAX_ERROR };

class Parser {
private:
  using string = std::string;

  std::map<string, TokenType> tokens_;

  std::any query_;
  QueryType query_type_;

  TokenType current_token_;
  Value current_value_;
  RC rc_;

  char to_upper(char ch)
  {
    if (ch >= 'a' && ch <= 'z') {
      return ch - 'a' + 'A';
    }
    return ch;
  }

  bool is_blank(char ch)
  {
    return ch == ' ' || ch == '\r' || ch == '\n';
  }

  void skip_blank_space()
  {
    while (p_ < input_.size() && is_blank(input_[p_])) {
      p_++;
    }
  }

  std::string str_to_upper(const std::string &s)
  {
    std::string res{s};
    for (auto &ch : res) {
      if (ch >= 'a' && ch <= 'z') {
        ch = ch - 'a' + 'A';
      }
    }
    return res;
  }

  std::string str_to_upper(const std::string_view &s)
  {
    std::string res{s};
    for (auto &ch : res) {
      if (ch >= 'a' && ch <= 'z') {
        ch = ch - 'a' + 'A';
      }
    }
    return res;
  }

  template <typename T>
  T get_current_value()
  {
    return std::any_cast<T>(current_value_);
  }

  /**
   * @brief select function
   **/
  RC parse_rel_attr();
  RC parse_select();
  RC parse_condition();
  RC parse_where_condition();
  RC parse_table_list();

  std::string_view input_;
  size_t p_ = 0;

public:
  Parser();

  ~Parser() = default;

  void init(std::string_view input);

  void next();

  RC parse();

  std::any &query()
  {
    return query_;
  }
  QueryType query_type() const
  {
    return query_type_;
  }

  // for test
  std::pair<std::any, TokenType> current_token()
  {
    return {current_value_, current_token_};
  }
};

Parser::Parser()
{
  tokens_.emplace("SELECT", TokenType::SELECT_T);
  tokens_.emplace("INSERT", TokenType::INSERT_T);
  tokens_.emplace("UPDATE", TokenType::UPDATE_T);
  tokens_.emplace("CREATE", TokenType::CREATE_T);
  tokens_.emplace("DELETE", TokenType::DELETE_T);

  tokens_.emplace("FROM", TokenType::FROM_T);
  tokens_.emplace("WHERE", TokenType::WHERE_T);
  tokens_.emplace("INTO", TokenType::INTO_T);
  tokens_.emplace("TABLE", TokenType::TABLE_T);
  tokens_.emplace("VALUES", TokenType::VALUES_T);
}

void Parser::init(std::string_view input)
{
  input_ = input;
  p_ = 0;
}

RC Parser::parse()
{
  // get first token
  bool stop = false;
  while (!stop) {
    next();
    switch (current_token_) {
      case TokenType::SELECT_T:
        query_type_ = QueryType::SELECT;
        query_ = Select{};
        return parse_select();
      case TokenType::DOT_T:
        break;
      default: {
        return RC::SYNTAX_ERROR;
      }
    }
  }
}

// lex
void Parser::next()
{
  skip_blank_space();
  char ch = input_[p_];
  while (p_ < input_.size()) {
    switch (ch) {
      // id
      case 'a' ... 'z':
      case 'A' ... 'Z':
      case '_': {
        size_t end = p_;
        for (; (input_[end] >= 'a' && input_[end] <= 'z') || (input_[end] >= 'A' && input_[end] <= 'Z') ||
               input_[end] == '_' || (input_[end] >= '0' && input_[end] <= '9');
             end++) {}

        auto id = std::string_view{input_.data() + p_, end - p_};
        auto to_upper_id = str_to_upper(id);
        char tmpch = input_[end];

        p_ = end;

        for (auto &token : tokens_) {
          if (token.first == to_upper_id) {
            current_token_ = token.second;
            return;
          }
        }

        current_value_ = string{id};
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

        // TODO support better float algo
      case '0' ... '9': {
        bool minus = current_token_ == TokenType::MINUS_T;
        long num = 0;
        size_t end = p_;
        for (; input_[end] >= '0' && input_[end] <= '9'; end++) {
          num = input_[end] - '0' + num * 10;
        }

        // float
        if (input_[end] == '.') {
          size_t old_end = end;
          end++;
          for (; input_[end] >= '0' && input_[end] <= '9'; end++) {}

          if (old_end == end - 1) {
            p_ = end;
            current_token_ = TokenType::BAD_EXPR;
            return;
          }

          string float_str{input_.data() + p_, end - p_ - 1};
          current_value_ = std::stof(float_str);
          current_token_ = TokenType::FLOAT_T;
          p_ = end;
          return;
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

      case ',': {
        current_token_ = TokenType::COMMAS_T;
        p_++;
        return;
      } break;

      case '*': {
        current_token_ = TokenType::STAR_T;
        p_++;
        return;
      } break;

      case '.': {
        current_token_ = TokenType::DOT_T;
        p_++;
        return;
      } break;

      case ';': {
        current_token_ = TokenType::COLON_T;
        p_++;
        return;
      } break;
      default:
        current_token_ = TokenType::BAD_EXPR;
        return;
    }
  }
}

RC Parser::parse_select()
{
  return parse_rel_attr();
}

RC Parser::parse_rel_attr()
{
  // (([tablename].)?[attr])* | \*
  auto &select = std::any_cast<Select &>(query_);
  while (true) {
    auto last_token = current_token_;
    next();
    switch (current_token_) {
      case TokenType::ID_T: {
        string tmp_name = std::move(get_current_value<string &>());
        next();
        if (current_token_ == TokenType::DOT_T) {
          next();
          if (current_token_ != TokenType::ID_T) {
            return RC::SYNTAX_ERROR;
          }
          RelAttr ra;
          ra.table_name_ = std::move(tmp_name);
          ra.attr_name_ = std::move(get_current_value<string &>());
          select.select_lists_.emplace_back(std::move(ra));
        } else if (current_token_ == TokenType::COMMAS_T) {
          RelAttr ra;
          ra.attr_name_ = std::move(tmp_name);
          select.select_lists_.emplace_back(std::move(ra));
        } else if (current_token_ == TokenType::FROM_T) {
          RelAttr ra;
          ra.attr_name_ = std::move(tmp_name);
          select.select_lists_.emplace_back(std::move(ra));
          return parse_table_list();
        } else {
          return RC::SYNTAX_ERROR;
        }
      } break;
      case TokenType::STAR_T: {
        RelAttr ra;
        ra.attr_name_ = "*";
        select.select_lists_.emplace_back(std::move(ra));
      } break;
      case TokenType::FROM_T: {
        if (select.select_lists_.empty()) {
          return RC::SYNTAX_ERROR;
        }
        return parse_table_list();
      } break;
      case TokenType::COMMAS_T: {
        if (last_token != TokenType::ID_T) {
          return RC::SYNTAX_ERROR;
        }
      } break;
      default:
        return RC::SYNTAX_ERROR;
    }
  }
}

RC Parser::parse_table_list()
{
  Select &s = std::any_cast<Select &>(query_);
  while (true) {
    auto last_token = current_token_;
    next();
    switch (current_token_) {
      case TokenType::ID_T: {
        if (last_token != TokenType::FROM_T && last_token != TokenType::COMMAS_T) {
          return RC::SYNTAX_ERROR;
        }
        s.tables_.emplace_back(std::move(get_current_value<string &>()));
      } break;
      case TokenType::WHERE_T:
        if (last_token != TokenType::ID_T) {
          return RC::SYNTAX_ERROR;
        }
        return parse_where_condition();
      case TokenType::COLON_T: {
        return last_token == TokenType::ID_T ? RC::SUCCESS : RC::SYNTAX_ERROR;
      } break;
      case TokenType::COMMAS_T: {
        if (last_token != TokenType::ID_T) {
          return RC::SYNTAX_ERROR;
        }
      } break;
      default:
        return RC::SYNTAX_ERROR;
    }
  }
}

// Current token is value | attrinfo
RC Parser::parse_condition()
{
  Condition c;
  Select &s = std::any_cast<Select &>(query_);
  if (current_token_ == TokenType::ID_T) {
    RelAttr attr;
    string tmp_name = get_current_value<string &>();
    next();
    switch (current_token_) {
      case TokenType::ASSIGN_T:
      case TokenType::NOT_EQ_T:
      case TokenType::GREATER_T:
      case TokenType::SMALLER_T:
        attr.attr_name_ = std::move(tmp_name);
        c.left_ = std::move(attr);
        break;
      case TokenType::DOT_T:
        next();
        if (current_token_ != TokenType::ID_T) {
          return RC::SYNTAX_ERROR;
        }
        attr.attr_name_ = std::move(get_current_value<string &>());
        attr.table_name_ = std::move(tmp_name);
        c.left_ = std::move(attr);
        next();
        break;
      default:
        return RC::SYNTAX_ERROR;
    }
    c.left_is_attr = true;
  } else {
    // normal value
    c.left_ = std::move(current_value_);
    c.left_is_attr = false;
    next();
  }

  switch (current_token_) {
    case TokenType::ASSIGN_T:
      c.cmp_ = CmpType::Equal;
      break;
    case TokenType::NOT_EQ_T:
      c.cmp_ = CmpType::Not_Equal;
      break;
    case TokenType::GREATER_T:
      c.cmp_ = CmpType::Greater;
      break;
    case TokenType::SMALLER_T:
      c.cmp_ = CmpType::Smaller;
      break;
    default:
      return RC::SYNTAX_ERROR;
  }

  next();
  // right
  if (current_token_ == TokenType::ID_T) {
    RelAttr attr;
    string tmp_name = get_current_value<string &>();

    if (p_ < input_.size() && input_[p_] == '.') {
      p_++;  // ok ? I feel bad because I don't use next().... :)
      next();
      if (current_token_ != TokenType::ID_T) {
        return RC::SYNTAX_ERROR;
      }
      attr.attr_name_ = std::move(get_current_value<string &>());
      attr.table_name_ = std::move(tmp_name);
    } else {
      attr.attr_name_ = std::move(tmp_name);
    }

    c.right_ = std::move(attr);
    c.right_is_attr = true;
  } else {
    // normal value
    c.right_ = std::move(current_value_);
    c.right_is_attr = false;
  }

  s.conditions_.emplace_back(std::move(c));

  return RC::SUCCESS;
}

// TODO when parse a wrong select.
RC Parser::parse_where_condition()
{
  while (true) {
    auto last_token = current_token_;
    next();
    switch (current_token_) {
      case TokenType::ID_T:
      case TokenType::STRING_T:
      case TokenType::FLOAT_T:
      case TokenType::INTEGER_T: {
        // TODO check last token
        if (parse_condition() != RC::SUCCESS) {
          return RC::SYNTAX_ERROR;
        }
      } break;
      case TokenType::COMMAS_T: {
        switch (last_token) {
          case TokenType::ID_T:
          case TokenType::STRING_T:
          case TokenType::FLOAT_T:
          case TokenType::INTEGER_T: {
            // do nothing
          } break;
          default:
            return RC::SYNTAX_ERROR;
        }
      } break;
      case TokenType::COLON_T:
        return RC::SUCCESS;
      default:
        return RC::SYNTAX_ERROR;
    }
  }
}
