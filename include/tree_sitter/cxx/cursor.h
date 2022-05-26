/**
 * @file tree_sitter/cpp/cursor.h
 * @brief Cursor.
 */
#pragma once

#include <memory>
#include <string>
#include "tree_sitter/api.h"
#include "tree_sitter/cxx/point.h"

namespace TreeSitter {

class Node;
class Tree;

/**
 * @brief Explores an AST.
 *
 * Created by Tree.walk()
 */
class Cursor {
public:
    /** @internal Created by Tree. */
    Cursor(const Tree* tree, TSTreeCursor cursor);
    /** @internal Copy constructor. */
    Cursor(const Cursor&);
    /** @internal Copy assignment constructor. */
    Cursor& operator=(const Cursor&);
    /** @internal Destructor. */
    ~Cursor();

    std::string nodeType() const;
    uint32_t nodeTypeId() const;
    std::string nodeText() const;
    uint32_t nodeId() const;
    bool nodeIsNamed() const;
    bool nodeIsMissing() const;
    Point startPosition() const;
    Point endPosition() const;
    uint32_t startIndex() const;
    uint32_t endIndex() const;

    void reset(Node node);
    Node currentNode();
    uint32_t currentFieldId();
    std::string currentFieldName();
    bool gotoParent();
    bool gotoFirstChild();
    bool gotoNextSibling();
private:
    struct Private;
    std::unique_ptr<Private> d;
};

}
