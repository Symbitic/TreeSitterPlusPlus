/**
 * @file tree_sitter/cpp/point.h
 * @brief Data structures for TreeSitterPlusPlus.
 */
#pragma once

#include <functional>
#include <unordered_map>
#include "tree_sitter/api.h"

namespace TreeSitter {

/**
 * An offset (usually in a UTF-8 string).
 */
using Index = uint32_t;

/**
 * A location in the source code.
 */
using Point = TSPoint;

/**
 * A range in the source code.
 */
using Range = TSRange;

/**
 * Represents edits made to a source code tree.
 */
struct Edit {
    Index startIndex;
    Index oldEndIndex;
    Index newEndIndex;
    Point startPosition;
    Point oldEndPosition;
    Point newEndPosition;
};

/**
 * @brief Callback for editing/parsing source code.
 */
using Input = std::function<std::string (
    Index startIndex,
    Point startPoint,
    Index endIndex
)>;

}
