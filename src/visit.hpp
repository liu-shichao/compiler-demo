#ifndef __VISIT_HPP__
#define __VISIT_HPP__

#include "koopa.h"

std::string Visit(const koopa_raw_slice_t &slice);
std::string Visit(const koopa_raw_function_t &func);
std::string Visit(const koopa_raw_basic_block_t &bb);
std::string Visit(const koopa_raw_value_t &value);
std::string Visit(const koopa_raw_program_t &program);
std::string convert_to_asm(std::string ir);
void free_koopa_program();

#endif