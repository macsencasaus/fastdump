#include <cassert>
#include <fstream>
#include <print>

#include <instructions.hpp>
#include "decision_tree.hpp"
#include "emit.hpp"

#define UNREACHABLE()                                                      \
  do {                                                                     \
    fprintf(stderr, "UNREACHABLE CODE REACHED: %s:%d in %s()\n", __FILE__, \
            __LINE__, __func__);                                           \
    abort();                                                               \
  } while (0)

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::println(stderr, "Usage: {} output-file", argv[0]);
    return 1;
  }

  const char* filename = argv[1];
  std::println("Writing to {}...", filename);

  std::ofstream f(filename);

  std::array<Instruction_Mask, instruction_count> masks;

  for (size_t i = 0; i < instruction_count; ++i) {
    masks[i] = Instruction_Mask::from(instruction_formats[i]);
  }

  for (size_t i = 0; i < instruction_count; ++i) {
    std::println("{:5} : {:032b}, {:032b}",
                 armv7_instruction_str_lut[instruction_formats[i].instr],
                 masks[i].mask, masks[i].value);
  }

  auto tree = Decision_Tree::build(masks);

  auto mask_table = tree.build_table();

  emit_code(f, mask_table);

  return 0;
}
