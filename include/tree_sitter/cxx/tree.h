/**
 * @file tree_sitter/cpp/tree.h
 * @brief Abstract Syntax Tree.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "tree_sitter/api.h"
#include "tree_sitter/cxx/point.h"

namespace TreeSitter {

class Language;
class Node;
class Cursor;

/**
 * @brief An abstract syntax tree.
 * 
 * Created by Parser.parse().
 */
class Tree {
public:
    /** @internal Create a new AST. */
    Tree(TSTree* tree, Language lang, const std::string &source);
    /** @internal Copy constructor. */
    Tree(const Tree& lang);
    /** @internal Copy assignment constructor. */
    Tree& operator=(const Tree& node);
    /** @internal Destructor. */
    ~Tree();

    /** The highest level node of this tree. */
    Node rootNode() const;
    /** The programming language used by this tree. */
    Language language() const;

    /** Create a copy of this tree. */
    Tree copy();

    /** @private Only used internally. */
    std::string source() const;
    /** @private Only used by Parser. */
    TSTree* tree() const;

    /** Add an edit to this tree. */
    void edit(Edit delta);
    /** Construct a walker to navigate this tree. */
    Cursor walk();
    /** A list of changed areas. */
    std::vector<Range> getChangedRanges(Tree other);
private:
    struct Private;
    std::unique_ptr<Private> d;
};

}
