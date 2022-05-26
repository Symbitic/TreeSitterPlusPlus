/**
 * @file tree_sitter/cpp/node.h
 * @brief AST node.
 */
#pragma once

#include <optional>
#include "tree_sitter/api.h"
#include "tree_sitter/cxx/point.h"

namespace TreeSitter {

class Cursor;
class Tree;

/**
 * @brief A node in an AST tree.
 * 
 * This makes heavy use of `std::optional` to avoid the need for pointers.
 */
class Node {
public:
    /** @internal Create a new Node. */
    Node(const Tree* tree, TSNode node);
    /** @internal Copy constructor. */
    Node(const Node& lang);
    /** @internal Copy assignment constructor. */
    Node& operator=(const Node& node);
    /** @internal Destructor. */
    ~Node();

    bool operator==(const Node& node);
    bool operator!=(Node node);

#if 0
    //Disabled for portability/safety reasons.
    uint64_t id() const;
#endif

    /** @internal Access to the internal TSNode. */
    TSNode node() const;
    /** @internal Access to the internal Tree. */
    const Tree* tree() const;

    /** Node ID. */
    int typeId() const;
    /** Node type. */
    std::string type() const;

    /**
     * @brief The text in the source code.
     */
    std::string text() const;

    /** Starting position. */
    Point startPosition() const;
    /** Ending position. */
    Point endPosition() const;
    /** Starting offset. */
    Index startIndex() const;
    /** Ending offset. */
    Index endIndex() const;

    /** The parent node. */
    Node parent() const;
    /** Number of children owned by this node. */
    uint32_t childCount() const;
    /** Every child belonging to this node. */
    std::vector<Node> children() const;
    /** Number of named children owned by this node. */
    uint32_t namedChildCount() const;
    /** Every named child belonging to this node. */
    std::vector<Node> namedChildren() const;

    /** Get the first child. */
    std::optional<Node> firstChild() const;
    /** Get the first named child. */
    std::optional<Node> firstNamedChild() const;
    /** Get the last child. */
    std::optional<Node> lastChild() const;
    /** Get the last named child. */
    std::optional<Node> lastNamedChild() const;
    /** Get the next sibling. */
    std::optional<Node> nextSibling() const;
    /** Get the next (named) sibling. */
    std::optional<Node> nextNamedSibling() const;
    /** Get the previous sibling. */
    std::optional<Node> previousSibling() const;
    /** Get the previous (named) sibling. */
    std::optional<Node> previousNamedSibling() const;

    /** Have changes been made to this node? */
    bool hasChanges();
    /** Was there an error while parsing the source? */
    bool hasError();
    /** Test if two nodes are identical. */
    bool equals(Node other);

    /** Returns `true` if this node has a name. */
    bool isNamed();
    /** Returns `true` if this node is null. */
    bool isNull();
    /** Returns `true` if this node is missing. */
    bool isMissing();

    /** Convert to a S-expression. */
    std::string sexpr();

    /**
     * @brief Returns a child node.
     */
    std::optional<Node> child(int index);

    /**
     * @brief Returns a named child node.
     */
    std::optional<Node> namedChild(int index);

    /** Returns a child based on its Field ID. */
    std::optional<Node> childForFieldId(int fieldId);
    /** Returns a child based on its Field Name. */
    std::optional<Node> childForFieldName(const std::string& fieldName);

    /**
     * @brief Returns a descendant.
     * @note Returns itself if not found.
     */
    Node descendantForIndex(int index);
    /** Returns a descendant. */
    Node descendantForIndex(int startIndex, int endIndex);

    /**
     * @brief Returns a list of descendants.
     */
    std::vector<Node> descendantsOfType(std::vector<std::string> types,
        Point startPosition = Point { 0, 0 },
        Point endPosition = Point { 0, 0 });
    /** Returns a list of descendants. */
    std::vector<Node> descendantsOfType(std::string type,
        Point startPosition = Point { 0, 0 },
        Point endPosition = Point { 0, 0 });

    /** Returns a named descendant. */
    Node namedDescendantForIndex(int index);
    /** Returns a named descendant. */
    Node namedDescendantForIndex(int startIndex, int endIndex);
    /** Returns a descendant. */
    Node descendantForPosition(Point position);
    /** Returns a descendant. */
    Node descendantForPosition(Point startPosition, Point endPosition);
    /** Returns a named descendant. */
    Node namedDescendantForPosition(Point position);
    /** Returns a named descendant. */
    Node namedDescendantForPosition(Point startPosition, Point endPosition);

    /**
     * @brief Create a Cursor to traverse this node.
     * 
     * @return New cursor object.
     */
    Cursor walk();
private:
    struct Private;
    std::unique_ptr<Private> d;
};

}
