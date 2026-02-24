#include "emit.hpp"

#include <fstream>
#include <print>

static void emit_segment_printer(std::ofstream& f,
                                 Segment::Kind kind,
                                 uint32_t segment_mask) {
  int shift = std::countr_zero(segment_mask);

  // there are two masks
  if (kind == Segment::Kind::WIDTH || kind == Segment::Kind::CIMM) {
    int shift_first_mask = std::countr_zero(~segment_mask >> shift) + shift;
    int shift2 =
        std::countr_zero(segment_mask >> shift_first_mask) + shift_first_mask;

    /* isolate first (lowest) contiguous range */
    uint32_t first = segment_mask & -segment_mask;  // lowest set bit
    uint32_t tmp = segment_mask + first;            // carry through first range
    uint32_t range1 = (tmp ^ segment_mask) >> 1 | first;

    /* second range is whatever is left */
    uint32_t segment_mask2 = segment_mask & ~range1;

    segment_mask = range1;

    std::print(f, "  b = (instr & {:#034b}u) >> {};\n", segment_mask2, shift2);
  }

  std::print(f, "  a = (instr & {:#034b}u) >> {};\n", segment_mask, shift);

  switch (kind) {
    case Segment::Kind::NONE:
    case Segment::Kind::BITS:
      assert(false);

    case Segment::Kind::S: {
      std::print(f, "  if (a) arena.append('s');\n");
    } break;
    case Segment::Kind::COND: {
      std::print(f, "  arena.append(condition_field[a]);\n");
    } break;
    case Segment::Kind::Rn:
    case Segment::Kind::Rd:
    case Segment::Kind::Rm:
    case Segment::Kind::Rs: {
      std::print(f, "  arena.append(general_reg_field[a]);\n");
    } break;
    case Segment::Kind::CONST: {
      std::print(f,
                 "  uint32_t s = (a >> 8) * 2;\n"
                 "  arena.appendu(std::rotr(a & 0xFFu, "
                 "std::bit_cast<int32_t>(s)));\n");
    } break;
    case Segment::Kind::CIMM: {
      std::print(f, "  arena.appendu((b << {}) | a);\n",
                 std::popcount(segment_mask));
    } break;
    case Segment::Kind::SHIFT_IMM: {
      std::print(f, "  arena.append(imm_shift_type_field[a]);\n");
    } break;
    case Segment::Kind::SHIFT_TYPE: {
      std::print(f, "  arena.append(shift_type_field[a]);\n");
    } break;
    case Segment::Kind::WIDTH: {
      std::print(f, "  arena.appendu(b - a);\n");
    } break;
    case Segment::Kind::BOPT: {
      std::print(f, "  arena.append(barrier_option_field[a]);\n");
    } break;
    case Segment::Kind::IOPT: {
      std::print(f, "  arena.append(ibarrier_option_field[a]);\n");
    } break;

    case Segment::Kind::REGS: {
      std::print(f, "  arena.append('{{');\n");
      std::print(f,
                 "  #pragma clang loop unroll(full)\n"
                 "  for (size_t i = 0; i < 16; ++i) {{\n"
                 "    if (i != 0) arena.append(\", \");\n"
                 "    if (a & (1u << i)) arena.append(general_reg_field[i]);\n"
                 "  }}\n");
      std::print(f, "  arena.append('}}');\n");
    } break;

    // appends the literal value
    case Segment::Kind::IMM:
    case Segment::Kind::LSB:
    case Segment::Kind::MSB:

    // coprocessor
    case Segment::Kind::COPROC:
    case Segment::Kind::OPC1:
    case Segment::Kind::OPC2:
    case Segment::Kind::CRd:
    case Segment::Kind::CRn:
    case Segment::Kind::CRm:

    {
      std::print(f, "  arena.appendu(a);\n");
    } break;
  }
}

static void emit_printer(std::ofstream& f,
                         const Decision_Tree::Leaf_Node* node,
                         size_t id) {
  std::print(f,
             "static void print_{}(String_Arena &arena, uint32_t instr) {{\n"
             "  uint32_t a, b; (void)instr; (void)a; (void)b;\n",
             id);

  auto emit_parser = [&](uint32_t idx) {
    const Instruction_Format& instr = instruction_formats[idx];
    const char* fmt = instr.fmt;

    auto read_var = [](const char* str) -> std::string_view {
      const char* base = str;
      for (; *str != '>'; ++str)
        ;
      return std::string_view(base, static_cast<size_t>(str - base));
    };

    auto read_text = [](const char* str) -> std::string_view {
      const char* base = str;
      for (; *str && *str != '<'; ++str)
        ;
      return std::string_view(base, static_cast<size_t>(str - base));
    };

    while (*fmt) {
      char ch = *fmt;
      if (ch == '<') {
        ++fmt;
        auto var = read_var(fmt);
        // std::println("var: {}, {}", var, var.size());
        const auto [kind, segment_mask] = instr.mask(var);

        fmt += var.size();
        assert(*fmt == '>');
        ++fmt;

        emit_segment_printer(f, kind, segment_mask);
        continue;
      }

      auto text = read_text(fmt);
      std::print(f, "  arena.append(\"{}\");\n", text);

      fmt += text.size();
    }
  };

  if (node->instr_idxs.size() == 1) {
    emit_parser(node->instr_idxs[0].second);
  } else {
    uint32_t else_idx;
    for (size_t i = 0; i < node->instr_idxs.size(); ++i) {
      uint32_t mask_value = node->instr_idxs[i].first;
      uint32_t idx = node->instr_idxs[i].second;

      if (node->instr_idxs[i].first == 0) {
        else_idx = idx;
        continue;
      }

      std::print(f, "if ((instr & {:#034b}u) == {:#034b}u) {{\n", node->mask,
                 mask_value);
      emit_parser(idx);
      std::print(f, "}} else ");
    }

    std::print(f, "{{\n");
    emit_parser(else_idx);
    std::print(f, "}}\n");
  }

  std::print(f, "}}\n");
}

void emit_code(std::ofstream& f, const Decision_Tree::Mask_Table& mask_table) {
  assert(mask_table.masks.size() == mask_table.subtable_idxs.size());

  std::println("Mask table:");

  for (size_t i = 0; i < mask_table.masks.size(); ++i) {
    uint32_t m = mask_table.masks[i];
    ssize_t s_idx = mask_table.subtable_idxs[i];
    std::println("  {:2}: {:034b} | {:2}", i, m, s_idx);
  }

  std::println();

  for (size_t i = 0; i < mask_table.leafs.size(); ++i) {
    auto ln = mask_table.leafs[i];
    std::print("  {:2} {{", i);
    for (size_t j = 0; j < ln->instr_idxs.size(); ++j) {
      if (j != 0)
        std::print(", ");
      std::print("{}", ln->instr_idxs[j].second);
    }
    std::println("}}");
  }

  std::print(f,
             "#include <bit>\n"
             "#include <cstdint>\n"
             "#include <cstddef>\n"
             "#include <sys/types.h>\n");

  std::print(f, "\n");

  std::print(f,
             "static constexpr const char *condition_field[] = {{\n"
             "  \"eq\",\n"
             "  \"ne\",\n"
             "  \"hs\",\n"
             "  \"lo\",\n"
             "  \"mi\",\n"
             "  \"pl\",\n"
             "  \"vs\",\n"
             "  \"vc\",\n"
             "  \"hi\",\n"
             "  \"ls\",\n"
             "  \"ge\",\n"
             "  \"lt\",\n"
             "  \"gt\",\n"
             "  \"le\",\n"
             "  \"\",\n"
             "  \"nv\",\n"
             "}};\n");

  std::print(f,
             "static constexpr const char *general_reg_field[] = {{\n"
             "  \"r0\",\n"
             "  \"r1\",\n"
             "  \"r2\",\n"
             "  \"r3\",\n"
             "  \"r4\",\n"
             "  \"r5\",\n"
             "  \"r6\",\n"
             "  \"r7\",\n"
             "  \"r8\",\n"
             "  \"r9\",\n"
             "  \"r10\",\n"
             "  \"fp\",\n"
             "  \"ip\",\n"
             "  \"sp\",\n"
             "  \"lr\",\n"
             "  \"pc\",\n"
             "}};\n");

  std::print(f, "static constexpr const char *imm_shift_type_field[] = {{\n");

  for (uint32_t i = 0; i < (1u << 7); ++i) {
    uint32_t type = i & 0b11;
    uint32_t imm = i >> 2;

    switch (type) {
      case 0: {
        if (imm)
          std::print(f, "  \", lsl #{}\",\n", imm);
        else
          std::print(f, "  \"\",\n");
      } break;
      case 1: {
        if (imm)
          std::print(f, "  \", lsr #{}\",\n", imm);
        else
          std::print(f, "  \", lsr #32\",\n");
      } break;
      case 2: {
        if (imm)
          std::print(f, "  \", asr #{}\",\n", imm);
        else
          std::print(f, "  \", asr #32\",\n");
      } break;
      case 3: {
        if (imm)
          std::print(f, "  \", ror #{}\",\n", imm);
        else
          std::print(f, "  \", rrx\",\n");
      } break;
    }
  }

  std::print(f, "}};\n");

  std::print(f,
             "static constexpr const char *shift_type_field[] = {{\n"
             "  \"lsl\",\n"
             "  \"lsr\",\n"
             "  \"asr\",\n"
             "  \"ror\",\n"
             "}};\n");

  std::print(f,
             "static constexpr const char *barrier_option_field[] = {{\n"
             "  \"#0\",\n"
             "  \"#1\",\n"
             "  \"oshst\",\n"
             "  \"osh\",\n"
             "  \"#4\",\n"
             "  \"#5\",\n"
             "  \"nshst\",\n"
             "  \"nsh\",\n"
             "  \"#8\",\n"
             "  \"#9\",\n"
             "  \"ishst\",\n"
             "  \"ish\",\n"
             "  \"#12\",\n"
             "  \"#13\",\n"
             "  \"st\",\n"
             "  \"sy\",\n"
             "}};\n");

  std::print(f,
             "static constexpr const char *ibarrier_option_field[] = {{\n"
             "  \"#0\",\n"
             "  \"#1\",\n"
             "  \"#2\",\n"
             "  \"#3\",\n"
             "  \"#4\",\n"
             "  \"#5\",\n"
             "  \"#6\",\n"
             "  \"#7\",\n"
             "  \"#8\",\n"
             "  \"#9\",\n"
             "  \"#10\",\n"
             "  \"#11\",\n"
             "  \"#12\",\n"
             "  \"#13\",\n"
             "  \"#14\",\n"
             "  \"sy\",\n"
             "}};\n");

  std::print(f, "\n");

  std::print(f, "static constexpr uint32_t masks[] = {{\n");
  for (auto m : mask_table.masks) {
    std::print(f, "  {:#034b}u,\n", m);
  }
  std::print(f, "}};\n");

  std::print(f, "\n");

  std::print(f, "static constexpr uint8_t shifts[] = {{\n");
  for (auto m : mask_table.masks) {
    if (m)
      std::print(f, "  {}u,\n", std::countr_zero(m));
    else
      std::print(f, "  0u,\n");
  }
  std::print(f, "}};\n");

  std::print(f, "\n");

  std::print(f, "static constexpr ssize_t subtable_idxs[] = {{\n");
  for (auto i : mask_table.subtable_idxs) {
    std::print(f, "  {}ll,\n", i);
  }
  std::print(f, "}};\n");

  std::print(f, "\n");

  std::print(f,
             "static inline size_t decode(uint32_t instr) {{\n"
             "  ssize_t idx = 0;\n"
             "  for (;;) {{\n"
             "    uint32_t mask = masks[idx];\n"
             "    uint32_t s = shifts[idx];\n"
             "    idx = subtable_idxs[idx];\n"
             "    if (idx <= 0) break;\n"
             "    ssize_t r = (instr & mask) >> s;\n"
             "    idx += r;\n"
             "  }}\n"
             "  return std::bit_cast<size_t>(-idx);\n"
             "}}\n");

  std::print(f, "using Print_Fn = void (*)(String_Arena &, uint32_t);\n");

  for (size_t i = 0; i < mask_table.leafs.size(); ++i) {
    emit_printer(f, mask_table.leafs[i], i);
  }

  std::print(f, "static constexpr const Print_Fn printers[] = {{\n");
  for (size_t i = 0; i < mask_table.leafs.size(); ++i) {
    std::print(f, "  print_{},\n", i);
  }
  std::print(f, "}};\n");
}
