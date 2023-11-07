#include "ast.hpp"

int BaseAST::cur_tmp_reg_id = -1;
std::unordered_map<std::string, int> BaseAST::symbol_table;