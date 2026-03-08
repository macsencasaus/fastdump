#ifndef INST
#define INST(mnemonic, fmt, ...) {INST_##mnemonic, padded_string<fmt_str_size>(fmt), (std::to_array<Segment, Instruction_Format::segment_count>(__VA_ARGS__))},
#endif

#ifndef INSTALT
#define INSTALT INST
#endif

#define B(bits) {Segment::Kind::BITS, sizeof(#bits) - 1, 0b##bits}
#define S       {Segment::Kind::S,          1}
#define W       {Segment::Kind::W,          1}
#define U       {Segment::Kind::U,          1}
#define COND    {Segment::Kind::COND,       4}
#define Rn      {Segment::Kind::Rn,         4}
#define Rd      {Segment::Kind::Rd,         4}
#define Rm      {Segment::Kind::Rm,         4}
#define Rs      {Segment::Kind::Rs,         4}
#define CONST   {Segment::Kind::CONST,     12}
#define IMM(v)  {Segment::Kind::IMM,        v}
#define SIMM    {Segment::Kind::SHIFT_IMM,  7}
#define TYPE    {Segment::Kind::SHIFT_TYPE, 2}
#define LSB     {Segment::Kind::LSB,        5}
#define MSB     {Segment::Kind::MSB,        5}

#define BOPT    {Segment::Kind::BOPT,       4}
#define IOPT    {Segment::Kind::IOPT,       4}

#define REGS    {Segment::Kind::REGS,      16}

// coprocessor
#define COPROC  {Segment::Kind::COPROC,     4}
#define OPC1    {Segment::Kind::OPC1,       4}
#define OPC2    {Segment::Kind::OPC2,       3}
#define CRn     {Segment::Kind::CRn,        4}
#define CRd     {Segment::Kind::CRd,        4}
#define CRm     {Segment::Kind::CRm,        4}

INST(   adc,   "adc<s><c>\t<Rd>, <Rn>, #<const>",          {COND, B(0010101), S, Rn, Rd, CONST})
INSTALT(adc,   "adc<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000101), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(adc,   "adc<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000101), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

INST(   add,   "add<s><c>\t<Rd>, <Rn>, #<const>",          {COND, B(0010100), S, Rn, Rd, CONST})
INSTALT(add,   "add<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000100), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(add,   "add<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000100), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})
INSTALT(add,   "add<s><c>\t<Rd>, sp, #<const>",            {COND, B(0010100), S, B(1101), Rd, CONST})
INSTALT(add,   "add<s><c>\t<Rd>, sp, <Rm><simm>",          {COND, B(0000100), S, B(1101), Rd, SIMM, B(0), Rm})

INST(   adr,   "adr<c>\t<Rd>, label",                      {COND, B(001010001111), Rd, CONST})
INSTALT(adr,   "adr<c>\t<Rd>, label",                      {COND, B(001001001111), Rd, CONST})

INST(   and,   "and<s><c>\t<Rd>, <Rn>, #<const>",          {COND, B(0010000), S, Rn, Rd, CONST})
INSTALT(and,   "and<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000000), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(and,   "and<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000000), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

INST(   asr,   "asr<s><c>\t<Rd>, <Rm>, #<imm>",            {COND, B(0001101), S, B(0000), Rd, IMM(5), B(100), Rm})
INSTALT(asr,   "asr<s><c>\t<Rd>, <Rn>, <Rm>",              {COND, B(0001101), S, B(0000), Rd, Rm, B(0101), Rn})

INST(   b,     "b<c>\tlabel",                              {COND, B(1010), IMM(24)})

INST(   bfc,   "bfc<c>\t<Rd>, #<lsb>, #<width>",           {COND, B(0111110), MSB, Rd, LSB, B(0011111)})
INST(   bfi,   "bfi<c>\t<Rd>, <Rn>, #<lsb>, #<width>",     {COND, B(0111110), MSB, Rd, LSB, B(001), Rn})

INST(   bic,   "bic<s><c>\t<Rd>, <Rn>, #<const>",          {COND, B(0011110), S, Rn, Rd, CONST})
INSTALT(bic,   "bic<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0001110), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(bic,   "bic<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0001110), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

INST(   bkpt,  "bkpt\t#<cimm>",                            {COND, B(00010010), IMM(12), B(0111), IMM(4)})

INST(   bl,    "bl<c>\tlabel",                             {COND, B(1011), IMM(24)})

// TODO: define H
INST(   blx,   "blx\tlabel",                               {B(1111101), IMM(25)})
INSTALT(blx,   "blx<c>\t<Rm>",                             {COND, B(000100101111111111110011), Rm})

INST(   bx,    "bx<c>\t<Rm>",                              {COND, B(000100101111111111110001), Rm})

INST(   bxj,   "bxj<c>\t<Rm>",                             {COND, B(000100101111111111110010), Rm})

INST(   cdp,   "cdp<c>\tp<coproc>, #<opc1>, c<CRd>, c<CRn>, c<CRm>, #<opc2>", {COND, B(1110), OPC1, CRn, CRd, COPROC, OPC2, B(0), CRm})
INST(   cdp2,  "cdp2\tp<coproc>, #<opc1>, c<CRd>, c<CRn>, c<CRm>, #<opc2>",   {B(11111110), OPC1, CRn, CRd, COPROC, OPC2, B(0), CRm})

INST(   clrex, "clrex",                                    {B(11110101011111111111000000011111)})

INST(   clz,   "clz<c>\t<Rd>, <Rm>",                       {COND, B(000101101111), Rd, B(11110001), Rm})

INST(   cmn,   "cmn<c>\t<Rn>, #<const>",                   {COND, B(00110111), Rn, B(0000), CONST})
INSTALT(cmn,   "cmn<c>\t<Rn>, <Rm><simm>",                 {COND, B(00010111), Rn, B(0000), SIMM, B(0), Rm})
INSTALT(cmn,   "cmn<c>\t<Rn>, <Rm>, <type> <Rs>",          {COND, B(00010111), Rn, B(0000), Rs, B(0), TYPE, B(1), Rm})

INST(   cmp,   "cmp<c>\t<Rn>, #<const>",                   {COND, B(00110101), Rn, B(0000), CONST})
INSTALT(cmp,   "cmp<c>\t<Rn>, <Rm><simm>",                 {COND, B(00010101), Rn, B(0000), SIMM, B(0), Rm})
INSTALT(cmp,   "cmp<c>\t<Rn>, <Rm>, <type> <Rs>",          {COND, B(00010101), Rn, B(0000), Rs, B(0), TYPE, B(1), Rm})

// TODO: CPS

INST(   csdb,  "csdb<c>",                                  {COND, B(0011001000001111000000010100)})

INST(   dbg,   "dbg<c>\t#<imm>",                           {COND, B(001100100000111100001111), IMM(4)})

INST(   dmb,   "dmb\t<bopt>",                              {B(1111010101111111111100000101), BOPT})
INST(   dsb,   "dsb\t<bopt>",                              {B(1111010101111111111100000100), BOPT})

// TODO: ENTERX

INST(   eor,   "eor<s><c>\t<Rd>, <Rn>, #<const>",          {COND, B(0010001), S, Rn, Rd, CONST})
INSTALT(eor,   "eor<s><c>\t<Rd>, <Rn>, <Rm><simm>",        {COND, B(0000001), S, Rn, Rd, SIMM, B(0), Rm})
INSTALT(eor,   "eor<s><c>\t<Rd>, <Rn>, <Rm>, <type> <Rs>", {COND, B(0000001), S, Rn, Rd, Rs, B(0), TYPE, B(1), Rm})

// TODO: ERET

// TODO: F?

// TODO: HB..

// TODO: HVC

INST(   isb,   "isb\t<iopt>",                              {B(1111010101111111111100000110), IOPT})

// TODO: IT

// TODO: LDC, LDC2 (add P, U, D, W)

INST(   ldm,   "ldm<c>\t<Rn><w>, <regs>",                  {COND, B(100010), W, B(1), Rn, REGS})

INST( ldmda,   "ldmda<c>\t<Rn><w>, <regs>",                {COND, B(100000), W, B(1), Rn, REGS})

INST( ldmdb,   "ldmdb<c>\t<Rn><w>, <regs>",                {COND, B(100100), W, B(1), Rn, REGS})

INST( ldmib,   "ldmib<c>\t<Rn><w>, <regs>",                {COND, B(100110), W, B(1), Rn, REGS})

// TODO: maybe add a special case for imm == 0
INST(   ldr,   "ldr<c>\t<Rs>, [<Rn>, #<u><imm>]<w>",       {COND, B(0101), U, B(0), W, B(1), Rn, Rs, IMM(12)})
INSTALT(ldr,   "ldr<c>\t<Rs>, [<Rn>], #<u><imm>",          {COND, B(0100), U, B(001), Rn, Rs, IMM(12)})

INSTALT(ldr,   "ldr<c>\t<Rs>, label",                      {COND, B(010110011111), Rs, IMM(12)})
INSTALT(ldr,   "ldr<c>\t<Rs>, -label",                     {COND, B(010100011111), Rs, IMM(12)})

INSTALT(ldr,   "ldr<c>\t<Rs>, [<Rn>, <u><Rm><simm>]<w>",   {COND, B(0111), U, B(0), W, B(1), Rn, Rs, SIMM, B(0), Rm})
INSTALT(ldr,   "ldr<c>\t<Rs>, [<Rn>], <u><Rm><simm>",      {COND, B(0110), U, B(001), Rn, Rs, SIMM, B(0), Rm})

INST(   ldrb,  "ldrb<c>\t<Rs>, [<Rn>, #<u><imm>]<w>",      {COND, B(0101), U, B(1), W, B(1), Rn, Rs, IMM(12)})
INSTALT(ldrb,  "ldrb<c>\t<Rs>, [<Rn>], #<u><imm>",         {COND, B(0100), U, B(101), Rn, Rs, IMM(12)})

INSTALT(ldrb,  "ldrb<c>\t<Rs>, label",                     {COND, B(010111011111), Rs, IMM(12)})
INSTALT(ldrb,  "ldrb<c>\t<Rs>, -label",                    {COND, B(010101011111), Rs, IMM(12)})

INSTALT(ldrb,  "ldrb<c>\t<Rs>, [<Rn>, <u><Rm><simm>]<w>",  {COND, B(0111), U, B(1), W, B(1), Rn, Rs, SIMM, B(0), Rm})
INSTALT(ldrb,  "ldrb<c>\t<Rs>, [<Rn>], <u><Rm><simm>",     {COND, B(0110), U, B(101), Rn, Rs, SIMM, B(0), Rm})

INST(   ldrbt, "ldrbt<c>\t<Rs>, [<Rn>], #<u><imm>",        {COND, B(0100), U, B(111), Rn, Rs, IMM(12)})
INSTALT(ldrbt, "ldrbt<c>\t<Rs>, [<Rn>], <u><Rm><simm>",    {COND, B(0110), U, B(111), Rn, Rs, SIMM, B(0), Rm})

#undef INST
#undef INSTALT

#undef B
#undef S
#undef W
#undef U
#undef COND
#undef Rn
#undef Rd
#undef Rm
#undef Rs
#undef CONST
#undef IMM
#undef CIMM
#undef SIMM
#undef TYPE
#undef LSB
#undef MSB
#undef BOPT
#undef IOPT

#undef REGS

// coprocessor
#undef COPROC
#undef OPC1
#undef OPC2
#undef CRn
#undef CRd
#undef CRm
