#include "tree_sitter/api.h"
#include "tree_sitter/cxx/cursor.h"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/tree.h"

using namespace TreeSitter;

struct Cursor::Private {
    uint32_t nodeTypeId;
    std::string nodeType;
    const Tree* tree = nullptr;
    TSTreeCursor cursor;
};

Cursor::Cursor(const Tree* tree, TSTreeCursor cursor)
    : d(std::make_unique<Private>())
{
        d->tree = tree;
        d->cursor = cursor;
}

Cursor::Cursor(const Cursor& cursor)
    : d(std::make_unique<Private>(*cursor.d)) { }

Cursor& Cursor::operator=(const Cursor &cursor) {
    *d = *cursor.d;
    return *this;
}

Cursor::~Cursor() {
    ts_tree_cursor_delete(&d->cursor);
}

void Cursor::reset(Node node) {
    ts_tree_cursor_reset(&d->cursor, node.node());
}

std::string Cursor::nodeType() const {
    const auto types = d->tree->language().nodeTypes();
    if (types.size() <= d->nodeTypeId) {
        return "";
    }
    return types[d->nodeTypeId];
}

uint32_t Cursor::nodeTypeId() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_symbol(node);
}

uint32_t Cursor::nodeId() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return (uint32_t)(size_t)node.id;
}

bool Cursor::nodeIsNamed() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_is_named(node) == 1;
}

bool Cursor::nodeIsMissing() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_is_missing(node) == 1;
}

std::string Cursor::nodeText() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    const uint32_t startIndex = ts_node_start_byte(node);
    const uint32_t endIndex = ts_node_end_byte(node);
    return d->tree->source().substr(startIndex, endIndex - startIndex);
}

Point Cursor::startPosition() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_start_point(node);
}

Point Cursor::endPosition() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_end_point(node);
}

uint32_t Cursor::startIndex() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_start_byte(node);
}

uint32_t Cursor::endIndex() const {
    TSNode node = ts_tree_cursor_current_node(&d->cursor);
    return ts_node_end_byte(node);
}

Node Cursor::currentNode() {
    TSNode current = ts_tree_cursor_current_node(&d->cursor);
    return Node(d->tree, current);
}

uint32_t Cursor::currentFieldId() {
    return ts_tree_cursor_current_field_id(&d->cursor);
}

std::string Cursor::currentFieldName() {
    const auto fields = d->tree->language().fields();
    if (fields.size() <= d->nodeTypeId) {
        return "";
    }
    return fields[currentFieldId()];
}

bool Cursor::gotoParent() {
    return ts_tree_cursor_goto_parent(&d->cursor) == 1;
}

bool Cursor::gotoFirstChild() {
    return ts_tree_cursor_goto_first_child(&d->cursor) == 1;
}

bool Cursor::gotoNextSibling() {
    return ts_tree_cursor_goto_next_sibling(&d->cursor) == 1;
}
