/**
 * @file tree_sitter/cpp/api.h
 * @brief C++ bindings for Tree-sitter.
 */

#pragma once

#include "tree_sitter/cxx/point.h"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/cursor.h"
#include "tree_sitter/cxx/tree.h"
#include "tree_sitter/cxx/parser.h"

namespace TreeSitter {

/**
 * @brief Language ABI version used by this library.
 */
constexpr int LanguageVersion = TREE_SITTER_LANGUAGE_VERSION;

/**
 * @brief Minimum ABI version required by this library.
 */
constexpr int MinimumCompatibleLanguageVersion = TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION;

}
