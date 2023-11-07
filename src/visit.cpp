#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <unordered_map>
#include "visit.hpp"

std::unordered_map<const koopa_raw_binary_t*, int> reg_id_map;
std::vector<int> g_used_ids(7, 0);

int getRegIdx(const koopa_raw_binary_t& binary) {
  const koopa_raw_binary_t* bp = &binary;
  if (reg_id_map.count(bp)) {
    return reg_id_map[bp];
  }
  return -1;
}
void setRegIdx(const koopa_raw_binary_t& binary, int idx) {
  const koopa_raw_binary_t* bp = &binary;
  reg_id_map[bp] = idx;
  g_used_ids[idx] = 1;
}

int makeOneRegId(std::vector<int>& used_ids) {
  for (int i = 0; i < 7; i ++) {
    if (!used_ids[i] && !g_used_ids[i]) {
      used_ids[i] = 1;
      return i;
    }
  }
  return -1;
}
std::string makeRegString(int id) {
  return "t" + std::to_string(id);
}

// 访问 raw program
std::string Visit(const koopa_raw_program_t &program) {
  std::string ret;
  ret += "\t.text\n";
  
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  std::cout << "------------2-------------" << std::endl;
  ret += Visit(program.values);
  std::cout << "-------------3------------" << std::endl;
  // 访问所有函数
  ret += Visit(program.funcs);
  std::cout << "-------------4------------" << std::endl;
  return ret;
}

// 访问 raw slice
std::string Visit(const koopa_raw_slice_t &slice) {
  std::string ret;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        std::cout << "KOOPA_RSIK_FUNCTION" << std::endl;
        ret += Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        std::cout << "KOOPA_RSIK_BASIC_BLOCK" << std::endl;
        ret += Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        std::cout << "KOOPA_RSIK_VALUE" << std::endl;
        ret += Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
  return ret;
}

// 访问函数
std::string Visit(const koopa_raw_function_t &func) {
  std::string ret;
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  ret += "\t.globl ";
  std::string main = func->name;
  main = main.substr(1);
  ret += main;
  ret += "\n";
  ret += main;
  ret +=  ":\n";
  ret += Visit(func->bbs);
  return ret;
}

// 访问基本块
std::string Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  
  std::cout << "koopa_raw_basic_block_t: " << bb->name << std::endl;
  return Visit(bb->insts);
}


std::string Visit(const koopa_raw_return_t &ret) {
  std::string retxx;
  std::cout << "Visit koopa_raw_return_t" << std::endl;
  if (ret.value->name == nullptr) std::cout << "name null." << std::endl;
  std::cout << "tag: " << ret.value->ty->tag << std::endl;
  retxx += ret.value->kind.tag == KOOPA_RVT_INTEGER ? "\tli a0, " : "\tmv a0, ";
  retxx += ret.value->kind.tag == KOOPA_RVT_INTEGER ? std::to_string(ret.value->kind.data.integer.value) : "t" + std::to_string(getRegIdx(ret.value->kind.data.binary));
  retxx += "\n";
  retxx += "\tret";
  return retxx;
}

std::string Visit(const koopa_raw_integer_t &integer) {
  std::cout << "Visit koopa_raw_integer_t" << std::endl;
  std::cout << integer.value << std::endl;
  return std::to_string(integer.value);
}

std::string get_op_value_str(const koopa_raw_value_t& value, std::vector<int>& used_ids) {
  std::string ret;
  if (value->kind.tag == KOOPA_RVT_BINARY) {
      int reg_id = getRegIdx(value->kind.data.binary);
      g_used_ids[reg_id] = 0;
      ret = "t" + std::to_string(reg_id);
  } else {
      int ret_reg_id = makeOneRegId(used_ids);
      ret = "t" + std::to_string(ret_reg_id);
  }
  return ret;
}

std::string get_sub_exp_str(const koopa_raw_value_t& value, std::string str_reg_id) {
  if (value->kind.tag == KOOPA_RVT_BINARY) return "";
  std::string ret;
  std::string str_value = std::to_string(value->kind.data.integer.value);
  ret += "\tli  " + str_reg_id + ", " + str_value + "\n";
  return ret;
}
void find_uesd_ids(const koopa_raw_binary_t& binary, std::vector<int>& used_ids) {
    if (binary.lhs->kind.tag == KOOPA_RVT_BINARY) {
      int reg_id = getRegIdx(binary.lhs->kind.data.binary);
      used_ids[reg_id] = 1;
    }
    if (binary.rhs->kind.tag == KOOPA_RVT_BINARY) {
      int reg_id = getRegIdx(binary.rhs->kind.data.binary);
      used_ids[reg_id] = 1;
    }
}
std::string binary_op(std::string op, const koopa_raw_binary_t& binary) {
  std::string ret;
  std::vector<int> used_ids(7, 0);
  find_uesd_ids(binary, used_ids);
  std::string str_l_reg_id = get_op_value_str(binary.lhs, used_ids);
  ret += get_sub_exp_str(binary.lhs, str_l_reg_id);
  std::string str_r_reg_id = get_op_value_str(binary.rhs, used_ids);
  ret += get_sub_exp_str(binary.rhs, str_r_reg_id);
  ret += "\t" + op +" " + str_l_reg_id + ", " + str_l_reg_id + ", " + str_r_reg_id + "\n";
  setRegIdx(binary, stoi(str_l_reg_id.substr(1)));
  return ret;
}

std::string Visit(const koopa_raw_binary_t& binary) {
  std::string ret;
  std::cout << binary.op << std::endl;
  if (binary.op == KOOPA_RBO_EQ) {
    ret += binary_op("xor", binary);
    std::string reg_id = makeRegString(getRegIdx(binary));
    ret += "\tseqz " + reg_id + ", " + reg_id + "\n";
  } else if (binary.op == KOOPA_RBO_NOT_EQ) {
    ret += binary_op("xor", binary);
    std::string reg_id = makeRegString(getRegIdx(binary));
    ret += "\tsnez " + reg_id + ", " + reg_id + "\n";
  } else if (binary.op == KOOPA_RBO_SUB) {
    ret += binary_op("sub", binary);
  } else if (binary.op == KOOPA_RBO_ADD) {
    ret += binary_op("add", binary);
  } else if (binary.op == KOOPA_RBO_MUL) {
    ret += binary_op("mul", binary);
  }else if (binary.op == KOOPA_RBO_DIV) {
    ret += binary_op("div", binary);
  }else if (binary.op == KOOPA_RBO_MOD) {
    ret += binary_op("rem", binary);
  } else if (binary.op == KOOPA_RBO_LT) {
    ret += binary_op("slt", binary);
  } else if (binary.op == KOOPA_RBO_GT) {
    ret += binary_op("sgt", binary);
  } else if (binary.op == KOOPA_RBO_LE) {
    ret += binary_op("sgt", binary);
    std::string reg_id = makeRegString(getRegIdx(binary));
    ret += "\tseqz " + reg_id + ", " + reg_id + "\n";
  // li    t0, 1
  // li    t1, 2
  // # 执行小于等于操作
  // sgt   t1, t0, t1
  // seqz  t1, t1
  } else if (binary.op == KOOPA_RBO_GE) {
    ret += binary_op("slt", binary);
    std::string reg_id = makeRegString(getRegIdx(binary));
    ret += "\tseqz " + reg_id + ", " + reg_id + "\n";
  // li    t0, 1
  // li    t1, 2
  // # 执行小于等于操作
  // sgt   t1, t0, t1
  // seqz  t1, t1
  } else if (binary.op == KOOPA_RBO_OR) {
    ret += binary_op("or", binary);
  } else if (binary.op == KOOPA_RBO_AND) {
    ret += binary_op("and", binary);
  }
  std::cout << "endl." << std::endl;

  return ret;
}
// 访问指令
std::string Visit(const koopa_raw_value_t &value) {
  std::string ret;
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      std::cout << "KOOPA_RVT_RETURN" << std::endl;
      ret = Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      std::cout << "KOOPA_RVT_INTEGER" << std::endl;
      ret = Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      // 访问 integer 指令
      std::cout << "KOOPA_RVT_BINARY "
      << ", kind.op: " << kind.data.binary.op
      << ", kind.lhs: " << kind.data.binary.lhs->kind.data.integer.value
      << ", lhs.tag: " << kind.data.binary.lhs->kind.tag
      << ", kind.rhs: " << kind.data.binary.rhs->kind.data.integer.value
      << ", rhs.tag: " << kind.data.binary.rhs->kind.tag << std::endl;
      ret = Visit(kind.data.binary);
      break;
      
    default:
      // 其他类型暂时遇不到
      std::cout << "untreated type: " << kind.tag << std::endl;
      assert(false);
  }
  return ret;
}

static koopa_program_t program;
static koopa_raw_program_builder_t builder;
std::string convert_to_asm(std::string ir) {
     // 解析字符串 str, 得到 Koopa IR 程序
    koopa_error_code_t ret = koopa_parse_from_string(ir.c_str(), &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错  
    // 创建一个 raw program builder, 用来构建 raw program
    builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    std::string ret_asm = Visit(raw);
    std::cout << "------------------" << std::endl;
    std::cout << ret_asm << std::endl;
    return ret_asm;
}

void free_koopa_program() {
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);
    koopa_delete_raw_program_builder(builder);
}