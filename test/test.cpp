/*
 * Copyright (c) 2014 Anders Wang Kristensen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#endif

#include <ujson/ujson.hpp>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <future>

#ifndef M_PI
#define M_E         2.7182818284590452354
#define M_LOG2E     1.4426950408889634074
#define M_LOG10E    0.43429448190325182765
#define M_LN2       0.69314718055994530942
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.78539816339744830962
#define M_1_PI      0.31830988618379067154
#define M_2_PI      0.63661977236758134308
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT1_2   0.70710678118654752440
#endif

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static char *utf32_to_utf8(char *str, std::uint32_t cp) {
    
    assert(cp <= 0x10FFFF);
    
    std::size_t num_bytes;
    if (cp <= 0x7F)
        num_bytes = 1;
    else if (cp <= 0x7FF)
        num_bytes = 2;
    else if (cp <= 0xFFFF)
        num_bytes = 3;
    else
        num_bytes = 4;
    
    static const std::uint32_t offset[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };
    switch (num_bytes) {
        case 4:
            str[3] = static_cast<char>((cp | 0x80) & 0xBF);
            cp >>= 6;
        case 3:
            str[2] = static_cast<char>((cp | 0x80) & 0xBF);
            cp >>= 6;
        case 2:
            str[1] = static_cast<char>((cp | 0x80) & 0xBF);
            cp >>= 6;
        case 1:
            str[0] = static_cast<char>(cp | offset[num_bytes]);
    }
    
    return str + num_bytes;
}

TEST_CASE("info") {
    std::printf("sizeof(ujson::value) = %lu bytes\n", sizeof(ujson::value));
    std::printf("sizeof(std::string) = %lu bytes\n", sizeof(std::string));
#ifdef UJSON_SHORT_STRING_OPTIMIZATION
    std::printf("short string max length = %u bytes\n", ujson::sso_max_length);
#endif
    std::printf("sizeof(std::shared_ptr<>) = %lu bytes\n",
                sizeof(std::shared_ptr<int>));
}

TEST_CASE("null") {

    using namespace ujson;

    value null0;
    REQUIRE(null0.is_null());
    REQUIRE(null0.type() == value_type::null);
    value null1(1);
    null1 = value();
    REQUIRE(null1.type() == value_type::null);
    value null2(1);
    null2 = null;
    REQUIRE(null2.type() == value_type::null);

    // parsing
    REQUIRE(parse("null") == null);
}

TEST_CASE("boolean") {

    using namespace ujson;

    value bool0(true);
    REQUIRE(bool0.is_boolean());
    REQUIRE(bool0.type() == value_type::boolean);
    REQUIRE(bool_cast(bool0) == true);
    bool0 = false;
    REQUIRE(bool0.type() == value_type::boolean);
    REQUIRE(bool_cast(bool0) == false);

    // true
    auto true_value = parse("true");
    REQUIRE(true_value.type() == value_type::boolean);
    REQUIRE(bool_cast(true_value) == true);

    // false
    auto false_value = parse("false");
    REQUIRE(false_value.type() == value_type::boolean);
    REQUIRE(bool_cast(std::move(false_value)) == false);
    REQUIRE(false_value.is_null());
}

TEST_CASE("number") {

    using namespace ujson;

    // ints
    value int0(std::numeric_limits<std::int32_t>::min());
    REQUIRE(int0.is_number());
    REQUIRE(int0.type() == value_type::number);
    REQUIRE(int32_cast(int0) == std::numeric_limits<std::int32_t>::min());
    value int1;
    int1 = std::numeric_limits<std::int32_t>::max();
    REQUIRE(int1.type() == value_type::number);
    REQUIRE(int32_cast(std::move(int1)) == std::numeric_limits<std::int32_t>::max());
    REQUIRE(int1.is_null());
    value int2(1e10);
    REQUIRE_THROWS(int32_cast(int2));
    int2 = -1e10;
    REQUIRE_THROWS(int32_cast(int2));

    // uints
    value uint0(std::numeric_limits<std::uint32_t>::min());
    REQUIRE(uint0.type() == value_type::number);
    REQUIRE(uint32_cast(uint0) == std::numeric_limits<std::uint32_t>::min());
    value uint1;
    uint1 = std::numeric_limits<std::uint32_t>::max();
    REQUIRE(uint1.type() == value_type::number);
    REQUIRE(uint32_cast(std::move(uint1)) == std::numeric_limits<std::uint32_t>::max());
    REQUIRE(uint1.is_null());
    value uint2(1e10);
    REQUIRE_THROWS(uint32_cast(uint2));
    uint2 = -1e0;
    REQUIRE_THROWS(uint32_cast(uint2));

    auto test_int = [](std::int32_t i) {
        auto str = to_string(i);
        auto v = parse(str);
        int e = int32_cast(v);
        REQUIRE(i == e);
    };

    // test all ints.. slow!
    std::int64_t start = std::numeric_limits<std::int32_t>::min();
    std::int64_t end = std::numeric_limits<std::int32_t>::max();
    std::int64_t step = 100000;
    for (std::int64_t i = start; i <= end; i += step) {
        test_int(static_cast<std::int32_t>(i));
    }

    // test some ints
    REQUIRE(parse("0") == 0);
    REQUIRE(parse("-0") == 0);
    REQUIRE(parse("1234") == 1234);
    REQUIRE(parse("-4321") == -4321);

    // doubles
    auto test_double = [](double d) {
        auto str = to_string(d);
        auto v = parse(str);
        double e = double_cast(std::move(v));
        REQUIRE(d == e);
        REQUIRE(v.is_null());
    };

    test_double(M_E);
    test_double(M_LOG2E);
    test_double(M_LOG10E);
    test_double(M_LN2);
    test_double(M_LN10);
    test_double(M_PI);
    test_double(M_PI_2);
    test_double(M_PI_4);
    test_double(M_1_PI);
    test_double(M_2_PI);
    test_double(M_2_SQRTPI);
    test_double(M_SQRT2);
    test_double(M_SQRT1_2);

    // infinity
    double infp = std::numeric_limits<double>::infinity();
    REQUIRE_THROWS(value v = infp);
    double infn = -std::numeric_limits<double>::infinity();
    REQUIRE_THROWS(value v = infn);

    // NaN
    double qnan = std::numeric_limits<double>::quiet_NaN();
    REQUIRE_THROWS(value v = qnan);
    double snan = std::numeric_limits<double>::signaling_NaN();
    REQUIRE_THROWS(value v = snan);

    // numbers require decimal before and after point
    REQUIRE_THROWS(parse("10."));
    REQUIRE_THROWS(parse(".01"));

    // decimal
    REQUIRE(parse("0.01") == 0.01);
    REQUIRE(parse("1000.01") == 1000.01);

    // scienfic
    REQUIRE(parse("1e3") == 1e3);
    REQUIRE(parse("1E3") == 1e3);
    REQUIRE(parse("2e-2") == 2e-2);
    REQUIRE(parse("2E-2") == 2e-2);
    REQUIRE(parse("3e+3") == 3e+3);
    REQUIRE(parse("3E+3") == 3e+3);
    REQUIRE(parse("1.23e3") == 1.23e3);
    REQUIRE(parse("1.23E3") == 1.23e3);
    REQUIRE(parse("2.23e-2") == 2.23e-2);
    REQUIRE(parse("2.23E-2") == 2.23e-2);
    REQUIRE(parse("3.23e+3") == 3.23e+3);
    REQUIRE(parse("3.23E+3") == 3.23e+3);

    // invalid numbers (thanks codestation)
    REQUIRE_THROWS(parse("1k2"));
    REQUIRE_THROWS(parse("1k2  "));

    // overflow
    REQUIRE_THROWS(parse("1.8e+308"));
}

TEST_CASE("string") {

    using namespace ujson;

#ifdef UJSON_SHORT_STRING_OPTIMIZATION
    // find std::string sso buffer size
    std::size_t sso;
    for (sso = sizeof(std::string); sso >= 1; --sso) {
        std::string str(sso, 'x');
        const char *ptr = str.data();
        const char *foo = reinterpret_cast<const char *>(&str);
        if (ptr >= foo && ptr < foo + sizeof(std::string))
            break;
    }
    
    // ensure small-string-optimization buffer size is correct
    REQUIRE(sso_max_length == sso);
#elif defined UJSON_REF_COUNTED_STRING
    // test that std::string really is ref counted
    std::string foo_bar("foo_bar");
    auto foo_bar_copy = foo_bar;
    REQUIRE(foo_bar.c_str() == foo_bar_copy.c_str());
#endif

    // string casts
    REQUIRE_THROWS(string_cast(null));
    REQUIRE_THROWS(string_cast(M_PI));
    const char *hello = "Hello, world!";
    REQUIRE(string_cast(hello) == hello);
    value hello_value(hello);
    REQUIRE(hello_value.is_string());
    REQUIRE(hello_value.type() == value_type::string);
    REQUIRE(string_cast(value(hello)) == hello);
    REQUIRE(std::strcmp(string_cast(hello_value).c_str(), hello) == 0);

    // move construct string into value
    std::string long_string(sizeof(string) + 1, 'x');
    const char *long_string_storage = long_string.c_str();
    value long_string_value(std::move(long_string));
    REQUIRE(string_cast(long_string_value).c_str() == long_string_storage);
    
    // move out again
    long_string = string_cast(std::move(long_string_value));
    REQUIRE(long_string.c_str() == long_string_storage);

    // move assign into into value
    long_string_value = std::move(long_string);
    REQUIRE(string_cast(long_string_value).c_str() == long_string_storage);

    // move out again
    long_string = string_cast(std::move(long_string_value));
    REQUIRE(long_string.c_str() == long_string_storage);

    // move assign into into value and copy value so they share string storage
    long_string_value = std::move(long_string);
    value shared_storage = long_string_value;
    REQUIRE(string_cast(shared_storage) == string_cast(long_string_value));

    // now can't move out, due to sharing, so a copy is made instead
    std::string long_string_copy = string_cast(std::move(shared_storage));
#ifdef UJSON_SHORT_STRING_OPTIMIZATION
    // must be a copy
    REQUIRE(long_string_copy.c_str() != long_string_storage);
#elif defined UJSON_REF_COUNTED_STRING
    // for refence counted strings, there is no copy
    REQUIRE(long_string_copy.c_str() == long_string_storage);
#endif

    // move out again, since no longer shared
    long_string = string_cast(std::move(long_string_value));
    REQUIRE(long_string.c_str() == long_string_storage);

    // test empty string
    REQUIRE(string_cast("").length() == 0);

    // assignment
    ujson::value assigned;
    std::string tmp("test");
    assigned = tmp;
    REQUIRE(assigned == tmp);
    tmp = "\xFF";
    REQUIRE_THROWS(assigned = tmp);

    // string with embedded zeros
    const char *zeros = "\0foo\0bar\0";
    REQUIRE(std::memcmp(string_cast(value(zeros, 9)).c_str(), zeros, 9) == 0);

    auto test_string = [](const char *str) {
        auto v = value(str);
        std::string json = to_string(v);
        REQUIRE(parse(json) == v);
    };

    // control chars
    test_string("\b\f\n\r\t");

    // quotes / backslash
    test_string("quotes > \"hello\" < ");
    test_string("backslash > \\ < ");

    // 0x01-0x7F (ascii subset, except quote, backslash, and control chars)
    for (int i = 0x1; i <= 0x7F; ++i) {

        if (i == '\\' || i == '\"')
            continue;

        if (i == '\b' || i == '\f' || i == '\n' || i == '\r' || i == '\t')
            continue;

        char tmp[2] = { static_cast<char>(i), '\0' };
        test_string(tmp);
    }

    // premature end of string
    REQUIRE_THROWS(parse("\"Hello, wor"));

    // examples from RFC-3629
    auto quote = [](const char *p) { return std::string("\"") + p + '"'; };

    // A<NOT IDENTICAL TO><ALPHA> (U+0041 U+2262 U+0391 U+002E)
    const char *example = "\x41\xE2\x89\xA2\xCE\x91\x2E";
    REQUIRE(parse(quote("\\u0041\\u2262\\u0391\\u002e")) == example);

    // missing trailing surrogate
    REQUIRE_THROWS(parse(quote("\\uD800")));

    // wrong trailing surrogate
    REQUIRE_THROWS(parse(quote("\\uD800\\uDBFF")));
    REQUIRE_THROWS(parse(quote("\\uD800\\uE000")));

    // Korean "hangugeo" (U+D55C U+AD6D U+C5B4)
    const char *korean = "\xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4";
    REQUIRE(parse(quote(korean)) == korean);

    // Japanese "nihongo" (U+65E5 U+672C U+8A9E)
    const char *japanese = "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E";
    REQUIRE(parse(quote(japanese)) == japanese);

    // Chinese "stump of tree" + BOM (U+FEFF U+233B4)
    const char *chinese = "\xEF\xBB\xBF\xF0\xA3\x8E\xB4";
    REQUIRE(parse(quote(chinese)) == chinese);
    REQUIRE(parse(quote("\\uFEFF\\uD84C\\uDFB4")) == chinese);

    // missing continuation
    const char *missing = "\xF0\xA3\x8";
    REQUIRE_THROWS(value v(missing));
    REQUIRE_THROWS(parse(quote(missing)));

    // non-shortest form UTF-8
    const char *non_shortest_form = "\xE0\x81\x81";
    REQUIRE_THROWS(value v(non_shortest_form));
    REQUIRE_THROWS(parse(quote(non_shortest_form)));

    // ill-formed
    const char *ill_formed1 = "\xC0\xAF";
    REQUIRE_THROWS(value v(ill_formed1));
    REQUIRE_THROWS(parse(quote(ill_formed1)));
    const char *ill_formed2 = "\xC1\xAF";
    REQUIRE_THROWS(value v(ill_formed2));
    REQUIRE_THROWS(parse(quote(ill_formed2)));
    const char *ill_formed3 = "\xE0\x9F\x80";
    REQUIRE_THROWS(value v(ill_formed3));
    REQUIRE_THROWS(parse(quote(ill_formed3)));
    REQUIRE_THROWS(parse(quote("\xF5")));

    // well-formed
    const char *well_formed1 = "\xF4\x80\x83\x92";
    REQUIRE_NOTHROW(value v(well_formed1));
    REQUIRE(parse(quote(well_formed1)) == well_formed1);

#if 1
    // test all utf-8 codepoints both as utf-8 and as \uXXXX escaped
    const character_encoding encodings[] = { character_encoding::ascii,
                                             character_encoding::utf8 };
    for (int b = 0; b < 2; ++b) {
        for (std::uint32_t cp = 0; cp <= 0x10FFFF; ++cp) {

            // skip reserved range
            if (cp >= 0xD800 && cp <= 0xDFFF)
                continue;

            char tmp[4];
            char *limit = utf32_to_utf8(tmp, cp);

            value value1(tmp, limit - tmp);
            std::string string =
                to_string(value1, { 0, encodings[b] });
            value value2 = parse(string);
            REQUIRE(value1 == value2);
        }
    }
#endif
}

struct foo_t {
    std::string bar;
    double baz;
};

static ujson::value to_json(foo_t const& f) {
    return ujson::object{ { "bar", f.bar }, { "baz", f.baz } };
}

TEST_CASE("array") {

    using namespace ujson;

    array a0;
    a0.push_back(object{ { "a", "b" } });
    a0.push_back(value());
    a0.push_back(true);
    a0.push_back(false);
    a0.push_back(1023);
    a0.push_back(M_PI);
    a0.push_back("Short");
    a0.push_back("Looooooooooooooooooooooooooooooooong");
    a0.push_back(object{ { "foo1", "bar1" }, { "foo2", "bar2" } });
    a0.push_back(array{ 1.0, 2.0, 3.0 });

    // premature end, no zero term
    char foo[] = { '"', '2', '"' };
    REQUIRE_THROWS(parse(foo, 2));
    REQUIRE_THROWS(parse("["));
    REQUIRE_THROWS(parse("\""));
    REQUIRE_THROWS(parse("{"));

    // no zero termiantor
    char one[] = { '1', '2' };
    REQUIRE(parse(one, 1) == 1);

    // copy construct
    value array0(a0);
    REQUIRE(array0.is_array());
    REQUIRE(array0.type() == value_type::array);
    REQUIRE(array_cast(array0) == a0);

    // move construct
    const value *array_data = a0.data();
    value array1(std::move(a0));
    REQUIRE(array1.type() == value_type::array);
    REQUIRE(array_cast(array0) == array1);
    a0 = array_cast(std::move(array1));
    REQUIRE(a0.data() == array_data);

    // move assign
    array1 = std::move(a0);
    value array2 = array1;
    REQUIRE(array1 == array2);
    REQUIRE(array_cast(array1).data() == array_data);
    REQUIRE(array_cast(array2).data() == array_data);
    a0 = array_cast(std::move(array1));
    REQUIRE(a0.data() != array_data);

    // construct from vector of T convertible to value
    std::vector<double> doubles = { 1.0, 2.0 };
    REQUIRE((doubles == array{ 1.0, 2.0 }));

    // crashes with msvc 2013 older than update 3 due to compiler bug
#if !defined (_MSC_FULL_VER) || _MSC_FULL_VER>=180030723

    // construct value array from array of arbitrary type that support to_json
    std::vector<foo_t> foos = { { "M_PI", M_PI }, { "M_LN2", M_LN2 } };
    REQUIRE(
        (foos == array{ object{ { "bar", "M_PI" }, { "baz", M_PI } },
                        object{ { "bar", "M_LN2" }, { "baz", M_LN2 } } }));
#endif

    parse("\"[1,2,3]\"");
}

TEST_CASE("object") {

    using namespace ujson;

    // premature end, no zero term
    char foo[] = { '"', '2', '"' };
    REQUIRE_THROWS(parse(foo, 2));
    REQUIRE_THROWS(parse("["));
    REQUIRE_THROWS(parse("\""));
    REQUIRE_THROWS(parse("{"));

    // no zero  terminator
    char one[] = { '1', '2' };
    REQUIRE(parse(one, 1) == 1);

    object o0;
    o0.push_back({ "null", null });
    o0.push_back({ "boolean", true });
    o0.push_back({ "number", M_PI });
    o0.push_back({ "array", array{ "a", "b" } });
    o0.push_back({ "object", object{ { "a", "b" } } });

    // copy construct
    value object0(o0);
    REQUIRE(object0.is_object());
    REQUIRE(object0.type() == value_type::object);
    std::stable_sort(o0.begin(), o0.end());
    REQUIRE(object_cast(object0) == o0);

    // move construct
    const auto *object_data = o0.data();
    value object1(std::move(o0));
    REQUIRE(object1.type() == value_type::object);
    REQUIRE(object_cast(object0) == object1);
    o0 = object_cast(std::move(object1));
    REQUIRE(o0.data() == object_data);

    // move assign
    object1 = std::move(o0);
    value object2 = object1;
    REQUIRE(object1 == object2);
    REQUIRE(object_cast(object1).data() == object_data);
    REQUIRE(object_cast(object2).data() == object_data);
    o0 = object_cast(std::move(object1));
    REQUIRE(o0.data() != object_data);

    // name must be valid utf-8
    REQUIRE_THROWS(value(object{ {"\xFF", null } }));
    REQUIRE_THROWS(parse("{ \"\xFF\" : null }"));

    // construct from map of T convertible to value
    std::map<std::string, double> doubles = { { "one", 1.0 }, { "two", 2.0 } };
    REQUIRE((doubles == object{ { "one", 1.0 }, { "two", 2.0 } }));

    // crashes with msvc 2013 older than update 3 due to compiler bug
#if !defined (_MSC_FULL_VER) || _MSC_FULL_VER>=180030723

    // construct value array from array of arbitrary type that support to_json
    std::map<std::string, foo_t> foos = { { "foo1", { "M_PI", M_PI } },
                                          { "foo2", { "M_LN2", M_LN2 } } };
    REQUIRE((foos ==
             object{ { "foo1", object{ { "bar", "M_PI" }, { "baz", M_PI } } },
                     { "foo2",
                       object{ { "bar", "M_LN2" }, { "baz", M_LN2 } } } }));
#endif
}

TEST_CASE("misc") {

    using namespace ujson;

    // swap
    value a(42), b("foo");
    swap(a, b);
    REQUIRE(a == "foo");
    REQUIRE(b == 42);

    // equality
    REQUIRE(a == "foo");
    REQUIRE(b != "foo");
}

// ---------------------------------------------------------------------------
// George Marsaglia's Multiply-With-Carry RNG

static std::uint32_t marsaglia_mwc() {
    static std::uint32_t z = 362436069, w = 521288629;
    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);
    return (z << 16) + (w & 65535);
}

// ---------------------------------------------------------------------------
// functions for generating random JSON for testing

static bool gen_bool() { return (marsaglia_mwc() & 0x1) == 0x1; }

static double gen_number() {
    switch (marsaglia_mwc() % 3) {
    case 0: {
        // random int
        auto i = static_cast<std::int32_t>(marsaglia_mwc());
        return double(i);
    }
    case 1: {
        // full precision double, such as measured data
        return double(marsaglia_mwc()) * marsaglia_mwc() / 4294967296.0;
    }
    default:
    case 2: {
        // decimal data, typically typed in
        double d =
            static_cast<std::int32_t>(marsaglia_mwc()) / 2147483.6480;
        int digits = 1 + marsaglia_mwc() % 6;
        double scale = std::pow(10.0, digits);
        d = std::floor(d * scale) / scale;
        return d;
    }
    }
}

static std::string gen_string(int max_len) {

    const int num_codepoints = marsaglia_mwc() % max_len;
    std::string result;
    for (int i = 0; i < num_codepoints; ++i) {
        double u = marsaglia_mwc() / 4294967296.0;
        assert(u >= 0.0 && u <= 1.0);
        if (u < 0.92) {
            auto cp = 0x20 + marsaglia_mwc() % (0x80 - 0x20);
            result += char(cp);
        } else if (u < 0.94) {
            auto cp = marsaglia_mwc() % 32;
            result += char(cp);
        } else if (u < 0.97) {
            auto cp = 0x7F + marsaglia_mwc() % (0x800 - 0x7F);
            char tmp[2];
            result.append(tmp, utf32_to_utf8(tmp, cp));
        } else if (u < 0.99) {
            auto cp = 0x800 + marsaglia_mwc() % (0xD800 - 0x800);
            char tmp[3];
            result.append(tmp, utf32_to_utf8(tmp, cp));
        } else if (u < 0.9915) {
            auto cp = 0xE000 + marsaglia_mwc() % (0x10000 - 0xE000);
            char tmp[3];
            result.append(tmp, utf32_to_utf8(tmp, cp));
        } else {
            auto cp = 0x10000 + marsaglia_mwc() % (0x110000 - 0x10000);
            char tmp[4];
            result.append(tmp, utf32_to_utf8(tmp, cp));
        }
    }
    return result;
}

static ujson::value gen_array(int depth);
static ujson::value gen_object(int depth);

const int max_array_object_depth = 12;

static ujson::value gen_array(int depth) {

    using namespace ujson;

    const int num_elements = 6;

    array result;
    result.reserve(num_elements);
    for (int i = 0; i < num_elements; ++i) {
        double u = marsaglia_mwc() / 4294967296.0;
        if (u < 0.05) {
            result.push_back(null);
        } else if (u < 0.15) {
            result.push_back(gen_bool());
        } else if (u < 0.45) {
            result.push_back(gen_number());
        } else if (u < 0.70) {
            result.push_back(gen_string(64));
        } else if (u < 0.85 && depth <= max_array_object_depth) {
            result.push_back(gen_array(depth + 1));
        } else if (depth <= max_array_object_depth) {
            result.push_back(gen_object(depth + 1));
        }
    }
    return result;
}

static ujson::value gen_object(int depth) {

    using namespace ujson;

    const int num_elements = 6;

    object result;
    for (int i = 0; i < num_elements; ++i) {
        double u = marsaglia_mwc() / 4294967296.0;
        auto key = gen_string(16);
        if (u < 0.05) {
            result.push_back({ std::move(key), null });
        } else if (u < 0.15) {
            result.push_back({ std::move(key), gen_bool() });
        } else if (u < 0.45) {
            result.push_back({ std::move(key), gen_number() });
        } else if (u < 0.70) {
            result.push_back({ std::move(key), gen_string(64) });
        } else if (u < 0.85 && depth <= max_array_object_depth) {
            result.push_back({ std::move(key), gen_array(depth + 1) });
        } else if (depth <= max_array_object_depth) {
            result.push_back({ std::move(key), gen_object(depth + 1) });
        }
    }
    return result;
}

TEST_CASE("performance", "[hide]") {
    
    using namespace ujson;

#ifdef _WIN32
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
#endif

#if 0
    auto string = string_cast(gen_string(640000));
    std::cout << string.length() << std::endl;

    {
        std::vector<double> timings(16);
        double best = 1e6;
        for (double &t : timings) {

#if _WIN32
            LARGE_INTEGER begin;
            QueryPerformanceCounter(&begin);
#else
            auto from = std::chrono::high_resolution_clock::now();
#endif
            value v(string);
     
#if _WIN32
            LARGE_INTEGER end;
            QueryPerformanceCounter(&end);
            t = 1e3 * double(end.QuadPart - begin.QuadPart) / freq.QuadPart;
#else
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = milliseconds(now - from);
            t = elapsed.count();
#endif
            best = std::min(best, t);
        }

        std::cout << "=> " << best << std::endl;
    }

    return;
#endif

    value result = gen_object(0);
    std::string tmp = to_string(result, indented_utf8);

    std::FILE *fp = std::fopen("test.json", "wb");
    if (fp) {
        std::fwrite(tmp.c_str(), tmp.length(), 1, fp);
        std::fclose(fp);
    }

    using milliseconds =
        std::chrono::duration<double, std::chrono::milliseconds::period>;

    std::vector<double> timings(64);
    double best = 1e6;
    for (double &t : timings) {

#if _WIN32
        LARGE_INTEGER begin;
        QueryPerformanceCounter(&begin);
#else
        auto from = std::chrono::high_resolution_clock::now();
#endif
        //std::string tmp = to_string(result, indented_utf8);
        auto parsed = parse(tmp);

#if _WIN32
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        t = 1e3 * double(end.QuadPart - begin.QuadPart) / freq.QuadPart;
#else
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = milliseconds(now - from);
        t = elapsed.count();
#endif
        best = std::min(best, t);
        REQUIRE(result == parsed);
    }

    for (double t : timings) {
        std::cout << t << std::endl;
    }
    std::cout << "=> " <<  best << std::endl;
    std::cout << tmp.length() << std::endl;
}

TEST_CASE("readme", "[hide]") {

    // null
    {
        ujson::value null_value; // null
        assert(ujson::null == null_value);
        std::cout << null_value << std::endl; // prints 'null'
    }

    // bool
    {
        ujson::value boolean(true);
        assert(bool_cast(boolean) == true);
        std::cout << boolean << std::endl; // prints 'true'
        boolean = false;
        assert(bool_cast(boolean) == false);
        std::cout << boolean << std::endl; // prints 'false'
    }

    // number
    {
        ujson::value number = M_PI;
        std::cout << number << std::endl; // prints '3.141592653589793'

        double d = double_cast(number); // d == M_PI

        // number = std::numeric_limits<double>::infinity(); // throws
        // bad_number

        number = 1024;
        std::cout << number << std::endl; // prints '1024'

        std::int32_t i = int32_cast(number); // i == 1024
    }

    // string
    {
        ujson::value value = "\xC2\xA9 ujson 2014"; // copyright symbol

        char title[] = {
            (char)0xC2, (char)0xB5, 'j', 's', 'o', 'n'
        }; // micro sign + json
        value = ujson::value(title, 6);

        // value = "\xF5"; // invalid utf-8; throws bad_string

        value = ujson::value(
            "valid", 5, ujson::validate_utf8::no); // skip utf-8 validation

        std::string string("ujson");
        value = string; // copy into value

        value = std::move(string); // move into value

        auto view = string_cast(value);
        std::cout << view.c_str() << std::endl; // prints 'ujson'

        string = string_cast(std::move(value)); // move string out of value
        assert(value.is_null());
    }

    // array
    {
        auto array = ujson::array{ true, M_PI, "a string" };
        ujson::value value(array);

        value = std::move(array);

        ujson::array const &ref = array_cast(value);

        array = array_cast(std::move(value));

        ujson::value value1 = std::move(array);
        ujson::value value2 =
            value1; // value2 shares immutable array with value1
        auto tmp1 = array_cast(std::move(value1)); // note: copy!
        auto tmp2 = array_cast(std::move(value2)); // move
    }

    // object
    {
        auto array = ujson::array{ 1, 2, 3 };
        auto object = ujson::object{ { "a null", ujson::null },
                                     { "a bool", true },
                                     { "a number", M_LN2 },
                                     { "a string", "Hello, world!" },
                                     { "an array", array } };
        ujson::value value(object);
        value = std::move(object);
        ujson::object const &ref = object_cast(value);
        object = object_cast(std::move(value));
        auto it = find(object, "a number");
        assert(it->second == M_LN2);
        object.push_back({ "invalid utf-8: \xFF", ujson::null });
        // value = std::move(object); // throws bad_string
    }

    // reading
    { auto value = ujson::parse("[ 1.0, 2.0, 3.0 ]"); }
    try {
        auto value = ujson::parse("[ 1.0, 2.0, 3.0 "); // invalid syntax
    } catch (std::exception const &e) {
        std::cout << e.what()
                  << std::endl; // prints 'Invalid syntax on line 1.'
    }

    // writing
    {
        auto array =
            ujson::array{ true, 1.0, "Sk\xC3\xA5l! \xF0\x9F\x8D\xBB" };
        auto object = ujson::object{ { "a null", ujson::null },
                                     { "a bool", true },
                                     { "a number", 1.61803398875 },
                                     { "a string", "R\xC3\xB8"
                                                   "dgr\xC3\xB8"
                                                   "d med fl\xC3\xB8"
                                                   "de." },
                                     { "an array", array } };
        std::cout << to_string(object) << std::endl;

        ujson::to_string_options compact_ascii;
        compact_ascii.indent_amount = 0;
        compact_ascii.encoding = ujson::character_encoding::ascii;
        std::cout << to_string(object, compact_ascii) << std::endl;
    }

    // details
    {
        ujson::value array(
            { ujson::null, true, 1.61803398875, "Hello, world!" });
        auto future =
            std::async(std::launch::async, [array] { /* do significant work */
            });
    }
}

struct book_t {
    std::string title;
    int year;
    std::vector<std::string> authors;
};

ujson::value to_json(book_t const &b) {
    return ujson::object{ { "title", b.title },
                          { "year", b.year },
                          { "authors", b.authors } };
}

book_t make_book(ujson::value v) {

    if (!v.is_object())
        throw std::invalid_argument("object expected for make_book");

    book_t book;
    std::vector<std::pair<std::string, ujson::value>> object =
        object_cast(std::move(v));

    auto it = find(object, "title");
    if (it == object.end() || !it->second.is_string())
        throw std::invalid_argument("'title' with type string not found");
    book.title = string_cast(std::move(it->second));

    it = find(object, "authors");
    if (it == object.end() || !it->second.is_array())
        throw std::invalid_argument("'authors' with type array not found");
    std::vector<ujson::value> array = array_cast(std::move(it->second));
    book.authors.reserve(array.size());
    for (auto it = array.begin(); it != array.end(); ++it) {
        if (!it->is_string())
            throw std::invalid_argument("'authors' must be array of strings");
        book.authors.push_back(string_cast(std::move(*it)));
    }

    it = find(object, "year");
    if (it == object.end() || !it->second.is_number())
        throw std::invalid_argument("'year' with type number not found");
    book.year = int32_cast(it->second);

    return book;
}

static bool operator==(book_t const &lhs, book_t const &rhs) {
    return lhs.title == rhs.title &&
           lhs.year == rhs.year &&
           lhs.authors == rhs.authors;
}

TEST_CASE("tutorial", "[hide]") {

    book_t book1{ "Elements of Programming",
                  2009,
                  { "Alexander A. Stepanov", "Paul McJones" } };
    book_t book2{ "The C++ Programming Language, 4th Edition",
                  2013,
                  { "Bjarne Stroustrup" } };
    std::vector<book_t> book_list{ book1, book2 };

    ujson::value value{ book_list };
    std::string json = to_string(value);
    std::cout << json << std::endl;

    ujson::value new_value = ujson::parse(json);
    assert(new_value == value);

    std::vector<ujson::value> array = array_cast(std::move(new_value));
    std::vector<book_t> new_book_list;
    new_book_list.reserve(array.size());
    for (auto it = array.begin(); it != array.end(); ++it)
        new_book_list.push_back(make_book(std::move(*it)));
    assert(new_book_list == book_list);
}
