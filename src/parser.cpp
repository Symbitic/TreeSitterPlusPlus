#include <stdexcept>
#include "tree_sitter/cxx/parser.h"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/tree.h"

using namespace TreeSitter;

static void noop(
    const std::string& message,
    std::unordered_map<std::string, std::string> params,
    const std::string& type
) {}

struct Parser::Private {
    int timeout = 0;
    Logger logger = Logger(noop);
    Language lang = Language();
    TSParser* parser = ts_parser_new();
};

Parser::Parser()
    : d(std::make_unique<Private>()) {}

Parser::Parser(Language::Syntax syntax) : Parser() {
    setLanguage(Language(syntax));
}

Parser::Parser(const Parser& parser)
    : d(std::make_unique<Private>(*parser.d)) { }

Parser::~Parser() {
    ts_parser_delete(d->parser);
};

void Parser::reset() {
    ts_parser_reset(d->parser);
}

Language Parser::language() const {
    return d->lang;
}

void Parser::setLanguage(Language lang) {
    const auto version = ts_language_version(lang.language());
    if (version < TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION ||
        TREE_SITTER_LANGUAGE_VERSION < version)
    {
        throw std::runtime_error("Incompatible language version");
    }

    d->lang = lang;
    ts_parser_set_language(d->parser, d->lang.language());
}

Logger Parser::logger() const {
    return d->logger;
}

void Parser::setLogger(Logger logger) {
    d->logger = logger;
    auto cb = [](void *payload, TSLogType type, const char *message_str) -> void {
        Parser* parser = static_cast<Parser*>(payload);
        std::string message(message_str);
        std::string param_sep = " ";
        size_t param_sep_pos = message.find(param_sep, 0);
        const std::string type_name = type == TSLogTypeParse ? "parse" : "lex";
        const std::string name = message.substr(0, param_sep_pos);
        std::unordered_map<std::string, std::string> params;
        while (param_sep_pos != std::string::npos) {
            size_t key_pos = param_sep_pos + param_sep.size();
            size_t value_sep_pos = message.find(":", key_pos);

            if (value_sep_pos == std::string::npos) {
                break;
            }

            size_t val_pos = value_sep_pos + 1;
            param_sep = ", ";
            param_sep_pos = message.find(param_sep, value_sep_pos);
            std::string key = message.substr(key_pos, (value_sep_pos - key_pos));
            std::string value = message.substr(val_pos, (param_sep_pos - val_pos));
            params[key] = value;
        }
        parser->logger()(name, params, type_name);
    };
    TSLogger log = { this, cb };
    ts_parser_set_logger(d->parser, log);
}

void Parser::resetLogger() {
    d->logger = noop;
    TSLogger logger = { d->parser, nullptr };
    ts_parser_set_logger(d->parser, logger);
}

uint64_t Parser::timeout() const {
    return ts_parser_timeout_micros(d->parser);
}

void Parser::setTimeout(uint64_t value) {
    ts_parser_set_timeout_micros(d->parser, value);
}

Tree Parser::parse(const std::string& input) {
    TSTree* tree = ts_parser_parse_string(d->parser, nullptr, input.c_str(), input.length());
    // TODO: if (tree == nullptr) {}
    return Tree(tree, language(), input);
}

Tree Parser::parse(Tree oldTree, const std::string& input) {
    TSTree* tree = ts_parser_parse_string(d->parser, oldTree.tree(), input.c_str(), input.length());
    // TODO: if (tree == nullptr) {}
    return Tree(tree, language(), input);
}
