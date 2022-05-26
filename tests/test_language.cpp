#include <string>
#include "boost/ut.hpp"
#include "tree_sitter/cxx/lang.h"

#include <iostream>

using namespace boost::ut;
using namespace boost::ut::spec;
using namespace TreeSitter;

int main() {
    describe("Language") = [] {
        it("initializes without crashing") = [] {
            const auto fn = [] { Language lang; };
            expect(nothrow(fn));
        };

        it("converts between the string and integer representations of fields") = [] {
                Language JavaScript(Language::JavaScript);
                const int nameId = JavaScript.fieldIdForName("name");
                const int bodyId = JavaScript.fieldIdForName("body");

                expect(nameId < JavaScript.fieldCount());
                expect(bodyId < JavaScript.fieldCount());
                expect(JavaScript.fieldNameForId(nameId) == std::string("name"));
                expect(JavaScript.fieldNameForId(bodyId) == std::string("body"));
        };
        it("handles invalid fields") = [] {
            Language JavaScript(Language::JavaScript);

            expect(JavaScript.fieldIdForName("namezzz") == -1);
            expect(JavaScript.fieldNameForId(-1) == "");
            expect(JavaScript.fieldNameForId(10000) == "");
        };

        it("converts between the string and integer representations of a node type") = [] {
            Language JavaScript(Language::JavaScript);
            const auto exportStatementId = JavaScript.idForNodeType("export_statement", true);
            const auto starId = JavaScript.idForNodeType("*", false);

            expect(exportStatementId < JavaScript.nodeTypeCount());
            expect(starId < JavaScript.nodeTypeCount());
            expect(true == JavaScript.nodeTypeIsNamed(exportStatementId));
            expect(std::string("export_statement") == JavaScript.nodeTypeForId(exportStatementId));
            expect(false == JavaScript.nodeTypeIsNamed(starId));
            expect(std::string("*") == JavaScript.nodeTypeForId(starId));
        };

        it("handles invalid types") = [] {
            Language JavaScript(Language::JavaScript);

            expect(JavaScript.nodeTypeForId(-1) == "ERROR");
            expect(JavaScript.nodeTypeForId(10000) == "ERROR");
            expect(JavaScript.idForNodeType("export_statement", false) == 0);
        };
    };
}
