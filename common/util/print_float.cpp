#include <cmath>

#include "third-party/fmt/core.h"
#include "common/goal_constants.h"
#include "third-party/dragonbox.h"
#include "print_float.h"
#include "common/util/Assert.h"

/*!
 * Convert a float to a string. The string is _always_ in this format:
 * [negative_sign] [at least 1 digit] [decimal point] [at least 1 digit]
 * and, if you trust the dragonbox library, should be the shortest possible representation
 * that round-trips through a properly implemented string -> float conversion.
 */
std::string float_to_string(float value, bool append_trailing_decimal) {
  constexpr int buff_size = 128;
  char buff[buff_size];
  float_to_cstr(value, buff, append_trailing_decimal);
  return {buff};
}

/*!
 * Wrapper around float_to_string, for printing meters. Unlike float_to_string, it does not append
 * decimals by default.
 */
std::string meters_to_string(float value, bool append_trailing_decimal) {
  return float_to_string(value / METER_LENGTH, append_trailing_decimal);
}

int float_to_cstr(float value, char* buffer, bool append_trailing_decimal) {
  ASSERT(std::isfinite(value));
  // dragonbox gives us:
  //  - an integer, representing the decimal value
  //  - sign
  //  - exponent

  int i = 0;

  // the exponent/significand representation of dragonbox is ambiguous with how it represents 0,
  // so just handle that as a special case
  if (value == 0) {
    buffer[i++] = '0';
    if (append_trailing_decimal) {
      buffer[i++] = '.';
      buffer[i++] = '0';
    }
    buffer[i++] = '\0';
  }

  auto decimal = jkj::dragonbox::to_decimal(value);

  // in all cases, we need to convert the decimal to characters.
  char digit_buff[64];
  int num_digits = 0;
  u64 significand = decimal.significand;
  while (significand) {
    digit_buff[num_digits++] = '0' + (significand % 10);
    significand /= 10;
  }

  if (decimal.exponent >= 0) {
    // needs 0 or more trailing zeros before decimal (no nonzeros after decimal).

    // print in four parts:
    // negative sign | digits | 000's | .0

    // part 1
    if (decimal.is_negative) {
      buffer[i++] = '-';
    }

    // part 2
    for (int digit = 0; digit < num_digits; digit++) {
      buffer[i++] = digit_buff[num_digits - (digit + 1)];
    }

    // part 3
    for (int j = 0; j < decimal.exponent; j++) {
      buffer[i++] = '0';
    }

    // part 4
    if (append_trailing_decimal) {
      buffer[i++] = '.';
      buffer[i++] = '0';
    }
    buffer[i++] = '\0';
  } else {
    // some nonzero digits after decimal.

    if (num_digits <= -decimal.exponent) {
      // all after the decimal
      // negative sign | 0. | 000's | digits

      // part 1
      if (decimal.is_negative) {
        buffer[i++] = '-';
      }

      // part 2
      buffer[i++] = '0';
      buffer[i++] = '.';

      // part 3
      int zeros = -decimal.exponent - num_digits;
      for (int j = 0; j < zeros; j++) {
        buffer[i++] = '0';
      }

      // part 4
      for (int digit = 0; digit < num_digits; digit++) {
        buffer[i++] = digit_buff[num_digits - (digit + 1)];
      }
      buffer[i++] = '\0';

    } else {
      // some before, some after.
      // negative sign | digits | . | digits

      // part 1
      if (decimal.is_negative) {
        buffer[i++] = '-';
      }

      // part 2
      int digits_before_decimal = num_digits + decimal.exponent;
      int digit = 0;
      for (; digit < digits_before_decimal; digit++) {
        buffer[i++] = digit_buff[num_digits - (digit + 1)];
      }

      // part 3
      buffer[i++] = '.';

      // part 4
      for (; digit < num_digits; digit++) {
        buffer[i++] = digit_buff[num_digits - (digit + 1)];
      }
      buffer[i++] = '\0';
    }
  }
  return i;
}
