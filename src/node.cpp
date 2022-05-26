#include "tree_sitter/cxx/cursor.h"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/tree.h"
#include "tree_sitter/api.h"

using namespace TreeSitter;

static inline bool point_lte(TSPoint a, TSPoint b) {
    return (a.row < b.row) || (a.row == b.row && a.column <= b.column);
}

struct Node::Private {
    const Tree* tree = nullptr;
    TSNode node;
    std::string text = "";
    std::vector<Node> children = {};
    std::vector<Node> namedChildren = {};
    TSTreeCursor cursor = {0};
};

Node::Node(const Tree* tree, TSNode node)
    : d(std::make_unique<Private>())
{
    d->tree = tree;
    d->node = node;
    const auto start = ts_node_start_byte(node);
    const auto end = ts_node_end_byte(node);
    const auto len = end - start;
    const auto source = tree->source();
    d->text = source.empty() ? "" : source.substr(start, len);
}

Node::Node(const Node& node)
    : d(std::make_unique<Private>(*node.d)) { }

Node& Node::operator=(const Node &node) {
    *d = *node.d;
    return *this;
}

bool Node::operator==(const Node &node) {
    return ts_node_eq(d->node, d->node);
}

bool Node::operator!=(Node node) {
    return !ts_node_eq(d->node, d->node);
}

Node::~Node() = default;

#if 0
uint64_t Node::id() const {
    uint64_t ret = reinterpret_cast<const uint64_t>(d->node.id);
    return ret;
}
#endif

TSNode Node::node() const
{
    return d->node;
}

const Tree* Node::tree() const
{
    return d->tree;
}

int Node::typeId() const {
    return ts_node_symbol(d->node);
}

std::string Node::type() const {
    return ts_node_type(d->node);
}

std::string Node::text() const {
    return d->text;
}

Point Node::startPosition() const {
    Point point;
    const auto pos = ts_node_start_point(d->node);
    point.row = pos.row;
    point.column = pos.column;
    return point;
}

Point Node::endPosition() const {
    Point point;
    const auto pos = ts_node_end_point(d->node);
    point.row = pos.row;
    point.column = pos.column;
    return point;
}

Index Node::startIndex() const {
    return ts_node_start_byte(d->node);
}

Index Node::endIndex() const {
    return ts_node_end_byte(d->node);
}

Node Node::parent() const {
    return Node(d->tree, ts_node_parent(d->node));
}

uint32_t Node::childCount() const {
    return ts_node_child_count(d->node);
}

std::vector<Node> Node::children() const {
    if (d->children.size() > 0) {
        return d->children;
    }
    uint32_t count = ts_node_child_count(d->node);
    d->children.reserve(count);
    if (count > 0) {
        ts_tree_cursor_reset(&d->cursor, d->node);
        const bool hasChildren = ts_tree_cursor_goto_first_child(&d->cursor);
        if (!hasChildren) {
            //TODO
        }
        TSNode node = ts_tree_cursor_current_node(&d->cursor);
        d->children.push_back(Node(d->tree, node));
        for (uint32_t i=1; i<count; i++) {
            //TODO: handle ret.
            ts_tree_cursor_goto_next_sibling(&d->cursor);
            node = ts_tree_cursor_current_node(&d->cursor);
            d->children.push_back(Node(d->tree, node));
        }
    }
    return d->children;
}

uint32_t Node::namedChildCount() const {
    return ts_node_named_child_count(d->node);
}

std::vector<Node> Node::namedChildren() const {
    if (d->namedChildren.size() > 0) {
        return d->namedChildren;
    }
    uint32_t count = ts_node_named_child_count(d->node);
    d->namedChildren.reserve(count);
    if (count > 0) {
        ts_tree_cursor_reset(&d->cursor, d->node);
        ts_tree_cursor_goto_first_child(&d->cursor);
        uint32_t i = 0;
        while (true) {
            TSNode child = ts_tree_cursor_current_node(&d->cursor);
            if (ts_node_is_named(child)) {
                d->namedChildren.push_back(Node(d->tree, child));
                i++;
                if (i == count) {
                    break;
                }
            }
            if (!ts_tree_cursor_goto_next_sibling(&d->cursor)) {
                break;
            }
        }
    }
    return d->namedChildren;
}

std::optional<Node> Node::firstChild() const {
    if (children().size() == 0) {
        return {};
    }
    return children().at(0);
}

std::optional<Node> Node::firstNamedChild() const {
    if (namedChildren().size() == 0) {
        return {};
    }
    return namedChildren().at(0);
}

std::optional<Node> Node::lastChild() const {
    if (children().size() == 0) {
        return {};
    }
    return children().at(d->children.size() - 1);
}

std::optional<Node> Node::lastNamedChild() const {
    if (namedChildren().size() == 0) {
        return {};
    }
    return namedChildren().at(namedChildren().size() - 1);
}

std::optional<Node> Node::nextSibling() const {
    TSNode sibling = ts_node_next_sibling(d->node);
    if (ts_node_is_null(sibling)) {
        return {};
    }
    return Node(d->tree, sibling);
}

std::optional<Node> Node::nextNamedSibling() const {
    TSNode sibling = ts_node_next_named_sibling(d->node);
    if (ts_node_is_null(sibling)) {
        return {};
    }
    return Node(d->tree, sibling);
}

std::optional<Node> Node::previousSibling() const {
    TSNode sibling = ts_node_prev_sibling(d->node);
    if (ts_node_is_null(sibling)) {
        return {};
    }
    return Node(d->tree, sibling);
}

std::optional<Node> Node::previousNamedSibling() const {
    TSNode sibling = ts_node_prev_named_sibling(d->node);
    if (ts_node_is_null(sibling)) {
        return {};
    }
    return Node(d->tree, sibling);
}

bool Node::hasChanges() {
    return ts_node_has_changes(d->node);
}

bool Node::hasError() {
    return ts_node_has_error(d->node);
}

bool Node::equals(Node other) {
    return ts_node_eq(d->node, other.d->node);
}

bool Node::isNamed() {
    return ts_node_is_named(d->node);
}

bool Node::isNull() {
    return ts_node_is_null(d->node);
}

bool Node::isMissing() {
    return ts_node_is_missing(d->node);
}

std::string Node::sexpr() {
    return ts_node_string(d->node);
}

std::optional<Node> Node::child(int index) {
    if (index < 0) {
        //TODO: should an exception be thrown?
        return {};
    } else if (index >= children().size()) {
        return {};
    } else {
        return children()[index];
    }
}

std::optional<Node> Node::namedChild(int index) {
    if (index < 0) {
        //TODO: should an exception be thrown?
        return {};
    } else if (index >= namedChildren().size()) {
        return {};
    } else {
        return namedChildren()[index];
    }
}

std::optional<Node> Node::childForFieldId(int fieldId) {
    const auto child = ts_node_child_by_field_id(d->node, fieldId);
    if (ts_node_is_null(child)) {
        return {};
    }
    return Node(d->tree, child);
}

std::optional<Node> Node::childForFieldName(const std::string& fieldName) {
    const auto fields = d->tree->language().fields();
    const auto it = std::find(fields.begin(), fields.end(), fieldName);
    if (it == fields.end()) {
        return {};
    }
    const auto fieldId = it - fields.begin();
    return childForFieldId(fieldId);
}

Node Node::descendantForIndex(int index) {
    return descendantForIndex(index, index);
}

Node Node::descendantForIndex(int startIndex, int endIndex) {
    const auto start = startIndex;
    const auto end = endIndex > start ? start : endIndex;
    const auto node = ts_node_descendant_for_byte_range(d->node, start, end);
    return Node(d->tree, node);
}

std::vector<Node> Node::descendantsOfType(std::vector<std::string> types, Point startPosition, Point endPosition) {
    std::vector<Node> result;
    std::vector<int> symbols;
    const auto typesBySymbol = d->tree->language().nodeTypes();
    const auto length = typesBySymbol.size();
    for (int i=0; i<length; i++) {
        const auto it = std::find(types.begin(), types.end(), typesBySymbol[i]);
        if (it != types.end()) {
            symbols.push_back(i);
        }
    }

    TSPoint start_point = { startPosition.row, startPosition.column };
    TSPoint end_point = { endPosition.row, endPosition.column };

    if (end_point.row == 0 && end_point.column == 0) {
        end_point = TSPoint { UINT32_MAX, UINT32_MAX };
    }

    ts_tree_cursor_reset(&d->cursor, d->node);

    bool already_visited_children = false;
    while (true) {
        TSNode descendant = ts_tree_cursor_current_node(&d->cursor);

        if (!already_visited_children) {
            // If this node is before the selected range, then avoid
            // descending into it.
            if (point_lte(ts_node_end_point(descendant), start_point)) {
                if (ts_tree_cursor_goto_next_sibling(&d->cursor)) {
                    already_visited_children = false;
                } else {
                    if (!ts_tree_cursor_goto_parent(&d->cursor)) {
                        break;
                    }
                    already_visited_children = true;
                }
                continue;
            }

            // If this node is after the selected range, then stop walking.
            if (point_lte(end_point, ts_node_start_point(descendant))) {
                break;
            }

            // Add the node to the result if its type matches one of the given
            // node types.
            const auto it = std::find(symbols.begin(), symbols.end(), ts_node_symbol(descendant));
            if (it != symbols.end()) {
                result.push_back(Node(d->tree, descendant));
            }

            // Continue walking.
            if (ts_tree_cursor_goto_first_child(&d->cursor)) {
                already_visited_children = false;
            } else if (ts_tree_cursor_goto_next_sibling(&d->cursor)) {
                already_visited_children = false;
            } else {
                if (!ts_tree_cursor_goto_parent(&d->cursor)) {
                    break;
                }
                already_visited_children = true;
            }
        } else {
            if (ts_tree_cursor_goto_next_sibling(&d->cursor)) {
                already_visited_children = false;
            } else {
                if (!ts_tree_cursor_goto_parent(&d->cursor)) {
                    break;
                }
            }
        }
    }

    return result;
}

std::vector<Node> Node::descendantsOfType(std::string type, Point startPosition, Point endPosition) {
    std::vector<std::string> types;
    types.push_back(type);
    return descendantsOfType(types, startPosition, endPosition);
}

Node Node::namedDescendantForIndex(int index) {
    return namedDescendantForIndex(index, index);
}

Node Node::namedDescendantForIndex(int startIndex, int endIndex) {
    if (endIndex < startIndex) {
        endIndex = startIndex;
    }
    const auto node = ts_node_named_descendant_for_byte_range(d->node, startIndex, endIndex);
    return Node(d->tree, node);
}

Node Node::descendantForPosition(Point position) {
    return descendantForPosition(position, position);
}

Node Node::descendantForPosition(Point startPosition, Point endPosition) {
    const auto node = ts_node_descendant_for_point_range(d->node, startPosition, endPosition);
    return Node(d->tree, node);
}

Node Node::namedDescendantForPosition(Point position) {
    return namedDescendantForPosition(position, position);
}

Node Node::namedDescendantForPosition(Point start, Point end) {
    auto descendant = ts_node_named_descendant_for_point_range(d->node, start, end);
    return Node(d->tree, descendant);
}

Cursor Node::walk() {
    TSTreeCursor cursor = ts_tree_cursor_new(d->node);
    return Cursor(d->tree, cursor);
}
