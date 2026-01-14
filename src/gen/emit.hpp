#ifndef EMIT_HPP
#define EMIT_HPP

#include "decision_tree.hpp"

void emit_code(std::ofstream& f, const Decision_Tree::Mask_Table& mask_table);

#endif  // EMIT_HPP
