#include <algorithm>
#include <map>
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

int main() {
    describe("Parser") = [] {
        it("initializes without crashing") = [] {
            Parser parser(Language::JavaScript);
        };

        describe(".setLogger()") = [] {
            it("calls the given callback for each parse event") = [] {
                Parser parser(Language::JavaScript);
                std::map<std::string, bool> debugMessages = {};
                Logger logger = [&debugMessages](
                    const std::string& message,
                    LoggerParams params,
                    const std::string& type
                ) {
                    debugMessages[message] = true;
                };
                parser.setLogger(logger);
                parser.parse("a + b + c");
                expect(true == debugMessages.contains("reduce"));
                expect(true == debugMessages.contains("accept"));
                expect(true == debugMessages.contains("shift"));
            };
            it("can disable debugging") = [] {
                Parser parser(Language::JavaScript);
                std::map<std::string, bool> debugMessages = {};
                Logger logger = [&debugMessages](
                    const std::string& message,
                    LoggerParams params,
                    const std::string& type
                ) {
                    debugMessages[message] = true;
                };
                parser.setLogger(logger);
                parser.resetLogger();
                parser.parse("a + b + c");
                expect(0 == debugMessages.size());
            };
        };

        describe(".parse()") = [] {
            it("can handle long input strings") = []() {
                Parser parser(Language::JavaScript);
                const int repeatCount = 10000;
                std::string inputString;
                inputString.reserve((repeatCount*2)+2);
                inputString.append("[");
                for (int i=0; i<repeatCount; i++) {
                    inputString.append("0,");
                }
                inputString.append("]");

                auto tree = parser.parse(inputString);
                auto node = tree.rootNode()
                    .firstChild().value()
                    .firstChild().value();
                expect("program" == tree.rootNode().type());
                expect(repeatCount == node.namedChildCount());
            };
            it("can use the C++ parser") = []() {
                Parser parser(Language::Cpp);
                auto tree = parser.parse("const char *s = R\"EOF(HELLO WORLD)EOF\";");
                auto node = tree.rootNode().firstChild().value();
                expect(4 == node.childCount());
                expect("const" == node.child(0).value().text());
                expect("char" == node.child(1).value().text());
            };
            it("can use the Python parser") = []() {
                Parser parser(Language::Python);
                auto tree = parser.parse("class A:\n  def b():\n    c()");
                auto node = tree.rootNode().firstChild().value();
                expect(4 == node.childCount());
                expect("class" == node.child(0).value().text());
                expect("A" == node.child(1).value().text());
            };
            it("can use the Rust parser") = []() {
                Parser parser(Language::Rust);
                auto tree = parser.parse("const x: &'static str = r###\"hello\"###;");
                auto node = tree.rootNode().firstChild().value();
                expect(7 == node.childCount());
                expect("const" == node.child(0).value().text());
                expect("x" == node.child(1).value().text());
            };
        };
    };
}
