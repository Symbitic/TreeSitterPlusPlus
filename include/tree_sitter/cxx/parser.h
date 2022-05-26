/**
 * @file tree_sitter/cpp/parser.h
 * @brief Source code parser.
 */
#pragma once

#include <functional>
#include <memory>
#include <string>
#include "tree_sitter/cxx/point.h"
#include "tree_sitter/cxx/lang.h"

namespace TreeSitter {

class Tree;

/**
 * @brief Key-value parameters for logging.
 */
using LoggerParams = std::unordered_map<std::string, std::string>;

/**
 * @brief Logging callback.
 * 
 * @param message Log message.
 * @param params Log params.
 * @param type Log domain.
 */
using Logger = std::function<void (
    const std::string& message,
    LoggerParams params,
    const std::string& type
)>;

/**
 * @brief Parse source code into an AST.
 * 
 * This is the main entry-point for users.
 * A parser is initialzed from a chosen language,
 * then parse() is called to parse a source code string into an AST.
 */
class Parser {
public:
    /**
     * @brief Construct a new Parser object.
     * By default, Language::C is used.
     */
    Parser();

    /**
     * @brief Construct a new Parser object
     * 
     * @param syntax Programming language to parse.
     */
    Parser(Language::Syntax syntax);
    /** @private Copy constructor. */
    Parser(const Parser& parser);
    /** Destructor. */
    ~Parser();

    /** Parse source code into an AST. */
    Tree parse(const std::string& input);

    /** Parse source code into an AST. */
    Tree parse(Tree oldTree, const std::string& input);

    /** Reset the internal state. */
    void reset();
  
    /** The current language. */
    Language language() const;
    /** Set the current language. */
    void setLanguage(Language language);

    /** The current logger. */
    Logger logger() const;
    /** Set the current logger. */
    void setLogger(Logger logFunc);
    /** Reset the logger. */
    void resetLogger();

    /**
     * @brief Get the timeout in milliseconds.
     * 
     * @return uint64_t Timeout length.
     */
    uint64_t timeout() const;

    /**
     * @brief Set the timeout in milliseconds.
     * 
     * @param value Timeout length.
     */
    void setTimeout(uint64_t value);
private:
    struct Private;
    std::unique_ptr<Private> d;
};

}
