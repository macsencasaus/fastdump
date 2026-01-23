#ifndef INST
#define INST(mnemonic, fmt, ...) {INST_##mnemonic, fmt, (std::to_array<Segment, Instruction_Format::segment_count>(__VA_ARGS__))},
#endif

#ifndef INSTALT
#define INSTALT INST
#endif

#define B(bits) {Segment::Kind::BITS, sizeof(#bits) - 1, 0b##bits}
#define S       {Segment::Kind::S,          1}
#define COND    {Segment::Kind::COND,       4}
#define Rn      {Segment::Kind::Rn,         4}
#define Rd      {Segment::Kind::Rd,         4}
#define Rm      {Segment::Kind::Rm,         4}
#define Rs      {Segment::Kind::Rs,         4}
#define IMM(v)  {Segment::Kind::IMM,        v}
#define SIMM    {Segment::Kind::SHIFT_IMM,  7}
#define TYPE    {Segment::Kind::SHIFT_TYPE, 2}
#define LSB     {Segment::Kind::LSB,        5}
#define MSB     {Segment::Kind::MSB,        5}

INST(   adc,  "adc<s><c>\t<Rd>, <Rn>, #<imm>",            {COND, B(0010101), S, Rn, Rd, IMM(12)})
INSTALT(adc,  "adc<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000101), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(adc,  "adc<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000101), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

INST(   add,  "add<s><c>\t<Rd>, <Rn>, #<imm>",            {COND, B(0010100), S, Rn, Rd, IMM(12)})
INSTALT(add,  "add<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000100), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(add,  "add<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000100), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})
INSTALT(add,  "add<s><c>\t<Rd>, sp, #<imm>",              {COND, B(0010100), S, B(1101), Rd, IMM(12)})
INSTALT(add,  "add<s><c>\t<Rd>, sp, <Rm><simm>",          {COND, B(0000100), S, B(1101), Rd, SIMM, B(0), Rm})

INST(   adr,  "adr<c>\t<Rd>, label",                      {COND, B(001010001111), Rd, IMM(12)})
INSTALT(adr,  "adr<c>\t<Rd>, label",                      {COND, B(001001001111), Rd, IMM(12)})

INST(   and,  "and<s><c>\t<Rd>, <Rn>, #<imm>",            {COND, B(0010000), S, Rn, Rd, IMM(12)})
INSTALT(and,  "and<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000000), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(and,  "and<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000000), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

INST(   asr,  "asr<s><c>\t<Rd>, <Rm>, #<imm>",            {COND, B(0001101), S, B(0000), Rd, IMM(5), B(100), Rm})
INSTALT(asr,  "asr<s><c>\t<Rd>, <Rn>, <Rm>",              {COND, B(0001101), S, B(0000), Rd, Rm, B(0101), Rn})

INST(   b,    "b<c>\tlabel",                              {COND, B(1010), IMM(24)})

INST(   bfc,  "bfc<c>\t<Rd>, #<lsb>, #<width>",           {COND, B(0111110), MSB, Rd, LSB, B(0011111)})
INST(   bfi,  "bfi<c>\t<Rd>, <Rn>, #<lsb>, #<width>",     {COND, B(0111110), MSB, Rd, LSB, B(001), Rn})

INST(   bic,  "bic<s><c>\t<Rd>, <Rn>, #<imm>",            {COND, B(0011110), S, Rn, Rd, IMM(12)})
INSTALT(bic,  "bic<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0001110), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(bic,  "bic<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0001110), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

INST(   bkpt, "bkpt\t#<cimm>",                            {COND, B(00010010), IMM(12), B(0111), IMM(4)})

INST(   bl,   "bl<c>\tlabel",                             {COND, B(1011), IMM(24)})

// TODO: define H
INST(   blx,  "blx\tlabel",                               {B(1111101), IMM(25)})
INSTALT(blx,  "blx<c>\t<Rm>",                             {COND, B(000100101111111111110011), Rm})

INST(   bx,   "bx<c>\t<Rm>",                              {COND, B(000100101111111111110001), Rm})

INST(   bxj,  "bxj<c>\t<Rm>",                             {COND, B(000100101111111111110010), Rm})


#undef INST
#undef INSTALT

#undef B
#undef S
#undef COND
#undef Rn
#undef Rd
#undef Rm
#undef Rs
#undef IMM
#undef SIMM
#undef TYPE
#undef LSB
#undef MSB
