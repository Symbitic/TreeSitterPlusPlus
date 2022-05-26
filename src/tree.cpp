#include "tree_sitter/cxx/cursor.h"
#include "tree_sitter/cxx/tree.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/lang.h"

using namespace TreeSitter;

struct Tree::Private {
    TSTree* tree = nullptr;
    Language lang;
    std::string source = "";
};

Tree::Tree(TSTree* tree, Language lang, const std::string &source)
    : d(std::make_unique<Private>())
{
    d->tree = tree;
    d->lang = lang;
    d->source = source;
}

Tree::Tree(const Tree& tree)
    : d(std::make_unique<Private>(*tree.d)) { }

Tree& Tree::operator=(const Tree &tree) {
    *d = *tree.d;
    return *this;
}

Tree::~Tree() {
    ts_tree_delete(d->tree);
}

Tree Tree::copy() {
    auto newTree = ts_tree_copy(d->tree);
    return Tree(newTree, d->lang, d->source);
}

Node Tree::rootNode() const {
    return Node(this, ts_tree_root_node(d->tree));
}

std::string Tree::source() const {
    return d->source;
}

TSTree* Tree::tree() const {
    return d->tree;
}

void Tree::edit(Edit delta) {
    TSInputEdit edit;
    edit.start_byte = delta.startIndex;
    edit.old_end_byte = delta.oldEndIndex;
    edit.new_end_byte = delta.newEndIndex;
    edit.start_point = delta.startPosition;
    edit.old_end_point = delta.oldEndPosition;
    edit.new_end_point = delta.newEndPosition;
    ts_tree_edit(d->tree, &edit);
}

Language Tree::language() const {
    return d->lang;
}

Cursor Tree::walk() {
    Node node = rootNode();
    return node.walk();
}

std::vector<Range> Tree::getChangedRanges(Tree other) {
    uint32_t range_count;
    std::vector<Range> result;
    TSRange *ranges = ts_tree_get_changed_ranges(d->tree, other.tree(), &range_count);
    for (uint32_t i=0; i<range_count; i++) {
        result.push_back(ranges[i]);
    }
    return result;
}
