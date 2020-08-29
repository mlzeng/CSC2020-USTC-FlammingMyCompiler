#ifndef SYSYC_INSTRUCTIONS_H
#define SYSYC_INSTRUCTIONS_H

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

const int imm_16_max = 65535;
const int imm_12_max = 4095;
const int imm_8_max = 255;

namespace InstGen {

const std::string spaces = std::string(4, ' ');
const std::string newline = "\n";

const std::string reg_name[] = {"r0",  "r1", "r2", "r3", "r4",  "r5",
                                "r6",  "r7", "r8", "r9", "r10", "r11",
                                "r12", "sp", "lr", "pc"};

const int max_reg_id = 15;

enum CmpOp {
  EQ, // ==
  NE, // !=
  GT, // >
  GE, // >=
  LT, // <
  LE, // <=
  NOP
};

class Value {
public:
  virtual bool is_reg() const = 0;
  virtual bool is_constant() const = 0;
  virtual bool has_shift() const = 0;
  virtual std::string getName() const = 0;
};

class Reg : public Value {
  int id;

public:
  explicit Reg(int id) : id(id) {
    if (id < 0 || id > max_reg_id) {
      std::cerr << "Invalid Reg ID!" << std::endl;
      abort();
    }
  }
  bool is_reg() const { return true; }
  bool is_constant() const { return false; }
  bool has_shift() const { return false; }
  int getID() const { return this->id; }
  std::string getName() const { return reg_name[id]; }
  const bool operator<(const Reg &rhs) const { return this->id < rhs.id; }
  const bool operator==(const Reg &rhs) const { return this->id == rhs.id; }
  const bool operator!=(const Reg &rhs) const { return this->id != rhs.id; }
};

class RegShift : public Value {
public:
  enum ShiftType { lsl, lsr, asl, asr };

private:
  int id;
  int shift;
  ShiftType _t;

public:
  explicit RegShift(int id, int shift, ShiftType _t = ShiftType::lsl)
      : id(id), shift(shift), _t(_t) {
    if (id < 0 || id > max_reg_id) {
      std::cerr << "Invalid Reg ID!" << std::endl;
      abort();
    }
    if (shift < 0 || shift > 31) {
      std::cerr << "Invalid Reg shift!" << std::endl;
      abort();
    }
  }
  bool is_reg() const { return true; }
  bool is_constant() const { return false; }
  bool has_shift() const { return true; }
  int getID() const { return this->id; }
  int getShift() const { return this->shift; }
  std::string getName() const {
    std::string shift_str;
    switch (this->_t) {
    case ShiftType::lsl:
      shift_str = "lsl";
      break;
    case ShiftType::asl:
      shift_str = "asl";
      break;
    case ShiftType::lsr:
      shift_str = "lsr";
      break;
    case ShiftType::asr:
      shift_str = "asr";
      break;
    default:
      break;
    }
    return reg_name[id] + ", " + shift_str + " " + "#" +
           std::to_string(this->getShift());
  }
  const bool operator<(const RegShift &rhs) const { return this->id < rhs.id; }
  const bool operator==(const RegShift &rhs) const {
    return this->id == rhs.id;
  }
  const bool operator!=(const RegShift &rhs) const {
    return this->id != rhs.id;
  }
};

const Reg sp = Reg(13);
const Reg lr = Reg(14);
const Reg pc = Reg(15);

class Addr {
  Reg reg;
  int offset;

public:
  explicit Addr(Reg reg, int offset) : reg(reg), offset(offset) {}
  Reg getReg() const { return this->reg; }
  int getOffset() const { return this->offset; }
  std::string getName() const {
    return "[" + reg.getName() + ", " + "#" + std::to_string(this->offset) +
           "]";
  }
};

class Constant : public Value {
  int value;

public:
  explicit Constant(int value) : value(value) {}
  bool is_reg() const { return false; }
  bool is_constant() const { return true; }
  bool has_shift() const { return false; }
  int getValue() const { return this->value; }
  std::string getName() const { return "#" + std::to_string(this->value); }
};

class Label {
  std::string label;
  int offset;

public:
  explicit Label(std::string label, int offset)
      : label(label), offset(offset) {}
  explicit Label(std::string label) : label(label), offset(0) {}
  std::string getName() const { return label + "+" + std::to_string(offset); }
};

std::string condCode(const CmpOp &cond);
std::string push(const std::vector<Reg> &reg_list);
std::string pop(const std::vector<Reg> &reg_list);
std::string mov(const Reg &target, const Value &source, const CmpOp &op = NOP);
std::string mvn(const Reg &target, const Value &source, const CmpOp &op = NOP);
std::string setValue(const Reg &target, const Constant &source);
std::string getAddress(const Reg &target, const Label &source);
std::string ldr(const Reg &target, const Addr &source);
std::string ldr(const Reg &target, const Label &source);
std::string ldr(const Reg &target, const Reg &base, const Reg &offset);
std::string ldr(const Reg &target, const Reg &base, const Reg &offset,
                const Constant &shift);
std::string str(const Reg &source, const Addr &target);
std::string str(const Reg &source, const Label &target);
std::string str(const Reg &target, const Reg &base, const Reg &offset);
std::string str(const Reg &target, const Reg &base, const Reg &offset,
                const Constant &shift);
std::string store(const Reg &source, const Reg &base, const Reg &offset);
std::string bl(const std::string &target_func_name);
std::string add(const Reg &target, const Reg &op1, const Value &op2);
std::string sub(const Reg &target, const Reg &op1, const Value &op2);
std::string rsb(const Reg &target, const Reg &op1, const Value &op2);
std::string and_(const Reg &target, const Reg &op1, const Value &op2);
std::string orr(const Reg &target, const Reg &op1, const Value &op2);
std::string eor(const Reg &target, const Reg &op1, const Value &op2);
std::string clz(const Reg &target, const Reg &op1);
std::string lsl(const Reg &target, const Reg &op1, const Value &op2);
std::string asl(const Reg &target, const Reg &op1,
                const Value &op2); // same as lsl
std::string lsr(const Reg &target, const Reg &op1, const Value &op2);
std::string asr(const Reg &target, const Reg &op1, const Value &op2);
std::string mul(const Reg &target, const Reg &op1, const Reg &op2);
std::string smmul(const Reg &target, const Reg &op1, const Reg &op2);
std::string mla(const Reg &target, const Reg &op1, const Reg &op2,
                const Reg &op3);
std::string smmla(const Reg &target, const Reg &op1, const Reg &op2,
                  const Reg &op3);
std::string mls(const Reg &target, const Reg &op1, const Reg &op2,
                const Reg &op3);
std::string smull(const Reg &target, const Reg &op1, const Reg &op2,
                  const Reg &op3);
std::string sdiv(const Reg &target, const Reg &op1, const Reg &op2);
std::string cmp(const Reg &lhs, const Value &rhs);
std::string b(const Label &target, const CmpOp &op = NOP);
std::string instConst(std::string (*inst)(const Reg &target, const Reg &op1,
                                          const Value &op2),
                      const Reg &target, const Reg &op1, const Constant &op2);
std::string instConst(std::string (*inst)(const Reg &op1, const Value &op2),
                      const Reg &op1, const Constant &op2);
std::string load(const Reg &target, const Addr &source);
std::string store(const Reg &source, const Addr &target);
std::string swi(const Constant &id);
std::string bic(const Reg &target, const Reg &v1, const Reg &v2, const Reg &v3);
std::string vvmul(const Reg &sum, const Reg &v1, const Reg &v2, const int len);
std::tuple<int, int, int> choose_multiplier(int d, int N);
std::string divConst(const Reg &target, const Reg &source,
                     const Constant &divisor);

}; // namespace InstGen

const InstGen::Reg vinst_temp_reg = InstGen::Reg(11);

#endif