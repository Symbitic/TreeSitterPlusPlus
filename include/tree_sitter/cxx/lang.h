/**
 * @file tree_sitter/cpp/lang.h
 * @brief Language support.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "tree_sitter/api.h"

namespace TreeSitter {

class Query;

/**
 * @brief A programming language.
 *
 * This class encapsulates access to a tree-sitter programming language.
 * At this time, the following built-in languages are supported.
 *
 *     - C
 *     - C++
 *     - C#
 *     - Go
 *     - Java
 *     - JavaScript
 *     - Python
 *     - Rust
 *     - TypeScript
 *     - TSX
 */
class Language {
public:
    /** Supported languages. */
    enum Syntax {
        C,
        Cpp,
        CSharp,
        Go,
        Java,
        JavaScript,
        Python,
        Rust,
        TypeScript,
        TSX
    };

    /**
     * @brief Construct a new Language object.
     */
    Language(Syntax syntax = Syntax::C);

    /**
     * @brief Create a custom language.
     */
    Language(const TSLanguage *lang);

    /** @internal Copy constructor. */
    Language(const Language& lang);
    /** @internal Copy assignment constructor. */
    Language& operator=(const Language& lang);
    /** Destructor. */
    ~Language();

    /** Parser version. */
    uint32_t version() const;
    /** Number of fields in this language. */
    int fieldCount() const;
    /** List of all recognized fields. */
    std::vector<std::string> fields() const;
    /** Number of node types in this language. */
    int nodeTypeCount() const;
    /** List of all recognized node types. */
    std::vector<std::string> nodeTypes() const;

    /**
     * @brief Allows access to the internal TSLanguage.
     *
     * @return Pointer to this TSLanguage.
     */
    const TSLanguage* language() const;

    /** Test if a field exists by ID. */
    bool hasFieldId(int fieldId);
    /** Test if a field exists by name. */
    bool hasFieldName(const std::string& fieldName);

    /** Return the name of a field. */
    std::string fieldNameForId(int fieldId);
    /** Return the ID of a field. */
    int fieldIdForName(const std::string& fieldName);

    /**
     * @brief Lookup the id for a node type.
     * @return int Node id (0 if not found).
     */
    int idForNodeType(const std::string& type, bool named);

    /**
     * @brief Lookup the name for a node type.
     * @return std::string Node name ("ERROR" if not found).
     */
    std::string nodeTypeForId(int typeId);

    /** Determines if a node type is named. */
    bool nodeTypeIsNamed(int typeId);
    /** Determines if a node type is visible. */
    bool nodeTypeIsVisible(int typeId);

    /**
     * @brief Query the source code.
     */
    Query query(const std::string &source);
private:
    void init();

    struct Private;
    std::unique_ptr<Private> d;
};

}
