#include "InstructionsGen.hh"

#include <cmath>
#include <tuple>

namespace InstGen {

std::string condCode(const CmpOp &cond) {
  std::string asm_code;
  switch (cond) {
  case EQ:
    asm_code += "eq";
    break;
  case NE:
    asm_code += "ne";
    break;
  case GT:
    asm_code += "gt";
    break;
  case GE:
    asm_code += "ge";
    break;
  case LT:
    asm_code += "lt";
    break;
  case LE:
    asm_code += "le";
    break;
  default:
    break;
  }
  return asm_code;
}

std::string push(const std::vector<Reg> &reg_list) {
  std::string asm_code;
  bool flag = false;
  asm_code += spaces;
  asm_code += "push";
  asm_code += " ";
  asm_code += "{";
  for (auto &i : reg_list) {
    if (flag) {
      asm_code += ", ";
    }
    asm_code += i.getName();
    flag = true;
  }
  asm_code += "}";
  asm_code += newline;
  return asm_code;
}

std::string pop(const std::vector<Reg> &reg_list) {
  std::string asm_code;
  bool flag = false;
  asm_code += spaces;
  asm_code += "pop";
  asm_code += " ";
  asm_code += "{";
  for (auto &i : reg_list) {
    if (flag) {
      asm_code += ", ";
    }
    asm_code += i.getName();
    flag = true;
  }
  asm_code += "}";
  asm_code += newline;
  return asm_code;
}

std::string mov(const Reg &target, const Value &source, const CmpOp &cond) {
  std::string asm_code;
  if (source.is_reg() && target.getName() == source.getName()) {
    return asm_code;
  }
  asm_code += spaces;
  asm_code += "mov";
  asm_code += condCode(cond);
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += source.getName();
  asm_code += newline;
  return asm_code;
}

std::string mvn(const Reg &target, const Value &source, const CmpOp &cond) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "mvn";
  asm_code += condCode(cond);
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += source.getName();
  asm_code += newline;
  return asm_code;
}

std::string setValue(const Reg &target, const Constant &source) {
  std::string asm_code;
  auto val = source.getValue();
  if (0 <= val && val <= imm_16_max) {
    asm_code += mov(target, Constant(val));
  } else if (-imm_8_max <= val && val <= 0) {
    asm_code += mvn(target, Constant(-val - 1));
  } else {
    uint32_t imm = source.getValue();
    uint32_t imm_low = imm & ((1 << 16) - 1);
    uint32_t imm_high = imm >> 16;
    asm_code += spaces;
    asm_code += "movw";
    asm_code += " ";
    asm_code += target.getName();
    asm_code += ", ";
    asm_code += "#" + std::to_string(imm_low);
    asm_code += newline;
    asm_code += spaces;
    asm_code += "movt";
    asm_code += " ";
    asm_code += target.getName();
    asm_code += ", ";
    asm_code += "#" + std::to_string(imm_high);
    asm_code += newline;
  }
  return asm_code;
}

std::string getAddress(const Reg &target, const Label &source) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "adrl";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += source.getName();
  asm_code += newline;
  return asm_code;
}

std::string ldr(const Reg &target, const Addr &source) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "ldr";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += source.getName();
  asm_code += newline;
  return asm_code;
}

std::string ldr(const Reg &target, const Label &source) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "ldr";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += source.getName();
  asm_code += newline;
  return asm_code;
}

std::string ldr(const Reg &target, const Reg &base, const Reg &offset) {
  return ldr(target, base, offset, Constant(0));
}

std::string ldr(const Reg &target, const Reg &base, const Reg &offset,
                const Constant &shift) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "ldr";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += "[";
  asm_code += base.getName();
  asm_code += ", ";
  asm_code += "+";
  asm_code += offset.getName();
  asm_code += ", ";
  asm_code += "lsl";
  asm_code += " ";
  asm_code += shift.getName();
  asm_code += "]";
  asm_code += newline;
  return asm_code;
}

std::string str(const Reg &source, const Addr &target) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "str";
  asm_code += " ";
  asm_code += source.getName();
  asm_code += ", ";
  asm_code += target.getName();
  asm_code += newline;
  return asm_code;
}

std::string str(const Reg &source, const Label &target) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "str";
  asm_code += " ";
  asm_code += source.getName();
  asm_code += ", ";
  asm_code += target.getName();
  asm_code += newline;
  return asm_code;
}

std::string str(const Reg &target, const Reg &base, const Reg &offset) {
  return str(target, base, offset, Constant(0));
}

std::string str(const Reg &target, const Reg &base, const Reg &offset,
                const Constant &shift) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "str";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += "[";
  asm_code += base.getName();
  asm_code += ", ";
  asm_code += "+";
  asm_code += offset.getName();
  asm_code += ", ";
  asm_code += "lsl";
  asm_code += " ";
  asm_code += shift.getName();
  asm_code += "]";
  asm_code += newline;
  return asm_code;
}

std::string bl(const std::string &target_func_name) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "bl";
  asm_code += " ";
  asm_code += target_func_name;
  asm_code += newline;
  return asm_code;
}

std::string add(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "add";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string sub(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "sub";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string rsb(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "rsb";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string and_(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "and";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string orr(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "orr";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string eor(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "eor";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string clz(const Reg &target, const Reg &op1) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "clz";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += newline;
  return asm_code;
}

std::string lsl(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "lsl";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string asl(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "asl";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string lsr(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "lsr";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string asr(const Reg &target, const Reg &op1, const Value &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "asr";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string mul(const Reg &target, const Reg &op1, const Reg &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "mul";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string smmul(const Reg &target, const Reg &op1, const Reg &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "smmul";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string mla(const Reg &target, const Reg &op1, const Reg &op2,
                const Reg &op3) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "mla";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += ", ";
  asm_code += op3.getName();
  asm_code += newline;
  return asm_code;
}

std::string smmla(const Reg &target, const Reg &op1, const Reg &op2,
                  const Reg &op3) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "smmla";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += ", ";
  asm_code += op3.getName();
  asm_code += newline;
  return asm_code;
}

std::string mls(const Reg &target, const Reg &op1, const Reg &op2,
                const Reg &op3) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "mls";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += ", ";
  asm_code += op3.getName();
  asm_code += newline;
  return asm_code;
}

std::string smull(const Reg &target, const Reg &op1, const Reg &op2,
                  const Reg &op3) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "smull";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += ", ";
  asm_code += op3.getName();
  asm_code += newline;
  return asm_code;
}

std::string sdiv(const Reg &target, const Reg &op1, const Reg &op2) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "sdiv";
  asm_code += " ";
  asm_code += target.getName();
  asm_code += ", ";
  asm_code += op1.getName();
  asm_code += ", ";
  asm_code += op2.getName();
  asm_code += newline;
  return asm_code;
}

std::string cmp(const Reg &lhs, const Value &rhs) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "cmp";
  asm_code += " ";
  asm_code += lhs.getName();
  asm_code += ", ";
  asm_code += rhs.getName();
  asm_code += newline;
  return asm_code;
}

std::string b(const Label &target, const CmpOp &cond) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "b";
  asm_code += condCode(cond);
  asm_code += " ";
  asm_code += target.getName();
  asm_code += newline;
  return asm_code;
}

std::string instConst(std::string (*inst)(const Reg &target, const Reg &op1,
                                          const Value &op2),
                      const Reg &target, const Reg &op1, const Constant &op2) {
  std::string asm_code;
  int val = op2.getValue();
  if (target == op1 && op2.getValue() == 0 && (inst == add || inst == sub)) {
    return asm_code;
  } else if (0 <= val && val <= imm_8_max) {
    asm_code += inst(target, op1, op2);
  } else {
    asm_code += setValue(vinst_temp_reg, op2);
    asm_code += inst(target, op1, vinst_temp_reg);
  }
  return asm_code;
}

std::string instConst(std::string (*inst)(const Reg &op1, const Value &op2),
                      const Reg &op1, const Constant &op2) {
  std::string asm_code;
  int val = op2.getValue();
  if (0 <= val && val <= imm_8_max) {
    asm_code += inst(op1, op2);
  } else {
    asm_code += setValue(vinst_temp_reg, op2);
    asm_code += inst(op1, vinst_temp_reg);
  }
  return asm_code;
}

std::string load(const Reg &target, const Addr &source) {
  std::string asm_code;
  int offset = source.getOffset();
  if (offset > imm_12_max || offset < -imm_12_max) {
    asm_code += InstGen::setValue(vinst_temp_reg, Constant(offset));
    asm_code += InstGen::ldr(target, source.getReg(), vinst_temp_reg);
  } else {
    asm_code += InstGen::ldr(target, source);
  }
  return asm_code;
}

std::string store(const Reg &source, const Addr &target) {
  assert(source != vinst_temp_reg);
  std::string asm_code;
  int offset = target.getOffset();
  if (offset > imm_12_max || offset < -imm_12_max) {
    asm_code += InstGen::setValue(vinst_temp_reg, Constant(offset));
    asm_code += InstGen::str(source, target.getReg(), vinst_temp_reg);
  } else {
    asm_code += InstGen::str(source, target);
  }
  return asm_code;
}

std::string swi(const Constant &id) {
  std::string asm_code;
  asm_code += spaces;
  asm_code += "swi";
  asm_code += " ";
  asm_code += id.getName();
  asm_code += newline;
  return asm_code;
}

std::string bic(const Reg &target, const Reg &v1, const Reg &v2,
                const Reg &v3) {
  std::string asm_code;
  asm_code += mov(vinst_temp_reg, Constant(1));
  asm_code += spaces + "bic" + " " + vinst_temp_reg.getName() + ", " +
              v1.getName() + ", " + vinst_temp_reg.getName() + ", " + "lsl" +
              " " + v2.getName() + newline;
  asm_code += spaces + "orr" + " " + target.getName() + ", " +
              vinst_temp_reg.getName() + ", " + v3.getName() + ", " + "lsl" +
              " " + v2.getName() + newline;
  return asm_code;
}

std::string vvmul(const Reg &target, const Reg &v1, const Reg &v2,
                  const int len) {
  std::string asm_code;
  if (len == 4) {
    asm_code += spaces + "vld1.32 {d18-d19}, [" + v1.getName() + "]" + newline;
    asm_code += spaces + "vld1.32 {d16-d17}, [" + v2.getName() + "]" + newline;
    asm_code += spaces + "vmul.i32 q8, q8, q9" + newline;
    asm_code += spaces + "vadd.i32 d16, d17, d16" + newline;
    asm_code += spaces + "vpadd.i32 d16, d16, d16" + newline;
    asm_code += spaces + "vmov.32 " + target.getName() + ", d16[0]" + newline;
  } else if (len == 8) {
    asm_code += spaces + "vld2.32 {d24-d27}, [" + v1.getName() + "]" + newline;
    asm_code += spaces + "vld2.32 {d20-d23}, [" + v2.getName() + "]" + newline;
    asm_code += spaces + "vmul.i32 q8, q12, q10" + newline;
    asm_code += spaces + "vmla.i32 q8, q13, q11" + newline;
    asm_code += spaces + "vadd.i32 d16, d17, d16" + newline;
    asm_code += spaces + "vpadd.i32 d16, d16, d16" + newline;
    asm_code += spaces + "vmov.32 " + target.getName() + ", d16[0]" + newline;
  } else {
    std::cerr << "Unsupported SIMD vector mul vector length: " << len
              << std::endl;
    abort();
  }
  return asm_code;
}

std::tuple<int, int, int> choose_multiplier(int d, int N) {
  assert(d >= 1);
  int l = ceil(log2((double)d - 0.5));
  int sh_post = l;
  uint64_t m_l = (((uint64_t)1) << (N + l)) / d;
  uint64_t m_h = ((((uint64_t)1) << (N + l)) + (((uint64_t)1) << (l))) / d;
  while ((m_l / 2 < m_h / 2) && (sh_post > 1)) {
    m_l /= 2;
    m_h /= 2;
    sh_post--;
  }
  sh_post--;
  return std::make_tuple((int)(m_h), sh_post, l);
};

std::string divConst(const Reg &target, const Reg &source,
                     const Constant &divisor) {
  const int N = 32;
  std::string asm_code;
  int d = divisor.getValue();
  assert(d != 0);
  int m, sh_post, l;
  std::tie(m, sh_post, l) = choose_multiplier(abs(d), N - 1);
  if (abs(d) == 1) {
    asm_code += mov(target, source);
  } else if (abs(d) == (1 << l)) {
    // q = SRA(n + SRL(SRA(n, l - 1), N - l), l);
    asm_code += asr(vinst_temp_reg, source, Constant(l - 1));
    asm_code += lsr(vinst_temp_reg, vinst_temp_reg, Constant(N - l));
    asm_code += add(vinst_temp_reg, vinst_temp_reg, source);
    asm_code += asr(target, vinst_temp_reg, Constant(l));
  } else if (m >= 0) {
    // q = SRA(MULSH(m, n), sh_post) - XSIGN(n);
    asm_code += setValue(vinst_temp_reg, Constant(m));
    asm_code += smmul(vinst_temp_reg, vinst_temp_reg, source);
    asm_code += asr(vinst_temp_reg, vinst_temp_reg, Constant(sh_post));
    asm_code +=
        add(target, vinst_temp_reg,
            RegShift(source.getID(), 31, InstGen::RegShift::ShiftType::lsr));
  } else {
    // q = SRA(n + MULSH(m - 2^N , n), sh_post) - XSIGN(n);
    asm_code += setValue(vinst_temp_reg, Constant(m));
    asm_code += smmla(vinst_temp_reg, vinst_temp_reg, source, source);
    asm_code += asr(vinst_temp_reg, vinst_temp_reg, Constant(sh_post));
    asm_code +=
        add(target, vinst_temp_reg,
            RegShift(source.getID(), 31, InstGen::RegShift::ShiftType::lsr));
  }
  if (d < 0) {
    asm_code += rsb(target, target, Constant(0));
  }
  return asm_code;
}

}; // namespace InstGen