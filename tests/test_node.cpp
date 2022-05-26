#include <string>
#include "boost/ut.hpp"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/parser.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/tree.h"

#include <iostream>

using namespace boost::ut;
using namespace boost::ut::spec;
using namespace TreeSitter;

bool operator==(Point rhs, const Point &lhs) {
    return rhs.column == lhs.column && rhs.row == lhs.row;
}

int main() {
    describe("Node") = [] {
        describe(".children") = [] {
            it("returns an array of child nodes") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("x10 + 1000");
                expect(1 == tree.rootNode().childCount());
                const auto sumNode = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                expect(3 == sumNode.childCount());
                expect("identifier" == sumNode.children()[0].type());
                expect("+" == sumNode.children()[1].type());
                expect("number" == sumNode.children()[2].type());
            };
        };
        describe(".namedChildren") = [] {
            it("returns an array of named child nodes") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("x10 + 1000");
                expect(1 == tree.rootNode().childCount());
                const auto sumNode = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                expect(2 == sumNode.namedChildCount());
                expect("identifier" == sumNode.namedChildren()[0].type());
                expect("number" == sumNode.namedChildren()[1].type());
            };
        };
        describe(".startIndex and .endIndex") = [] {
            it("returns the character index where the node starts/ends in the text") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("a👍👎1 / b👎c👎");
                const auto node = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                expect(0 == node.startIndex());
                expect(23 == node.endIndex());
                /*
                expect(0 == node.children()[0].startIndex());
                expect(6 == node.children()[0].endIndex());
                expect(7 == node.children()[1].startIndex());
                expect(8 == node.children()[1].endIndex());
                expect(9 == node.children()[2].startIndex());
                expect(15 == node.children()[2].endIndex());
                */
            };
        };
        describe(".startPosition and .endPosition") = [] {
            it("returns the row and column where the node starts/ends in the text") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("x10 + 1000");
                const auto node = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                expect("binary_expression" == node.type());

                expect(Point({ 0, 0 }) == node.startPosition());
                expect(Point({ 0, 10 }) == node.endPosition());
                expect(Point({ 0, 0 }) == node.children()[0].startPosition());
                expect(Point({ 0, 3 }) == node.children()[0].endPosition());
                expect(Point({ 0, 4 }) == node.children()[1].startPosition());
                expect(Point({ 0, 5 }) == node.children()[1].endPosition());
                expect(Point({ 0, 6 }) == node.children()[2].startPosition());
                expect(Point({ 0, 10 }) == node.children()[2].endPosition());
            };
        };
        describe(".parent()") = [] {
            it("returns the node's parent") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("x10 + 1000");
                auto sumNode = tree.rootNode()
                    .firstChild().value();
                auto variableNode = sumNode
                    .firstChild().value();
                expect(sumNode == variableNode.parent());
                expect(tree.rootNode() == sumNode.parent());
            };
        };
        describe(".child()") = [] {
            it("returns null when the node has no children") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("x10 + 1000");
                auto sumNode = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                auto variableNode = sumNode
                    .firstChild().value();
                expect(false == variableNode.firstChild().has_value());
                expect(false == variableNode.lastChild().has_value());
                expect(false == variableNode.firstNamedChild().has_value());
                expect(false == variableNode.lastNamedChild().has_value());
                expect(false == variableNode.child(1).has_value());
            };
        };
        describe(".nextSibling and .previousSibling") = [] {
            it("returns the node's next and previous sibling") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("x10 + 1000");
                auto node = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                expect(node.child(1).value() == node.child(0).value().nextSibling().value());
                expect(node.child(2).value() == node.child(1).value().nextSibling().value());
                expect(node.child(0).value() == node.child(1).value().previousSibling().value());
                expect(node.child(1).value() == node.child(2).value().previousSibling().value());
            };
        };
        describe(".hasError()") = [] {
            it("returns true if the node contains an error") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("1 + 2 * * 3");
                auto node = tree.rootNode();
                auto sum = node
                    .firstChild().value()
                    .firstChild().value();
                expect(true == sum.hasError());
                expect(false == sum.child(0).value().hasError());
                expect(false == sum.child(1).value().hasError());
                expect(true == sum.child(2).value().hasError());
            };
        };
        describe(".isMissing()") = [] {
            it("returns true if the node is missing from the source") = [] {
                Parser parser(Language::JavaScript);
                auto tree = parser.parse("(2 ||)");
                auto node = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value()
                    .firstNamedChild().value();
                expect("binary_expression" == node.type());
                expect(true == node.hasError());
                expect(true != node.child(0).value().isMissing());
                expect(true != node.child(1).value().isMissing());
                expect(true == node.child(2).value().isMissing());
            };
        };
        describe(".text()") = [] {
            it("returns the text of a generated node") = [] {
                Parser parser(Language::JavaScript);
                std::string source("const mysum = 2 * 2");
                auto tree = parser.parse(source);
                auto node = tree.rootNode().firstChild().value();
                auto childNode = node.child(1).value();
                expect(source == node.text());
                expect("const" == node.child(0).value().text());
                expect("mysum = 2 * 2" == node.child(1).value().text());
                expect("mysum" == childNode.child(0).value().text());
                expect("=" == childNode.child(1).value().text());
                expect("2 * 2" == childNode.child(2).value().text());
            };
        };
    };
}
