/*
Unfinished.
See https://github.com/tree-sitter/tree-sitter/blob/master/lib/binding_web/test/query-test.js
*/
#include <algorithm>
#include <map>
#include <string>
#include "boost/ut.hpp"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/parser.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/query.h"
#include "tree_sitter/cxx/tree.h"

#include <iostream>

using namespace boost::ut;
using namespace boost::ut::spec;
using namespace TreeSitter;

template <class E>
constexpr auto doesthrow(Language lang, const std::string query, const std::string err) {
    return expect(nothrow([&lang, &query, &err]{
        try {
            lang.query(query);
        } catch (E e) {
            expect(e.what() == err);
        }
    }));
}

struct Capture {
    std::string name;
    std::string text;
};
struct Match {
    uint32_t pattern;
    std::vector<Capture> captures;
};

int main() {
    describe("Query") = [] {
        describe(".constructor") = [] {
            it("throws an error on invalid patterns") = [] {
                Language JavaScript(Language::JavaScript);
                doesthrow<std::logic_error>(JavaScript,
                    "(function_declaration wat)",
                    "Invalid syntax at offset 22 ('wat)')"
                ) << "Captures invalid syntax";
                doesthrow<std::range_error>(JavaScript,
                    "(non_existent)",
                    "Invalid node name 'non_existent'"
                ) << "Captures invalid nodes";
                doesthrow<std::range_error>(JavaScript,
                    "(function_declaration non_existent:(identifier))",
                    "Invalid field name 'non_existent'"
                ) << "Captures invalid fields";
                doesthrow<std::runtime_error>(JavaScript,
                    "(function_declaration name:(statement_block))",
                    "Invalid structure at offset 22 ('name:(statement_block))')"
                ) << "Captures invalid structures";
            };

            it("throws an error on invalid predicates") = [] {
                Language JavaScript(Language::JavaScript);
                doesthrow<std::range_error>(JavaScript,
                    "((identifier) @abc (#eq? @ab hi))",
                    "Invalid capture name @ab"
                ) << "Captures invalid captures";
                doesthrow<std::range_error>(JavaScript,
                    "((identifier) @abc (#eq?))",
                    "Wrong number of arguments to `#eq?` predicate"
                ) << "Captures wrong number of arguments";
            };
        };
    };
}
