// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew Glazar
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef QUICK_LINT_JS_ERROR_H
#define QUICK_LINT_JS_ERROR_H

#include <iosfwd>
#include <quick-lint-js/char8.h>
#include <quick-lint-js/cli-location.h>
#include <quick-lint-js/gmo.h>
#include <quick-lint-js/language.h>
#include <quick-lint-js/lex.h>
#include <quick-lint-js/location.h>

#define QLJS_X_ERROR_TYPES                                                     \
  QLJS_ERROR_TYPE(                                                             \
      error_assignment_before_variable_declaration,                            \
      {                                                                        \
        identifier assignment;                                                 \
        identifier declaration;                                                \
      },                                                                       \
      .error("variable assigned before its declaration"_gmo_message,       \
             assignment)                                                       \
          .note("variable declared here"_gmo_message, declaration))        \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_assignment_to_const_global_variable, { identifier assignment; },   \
      .error("assignment to const global variable"_gmo_message,            \
             assignment))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_assignment_to_const_variable,                                      \
      {                                                                        \
        identifier declaration;                                                \
        identifier assignment;                                                 \
        variable_kind var_kind;                                                \
      },                                                                       \
      .error("assignment to const variable"_gmo_message, assignment)       \
          .note("const variable declared here"_gmo_message, declaration))  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_assignment_to_const_variable_before_its_declaration,               \
      {                                                                        \
        identifier declaration;                                                \
        identifier assignment;                                                 \
        variable_kind var_kind;                                                \
      },                                                                       \
      .error("assignment to const variable before its declaration"_gmo_message,       \
             assignment)                                                       \
          .note("const variable declared here"_gmo_message, declaration))  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_assignment_to_undeclared_variable, { identifier assignment; },     \
      .error("assignment to undeclared variable"_gmo_message, assignment)) \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_big_int_literal_contains_decimal_point,                            \
      { source_code_span where; },                                             \
      .error("BigInt literal contains decimal point"_gmo_message, where))  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_big_int_literal_contains_exponent, { source_code_span where; },    \
      .error("BigInt literal contains exponent"_gmo_message, where))       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_cannot_declare_class_named_let, { source_code_span name; },        \
      .error("classes cannot be named 'let'"_gmo_message, name))           \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_cannot_declare_variable_named_let_with_let,                        \
      { source_code_span name; },                                              \
      .error("let statement cannot declare variables named 'let'"_gmo_message,        \
             name))                                                            \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_cannot_export_let, { source_code_span export_name; },              \
      .error("cannot export variable named 'let'"_gmo_message,             \
             export_name))                                                     \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_cannot_import_let, { source_code_span import_name; },              \
      .error("cannot import 'let'"_gmo_message, import_name))              \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_character_disallowed_in_identifiers,                               \
      { source_code_span character; },                                         \
      .error("character is not allowed in identifiers"_gmo_message,        \
             character))                                                       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_escaped_character_disallowed_in_identifiers,                       \
      { source_code_span escape_sequence; },                                   \
      .error(                                                                  \
          "escaped character is not allowed in identifiers"_gmo_message,   \
          escape_sequence))                                                    \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_escaped_code_point_in_identifier_out_of_range,                     \
      { source_code_span escape_sequence; },                                   \
      .error("code point out of range"_gmo_message, escape_sequence))      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_expected_expression_before_newline, { source_code_span where; },   \
      .error("expected expression before newline"_gmo_message, where))     \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_expected_expression_before_semicolon, { source_code_span where; }, \
      .error("expected expression before semicolon"_gmo_message, where))   \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_expected_hex_digits_in_unicode_escape,                             \
      { source_code_span escape_sequence; },                                   \
      .error("expected hexadecimal digits in Unicode escape sequence"_gmo_message,    \
             escape_sequence))                                                 \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_invalid_binding_in_let_statement, { source_code_span where; },     \
      .error("invalid binding in let statement"_gmo_message, where))       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_invalid_expression_left_of_assignment,                             \
      { source_code_span where; },                                             \
      .error("invalid expression left of assignment"_gmo_message, where))  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_invalid_lone_literal_in_object_literal,                            \
      { source_code_span where; },                                             \
      .error("invalid lone literal in object literal"_gmo_message, where)) \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_invalid_utf_8_sequence, { source_code_span sequence; },            \
      .error("invalid UTF-8 sequence"_gmo_message, sequence))              \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_keywords_cannot_contain_escape_sequences,                          \
      { source_code_span escape_sequence; },                                   \
      .error("keywords cannot contain escape sequences"_gmo_message,       \
             escape_sequence))                                                 \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_let_with_no_bindings, { source_code_span where; },                 \
      .error("let with no bindings"_gmo_message, where))                   \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_missing_comma_between_object_literal_entries,                      \
      { source_code_span where; },                                             \
      .error("missing comma between object literal entries"_gmo_message,   \
             where))                                                           \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_missing_operand_for_operator, { source_code_span where; },         \
      .error("missing operand for operator"_gmo_message, where))           \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_missing_semicolon_after_expression, { source_code_span where; },   \
      .error("missing semicolon after expression"_gmo_message, where))     \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_number_literal_contains_consecutive_underscores,                   \
      { source_code_span underscores; },                                       \
      .error(                                                                  \
          "number literal contains consecutive underscores"_gmo_message,   \
          underscores))                                                        \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_number_literal_contains_trailing_underscores,                      \
      { source_code_span underscores; },                                       \
      .error("number literal contains trailing underscore(s)"_gmo_message, underscores)) \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_octal_literal_may_not_have_exponent,                               \
      { source_code_span characters; },                                        \
      .error("octal literal may not have exponent"_gmo_message,            \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_octal_literal_may_not_have_decimal,                                \
      { source_code_span characters; },                                        \
      .error("octal literal may not have decimal"_gmo_message,             \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_octal_literal_may_not_be_big_int,                                  \
      { source_code_span characters; },                                        \
      .error("octal literal may not be BigInt"_gmo_message, characters))   \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_redeclaration_of_global_variable, { identifier redeclaration; },   \
      .error("redeclaration of global variable"_gmo_message,               \
             redeclaration))                                                   \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_redeclaration_of_variable,                                         \
      {                                                                        \
        identifier redeclaration;                                              \
        identifier original_declaration;                                       \
      },                                                                       \
      .error("redeclaration of variable: {0}"_gmo_message, redeclaration)  \
          .note("variable already declared here"_gmo_message,              \
                original_declaration))                                         \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_regexp_literal_flags_cannot_contain_unicode_escapes,               \
      { source_code_span escape_sequence; },                                   \
      .error("RegExp literal cannot contain Unicode escapes"_gmo_message,  \
             escape_sequence))                                                 \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_stray_comma_in_let_statement, { source_code_span where; },         \
      .error("stray comma in let statement"_gmo_message, where))           \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unclosed_block_comment, { source_code_span comment_open; },        \
      .error("unclosed block comment"_gmo_message, comment_open))          \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unclosed_identifier_escape_sequence,                               \
      { source_code_span escape_sequence; },                                   \
      .error("unclosed identifier escape sequence"_gmo_message,            \
             escape_sequence))                                                 \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unclosed_regexp_literal, { source_code_span regexp_literal; },     \
      .error("unclosed regexp literal"_gmo_message, regexp_literal))       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unclosed_string_literal, { source_code_span string_literal; },     \
      .error("unclosed string literal"_gmo_message, string_literal))       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unclosed_template, { source_code_span incomplete_template; },      \
      .error("unclosed template"_gmo_message, incomplete_template))        \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_at_character, { source_code_span character; },          \
      .error("unexpected '@'"_gmo_message, character))                     \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_backslash_in_identifier,                                \
      { source_code_span backslash; },                                         \
      .error("unexpected '\\' in identifier"_gmo_message, backslash))      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_characters_in_number, { source_code_span characters; }, \
      .error("unexpected characters in number literal"_gmo_message,        \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_control_character, { source_code_span character; },     \
      .error("unexpected control character"_gmo_message, character))       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_characters_in_binary_number,                            \
      { source_code_span characters; },                                        \
      .error("unexpected characters in binary literal"_gmo_message,        \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_characters_in_octal_number,                             \
      { source_code_span characters; },                                        \
      .error("unexpected characters in octal literal"_gmo_message,         \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_characters_in_hex_number,                               \
      { source_code_span characters; },                                        \
      .error("unexpected characters in hex literal"_gmo_message,           \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_no_digits_in_binary_number, { source_code_span characters; },      \
      .error("binary number literal has no digits"_gmo_message,            \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_no_digits_in_hex_number, { source_code_span characters; },         \
      .error("hex number literal has no digits"_gmo_message, characters))  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_no_digits_in_octal_number, { source_code_span characters; },       \
      .error("octal number literal has no digits"_gmo_message,             \
             characters))                                                      \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_hash_character, { source_code_span where; },            \
      .error("unexpected '#'"_gmo_message, where))                         \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unexpected_identifier, { source_code_span where; },                \
      .error("unexpected identifier"_gmo_message, where))                  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unmatched_indexing_bracket, { source_code_span left_square; },     \
      .error("unmatched indexing bracket"_gmo_message, left_square))       \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_unmatched_parenthesis, { source_code_span where; },                \
      .error("unmatched parenthesis"_gmo_message, where))                  \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_use_of_undeclared_variable, { identifier name; },                  \
      .error("use of undeclared variable: {0}"_gmo_message, name))         \
                                                                               \
  QLJS_ERROR_TYPE(                                                             \
      error_variable_used_before_declaration,                                  \
      {                                                                        \
        identifier use;                                                        \
        identifier declaration;                                                \
      },                                                                       \
      .error("variable used before declaration: {0}"_gmo_message, use)     \
          .note("variable declared here"_gmo_message, declaration))        \
  /* END */

namespace quick_lint_js {
#define QLJS_ERROR_TYPE(name, struct_body, format_call) struct name struct_body;
QLJS_X_ERROR_TYPES
#undef QLJS_ERROR_TYPE

class error_reporter {
 public:
  error_reporter() noexcept = default;

  error_reporter(const error_reporter &) noexcept = default;
  error_reporter &operator=(const error_reporter &) noexcept = default;

  error_reporter(error_reporter &&) noexcept = default;
  error_reporter &operator=(error_reporter &&) noexcept = default;

  virtual ~error_reporter() = default;

#define QLJS_ERROR_TYPE(name, struct_body, format) \
  virtual void report(name) = 0;
  QLJS_X_ERROR_TYPES
#undef QLJS_ERROR_TYPE

  virtual void report_fatal_error_unimplemented_character(
      const char *qljs_file_name, int qljs_line, const char *qljs_function_name,
      const char8 *character) = 0;
  virtual void report_fatal_error_unimplemented_token(
      const char *qljs_file_name, int qljs_line, const char *qljs_function_name,
      token_type, const char8 *token_begin) = 0;

  static void write_fatal_error_unimplemented_character(
      const char *qljs_file_name, int qljs_line, const char *qljs_function_name,
      const char8 *character, const cli_locator *, std::ostream &);
  static void write_fatal_error_unimplemented_token(
      const char *qljs_file_name, int qljs_line, const char *qljs_function_name,
      token_type, const char8 *token_begin, const cli_locator *,
      std::ostream &);
};

class null_error_reporter : public error_reporter {
 public:
  static null_error_reporter instance;

#define QLJS_ERROR_TYPE(name, struct_body, format) \
  void report(name) override {}
  QLJS_X_ERROR_TYPES
#undef QLJS_ERROR_TYPE

  void report_fatal_error_unimplemented_character(const char *, int,
                                                  const char *,
                                                  const char8 *) override {}
  void report_fatal_error_unimplemented_token(const char *, int, const char *,
                                              token_type,
                                              const char8 *) override {}
};
inline null_error_reporter null_error_reporter::instance;
}

#endif
