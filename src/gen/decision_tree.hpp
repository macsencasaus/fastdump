#ifndef NODE_HPP
#define NODE_HPP

#include <bitset>
#include <memory_resource>
#include <variant>

#include <instructions.hpp>

struct Decision_Tree {
  using Instruction_Masks = std::array<Instruction_Mask, instruction_count>;

  struct General_Node;
  struct Leaf_Node;
  using Node = std::variant<General_Node, Leaf_Node>;

  static constexpr uint32_t mask_max_bit_width = 4u;

  struct General_Node {
    std::array<Node*, 1u << mask_max_bit_width> children;
    std::bitset<instruction_count> set;
    uint32_t mask;

    General_Node(const std::bitset<instruction_count>& set);

    static Node* build(Decision_Tree& tree,
                       const std::bitset<instruction_count>& set);
  };

  struct Leaf_Node {
    std::span<std::pair</* mask value */ uint32_t, /* idx */ uint32_t>>
        instr_idxs;
    uint32_t mask;

    static Node* build(Decision_Tree& tree, uint32_t idx);
    static Node* build(Decision_Tree& tree,
                       const std::bitset<instruction_count>& set);
  };

  struct Mask_Table {
    std::vector<uint32_t> masks;
    std::vector<ssize_t> subtable_idxs;
    std::vector<const Leaf_Node*> leafs;
  };

  struct Table_Builder {
    Mask_Table& t;

    ssize_t operator()(const General_Node&);
    ssize_t operator()(const Leaf_Node&);
  };

  const Instruction_Masks& masks;
  std::pmr::polymorphic_allocator<Node> alloc;

  Node* root;

  Decision_Tree(const Instruction_Masks& masks,
                std::pmr::memory_resource* pool);

  static Decision_Tree build(
      const Instruction_Masks& masks,
      std::pmr::memory_resource* pool = std::pmr::get_default_resource());

  Mask_Table build_table();
};

#endif
