#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "ast.hpp"
#include "koopa.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
// 函数声明略
// ...
std::string Visit(const koopa_raw_slice_t &slice);
std::string Visit(const koopa_raw_function_t &func);
std::string Visit(const koopa_raw_basic_block_t &bb);
std::string Visit(const koopa_raw_value_t &value);

// 访问 raw program
std::string Visit(const koopa_raw_program_t &program) {
  std::string ret;
  ret += "\t.text\n";
  
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  ret += Visit(program.values);
  // 访问所有函数
  ret += Visit(program.funcs);
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
  retxx += "\tli a0, ";
  retxx += Visit(reinterpret_cast<koopa_raw_value_t>(ret.value));
  retxx += "\n";
  retxx += "\tret";
  return retxx;
}

std::string Visit(const koopa_raw_integer_t &integer) {
  std::cout << "Visit koopa_raw_integer_t" << std::endl;
  std::cout << integer.value << std::endl;
  return std::to_string(integer.value);
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
    default:
      // 其他类型暂时遇不到
      std::cout << "default" << std::endl;
      assert(false);
  }
  return ret;
}


// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  std::ofstream outf(output);

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto retxx = yyparse(ast);
  assert(!retxx);

  // dump AST
  std::string result = ast->Dump();
  cout << result << endl;
  // 解析字符串 str, 得到 Koopa IR 程序
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(result.c_str(), &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
 
 
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

 
  std::string ret_asm = Visit(raw);
  std::cout << "------------------" << std::endl;
  std::cout << ret_asm << std::endl;


  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);

  // 处理 raw program
  // ...

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);

  return 0;
}
