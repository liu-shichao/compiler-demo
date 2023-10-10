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
    std::cout << type << std::endl;
    return type;
  }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  std::string Dump() const override {
    std::string ret = "%entry:             // 入口基本块\n";
    ret += stmt->Dump();
    return ret;
  }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::string Dump() const override {
    std::string ret;
    if (exp->Value() == -1) {
      ret += "\tret ";
      ret += exp->Dump();
    } else {
      ret += exp->Dump();
      ret += '\n';
      ret += "\tret %";
      ret += std::to_string(exp->Value());
    }
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
        ret += exp->Dump();
      } else if (type == PrimaryExpType::NUMBER) {
        ret += std::to_string(number);
      }
      return ret;
    }
    int Value() const override {
      if (type == PrimaryExpType::EXP) {
        return exp->Value();
      } else if (type == PrimaryExpType::NUMBER) {
        return -1;
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
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    std::string Dump() const override {
      std::string ret;
      if (type == UnaryExpType::PRIMARY_EXP) {
        ret += primary_exp->Dump();
      } else if (type == UnaryExpType::UNARYOP_UNARYEXP) {
        if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::ADD) {
          ret += unary_exp->Dump();
        } else if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::MINUS) {
          ret += unary_exp->Dump(); 
          ret += "\n";       
          ret += "\t";  
          ret += "%" + std::to_string(Value()) + " = ";
          ret += "sub 0, %";
          ret += std::to_string(unary_exp->Value());
        } else if (((UnaryOpAST*)(unary_op.get()))->type== UnaryOpAST::UnaryOpType::NEGATION) {
          if (unary_exp->Value() == -1) {
          } else {
            ret += unary_exp->Dump();
            ret += "\n";
          }
          ret += "\t";
          ret += "%" + std::to_string(Value()) + " = ";
          if (unary_exp->Value() == -1) {
            ret += "eq ";
            ret += unary_exp->Dump();
          } else {
            ret += "eq %";
            ret += std::to_string(unary_exp->Value());
          }
          ret += ", 0";
        }
      }
      return ret;
    }
    int Value() const override {
      if (type == UnaryExpType::PRIMARY_EXP) {
        return primary_exp->Value();
      } else if (type == UnaryExpType::UNARYOP_UNARYEXP) {
        if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::ADD) {
          return unary_exp->Value();
        }
        return unary_exp->Value() + 1;
      }
    }
};


// ...
#endif