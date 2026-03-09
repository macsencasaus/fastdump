#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <print>
#include <ranges>
#include <string_view>

#include "util.hpp"

#define OPTIMIZED 1

struct Segment {
  enum class Kind {
    NONE,
    BITS,
    S,
    W,
    U,
    COND,  // condition bits
    Rn,
    Rd,
    Rm,
    Rs,
    Rt,
    Rt2,
    CONST,  // 12 bit modified constant
    IMM,    // immediate
    CIMM,  // composite immediate (two immediate fields that are concatenated)
    SHIFT_IMM,   // shift by immediate
    SHIFT_TYPE,  // shift type (shift by register)
    LSB,
    MSB,
    WIDTH,

    BOPT,  // memory barrier option
    IOPT,  // instruction barrier option

    REGS,  // register list

    // coprocessor
    COPROC,
    OPC1,
    OPC2,
    CRd,
    CRn,
    CRm,
  } kind;

  unsigned bit_length;
  uint32_t value = 0u;
};

static consteval size_t find_fmt_str_size() {
  auto fmt_strs = std::to_array({
#define INST(mnemonic, fmt_str, ...) fmt_str,
#include "armv7_instruction_table.inc"
  });
  size_t res = 0zu;
  for (auto fmt_str : fmt_strs) {
    res = std::max(res, std::char_traits<char>::length(fmt_str));
  }
  return (res + 1) * 2;  // just an upper bound
}

static constexpr size_t fmt_str_size = find_fmt_str_size();

enum Instruction : uint32_t {
  INST_NONE,
#define INST(mnemonic, ...) INST_##mnemonic,
#define INSTALT(...)
#include "armv7_instruction_table.inc"
};

struct Instruction_Format {
  static constexpr size_t segment_count = 12;

  // Format string type
  using Fmt_Str = std::array<char, fmt_str_size>;

  Instruction instr;
  Fmt_Str fmt;
  std::array<Segment, segment_count> segments;

  // bit mask for each segment
  std::array<uint32_t, segment_count> masks;

  consteval Instruction_Format()
      : instr{INST_NONE}, fmt{}, segments{}, masks{} {}

  constexpr Instruction_Format(Instruction instr,
                               Fmt_Str fmt,
                               std::array<Segment, segment_count> segments)
      : instr{instr}, fmt{fmt}, segments{segments}, masks{} {
    uint32_t cur_mask = 0;

    for (size_t i = 0; i < segment_count; ++i) {
      const Segment& segment = segments[i];
      if (segment.kind == Segment::Kind::NONE)
        break;

      assert(segment.bit_length > 0);

      uint32_t bits = static_cast<uint32_t>((1ull << segment.bit_length) - 1);
      cur_mask = bits << (static_cast<uint32_t>(std::countr_zero(cur_mask)) -
                          segment.bit_length);

      masks[i] = cur_mask;
    }
  }

  static constexpr Segment::Kind kind_from_var(std::string_view var) {
    return var == "s"       ? Segment::Kind::S
           : var == "w"     ? Segment::Kind::W
           : var == "u"     ? Segment::Kind::U
           : var == "c"     ? Segment::Kind::COND
           : var == "Rn"    ? Segment::Kind::Rn
           : var == "Rd"    ? Segment::Kind::Rd
           : var == "Rm"    ? Segment::Kind::Rm
           : var == "Rs"    ? Segment::Kind::Rs
           : var == "Rt"    ? Segment::Kind::Rt
           : var == "Rt2"   ? Segment::Kind::Rt2
           : var == "const" ? Segment::Kind::CONST
           : var == "imm"   ? Segment::Kind::IMM
           : var == "cimm"  ? Segment::Kind::CIMM
           : var == "simm"  ? Segment::Kind::SHIFT_IMM
           : var == "type"  ? Segment::Kind::SHIFT_TYPE
           : var == "msb"   ? Segment::Kind::MSB
           : var == "lsb"   ? Segment::Kind::LSB
           : var == "width" ? Segment::Kind::WIDTH

           : var == "bopt" ? Segment::Kind::BOPT
           : var == "iopt" ? Segment::Kind::IOPT

           : var == "regs" ? Segment::Kind::REGS

           // coprocessor
           : var == "coproc" ? Segment::Kind::COPROC
           : var == "opc1"   ? Segment::Kind::OPC1
           : var == "opc2"   ? Segment::Kind::OPC2
           : var == "CRd"    ? Segment::Kind::CRd
           : var == "CRn"    ? Segment::Kind::CRn
           : var == "CRm"    ? Segment::Kind::CRm
                             : Segment::Kind::NONE;
  }

  constexpr std::pair<Segment::Kind, uint32_t> mask(
      std::string_view var) const {
    Segment::Kind kind = kind_from_var(var);

    if (kind == Segment::Kind::NONE) {
      std::println("unknown var: {}", var);
    }
    assert(kind != Segment::Kind::NONE);

    // fields that are composition of other fields
    if (kind == Segment::Kind::WIDTH || kind == Segment::Kind::CIMM) { 
      uint32_t mask = 0u;
      for (const auto& [segment, segment_mask] :
           std::views::zip(segments, masks)) {
        if (kind == Segment::Kind::WIDTH &&
            (segment.kind == Segment::Kind::LSB ||
             segment.kind == Segment::Kind::MSB))
          mask |= segment_mask;

        if (kind == Segment::Kind::CIMM && segment.kind == Segment::Kind::IMM)
          mask |= segment_mask;
      }
      return std::make_pair(kind, mask);
    }

    for (const auto& [segment, mask] : std::views::zip(segments, masks)) {
      assert(segment.kind != Segment::Kind::NONE);

      if (segment.kind == Segment::Kind::Rt && kind == Segment::Kind::Rt2)
        return std::make_pair(kind, mask);

      if (segment.kind == kind)
        return std::make_pair(kind, mask);
    }

    UNREACHABLE();
  }
};

static constexpr auto default_instruction_formats =
    std::to_array<Instruction_Format>({
#include "armv7_instruction_table.inc"
    });

static constexpr auto armv7_instruction_str_lut = std::to_array({
    "",
#define INST(mnemonic, ...) #mnemonic,
#define INSTALT(...)
#include "armv7_instruction_table.inc"
});

struct Instruction_Mask {
  uint32_t mask;   // which bits are significant (non wildcard)
  uint32_t value;  // value of significant bits

  static constexpr Instruction_Mask from(const Instruction_Format& instr) {
    uint32_t mask = 0u;
    uint32_t value = 0u;

    size_t i = 32;

    for (const Segment& segment : instr.segments) {
      if (segment.kind == Segment::Kind::NONE)
        break;

      i -= segment.bit_length;

      if (segment.kind == Segment::Kind::BITS) {
        value |= (segment.value) << i;
        mask |= static_cast<uint32_t>(((1ull << segment.bit_length) - 1u) << i);
      }
    }

    assert(i == 0 && "Instruction does not specify 32 bits");

    return {mask, value};
  }

  inline bool operator[](uint8_t i) const {
    assert(i < 32 && "Index into mask must be less than 32");
    return value & (1 << i);
  }
};

// Optimizer:
// expand format strings to reduce conditionals in printing functions

static consteval bool is_foldable_segment(Segment::Kind kind) {
  auto foldable_segment_kinds = std::to_array<Segment::Kind>({
      Segment::Kind::S,
      Segment::Kind::W,
      Segment::Kind::U,
      Segment::Kind::SHIFT_TYPE,
  });

  for (auto foldable_segment : foldable_segment_kinds) {
    if (foldable_segment == kind)
      return true;
  }
  return false;
}

static consteval size_t optimized_format_count() {
  size_t res = 0zu;

  for (auto& instr_format : default_instruction_formats) {
    size_t count = 1zu;

    for (auto segment : instr_format.segments) {
      if (is_foldable_segment(segment.kind))
        count *= (1 << segment.bit_length);
    }

    res += count;
  }

  return res;
}

static consteval std::pair</* i */ size_t, /* n */ size_t>
find_format_segment_literal(const Instruction_Format& instr,
                            Segment::Kind kind) {
  constexpr auto read_var = [](const char* str) -> std::string_view {
    const char* base = str;
    for (; *str != '>'; ++str)
      ;
    return std::string_view(base, static_cast<size_t>(str - base));
  };

  auto fmt = instr.fmt.begin();
  for (; *fmt; ++fmt) {
    char ch = *fmt;
    if (ch == '<') {
      size_t i = static_cast<size_t>(fmt - instr.fmt.begin());
      ++fmt;
      auto var = read_var(fmt);

      auto var_kind = Instruction_Format::kind_from_var(var);

      fmt += var.size();

      if (var_kind == kind) {
        return std::make_pair(i, var.size() + 2);
      }
    }
  }
  UNREACHABLE();
}

static consteval Instruction_Format::Fmt_Str segment_literal(Segment::Kind kind,
                                                             uint32_t value) {
  using FS = Instruction_Format::Fmt_Str;

  static constexpr FS shift_type_field[] = {
      {"lsl"},
      {"lsr"},
      {"asr"},
      {"ror"},
  };

  switch (kind) {
    default:
      UNREACHABLE();

    case Segment::Kind::S:
      return value ? FS("s") : FS("");
    case Segment::Kind::W:
      return value ? FS("!") : FS("");
    case Segment::Kind::U:
      return value ? FS("") : FS("-");
    case Segment::Kind::SHIFT_TYPE:
      return shift_type_field[value];
  }
}

using Optimized_Format_Array =
    std::array<Instruction_Format, optimized_format_count()>;

static consteval void optimize_format(
    const Instruction_Format& fmt,
    Optimized_Format_Array::iterator& inserter) {
  for (size_t i = 0; i < fmt.segments.size(); ++i) {
    const auto& segment = fmt.segments[i];

    if (segment.kind == Segment::Kind::NONE)
      break;

    if (is_foldable_segment(segment.kind)) {
      uint32_t combinations = (1u << segment.bit_length);

      // modify current segment to make it a bit sequence
      for (uint32_t seq = 0; seq < combinations; ++seq) {
        auto new_fmt = fmt;
        new_fmt.segments[i].kind = Segment::Kind::BITS;
        new_fmt.segments[i].value = seq;

        auto [i, n] = find_format_segment_literal(new_fmt, segment.kind);
        auto lit = segment_literal(segment.kind, seq);
        slice_and_replace(new_fmt.fmt, i, n, lit);

        optimize_format(new_fmt, inserter);
      }

      return;
    }
  }
  *(inserter++) = fmt;
}

static consteval auto build_optimized_formats() {
  Optimized_Format_Array result;
  auto inserter = result.begin();

  for (auto& instr_format : default_instruction_formats) {
    optimize_format(instr_format, inserter);
  }

  return result;
}

static constexpr auto optimized_instruction_formats = build_optimized_formats();

#if OPTIMIZED == 1
static constexpr auto& instruction_formats = optimized_instruction_formats;
#else
static constexpr auto& instruction_formats = default_instruction_formats;
#endif

#endif  // INSTRUCTIONS_HPP
