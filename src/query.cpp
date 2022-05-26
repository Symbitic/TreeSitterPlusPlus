#include "tree_sitter/cxx/cursor.h"
#include "tree_sitter/cxx/lang.h"
#include "tree_sitter/cxx/node.h"
#include "tree_sitter/cxx/query.h"
#include "tree_sitter/api.h"

using namespace TreeSitter;

struct Query::Private {
    TSQuery* query;
    TSQueryCursor* cursor;
    std::vector<std::string> captureNames;
    std::vector<std::vector<TextPredicate>> textPredicates;
    std::vector<std::vector<PredicateResult>> predicates;
    std::vector<Properties> setProperties;
    std::vector<Properties> assertedProperties;
    std::vector<Properties> refutedProperties;
    bool exceededMatchLimit;
};

Query::Query(
    TSQuery* query,
    std::vector<std::string> captureNames,
    std::vector<std::vector<TextPredicate>> textPredicates,
    std::vector<std::vector<PredicateResult>> predicates,
    std::vector<Properties> setProperties,
    std::vector<Properties> assertedProperties,
    std::vector<Properties> refutedProperties
)
    : d(std::make_unique<Private>())
{
    d->query = query;
    d->captureNames = captureNames;
    d->textPredicates = textPredicates;
    d->predicates = predicates;
    d->setProperties = setProperties;
    d->assertedProperties = assertedProperties;
    d->refutedProperties = refutedProperties;
    d->exceededMatchLimit = false;
    d->cursor = ts_query_cursor_new();
}

Query::Query(const Query& query)
    : d(std::make_unique<Private>(*query.d)) { }

Query& Query::operator=(const Query &query) {
    *d = *query.d;
    return *this;
}

Query::~Query() {
    ts_query_cursor_delete(d->cursor);
    ts_query_delete(d->query);
}

std::vector<std::string> Query::captureNames() const {
    return d->captureNames;
}

struct MatchResult {
    uint32_t pattern;
    uint32_t captureCount;
    uint32_t captureIndex;
    std::vector<TSQueryCapture> captures;
};

// ts_query_matches_wasm
static std::tuple<uint32_t, std::vector<MatchResult>, bool> queryMatches(
    TSQuery* query,
    TSQueryCursor* cursor,
  Node node,
  uint32_t start_row,
  uint32_t start_column,
  uint32_t end_row,
  uint32_t end_column,
  uint32_t match_limit
) {
    if (match_limit == 0) {
        ts_query_cursor_set_match_limit(cursor, UINT32_MAX);
    } else {
        ts_query_cursor_set_match_limit(cursor, match_limit);
    }

  TSPoint start_point = { start_row, start_column };
  TSPoint end_point = { end_row, end_column };
  ts_query_cursor_set_point_range(cursor, start_point, end_point);
  ts_query_cursor_exec(cursor, query, node.node());

  uint32_t index = 0;
  uint32_t match_count = 0;
  TSQueryMatch match;
  std::vector<MatchResult> results;

  while (ts_query_cursor_next_match(cursor, &match)) {
    match_count++;
    MatchResult result;
    result.pattern = match.pattern_index;
    result.captureCount = match.capture_count;
    for (uint32_t i=0; i<match.capture_count; i++) {
        const TSQueryCapture *capture = &match.captures[i];
        TSQueryCapture c = { capture->node, capture->index };
        result.captures.push_back(c);
    }
    results.push_back(result);
  }

  bool did_exceed_match_limit =
    ts_query_cursor_did_exceed_match_limit(cursor);
  return std::make_tuple(match_count, results, did_exceed_match_limit);
}

static std::tuple<uint32_t, std::vector<MatchResult>, bool> queryCaptures(
    TSQuery* query,
    TSQueryCursor* cursor,
    Node node,
  uint32_t start_row,
  uint32_t start_column,
  uint32_t end_row,
  uint32_t end_column,
  uint32_t match_limit
) {
  if (match_limit == 0) {
    ts_query_cursor_set_match_limit(cursor, UINT32_MAX);
  } else {
    ts_query_cursor_set_match_limit(cursor, match_limit);
  }

  TSPoint start_point = { start_row, start_column };
  TSPoint end_point = { end_row, end_column };
  ts_query_cursor_set_point_range(cursor, start_point, end_point);
  ts_query_cursor_exec(cursor, query, node.node());

  uint32_t index = 0;
  uint32_t capture_count = 0;
  TSQueryMatch match;
  uint32_t capture_index;
  std::vector<MatchResult> result;

  while (ts_query_cursor_next_capture(
    cursor,
    &match,
    &capture_index
  )) {
    capture_count++;
    MatchResult matchResult;
    matchResult.pattern = match.pattern_index;
    matchResult.captureCount = match.capture_count;
    matchResult.captureIndex = capture_index;
    for (uint32_t i=0; i<match.capture_count; i++) {
        const TSQueryCapture *capture = &match.captures[i];
        TSQueryCapture c = { capture->node, capture->index };
        matchResult.captures.push_back(c);
    }
    result.push_back(matchResult);
  }

  bool did_exceed_match_limit =
    ts_query_cursor_did_exceed_match_limit(cursor);
  return std::make_tuple(capture_count, result, did_exceed_match_limit);
}

std::vector<Query::Match> Query::matches(Node node, Point startPosition, Point endPosition, Options options) {
    const uint32_t matchLimit = options.matchLimit;
    //std::vector<Match> result;

    auto ret = queryMatches(
        d->query,
        d->cursor,
        node,
      startPosition.row,
      startPosition.column,
      endPosition.row,
      endPosition.column,
      matchLimit
    );

    uint32_t rawCount = std::get<0>(ret);
    std::vector<MatchResult> matchResults = std::get<1>(ret);
    bool didExceedMatchLimit = std::get<2>(ret);
    std::vector<Match> result(rawCount);

    d->exceededMatchLimit = didExceedMatchLimit;
    auto tree = node.tree();

    for (uint32_t i=0; i<rawCount; i++) {
        const uint32_t pattern = matchResults[i].pattern;
        const uint32_t captureCount = matchResults[i].captureCount;
        std::vector<Capture> captures;
        captures.reserve(captureCount);

        for (int j=0; j<captureCount; j++) {
            TSQueryCapture capture = matchResults[i].captures.at(j);
            std::string name = d->captureNames[capture.index];
            Node n(tree, capture.node);
            captures.push_back({ name, n });
        }

        bool every = true;
        for (auto fn : d->textPredicates[pattern]) {
            if (!fn(captures)) {
                every = false;
                break;
            }
        }
        if (every) {
            result.push_back({ pattern, captures });
        }
    }

    result.shrink_to_fit();

    return result;
}

std::vector<Query::Capture> Query::captures(Node node, Point startPosition, Point endPosition, Options options) {
    uint32_t matchLimit = options.matchLimit;

    auto ret = queryCaptures(
        d->query,
        d->cursor,
        node,
      startPosition.row,
      startPosition.column,
      endPosition.row,
      endPosition.column,
      matchLimit
    );
    uint32_t count = std::get<0>(ret);
    std::vector<MatchResult> matchResults = std::get<1>(ret);
    const bool didExceedMatchLimit = std::get<2>(ret);
    std::vector<Capture> result;
    d->exceededMatchLimit = didExceedMatchLimit;
    auto tree = node.tree();

    for (uint32_t i=0; i<count; i++) {
        const uint32_t pattern = matchResults[i].pattern;
        const uint32_t captureCount = matchResults[i].captureCount;
        const uint32_t captureIndex = matchResults[i].captureIndex;
        std::vector<Capture> captures;

        for (auto c : matchResults[i].captures) {
            std::string name = d->captureNames[c.index];
            Node n(tree, c.node);
            captures.push_back({ name, n });
        }

        bool every = true;
        for (auto fn : d->textPredicates[pattern]) {
            if (!fn(captures)) {
                every = false;
                break;
            }
        }
        if (every) {
            Capture capture = captures[captureIndex];
            result.push_back(capture);
        }
    }

    return result;
}

std::vector<Query::PredicateResult> Query::predicatesForPattern(int patternIndex) {
    if (patternIndex >= d->predicates.size()) {
        return {};
    }
    return d->predicates[patternIndex];
}
