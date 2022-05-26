#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include "tree_sitter/api.h"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/query.h"
#include "tree_sitter/langs.h"

using namespace TreeSitter;

static const std::regex QUERY_WORD_REGEX("([\\w-.]*)");

struct Step {
    std::string type;
    std::string name;
    std::string value;
};

struct Language::Private {
    int version = 0;
    int fieldCount = 0;
    int nodeTypeCount = 0;
    const TSLanguage* lang = nullptr;
    std::unordered_map<int, std::string> types = {};
    std::unordered_map<int, std::string> fields = {};
};

Language::Language(Syntax syntax)
    : d(std::make_unique<Private>())
{
    switch (syntax) {
    case Syntax::C:
        d->lang = tree_sitter_c();
        break;
    case Syntax::Cpp:
        d->lang = tree_sitter_cpp();
        break;
    case Syntax::CSharp:
        d->lang = tree_sitter_c_sharp();
        break;
    case Syntax::Go:
        d->lang = tree_sitter_go();
        break;
    case Syntax::Java:
        d->lang = tree_sitter_java();
        break;
    case Syntax::JavaScript:
        d->lang = tree_sitter_javascript();
        break;
    case Syntax::Python:
        d->lang = tree_sitter_python();
        break;
    case Syntax::Rust:
        d->lang = tree_sitter_rust();
        break;
    case Syntax::TypeScript:
        d->lang = tree_sitter_typescript();
        break;
    case Syntax::TSX:
        d->lang = tree_sitter_tsx();
        break;
    default:
        throw std::runtime_error("Unrecognized language");
    }
    init();
}

Language::Language(const TSLanguage *lang)
    : d(std::make_unique<Private>())
{
  if (lang == nullptr) {
    throw std::runtime_error("Invalid language pointer");
  }
  d->lang = lang;
  init();
}

Language::Language(const Language& lang)
    : d(std::make_unique<Private>(*lang.d)) { }

Language& Language::operator=(const Language &lang) {
    *d = *lang.d;
    return *this;
}

Language::~Language() = default;

void Language::init() {
    const int symbolCount = ts_language_symbol_count(d->lang);
    const int fieldCount = ts_language_field_count(d->lang);

    for (int i=0; i<symbolCount; i++) {
        if (ts_language_symbol_type(d->lang, i) < 2) {
            d->types[i] = ts_language_symbol_name(d->lang, i);
        }
    }

    for (int i=0; i<fieldCount; i++) {
        const char* fieldName = ts_language_field_name_for_id(d->lang, i);
        if (fieldName != nullptr) {
            d->fields[i] = fieldName;
        }
    }

    d->nodeTypeCount = symbolCount;
    d->fieldCount = fieldCount;
    d->version = ts_language_version(d->lang);
}

uint32_t Language::version() const {
    return d->version;
}

int Language::fieldCount() const {
    return d->fieldCount;
}

std::vector<std::string> Language::fields() const {
    std::vector<std::string> fields;
    for (auto field : d->fields) {
        fields.push_back(field.second);
    }
    return fields;
}

int Language::nodeTypeCount() const {
    return d->nodeTypeCount;
}

std::vector<std::string> Language::nodeTypes() const {
    std::vector<std::string> nodeTypes;
    for (auto nodeType : d->types) {
        nodeTypes.push_back(nodeType.second);
    }
    return nodeTypes;
}

const TSLanguage* Language::language() const {
    return d->lang;
}

// TODO: should ret == 0 return -1 instead? 0 = error here.
int Language::idForNodeType(const std::string& type, bool named) {
    int ret = ts_language_symbol_for_name(d->lang, type.c_str(), type.length(), named);
    return ret;
}

std::string Language::nodeTypeForId(int typeId) {
    const auto ret = ts_language_symbol_name(this->d->lang, typeId);
    if (ret == nullptr) {
      return "ERROR";
    }
    return std::string(ret);
}

bool Language::nodeTypeIsNamed(int typeId) {
    const TSSymbolType symbolType = ts_language_symbol_type(d->lang, typeId);
    return symbolType == TSSymbolTypeRegular;
}

bool Language::nodeTypeIsVisible(int typeId) {
    const TSSymbolType symbolType = ts_language_symbol_type(d->lang, typeId);
    return symbolType <= TSSymbolTypeAnonymous;
}

bool Language::hasFieldId(int fieldId) {
    return d->fields.find(fieldId) != d->fields.end();
}

std::string Language::fieldNameForId(int fieldId) {
    if (!hasFieldId(fieldId)) {
        return "";
    }
    return d->fields.find(fieldId)->second;
}

int Language::fieldIdForName(const std::string& fieldName) {
    for (auto field : d->fields) {
        if (field.second == fieldName) {
            return field.first;
        }
    }
    return -1;
}

bool Language::hasFieldName(const std::string &fieldName) {
    // TODO: might keep vector<std::string> fieldNames for faster lookup.
    return fieldIdForName(fieldName) == -1;
}

Query Language::query(const std::string &source) {
    uint32_t errorOffset; // TRANSFER_BUFFER
    TSQueryError errorId; // TRANSFER_BUFFER + SIZE_OF_INT

    TSQuery* address = ts_query_new(
      d->lang,
      source.c_str(),
      source.length(),
      &errorOffset,
      &errorId
    );

    if (!address) {
      std::ostringstream err;
      std::smatch match;
      const std::string suffixStr = source.substr(errorOffset, 100);
      const auto end = source.find("\n");
      const std::string suffix = suffixStr.substr(0, end);
      std::regex_search(suffix, match, QUERY_WORD_REGEX);
      const std::string word = match[0].str();

      switch (errorId) {
        case TSQueryErrorNodeType:
          err << "Invalid node name '" << word << "'";
          throw std::range_error(err.str());
        case TSQueryErrorField:
          err << "Invalid field name '" << word << "'";
          throw std::range_error(err.str());
        case TSQueryErrorCapture:
          err << "Invalid capture name @" << word;
          throw std::range_error(err.str());
        case TSQueryErrorStructure:
          // TypeError
          err << "Invalid structure at offset " << errorOffset << " ('" << suffix << "')";
          throw std::runtime_error(err.str());
        default:
          // SyntaxError
          err << "Invalid syntax at offset " << errorOffset << " ('" << suffix << "')";
          throw std::logic_error(err.str());
      }
    }

    const uint32_t stringCount = ts_query_string_count(address);
    const uint32_t captureCount = ts_query_capture_count(address);
    const uint32_t patternCount = ts_query_pattern_count(address);
    std::vector<std::string> captureNames(captureCount);
    std::vector<std::string> stringValues(stringCount);
    for (int i=0; i<captureCount; i++) {
      uint32_t len;
      const auto nameAddress = ts_query_capture_name_for_id(
        address,
        i,
        &len
      );
      captureNames.push_back(std::string(nameAddress, len));
    }

    for (int i=0; i<stringCount; i++) {
      uint32_t len;
      const auto valueAddress = ts_query_string_value_for_id(
        address,
        i,
        &len
      );
      stringValues.push_back(std::string(valueAddress, len));
    }

    std::vector<Query::Properties> setProperties(patternCount);
    std::vector<Query::Properties> assertedProperties(patternCount);
    std::vector<Query::Properties> refutedProperties(patternCount);
    std::vector<std::vector<Query::PredicateResult>> predicates(patternCount);
    std::vector<std::vector<Query::TextPredicate>> textPredicates(patternCount);

    for (int i=0; i<patternCount; i++) {
      uint32_t stepCount;
      const TSQueryPredicateStep* predicatesAddress = ts_query_predicates_for_pattern(
        address,
        i,
        &stepCount
      );

      predicates[i] = {};
      textPredicates[i] = {};

      std::vector<Step> steps;
      const TSQueryPredicateStep* stepAddress = predicatesAddress;

      for (uint32_t j=0; j<stepCount; j++) {
        const auto stepType = stepAddress[i].type;
        const uint32_t stepValueId = stepAddress[i].value_id;
        if (stepType == TSQueryPredicateStepTypeCapture) {
          steps.push_back({ "capture", captureNames[stepValueId], "" });
        } else if (stepType == TSQueryPredicateStepTypeString) {
          steps.push_back({ "string", "", stringValues[stepValueId] });
        } else if (steps.size() > 0) {
          if (steps[0].type != "string") {
            throw std::runtime_error("Predicates must begin with a literal value");
          }
          const std::string operatorName = steps[0].value;
          bool isPositive = true;
          if (operatorName == "not-eq?") {
            isPositive = false;
          } else if (operatorName == "eq?") {
            if (steps.size() != 3) {
              //Expected 2, got ${steps.length - 1}
              throw std::runtime_error("Wrong number of arguments to `#eq?` predicate");
            } else if(steps[1].type != "capture") {
              //Got "${steps[1].value}:
              throw std::runtime_error("First argument of `#eq?` predicate must be a capture");
            } else if (steps[2].type == "capture") {
              const std::string captureName1 = steps[1].name;
              const std::string captureName2 = steps[2].name;
              Query::TextPredicate fn = [&captureName1, &captureName2, &isPositive](std::vector<Query::Capture> captures) -> bool {
                std::optional<Node> node1;
                std::optional<Node> node2;
                for (auto c : captures) {
                  if (c.name == captureName1) node1 = c.node;
                  if (c.name == captureName2) node2 = c.node;
                }
                if (!node1.has_value() || !node2.has_value()) return true;
                return (node1.value().text() == node2.value().text()) == isPositive;
              };
              textPredicates[i].push_back(fn);
            } else {
              const std::string captureName = steps[1].name;
              const std::string stringValue = steps[2].value;
              Query::TextPredicate fn = [&captureName, &stringValue, &isPositive](std::vector<Query::Capture> captures) -> bool {
                for (auto c : captures) {
                  if (c.name == captureName) {
                    return (c.node.text() == stringValue) == isPositive;
                  };
                }
                return true;
              };
              textPredicates[i].push_back(fn);
            }
          } else if (operatorName == "not-match?") {
            isPositive = false;
          } else if (operatorName == "match?") {
            if (steps.size() != 3) {
              // Expected 2, got ${steps.length - 1}
              throw std::runtime_error("Wrong number of arguments to `#match?` predicate");
            } else if (steps[1].type != "capture") {
              // Got "${steps[1].value}"
              throw std::runtime_error("First argument of `#match?` predicate must be a capture");
            } else if (steps[2].type != "string") {
              // Got @${steps[2].value}
              throw std::runtime_error("Second argument of `#match?` predicate must be a string");
            }
            const std::string captureName = steps[1].name;
            const std::regex regex(steps[2].value);
            Query::TextPredicate fn = [&captureName, &regex, &isPositive](std::vector<Query::Capture> captures) -> bool {
              for (auto c : captures) {
                if (c.name == captureName) {
                  return std::regex_match(c.name, regex) == isPositive;
                }
              }
              return true;
            };
            textPredicates[i].push_back(fn);
          } else if (operatorName == "set!") {
            if (steps.size() < 2 || steps.size() > 3) {
              // Expected 1 or 2. Got ${steps.length - 1}
              throw std::runtime_error("Wrong number of arguments to `#set!` predicate");
            }
            for (auto s : steps) {
              if (s.type != "string") {
                throw std::runtime_error("Arguments to `#set!` predicate must be a strings");
              }
            }
            //if (!setProperties[i]) setProperties[i] = {};
            //setProperties[i][steps[1].value] = steps[2] ? steps[2].value : "";
            setProperties[i][steps[1].value] = steps.size() == 3 ? steps[2].value : "";
          } else if (operatorName == "is?" || operatorName == "is-not?") {
            if (steps.size() < 2 || steps.size() > 3) {
              const std::string err = operatorName == "is?"
                ? "Wrong number of arguments to `#is?` predicate"
                : "Wrong number of arguments to `#is-not?` predicate";
              throw std::runtime_error(err);
            }
            for (auto s : steps) {
              if (s.type != "string") {
                const std::string err = operatorName == "is?"
                  ? "Arguments to `#is?` predicate must be a strings"
                  : "Arguments to `#is-not?` predicate must be a strings";
                throw std::runtime_error(err);
              }
            }
            if (operatorName == "is?") {
              //if (!assertedProperties[i]) assertedProperties[i] = {};
              assertedProperties[i][steps[1].value] = steps.size() == 3 ? steps[2].value : "";
            } else {
              //if (!refutedProperties[i]) refutedProperties[i] = {};
              refutedProperties[i][steps[1].value] = steps.size() == 3 ? steps[2].value : "";
            }
          } else {
            // predicates[i].push({operator, operands: steps.slice(1)});
            std::vector<Query::Operand> ops;
            for (int i=1; i<steps.size(); i++) {
              Query::Operand op = { steps[i].name, steps[i].type };
              ops.push_back(op);
            }
            predicates[i].push_back({ operatorName, ops });
          }
          steps.clear();
        }
      }
    }

    return Query(
      address,
      captureNames,
      textPredicates,
      predicates,
      setProperties,
      assertedProperties,
      refutedProperties
    );
}
