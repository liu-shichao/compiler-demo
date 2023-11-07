#ifndef __AST_HPP__
#define __AST_HPP__

#include <memory>
#include <iostream>

class UnaryOpAST;

// 所有 AST 的基类
class BaseAST {
 public:
  int value_idx = - 1000;

  static int cur_tmp_reg_id;
  virtual bool IsValue() {
    return Value() == -1;
  };
  virtual ~BaseAST() = default;
  virtual std::string Dump() = 0;
  virtual int Value() {return value_idx;};
  static int makeTempRegId() {
    cur_tmp_reg_id ++;
    return cur_tmp_reg_id;
  }
  std::string GetChildExp(const std::unique_ptr<BaseAST>& child) const {
    std::string ret;
    if (!child->IsValue()) {
      ret += child->Dump();
      ret += ret == "" ? "" : "\n";
    }
    return ret;
  }
  std::string GetChildValue(const std::unique_ptr<BaseAST>& child) const {
    std::string ret;
    if (child->IsValue()) {
      ret += child->Dump();
    } else {
      ret += "%" + std::to_string(child->Value());
    }
    return ret;
  }
  virtual void calcValue() {
    value_idx = makeTempRegId();
  };
  std::string DumpBinaryExp(std::string op_str,
                        const std::unique_ptr<BaseAST>& left,
                        const std::unique_ptr<BaseAST>& right) {
    std::string ret;
    ret += GetChildExp(left);
    ret += GetChildExp(right);
    calcValue();
    ret += "\t%" + std::to_string(Value());
    ret += " = " + op_str + " ";
    ret += GetChildValue(left);
    ret += ", ";
    ret += GetChildValue(right);
    return ret;
  }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  std::string Dump() override {
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
  std::string Dump() override {
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
  std::string Dump() override {
    std::cout << type << std::endl;
    return type;
  }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  std::string Dump() override {
    std::string ret = "%entry:             // 入口基本块\n";
    ret += stmt->Dump();
    return ret;
  }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::string Dump() override {
    std::string ret;
    if (exp->IsValue()) {
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
  std::string Dump() override {
    std::string ret;
    ret += unary_exp->Dump();
    return ret;
  }
  int Value() override{
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
    std::string Dump() override {
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
    std::unique_ptr<BaseAST> exp;
    int number;
    std::string Dump() override {
      std::string ret;
      if (type == PrimaryExpType::EXP) {
        ret += exp->Dump();
      } else if (type == PrimaryExpType::NUMBER) {
        ret += std::to_string(number);
      }
      return ret;
    }
    int Value() override {
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
    std::string Dump() override {
      std::string ret;
      if (type == UnaryExpType::PRIMARY_EXP) {
        ret += primary_exp->Dump();
      } else if (type == UnaryExpType::UNARYOP_UNARYEXP) {
        if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::ADD) {
          ret += unary_exp->Dump();
        } else if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::MINUS) {
          ret += GetChildExp(unary_exp);
          calcValue();
          ret += "\t%" + std::to_string(Value()) + " = ";
          ret += "sub 0, ";
          ret += GetChildValue(unary_exp);
        } else if (((UnaryOpAST*)(unary_op.get()))->type== UnaryOpAST::UnaryOpType::NEGATION) {
          ret += GetChildExp(unary_exp);
          calcValue();
          ret += "\t%" + std::to_string(Value()) + " = ";
          ret += "eq ";
          ret += GetChildValue(unary_exp);
          ret += ", 0";
        }
      }
      return ret;
    }
    int Value() override {
      if (type == UnaryExpType::PRIMARY_EXP) {
        return primary_exp->Value();
      } else if (type == UnaryExpType::UNARYOP_UNARYEXP) {
        if (((UnaryOpAST*)(unary_op.get()))->type == UnaryOpAST::UnaryOpType::ADD) {
          return unary_exp->Value();
        }
        return value_idx;
      }
    }
};

class MulExpAST : public BaseAST {
  public:
    enum class MultExpType {
      UNARYEXP,
      MULEXP_MULT_UNARYEXP,
      MULEXP_DIV_UNARYEXP,
      MULEXP_MOD_UNARYEXP
    };
    MultExpType type;
    std::unique_ptr<BaseAST> unaryexp;
    std::unique_ptr<BaseAST> mulexp;
    // bool IsValue() const override {
    //   if (type == MultExpType::UNARYEXP) {
    //     return unaryexp->IsValue();
    //   } else {
    //     return false; 
    //   } 
    // }
    int Value() override {
      if (type == MultExpType::UNARYEXP) {
        return unaryexp->Value();
      } 
      return value_idx;
    }
    std::string Dump() override {
      std::string ret;
      if (type == MultExpType::UNARYEXP) {
        ret += unaryexp->Dump();
      } else if (type == MultExpType::MULEXP_MULT_UNARYEXP) {
        ret += DumpBinaryExp("mul", mulexp, unaryexp);
      } else if (type == MultExpType::MULEXP_DIV_UNARYEXP) {
        ret += DumpBinaryExp("div", mulexp, unaryexp);
      } else if (type == MultExpType::MULEXP_MOD_UNARYEXP) {
        ret += DumpBinaryExp("mod", mulexp, unaryexp);
      } 
      return ret;
    }
};

class AddExpAST : public BaseAST {
  public:
    enum class AddExpType {
      MULEXP,
      ADDEXP_ADD_MULEXP,
      ADDEXP_MINUS_MULEXP
    };
    AddExpType type;
    std::unique_ptr<BaseAST> mulexp;
    std::unique_ptr<BaseAST> addexp;
    int Value() override {
      if (type == AddExpType::MULEXP) {
        return mulexp->Value();
      } 
      return value_idx;
    }
    std::string Dump() override {
      std::string ret;
      if (type == AddExpType::MULEXP) {
        ret += mulexp->Dump();
      } else if (type == AddExpType::ADDEXP_ADD_MULEXP) {
        ret += DumpBinaryExp("add", addexp, mulexp);
      } else if (type == AddExpType::ADDEXP_MINUS_MULEXP) {
        ret += DumpBinaryExp("sub", addexp, mulexp);
      }
      return ret;
    }
};


class LOrExpAST : public BaseAST {
  public:
    enum class LOrExpType {
      LANDEXP,
      LOREXP_OR_LANDEXP
    };
    LOrExpType type;
    std::unique_ptr<BaseAST> landexp;
    std::unique_ptr<BaseAST> lorexp;

    int Value() override {
      if (type == LOrExpType::LANDEXP) {
        value_idx = landexp->Value();
      }
      return value_idx;
    }
    // void calcValue() {
    //   if (xxxxxx != -100) return;
    //   if (type == LOrExpType::LANDEXP) {
    //     xxxxxx = landexp->Value();
    //   std::cout << "xxxxxxssssssssxxxxxxxxxxxxxxxxxxx" << std::endl;
    //   } else {
    //     xxxxxx = makeTempRegId();
    //   std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;
    //   }
    // }
    std::string Dump() override {
      std::string ret;
      if (type == LOrExpType::LANDEXP) {
        ret += landexp->Dump();
      } else if (type == LOrExpType::LOREXP_OR_LANDEXP) {
        ret += GetChildExp(landexp);
        ret += GetChildExp(lorexp);
        value_idx = makeTempRegId();
        int temp_value_idx = value_idx;
        ret += "\t%" + std::to_string(value_idx) + " = ";
        ret += "or " + GetChildValue(landexp);
        ret += ", " + GetChildValue(lorexp) + "\n";

        value_idx = makeTempRegId();
        ret += "\t%" + std::to_string(value_idx);
        ret += " = ne %" + std::to_string(temp_value_idx);
        ret += ", 0";
      }
      return ret;
    }
};


class LAndExpAST : public BaseAST {
  public:
    enum class LAndExpType {
      EQEXP,
      LANDEXP_AND_EQEXP
    };
    LAndExpType type;
    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> landexp;
    int Value() override {
      if (type == LAndExpType::EQEXP) {
        return eqexp->Value();
      } 
      return value_idx;
    }
    std::string Dump() override {
      std::string ret;
      if (type == LAndExpType::EQEXP) {
        ret += eqexp->Dump();
      } else if (type == LAndExpType::LANDEXP_AND_EQEXP) {
        ret += GetChildExp(eqexp);
        ret += GetChildExp(landexp);
        
        int v1 = makeTempRegId();
        ret += "%" + std::to_string(v1) + " = ";
        ret += "ne " + GetChildValue(eqexp) + ", 0\n";

        int v2 = makeTempRegId();
        ret += "%" + std::to_string(v2) + " = ";
        ret += "ne " + GetChildValue(landexp) + ", 0\n";

        
        value_idx = makeTempRegId();
        ret += "%" + std::to_string(value_idx) + " = ";
        ret += "and %" + std::to_string(v1);
        ret += ", %" + std::to_string(v2);

      }
      return ret;
    }
};

class EqExpAST : public BaseAST {
  public:
    enum class EqExpType {
      RELEXP,
      EQEXP_EQ_RELEXP,
      EQEXP_NE_RELEXP
    };
    EqExpType type;
    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> relexp;
    int Value() override {
      if (type == EqExpType::RELEXP) {
        return relexp->Value();
      } else {
        return value_idx;
      }
    }
    std::string Dump() override {
      std::string ret;
      if (type == EqExpType::RELEXP) {
        ret += relexp->Dump();
      } else if (type == EqExpType::EQEXP_EQ_RELEXP) {
        ret += DumpBinaryExp("eq", eqexp, relexp);
      } else if (type == EqExpType::EQEXP_NE_RELEXP) {
        ret += DumpBinaryExp("ne", eqexp, relexp);
      }
      return ret;
    }
};



class RelExpAST : public BaseAST {
  public:
    enum class RelExpType {
      ADDEXP,
      RELEXP_LT_ADDEXP,
      RELEXP_GT_ADDEXP,
      RELEXP_LE_ADDEXP,
      RELEXP_GE_ADDEXP
    };
    RelExpType type;
    std::unique_ptr<BaseAST> addexp;
    std::unique_ptr<BaseAST> relexp;
    int Value() override {
      if (type == RelExpType::ADDEXP) {
        return addexp->Value();
      } else {
        return value_idx;
      }
    }
    std::string Dump() override {
      std::string ret;
      if (type == RelExpType::ADDEXP) {
        ret += addexp->Dump();
      } else if (type == RelExpType::RELEXP_LT_ADDEXP) {
        ret += DumpBinaryExp("lt", relexp, addexp);
      } else if (type == RelExpType::RELEXP_GT_ADDEXP) {
        ret += DumpBinaryExp("gt", relexp, addexp);
      } else if (type == RelExpType::RELEXP_LE_ADDEXP) {
        ret += DumpBinaryExp("le", relexp, addexp);
      } else if (type == RelExpType::RELEXP_GE_ADDEXP) {
        ret += DumpBinaryExp("ge", relexp, addexp);
      }
      return ret;
    }
};




// ...
#endif