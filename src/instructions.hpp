#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <string.h>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>

struct Segment {
  enum class Kind {
    NONE,
    BITS,
    S,
    COND,  // condition bits
    Rn,
    Rd,
    Rm,
    Rs,
    IMM,   // immediate
    CIMM,  // composite immediate (two immediate fields that are concatenated)
    SHIFT_IMM,   // barrel shift by immediate
    SHIFT_TYPE,  // barrel shift type (shift by register)
    LSB,
    MSB,
    WIDTH,
    BOPT,  // memory barrier option
    IOPT,  // instruction barrier option
  } kind;

  unsigned bit_length;
  uint32_t value = 0u;
};

enum Instruction : uint32_t {
#define INST(mnemonic, ...) INST_##mnemonic,
#define INSTALT(...)
#include "armv7_instruction_table.inc"
};

struct Instruction_Format {
  static constexpr size_t segment_count = 12;

  Instruction instr;
  const char* fmt;
  std::array<Segment, segment_count> segments;

  // bit mask for each segment
  std::array<uint32_t, segment_count> masks;

  constexpr Instruction_Format(Instruction instr,
                               const char* fmt,
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

  constexpr std::pair<Segment::Kind, uint32_t> mask(
      std::string_view var) const {
    Segment::Kind kind = var == "s"       ? Segment::Kind::S
                         : var == "c"     ? Segment::Kind::COND
                         : var == "Rn"    ? Segment::Kind::Rn
                         : var == "Rd"    ? Segment::Kind::Rd
                         : var == "Rm"    ? Segment::Kind::Rm
                         : var == "Rs"    ? Segment::Kind::Rs
                         : var == "imm"   ? Segment::Kind::IMM
                         : var == "cimm"  ? Segment::Kind::CIMM
                         : var == "simm"  ? Segment::Kind::SHIFT_IMM
                         : var == "type"  ? Segment::Kind::SHIFT_TYPE
                         : var == "msb"   ? Segment::Kind::MSB
                         : var == "lsb"   ? Segment::Kind::LSB
                         : var == "width" ? Segment::Kind::WIDTH
                         : var == "bopt"  ? Segment::Kind::BOPT
                         : var == "iopt"  ? Segment::Kind::IOPT
                                          : Segment::Kind::NONE;

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

      if (segment.kind == kind)
        return std::make_pair(kind, mask);
    }

    assert(false);
  }
};

static constexpr auto instruction_formats = std::to_array<Instruction_Format>({
#include "armv7_instruction_table.inc"
});

static constexpr uint32_t instruction_count = instruction_formats.size();

static constexpr auto armv7_instruction_str_lut = std::to_array({
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

#endif  // INSTRUCTIONS_HPP
