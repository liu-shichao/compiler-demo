#ifndef __AST_HPP__
#define __AST_HPP__

#include <iostream>

class UnaryOpAST;

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual std::string Dump() const = 0;
  virtual int Value() const {return 0;};
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  std::string Dump() const override {
    // std::cout << "CompUnitAST { ";
    std::string ret = func_def->Dump();
    return ret;
    // std::cout << " }";
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::string Dump() const override {
    std::string ret;
    ret = "fun @";
    ret += ident;
    ret += "(): ";
    ret += func_type->Dump();
    ret += " { // main 函数的定义\n";
    ret += block->Dump();
    ret += " }";
    return ret;
  }
};

class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  std::string Dump() const override {
    return type;
  }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  std::string Dump() const override {
    std::string ret = "%entry:             // 入口基本块\n";
    ret += "\t";
    ret += stmt->Dump();
    return ret;
  }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::string Dump() const override {
    std::string ret;
    ret += exp->Dump();
    ret += '\n';
    ret += "\tret $";
    ret += exp->Value();
    ret += '\n';
    return ret;
  }
};

class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> unary_exp;
  std::string Dump() const override {
    std::string ret;
    ret += unary_exp->Dump();
    return ret;
  }
  int Value() const override{
    return unary_exp->Value();
  }
};

class UnaryOpAST : public BaseAST {
  public:
    enum class UnaryOpType {
      ADD,
      MINUS,
      NEGATION
    };
    UnaryOpType type;
    std::string Dump() const override {
      std::string ret;
      if (type == UnaryOpType::ADD) {
        ret += "+";
      } else if (type ==  UnaryOpType::MINUS) {
        ret += "-";
      } else if (type ==  UnaryOpType::NEGATION) {
        ret += "!";
      }
      return ret;
    }
};

class PrimaryExpAST : public BaseAST {
  public:
    enum class PrimaryExpType {
      EXP,
      NUMBER
    };
    PrimaryExpType type;
    int idx;
    std::unique_ptr<BaseAST> exp;
    int number;
    std::string Dump() const override {
      std::string ret;
      if (type == PrimaryExpType::EXP) {
        ret += "$" + std::to_string(idx) + exp->Dump();
      } else if (type == PrimaryExpType::NUMBER) {
        ret += std::to_string(number);
      }
      return ret;
    }
    int Value() const override {
      if (type == PrimaryExpType::EXP) {
        return exp->Value() + 1;
      } else if (type == PrimaryExpType::NUMBER) {
        return 1;
      }
    }
};

class UnaryExpAST : public BaseAST {
  public:
    enum class UnaryExpType {
      PRIMARY_EXP,
      UNARYOP_UNARYEXP
    };
    UnaryExpType type;
    int idx;
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    std::string Dump() const override {
      std::string ret;
      if (type == UnaryExpType::PRIMARY_EXP) {
        if (((PrimaryExpAST*)(primary_exp.get()))->type == PrimaryExpAST::PrimaryExpType::EXP) {
          ret += "$" + std::to_string(primary_exp->Value());
        } else {
          ret += primary_exp->Dump();
        }
      } else if (type == UnaryExpType::UNARYOP_UNARYEXP) {
        if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::ADD) {
          ret += unary_exp->Dump();
        } else if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::MINUS) {
          ret += unary_exp->Dump();          
          ret += "$" + std::to_string(idx) + " = ";
          ret += "sub 0, $";
          ret += std::to_string(unary_exp->Value());
          ret += "\n";
        } else if (((UnaryOpAST*)(unary_op.get()))->type== UnaryOpAST::UnaryOpType::NEGATION) {
          ret += unary_exp->Dump();
          ret += "$" + std::to_string(idx) + " = ";
          ret += "eq $";
          ret += std::to_string(unary_exp->Value());
          ret += ", 0";
          ret += "\n";
        }
      }
      return ret;
    }
    int Value() const override {
      if (type == UnaryExpType::PRIMARY_EXP) {
        return primary_exp->Value();
      } else if (type == UnaryExpType::UNARYOP_UNARYEXP) {
        return unary_exp->Value();
      }
    }
};


// ...
#endif