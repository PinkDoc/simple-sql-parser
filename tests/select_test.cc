#include "../sql_parser.hpp"
#include <cassert>

struct SelectTester {
  std::string input;
  Parser parser;

  void init();
  void do_test();
};

void SelectTester::init()
{
  std::string_view str{input.data(), input.size()};
  parser.init(str);
}

void SelectTester::do_test()
{
  auto rc = parser.parse();
  assert(rc == RC::SUCCESS);
  auto &query = parser.query();
  assert(parser.query_type() == QueryType::SELECT);
  auto &select = std::any_cast<Select &>(query);

  int index = 0;
  assert(select.select_lists_[index].attr_name_ == "name1");
  assert(select.select_lists_[index].table_name_ == "table1");
}

int main()
{
  SelectTester t;
  t.input = "Select table1.name1 , table2.name2 , name3 from table1 , table2, table3 where table1.id=3, table2.id=4, "
            "4=table3.id;";
  t.init();
  t.do_test();
}