#include "decision_tree.hpp"

#include <bit>
#include <print>

static inline bool is_bit_set(uint32_t v, uint8_t bit) {
  return v & (1u << bit);
}

Decision_Tree::General_Node::General_Node(
    const std::bitset<instruction_count>& set)
    : set{set} {}

Decision_Tree::Node* Decision_Tree::General_Node::build(
    Decision_Tree& tree,
    const std::bitset<instruction_count>& set) {
  std::println("--- General ---");
  std::println("set: {}", set.to_string());
  uint32_t allowed_bits = std::numeric_limits<uint32_t>::max();

  const auto& instr_masks = tree.masks;

  for (size_t i = 0; i < instruction_count; ++i) {
    if (set[i])
      allowed_bits &= instr_masks[i].mask;
  }

  uint32_t discriminating_bits = 0u;

  // TODO: optimize
  for (uint8_t bit = 0; bit < 32; ++bit) {
    if (!is_bit_set(allowed_bits, bit))
      continue;

    uint32_t ones = 0u, zeros = 0u;

    for (size_t i = 0; i < instruction_count; ++i) {
      if (!set[i])
        continue;

      if (instr_masks[i][bit])
        ++ones;
      else
        ++zeros;
    }

    if (ones && zeros)
      discriminating_bits |= (1u << bit);
  }

  std::println("allowed bits:        {:032b}", allowed_bits);
  std::println("discriminating bits: {:032b}", discriminating_bits);

  if (discriminating_bits == 0)
    return Leaf_Node::build(tree, set);

  // Node* node = tree.alloc.new_object<Node>(General_Node{set});
  Node* node = tree.alloc.new_object<Node>(General_Node{set});
  General_Node& gnode = std::get<General_Node>(*node);

  // take the most significant contiguous max_bit_width bits as the mask
  uint32_t mask = 0u;
  uint8_t bit = 31u;
  uint8_t mask_size = 0u;

  for (; !is_bit_set(discriminating_bits, bit); --bit)
    ;

  for (; mask_size < mask_max_bit_width && is_bit_set(discriminating_bits, bit);
       --bit, ++mask_size) {
    mask |= 1u << bit;
  }

  gnode.mask = mask;

  std::println("mask:                {:032b}", mask);

  uint32_t a = 1u << (bit + 1);

  for (uint32_t i = 0; i < (1u << mask_size); ++i) {
    uint32_t m = i * a;

    uint32_t n = 0;
    std::bitset<instruction_count> child_set;

    // std::println("m: {:032b}", m);
    uint32_t leaf_idx;
    for (uint32_t j = 0u; j < instruction_count; ++j) {
      if (!gnode.set[j])
        continue;

      if ((instr_masks[j].value & mask) == m) {
        child_set[j] = true;
        ++n;
        leaf_idx = j;
      }
    }

    if (n > 1) {
      gnode.children[i] = General_Node::build(tree, child_set);
    } else if (n == 1) {
      gnode.children[i] = Leaf_Node::build(tree, leaf_idx);
    } else {
      gnode.children[i] = nullptr;
    }
  }

  return node;
}

Decision_Tree::Node* Decision_Tree::Leaf_Node::build(Decision_Tree& tree,
                                                     uint32_t idx) {
  auto instr = tree.alloc.new_object<std::pair<uint32_t, uint32_t>>(0, idx);
  return tree.alloc.new_object<Node>(Leaf_Node{instr, 1, 0u});
}

Decision_Tree::Node* Decision_Tree::Leaf_Node::build(
    Decision_Tree& tree,
    const std::bitset<instruction_count>& set) {
  std::println("--- Special ---");

  uint32_t ors = 0u, ands = std::numeric_limits<uint32_t>::max();

  for (uint32_t i = 0; i < instruction_count; ++i) {
    if (!set[i])
      continue;
    uint32_t v = tree.masks[i].value;
    ors |= v;
    ands &= v;
  }

  uint32_t discriminating_bits = ors ^ ands;

  assert(discriminating_bits != 0);

  std::println("different_bits: {:032b}", discriminating_bits);

  // minimal contiguous mask

  int lo = std::countr_zero(discriminating_bits);
  int hi = 31 - std::countl_zero(discriminating_bits);

  uint32_t mask = static_cast<uint32_t>(((1ull << (hi - lo + 1)) - 1) << lo);

  std::println("mask:           {:032b}", mask);

  uint32_t instr_count = static_cast<uint32_t>(set.count());
  auto instrs_idxs =
      tree.alloc.allocate_object<std::pair<uint32_t, uint32_t>>(instr_count);

  uint32_t ch = 0;
  for (uint32_t i = 0; i < instruction_count; ++i) {
    if (!set[i])
      continue;
    uint32_t v = tree.masks[i].value;
    uint32_t mask_value = mask & v;
    instrs_idxs[ch++] = std::make_pair(mask_value, i);

    std::println("                {:032b} : {:032b}", v, mask_value);
  }

  return tree.alloc.new_object<Node>(Leaf_Node{instrs_idxs, instr_count, mask});
}

Decision_Tree::Decision_Tree(const Instruction_Masks& masks,
                             std::pmr::memory_resource* pool)
    : masks{masks}, alloc{pool} {}

Decision_Tree Decision_Tree::build(const Instruction_Masks& masks,
                                   std::pmr::memory_resource* pool) {
  Decision_Tree tree(masks, pool);
  tree.root = General_Node::build(tree, ~std::bitset<instruction_count>{});

  return tree;
}

Decision_Tree::Mask_Table Decision_Tree::build_table() {
  Mask_Table mt;
  Table_Builder tb{mt};
  std::visit(tb, *root);
  return mt;
}

ssize_t Decision_Tree::Table_Builder::operator()(const General_Node& gnode) {
  auto& mask_table = t.masks;
  auto& subtable_idxs = t.subtable_idxs;

  if (mask_table.size() == 0) {
    mask_table.push_back(gnode.mask);
    subtable_idxs.push_back(1);
  }

  size_t subtable_idx = mask_table.size();
  size_t children_count = 1u << std::popcount(gnode.mask);

  for (size_t i = 0; i < children_count; ++i) {
    const Node* c = gnode.children[i];
    if (auto gc = std::get_if<General_Node>(c)) {
      mask_table.push_back(gc->mask);
    } else {
      mask_table.push_back(0u);
    }
  }

  subtable_idxs.resize(subtable_idx + children_count);

  for (size_t i = 0; i < children_count; ++i) {
    const Node* c = gnode.children[i];
    subtable_idxs[subtable_idx + i] = c ? std::visit(*this, *c) : 0;
  }

  assert(subtable_idx <
         static_cast<size_t>(std::numeric_limits<ssize_t>::max()));
  return static_cast<ssize_t>(subtable_idx);
}

ssize_t Decision_Tree::Table_Builder::operator()(const Leaf_Node& snode) {
  auto& leaf_table = t.leafs;
  ssize_t leaf_table_idx = static_cast<ssize_t>(leaf_table.size());
  leaf_table.push_back(&snode);
  return -leaf_table_idx;
}
