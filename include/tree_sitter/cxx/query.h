/**
 * @file tree_sitter/cpp/query.h
 * @brief Source code query.
 */
#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "tree_sitter/api.h"
#include "tree_sitter/cxx/point.h"
#include "tree_sitter/cxx/node.h"

namespace TreeSitter {

/**
 * @brief A class to query souce code.
 */
class Query {
public:
    /**
     * @brief Options for querying source code.
     */
    struct Options {
        /** Max number of matches. */
        uint32_t matchLimit;
    };

    /**
     * @brief A query result.
     */
    struct Capture {
        /** Result name. */
        std::string name;
        /** Captured node. */
        Node node;
    };

    /**
     * @brief Query results.
     */
    struct Match {
        /** Match pattern. */
        uint32_t pattern;
        /** A list of captures. */
        std::vector<Capture> captures;
    };

    /**
     * @brief A source code type.
     */
    struct Operand {
        /** Operand name. */
        std::string name;
        /** Operand type. */
        std::string type;
    };

    /**
     * @brief A list of operators.
     */
    struct PredicateResult {
        /** Operator. */
        std::string operatorName;
        /** Operands. */
        std::vector<Operand> operands;
    };

    /** @private Query map */
    using Properties = std::unordered_map<std::string, std::string>;

    /** @internal Function for testing results. */
    using TextPredicate = std::function<bool (std::vector<Capture> captures)>;

    /** @internal Create a new Query. */
    Query(
        TSQuery* query,
        std::vector<std::string> captureNames,
        std::vector<std::vector<TextPredicate>> textPredicates,
        std::vector<std::vector<PredicateResult>> predicates,
        std::vector<Properties> setProperties,
        std::vector<Properties> assertedProperties,
        std::vector<Properties> refutedProperties
    );
    /** @internal Copy constructor. */
    Query(const Query& query);
    /** @internal Copy assignment constructor. */
    Query& operator=(const Query& query);
    /** @internal Destructor. */
    ~Query();

    /**
     * @brief A list of every captured name.
     * 
     * @return A list of strings.
     */
    std::vector<std::string> captureNames() const;

    /**
     * @brief Get a list of predicates.
     * 
     * @param patternIndex Pattern to use.
     * @return Predicates list.
     */
    std::vector<PredicateResult> predicatesForPattern(int patternIndex);

    /*
     * TODO:
     * Should I use `matches(Node node)`, `matches(Node node, Point startPosition)`
     * and other overloads?
     */

    /**
     * @brief Find matches for a query.
     */
    std::vector<Match> matches(Node node, Point startPosition = { 0, 0 }, Point endPosition = { 0, 0 }, Options options = { 0 });
    std::vector<Capture> captures(Node node, Point startPosition = { 0, 0 }, Point endPosition = { 0, 0 }, Options options = { 0 });

private:
    struct Private;
    std::unique_ptr<Private> d;
};

}
