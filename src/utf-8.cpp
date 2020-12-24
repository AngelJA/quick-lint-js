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

#include <quick-lint-js/narrow-cast.h>
#include <quick-lint-js/padded-string.h>
#include <quick-lint-js/unreachable.h>
#include <quick-lint-js/utf-8.h>
#include <quick-lint-js/warning.h>

namespace quick_lint_js {
// See: https://www.unicode.org/versions/Unicode11.0.0/ch03.pdf
char8* encode_utf_8(char32_t code_point, char8* out) {
  auto byte = [](char32_t x) -> char8 {
    QLJS_ASSERT(x <= 0x100);
    return static_cast<char8>(x);
  };
  char32_t continuation = 0b1000'0000;
  if (code_point >= 0x10000) {
    *out++ = byte(0b1111'0000 | (code_point >> 18));
    *out++ = byte(continuation | ((code_point >> 12) & 0b0011'1111));
    *out++ = byte(continuation | ((code_point >> 6) & 0b0011'1111));
    *out++ = byte(continuation | ((code_point >> 0) & 0b0011'1111));
  } else if (code_point >= 0x0800) {
    *out++ = byte(0b1110'0000 | (code_point >> 12));
    *out++ = byte(continuation | ((code_point >> 6) & 0b0011'1111));
    *out++ = byte(continuation | ((code_point >> 0) & 0b0011'1111));
  } else if (code_point >= 0x80) {
    *out++ = byte(0b1100'0000 | (code_point >> 6));
    *out++ = byte(continuation | ((code_point >> 0) & 0b0011'1111));
  } else {
    *out++ = byte(code_point);
  }
  return out;
}

namespace {
QLJS_WARNING_PUSH
QLJS_WARNING_IGNORE_GCC("-Wattributes")
// See: https://www.unicode.org/versions/Unicode11.0.0/ch03.pdf
[[gnu::always_inline]] decode_utf_8_result
    decode_utf_8_inline(padded_string_view input) noexcept {
  auto is_continuation_byte = [](std::uint8_t byte) noexcept -> bool {
    return (byte & 0b1100'0000) == 0b1000'0000;
  };

  enum byte_class : signed char {
    X = 0,
    A, // 0x00..0x7f
    B, // 0x80..0x8f
    C, // 0xc2..0xdf
    D, // 0xe0
    E, // 0xa0..0xbf
    F, // 0xe1..0xec
  };
  static constexpr byte_class byte_to_class[256/*@@@*/] = {
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x00..0x0f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x10..0x1f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x20..0x2f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x30..0x3f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x40..0x4f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x50..0x5f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x60..0x6f
    A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A, // 0x70..0x7f
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B, // 0x80..0x8f
    X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X, // 0x90..0x9f
    E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E, // 0xa0..0xaf
    E,E,E,E,E,E,E,E,E,E,E,E,E,E,E,E, // 0xb0..0xbf
    X,X,C,C,C,C,C,C,C,C,C,C,C,C,C,C, // 0xc0..0xcf
    C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C, // 0xd0..0xdf
    D,F,F,F,F,F,F,F,F,F,F,F,F, // 0xe0..0xef
  };

  enum class state : signed char {
    nyi = -50, // @@@ delete
    er2 = -12,
    er1 = -11,
    ok3 = -3,
    ok2 = -2,
    ok1 = -1,
    initial = 0,

    // U+0080..U+07FF
    c, // 0xc2..0xdf

    // U+0800..U+0FFF
    d, // 0xe0
    de, // 0xe0 0xa0..0xbf

    // U+1000..U+CFFF
    f, // 0xe1..0xec
    ff, // 0xe1..0xec 0x80..0xbf
  };

  static constexpr state state_transitions[][6] = {
    // initial     c           d           de          f           ff
      {state::nyi, state::er1, state::er1, state::er2, state::nyi, state::nyi}, // X @@@ delete
      {state::ok1, state::er1, state::er1, state::er2, state::er1, state::er2}, // A 0x00..0x7f
      {state::er1, state::ok2, state::er1, state::ok3, state::ff,  state::ok3}, // B 0x80..0x8f
      {state::c,   state::er1, state::er1, state::er2, state::er1, state::er2}, // C 0xc2..0xdf
      {state::d,   state::er1, state::er1, state::er2, state::er1, state::er2}, // D 0xe0
      {state::er1, state::ok2, state::de,  state::ok3, state::ff,  state::ok3}, // E 0xa0..0xbf
      {state::f,   state::er1, state::er1, state::er2, state::er1, state::er2}, // F 0xe1..0xec
  };
  static auto is_end_state = [](state s) -> bool {
    return static_cast<signed char>(s) < 0;
  };

  const std::uint8_t* c = reinterpret_cast<const std::uint8_t*>(input.data());
  // @@@ fold this into the state machine.
  if (input.size() == 0) {
    return decode_utf_8_result{
        .size = 0,
        .code_point = 0,
        .ok = false,
    };
  }

  const std::uint8_t* cc = c; // @@@ rename
  state current_state = state::initial;
  for (;;) {
    current_state = state_transitions[byte_to_class[*cc]][static_cast<signed char>(current_state)];
    if (is_end_state(current_state)) {
      break;
    }
    ++cc;
  }
  switch (current_state) {
    case state::initial:
    case state::c:
    case state::f:
    case state::ff:
    case state::d:
    case state::de:
      QLJS_UNREACHABLE();
      break;
    case state::er1:
      return decode_utf_8_result{
          .size = 1,
          .code_point = 0,
          .ok = false,
      };
    case state::er2:
      return decode_utf_8_result{
          .size = 2,
          .code_point = 0,
          .ok = false,
      };
    case state::ok1:
      return decode_utf_8_result{
          .size = 1,
          .code_point = *c,
          .ok = true,
      };
    case state::ok2:
      return decode_utf_8_result{
          .size = 2,
          .code_point =
              (char32_t(c[0] & 0b0001'1111) << 6) | (c[1] & 0b0011'1111),
          .ok = true,
      };
    case state::ok3:
      return decode_utf_8_result{
          .size = 3,
          .code_point = (char32_t(c[0] & 0b0000'1111) << 12) |
                        (char32_t(c[1] & 0b0011'1111) << 6) |
                        (c[2] & 0b0011'1111),
          .ok = true,
      };
    case state::nyi:
      goto nyi;
  }

nyi:
  if (c[0] <= 0x7f) {
    // 1-byte sequence (0x00..0x7f, i.e. ASCII).
    return decode_utf_8_result{
        .size = 1,
        .code_point = *c,
        .ok = true,
    };
  } else if ((c[0] & 0b1110'0000) == 0b1100'0000) {
    // 2-byte sequence (0xc0..0xdf).
    static_assert(padded_string::padding_size >= 1);
    bool byte_0_ok = c[0] >= 0xc2;
    bool byte_1_ok = is_continuation_byte(c[1]);
    if (byte_0_ok && byte_1_ok) {
      return decode_utf_8_result{
          .size = 2,
          .code_point =
              (char32_t(c[0] & 0b0001'1111) << 6) | (c[1] & 0b0011'1111),
          .ok = true,
      };
    } else {
      return decode_utf_8_result{
          .size = 1,
          .code_point = 0,
          .ok = false,
      };
    }
  } else if ((c[0] & 0b1111'0000) == 0b1110'0000) {
    // 3-byte sequence (0xe0..0xef).
    static_assert(padded_string::padding_size >= 2);
    bool byte_1_ok = (c[0] == 0xe0 ? 0xa0 <= c[1] && c[1] <= 0xbf
                                   : c[0] == 0xed ? 0x80 <= c[1] && c[1] <= 0x9f
                                                  : is_continuation_byte(c[1]));
    bool byte_2_ok = is_continuation_byte(c[2]);
    if (byte_1_ok && byte_2_ok) {
      return decode_utf_8_result{
          .size = 3,
          .code_point = (char32_t(c[0] & 0b0000'1111) << 12) |
                        (char32_t(c[1] & 0b0011'1111) << 6) |
                        (c[2] & 0b0011'1111),
          .ok = true,
      };
    } else {
      return decode_utf_8_result{
          .size = byte_1_ok ? 2 : 1,
          .code_point = 0,
          .ok = false,
      };
    }
  } else if ((c[0] & 0b1111'1000) == 0b1111'0000) {
    // 4-byte sequence (0xf0..0xf7).
    static_assert(padded_string::padding_size >= 3);
    bool byte_0_ok = c[0] <= 0xf4;
    bool byte_1_ok = (c[0] == 0xf0 ? 0x90 <= c[1] && c[1] <= 0xbf
                                   : c[0] == 0xf4 ? 0x80 <= c[1] && c[1] <= 0x8f
                                                  : is_continuation_byte(c[1]));
    bool byte_2_ok = is_continuation_byte(c[2]);
    bool byte_3_ok = is_continuation_byte(c[3]);
    if (byte_0_ok && byte_1_ok && byte_2_ok && byte_3_ok) {
      return decode_utf_8_result{
          .size = 4,
          .code_point = (char32_t(c[0] & 0b0000'0111) << 18) |
                        (char32_t(c[1] & 0b0011'1111) << 12) |
                        (char32_t(c[2] & 0b0011'1111) << 6) |
                        (c[3] & 0b0011'1111),
          .ok = true,
      };
    } else {
      return decode_utf_8_result{
          .size = byte_0_ok && byte_1_ok ? byte_2_ok ? 3 : 2 : 1,
          .code_point = 0,
          .ok = false,
      };
    }
  } else {
    // Continuation byte (0x80..0xbf), or 5-byte or longer sequence
    // (0xf8..0xff).
    return decode_utf_8_result{
        .size = 1,
        .code_point = 0,
        .ok = false,
    };
  }
}
QLJS_WARNING_POP
}

decode_utf_8_result decode_utf_8(padded_string_view input) noexcept {
  return decode_utf_8_inline(input);
}

const char8* advance_lsp_characters_in_utf_8(string8_view utf_8,
                                             int character_count) noexcept {
  const char8* c = utf_8.data();
  const char8* end = c + utf_8.size();
  if (narrow_cast<std::size_t>(character_count) >= utf_8.size()) {
    return end;
  }

  // TODO(strager): Avoid this copy!
  padded_string utf_8_copy(utf_8);
  c = utf_8_copy.data();
  end = utf_8_copy.null_terminator();

  int characters = 0;
  while (characters < character_count && c != end) {
    decode_utf_8_result result =
        decode_utf_8_inline(padded_string_view(c, end));
    if (result.ok && result.code_point >= 0x10000) {
      // Count non-BMP characters as two characters.
      if (characters + 1 >= character_count) {
        // Middle of implied UTF-16 surrogate pair. Stop at the beginning of
        // the character. We can't reasonably return the middle of the
        // character; there is no valid middle of a UTF-8 character.
        break;
      }
      c += result.size;
      characters += 2;
    } else if (result.ok) {
      c += result.size;
      characters += 1;
    } else {
      // Count invalid sequences as one character per byte.
      c += 1;
      characters += 1;
    }
  }

  return utf_8.data() + (c - utf_8_copy.data());
}

std::ptrdiff_t count_lsp_characters_in_utf_8(padded_string_view utf_8,
                                             int offset) noexcept {
  const char8* c = utf_8.data();
  const char8* end = utf_8.null_terminator();
  const char8* stop = c + offset;
  std::ptrdiff_t count = 0;
  while (c < stop) {
    decode_utf_8_result result = decode_utf_8(padded_string_view(c, end));
    if (result.ok) {
      if (c + result.size > stop) {
        break;
      }
      c += result.size;
      if (result.code_point >= 0x10000) {
        count += 2;
      } else {
        count += 1;
      }
    } else {
      c += 1;
      count += 1;
    }
  }
  return count;
}
}
