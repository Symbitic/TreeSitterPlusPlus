#ifndef TREE_SITTER_LANGS_H_
#define TREE_SITTER_LANGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TREE_SITTER_LANGUAGE_VERSION
typedef struct TSLanguage TSLanguage;
#endif

const TSLanguage *tree_sitter_c();
const TSLanguage *tree_sitter_cpp();
const TSLanguage *tree_sitter_c_sharp();
const TSLanguage *tree_sitter_go();
const TSLanguage *tree_sitter_java();
const TSLanguage *tree_sitter_javascript();
const TSLanguage *tree_sitter_python();
const TSLanguage *tree_sitter_rust();
const TSLanguage *tree_sitter_typescript();
const TSLanguage *tree_sitter_tsx();

#ifdef __cplusplus
}
#endif

#endif  // TREE_SITTER_API_H_