#define SV_IMPLEMENTATION
#define SV_ADD_STD_STRING_VIEW_CONVERSION
#define SVDEF static inline
#include "../sv.h"

#include "minitest.h"

#include <inttypes.h>
#include <math.h>

MT_DEFINE_TEST(empty)
{
    StringView sv = sv_empty();
    MT_CHECK_THAT(sv.begin == NULL);
    MT_CHECK_THAT(sv.length == 0);
}

MT_DEFINE_TEST(from_cstr)
{
    {
        StringView sv = sv_from_cstr("foo");
        MT_ASSERT_THAT(sv.begin != NULL);
        MT_CHECK_THAT(sv.length == strlen("foo"));
    }
    {
        const char *src = "Hello World";
        StringView sv = sv_from_cstr(src);
        MT_ASSERT_THAT(sv.begin == src);
        MT_CHECK_THAT(sv.length == strlen(src));
    }
}

MT_DEFINE_TEST(from_parts)
{
    const char *src = "Foo Bar";
    StringView sv = sv_from_parts(src + 4, 2);
    MT_ASSERT_THAT(sv.begin != NULL);
    MT_CHECK_THAT(sv.begin == src + 4);
    MT_CHECK_THAT(sv.length == 2);
    MT_CHECK_THAT(sv.begin[0] == 'B');
    MT_CHECK_THAT(sv.begin[1] == 'a');
}

MT_DEFINE_TEST(lit)
{
    StringView sv = SV_LIT("Hello");
    MT_ASSERT_THAT(sv.begin != NULL);
    MT_CHECK_THAT(sv.length == 5);
    MT_CHECK_THAT(sv.begin[0] == 'H');
    MT_CHECK_THAT(sv.begin[1] == 'e');
    MT_CHECK_THAT(sv.begin[2] == 'l');
    MT_CHECK_THAT(sv.begin[3] == 'l');
    MT_CHECK_THAT(sv.begin[4] == 'o');
}

MT_DEFINE_TEST(is_empty)
{
    StringView sv = sv_empty();
    MT_CHECK_THAT(sv_is_empty(sv) == true);

    sv = sv_from_cstr("Foo");
    MT_CHECK_THAT(sv_is_empty(sv) == false);

    MT_CHECK_THAT(sv_is_empty(SV_LIT("")) == true);
}

MT_DEFINE_TEST(at)
{
    StringView sv = sv_from_cstr("Hello World");
    MT_CHECK_THAT(sv_at(sv, 0) == 'H');
    MT_CHECK_THAT(sv_at(sv, 3) == 'l');
    MT_CHECK_THAT(sv_at(sv, 4) == 'o');
}

MT_DEFINE_TEST(first)
{
    {
        StringView sv = sv_from_cstr("Hello World");
        MT_CHECK_THAT(sv_first(sv) == 'H');
    }
    {
        StringView sv = sv_from_cstr("Foo Bar Baz");
        MT_CHECK_THAT(sv_first(sv) == 'F');
    }
}

MT_DEFINE_TEST(last)
{
    {
        StringView sv = sv_from_cstr("Hello World");
        MT_CHECK_THAT(sv_last(sv) == 'd');
    }
    {
        StringView sv = sv_from_cstr("Foo Bar Baz");
        MT_CHECK_THAT(sv_last(sv) == 'z');
    }
}

MT_DEFINE_TEST(substr)
{
    StringView sv  = sv_from_cstr("Hello World");

    StringView sub = sv_substr(sv, 3, 5);
    MT_ASSERT_THAT(sub.begin != NULL);
    MT_ASSERT_THAT(sub.begin == sv.begin + 3);
    MT_CHECK_THAT(sub.length == 5);

    MT_CHECK_THAT(sub.begin[0] == 'l');
    MT_CHECK_THAT(sub.begin[1] == 'o');
    MT_CHECK_THAT(sub.begin[2] == ' ');
    MT_CHECK_THAT(sub.begin[3] == 'W');
    MT_CHECK_THAT(sub.begin[4] == 'o');
}

MT_DEFINE_TEST(eq)
{
    {
        StringView one = sv_from_cstr("foo");
        StringView two = sv_from_cstr("foo");
        MT_CHECK_THAT(sv_eq(one, two) == true);

        two = sv_from_cstr("fo");
        MT_CHECK_THAT(sv_eq(one, two) == false);
    }
    {
        // Make sure null-termination of string views are
        // not affecting result
        StringView base = sv_from_cstr("Foo Bar Baz");
        StringView one = sv_substr(base, 0, 3);
        StringView two = sv_substr(base, 0 ,3);
        MT_CHECK_THAT(sv_eq(one, two) == true);

        two = sv_substr(base, 1 ,3);
        MT_CHECK_THAT(sv_eq(one, two) == false);
    }
}

MT_DEFINE_TEST(eq_cstr_basic)
{
    {
        StringView sv = sv_from_cstr("foo");
        const char *cstr = "foo";
        MT_CHECK_THAT(sv_eq_cstr(sv, cstr) == true);

        sv = sv_from_cstr("fo");
        MT_CHECK_THAT(sv_eq_cstr(sv, cstr) == false);
    }
}

MT_DEFINE_TEST(eq_cstr_empty_and_null)
{
    {
        StringView sv = sv_from_cstr("");
        MT_CHECK_THAT(sv_eq_cstr(sv, "") == true);
        MT_CHECK_THAT(sv_eq_cstr(sv, "x") == false);
    }
    {
        StringView sv = sv_from_cstr("data");
        MT_CHECK_THAT(sv_eq_cstr(sv, NULL) == false);
    }
}

MT_DEFINE_TEST(eq_cstr_case_sensitive)
{
    StringView sv = sv_from_cstr("Foo");
    MT_CHECK_THAT(sv_eq_cstr(sv, "Foo") == true);
    MT_CHECK_THAT(sv_eq_cstr(sv, "foo") == false);
}

MT_DEFINE_TEST(eq_cstr_embedded_nul_behavior)
{

    const char c_with_nul[] = { 'a', '\0', 'b', '\0' };

    StringView sv1 = sv_from_cstr("a");
    StringView sv2 = sv_from_cstr("ab");

    MT_CHECK_THAT(sv_eq_cstr(sv1, c_with_nul) == true);
    MT_CHECK_THAT(sv_eq_cstr(sv2, c_with_nul) == false);
}

MT_DEFINE_TEST(eq_cstr_subview)
{
    StringView base = sv_from_cstr("xxbananaYY");
    StringView sv   = { base.begin + 2, 6 };
    MT_CHECK_THAT(sv_eq_cstr(sv, "banana") == true);
    MT_CHECK_THAT(sv_eq_cstr(sv, "bananas") == false);
}

MT_DEFINE_TEST(eq_cstr_n_basic)
{
    {
        StringView sv = sv_from_cstr("hello");
        MT_CHECK_THAT(sv_eq_cstr_n(sv, "hello", 5) == true);
        MT_CHECK_THAT(sv_eq_cstr_n(sv, "hell",  4) == false);
        MT_CHECK_THAT(sv_eq_cstr_n(sv, "hello!", 6) == false);
    }
    {
        StringView sv = sv_from_cstr("");
        MT_CHECK_THAT(sv_eq_cstr_n(sv, "", 0) == true);
        MT_CHECK_THAT(sv_eq_cstr_n(sv, "x", 1) == false);
    }
}

MT_DEFINE_TEST(eq_cstr_n_null_cstr)
{
    StringView sv = sv_from_cstr("data");
    MT_CHECK_THAT(sv_eq_cstr_n(sv, NULL, 4) == false);

    StringView sv_null = {NULL, 0};
    MT_CHECK_THAT(sv_eq_cstr_n(sv_null, NULL, 0) == false);
}

MT_DEFINE_TEST(eq_cstr_n_length_mismatch)
{
    StringView sv = sv_from_cstr("abc");
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "abc", 4) == false);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "abcd", 3) == true);
}

MT_DEFINE_TEST(eq_cstr_n_binary_bytes)
{
    const char buf1[] = { 'a', '\0', 'b', 'c' };
    const char buf2[] = { 'a', '\0', 'b', 'd' };

    StringView sv = { buf1, sizeof buf1 };

    MT_CHECK_THAT(sv_eq_cstr_n(sv, buf1, sizeof buf1) == true);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, buf2, sizeof buf2) == false);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, buf1, 3) == false);
}

MT_DEFINE_TEST(eq_cstr_n_subview)
{
    StringView base = sv_from_cstr("xxbananaYY");
    StringView sv   = { base.begin + 2, 6 };
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "banana", 6) == true);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "bananas", 7) == false);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "banan",  5) == false);
}

MT_DEFINE_TEST(eq_cstr_n_case_sensitive)
{
    StringView sv = sv_from_cstr("Hello");
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "Hello", 5) == true);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, "hello", 5) == false);
}

MT_DEFINE_TEST(eq_cstr_vs_eq_cstr_n_equivalence)
{
    const char *c = "cherry";
    StringView sv = sv_from_cstr("cherry");

    MT_CHECK_THAT(sv_eq_cstr(sv, c) == true);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, c, strlen(c)) == true);

    sv = sv_from_cstr("che");
    MT_CHECK_THAT(sv_eq_cstr(sv, c) == false);
    MT_CHECK_THAT(sv_eq_cstr_n(sv, c, strlen(c)) == false);
}

MT_DEFINE_TEST(starts_with)
{
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView prefix = sv_from_cstr("");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == true);
    }
    {
        StringView sv     = sv_from_cstr("");
        StringView prefix = sv_from_cstr("verb");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == false);
    }
    {
        StringView sv     = sv_from_cstr("");
        StringView prefix = sv_from_cstr("");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == true);
    }
    {
        StringView sv     = sv_from_cstr("-");
        StringView prefix = sv_from_cstr("--");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == false);
    }
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView prefix = sv_from_cstr("--");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == true);
    }
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView prefix = sv_from_cstr("--verbose");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == true);
    }
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView prefix = sv_from_cstr("verbose");
        MT_CHECK_THAT(sv_starts_with(sv, prefix) == false);
    }
}

MT_DEFINE_TEST(ends_with)
{
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView suffix = sv_from_cstr("");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == true);
    }
    {
        StringView sv     = sv_from_cstr("");
        StringView suffix = sv_from_cstr("verb");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == false);
    }
    {
        StringView sv     = sv_from_cstr("");
        StringView suffix = sv_from_cstr("");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == true);
    }
    {
        StringView sv     = sv_from_cstr("e");
        StringView suffix = sv_from_cstr("se");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == false);
    }
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView suffix = sv_from_cstr("se");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == true);
    }
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView suffix = sv_from_cstr("--verbose");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == true);
    }
    {
        StringView sv     = sv_from_cstr("--verbose");
        StringView suffix = sv_from_cstr("--");
        MT_CHECK_THAT(sv_ends_with(sv, suffix) == false);
    }
}

MT_DEFINE_TEST(find_char)
{
    StringView sv = sv_empty();
    MT_CHECK_THAT(sv_find_char(sv, 'a') == SV_NPOS);

    sv = sv_from_cstr("Hello world");
    MT_CHECK_THAT(sv_find_char(sv, 'H') == 0);
    MT_CHECK_THAT(sv_find_char(sv, 'e') == 1);
    MT_CHECK_THAT(sv_find_char(sv, 'l') == 2);
    MT_CHECK_THAT(sv_find_char(sv, 'k') == SV_NPOS);
}

MT_DEFINE_TEST(rfind_char)
{
    StringView sv = sv_empty();
    MT_CHECK_THAT(sv_rfind_char(sv, 'a') == SV_NPOS);

    sv = sv_from_cstr("Hello world");
    MT_CHECK_THAT(sv_rfind_char(sv, 'H') == 0);
    MT_CHECK_THAT(sv_rfind_char(sv, 'e') == 1);
    MT_CHECK_THAT(sv_rfind_char(sv, 'l') == 9);
    MT_CHECK_THAT(sv_rfind_char(sv, 'o') == 7);
    MT_CHECK_THAT(sv_rfind_char(sv, 'k') == SV_NPOS);
}

MT_DEFINE_TEST(find_char_from)
{
    StringView sv = sv_empty();
    MT_CHECK_THAT(sv_find_char_from(sv, 0, 'a') == SV_NPOS);

    sv = sv_from_cstr("Hello world");
    MT_CHECK_THAT(sv_find_char_from(sv,  0, 'H') == 0);
    MT_CHECK_THAT(sv_find_char_from(sv,  0, 'e') == 1);
    MT_CHECK_THAT(sv_find_char_from(sv,  1, 'e') == 1);
    MT_CHECK_THAT(sv_find_char_from(sv,  2, 'l') == 2);
    MT_CHECK_THAT(sv_find_char_from(sv,  3, 'l') == 3);
    MT_CHECK_THAT(sv_find_char_from(sv, 11, 'd') == SV_NPOS);
    MT_CHECK_THAT(sv_find_char_from(sv,  0, 'k') == SV_NPOS);
}

MT_DEFINE_TEST(rfind_char_from)
{
    StringView sv = sv_empty();
    MT_CHECK_THAT(sv_rfind_char_from(sv, 0, 'a') == SV_NPOS);

    sv = sv_from_cstr("Hello world");
    MT_CHECK_THAT(sv_rfind_char_from(sv,  0, 'H') == 0);
    MT_CHECK_THAT(sv_rfind_char_from(sv,  0, 'e') == SV_NPOS);
    MT_CHECK_THAT(sv_rfind_char_from(sv,  1, 'e') == 1);
    MT_CHECK_THAT(sv_rfind_char_from(sv,  2, 'l') == 2);
    MT_CHECK_THAT(sv_rfind_char_from(sv,  6, 'o') == 4);
    MT_CHECK_THAT(sv_rfind_char_from(sv,  8, 'o') == 7);
    MT_CHECK_THAT(sv_rfind_char_from(sv, 11, 'd') == 10); // Clamps
    MT_CHECK_THAT(sv_rfind_char_from(sv,  0, 'k') == SV_NPOS);
}

MT_DEFINE_TEST(find_substr)
{
    {
        StringView hay    = sv_empty();
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_empty();
        StringView needle = sv_from_cstr("a");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("");
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("world");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 6);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("ld");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 9);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello world");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello world!");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("banana");
        StringView needle = sv_from_cstr("ana");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 1);
    }
    {
        StringView hay    = sv_from_cstr("aaaaa");
        StringView needle = sv_from_cstr("aaa");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("hello");
        MT_CHECK_THAT(sv_find_substr(hay, needle) == SV_NPOS);
    }
}

MT_DEFINE_TEST(rfind_substr)
{
    {
        StringView hay    = sv_empty();
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_empty();
        StringView needle = sv_from_cstr("a");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("");
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("world");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 6);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("ld");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 9);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello world");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello world!");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("banana");
        StringView needle = sv_from_cstr("ana");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 3);
    }
    {
        StringView hay    = sv_from_cstr("aaaaa");
        StringView needle = sv_from_cstr("aaa");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == 2);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("hello");
        MT_CHECK_THAT(sv_rfind_substr(hay, needle) == SV_NPOS);
    }
}

MT_DEFINE_TEST(find_substr_from)
{
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == 0);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("world");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == 6);
        MT_CHECK_THAT(sv_find_substr_from(hay, 5, needle) == 6);
        MT_CHECK_THAT(sv_find_substr_from(hay, 7, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("banana");
        StringView needle = sv_from_cstr("ana");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == 1);
        MT_CHECK_THAT(sv_find_substr_from(hay, 2, needle) == 3);
    }
    {
        StringView hay    = sv_from_cstr("aaaaa");
        StringView needle = sv_from_cstr("aaa");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == 0);
        MT_CHECK_THAT(sv_find_substr_from(hay, 1, needle) == 1);
        MT_CHECK_THAT(sv_find_substr_from(hay, 2, needle) == 2);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == 0);
        MT_CHECK_THAT(sv_find_substr_from(hay, 3, needle) == 3);
        MT_CHECK_THAT(sv_find_substr_from(hay, 5, needle) == 5);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("Hello world");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("");
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_find_substr_from(hay, 0, needle) == 0);
    }
}

MT_DEFINE_TEST(rfind_substr_from)
{
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 10, needle) == 0);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 4,  needle) == 0);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 3,  needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("world");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 10, needle) == 6);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 7,  needle) == 6);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 5,  needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("banana");
        StringView needle = sv_from_cstr("ana");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 5, needle) == 3);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 2, needle) == 1);
    }
    {
        StringView hay    = sv_from_cstr("aaaaa");
        StringView needle = sv_from_cstr("aaa");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 4, needle) == 2);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 2, needle) == 2);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 1, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 0, needle) == 0);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 3, needle) == 3);
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 5, needle) == 5);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("Hello world");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 0, needle) == SV_NPOS);
    }
    {
        StringView hay    = sv_from_cstr("");
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_rfind_substr_from(hay, 0, needle) == 0);
    }
}

MT_DEFINE_TEST(find_substr_cstr)
{
    {
        StringView hay = sv_from_cstr("Hello world");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "Hello") == 0);
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "world") == 6);
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "lo wo") == 3);
    }
    {
        StringView hay = sv_from_cstr("banana");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "ana") == 1);
    }
    {
        StringView hay = sv_from_cstr("aaaaa");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "aaa") == 0);
    }
    {
        StringView hay = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "") == 0);
    }
    {
        StringView hay = sv_from_cstr("");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "") == 0);
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "a") == SV_NPOS);
    }
    {
        StringView hay = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "Hello world") == SV_NPOS);
    }
    {
        StringView hay = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_find_substr_cstr(hay, "hello") == SV_NPOS);
    }
}

MT_DEFINE_TEST(contains)
{
    {
        StringView hay    = sv_empty();
        StringView needle = sv_from_cstr("");
        MT_CHECK_THAT(sv_contains(hay, needle) == true);
    }
    {
        StringView hay    = sv_empty();
        StringView needle = sv_from_cstr("a");
        MT_CHECK_THAT(sv_contains(hay, needle) == false);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_contains(hay, needle) == true);
    }
    {
        StringView hay    = sv_from_cstr("Hello world");
        StringView needle = sv_from_cstr("world");
        MT_CHECK_THAT(sv_contains(hay, needle) == true);
    }
    {
        StringView hay    = sv_from_cstr("banana");
        StringView needle = sv_from_cstr("ana");
        MT_CHECK_THAT(sv_contains(hay, needle) == true);
    }
    {
        StringView hay    = sv_from_cstr("banana");
        StringView needle = sv_from_cstr("apple");
        MT_CHECK_THAT(sv_contains(hay, needle) == false);
    }
    {
        StringView hay    = sv_from_cstr("Hello");
        StringView needle = sv_from_cstr("hello");
        MT_CHECK_THAT(sv_contains(hay, needle) == false);
    }
}

MT_DEFINE_TEST(contains_cstr)
{
    {
        StringView hay = sv_from_cstr("");
        MT_CHECK_THAT(sv_contains_cstr(hay, "") == true);
        MT_CHECK_THAT(sv_contains_cstr(hay, "a") == false);
    }
    {
        StringView hay = sv_from_cstr("Hello world");
        MT_CHECK_THAT(sv_contains_cstr(hay, "Hello") == true);
        MT_CHECK_THAT(sv_contains_cstr(hay, "world") == true);
        MT_CHECK_THAT(sv_contains_cstr(hay, "lo wo") == true);
        MT_CHECK_THAT(sv_contains_cstr(hay, "bye") == false);
    }
    {
        StringView hay = sv_from_cstr("aaaaa");
        MT_CHECK_THAT(sv_contains_cstr(hay, "aaa") == true);
    }
    {
        StringView hay = sv_from_cstr("Hello");
        MT_CHECK_THAT(sv_contains_cstr(hay, "Hello world") == false);
        MT_CHECK_THAT(sv_contains_cstr(hay, "hello") == false);
    }
}

MT_DEFINE_TEST(trim_left)
{
    {
        StringView sv = sv_from_cstr("");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_left(sv), ""));
    }
    {
        StringView sv = sv_from_cstr("   ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_left(sv), ""));
    }
    {
        StringView sv = sv_from_cstr("foo");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_left(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("    foo");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_left(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("    foo  ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_left(sv), "foo  "));
    }
    {
        StringView sv = sv_from_cstr("    \tfoo  ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_left(sv), "foo  "));
    }
}

MT_DEFINE_TEST(trim_right)
{
    {
        StringView sv = sv_from_cstr("");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_right(sv), ""));
    }
    {
        StringView sv = sv_from_cstr("   ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_right(sv), ""));
    }
    {
        StringView sv = sv_from_cstr("foo");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_right(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("foo    ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_right(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("  foo    ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_right(sv), "  foo"));
    }
    {
        StringView sv = sv_from_cstr("  foo    \t");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim_right(sv), "  foo"));
    }
}

MT_DEFINE_TEST(trim)
{
    {
        StringView sv = sv_from_cstr("");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim(sv), ""));
    }
    {
        StringView sv = sv_from_cstr("   ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim(sv), ""));
    }
    {
        StringView sv = sv_from_cstr("foo");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("foo    ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("  foo    ");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim(sv), "foo"));
    }
    {
        StringView sv = sv_from_cstr("  foo    \t");
        MT_CHECK_THAT(sv_eq_cstr(sv_trim(sv), "foo"));
    }
}

MT_DEFINE_TEST(take_basic)
{
    StringView sv = sv_from_cstr("abcde");

    StringView t = sv_take(sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(t, "ab"));

    MT_CHECK_THAT(sv_eq_cstr(sv, "abcde"));
}

MT_DEFINE_TEST(take_zero_and_empty)
{
    {
        StringView sv = sv_from_cstr("data");
        StringView t  = sv_take(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(t, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, "data"));
    }
    {
        StringView sv = sv_from_cstr("");
        StringView t  = sv_take(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(t, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(take_exact_length)
{
    StringView sv = sv_from_cstr("xyz");
    StringView t  = sv_take(sv, 3);
    MT_CHECK_THAT(sv_eq_cstr(t, "xyz"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "xyz"));
}

MT_DEFINE_TEST(take_equivalence_with_substr)
{
    StringView sv = sv_from_cstr("hello");
    for (size_t n = 0; n <= sv.length; ++n) {
        StringView a = sv_take(sv, n);
        StringView b = sv_substr(sv, 0, n);
        MT_CHECK_THAT(a.begin == b.begin && a.length == b.length);
    }
}

MT_DEFINE_TEST(take_nested_chaining_does_not_mutate)
{
    StringView sv = sv_from_cstr("abcdef");
    StringView a  = sv_take(sv, 4);
    StringView b  = sv_take(sv, 2);
    StringView c  = sv_take(a, 2);

    MT_CHECK_THAT(sv_eq_cstr(a, "abcd"));
    MT_CHECK_THAT(sv_eq_cstr(b, "ab"));
    MT_CHECK_THAT(sv_eq_cstr(c, "ab"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
}

MT_DEFINE_TEST(drop_basic)
{
    {
        StringView sv = sv_from_cstr("abcde");
        StringView r  = sv_drop(sv, 2);
        MT_CHECK_THAT(sv_eq_cstr(r, "cde"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "abcde"));
    }
    {
        StringView sv = sv_from_cstr("hello");
        StringView r  = sv_drop(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(r, "hello"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "hello"));
    }
}

MT_DEFINE_TEST(drop_exact_and_empty)
{
    {
        StringView sv = sv_from_cstr("xyz");
        StringView r  = sv_drop(sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(r, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, "xyz"));
    }
    {
        StringView sv = sv_from_cstr("");
        StringView r  = sv_drop(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(r, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(drop_equivalence_with_substr)
{
    StringView sv = sv_from_cstr("abcdef");
    for (size_t n = 0; n <= sv.length; ++n) {
        StringView a = sv_drop(sv, n);
        StringView b = sv_substr(sv, n, sv.length - n);
        MT_CHECK_THAT(a.begin == b.begin && a.length == b.length);
    }
}

MT_DEFINE_TEST(drop_pairs_with_take)
{
    StringView sv = sv_from_cstr("abcdef");

    StringView head2 = sv_take(sv, 2);
    StringView tail2 = sv_drop(sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(head2, "ab"));
    MT_CHECK_THAT(sv_eq_cstr(tail2, "cdef"));

    StringView head0 = sv_take(sv, 0);
    StringView tail0 = sv_drop(sv, 0);
    MT_CHECK_THAT(sv_eq_cstr(head0, ""));
    MT_CHECK_THAT(sv_eq_cstr(tail0, "abcdef"));

    StringView head6 = sv_take(sv, 6);
    StringView tail6 = sv_drop(sv, 6);
    MT_CHECK_THAT(sv_eq_cstr(head6, "abcdef"));
    MT_CHECK_THAT(sv_eq_cstr(tail6, ""));
}

MT_DEFINE_TEST(drop_does_not_mutate)
{
    StringView sv = sv_from_cstr("abcdef");
    (void)sv_drop(sv, 1);
    (void)sv_drop(sv, 3);
    MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
}

MT_DEFINE_TEST(take_last_basic)
{
    {
        StringView sv = sv_from_cstr("abcdef");
        StringView t  = sv_take_last(sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(t, "def"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
    }
    {
        StringView sv = sv_from_cstr("hello");
        StringView t  = sv_take_last(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(t, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, "hello"));
    }
}

MT_DEFINE_TEST(take_last_exact_and_empty)
{
    {
        StringView sv = sv_from_cstr("xyz");
        StringView t  = sv_take_last(sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(t, "xyz"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "xyz"));
    }
    {
        StringView sv = sv_from_cstr("");
        StringView t  = sv_take_last(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(t, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(take_last_equivalence_with_substr)
{
    StringView sv = sv_from_cstr("abcdef");
    for (size_t n = 0; n <= sv.length; ++n) {
        StringView a = sv_take_last(sv, n);
        StringView b = sv_substr(sv, sv.length - n, n);
        MT_CHECK_THAT(a.begin == b.begin && a.length == b.length);
    }
}

MT_DEFINE_TEST(take_last_pairs_with_drop_last)
{

    StringView sv = sv_from_cstr("abcdef");

    StringView tail2 = sv_take_last(sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(tail2, "ef"));

    StringView tail0 = sv_take_last(sv, 0);
    MT_CHECK_THAT(sv_eq_cstr(tail0, ""));

    StringView tail6 = sv_take_last(sv, 6);
    MT_CHECK_THAT(sv_eq_cstr(tail6, "abcdef"));


    MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
}

MT_DEFINE_TEST(take_last_does_not_mutate)
{
    StringView sv = sv_from_cstr("abcdef");
    (void)sv_take_last(sv, 1);
    (void)sv_take_last(sv, 3);
    MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
}

MT_DEFINE_TEST(drop_last_basic)
{
    {
        StringView sv = sv_from_cstr("abcdef");
        StringView r  = sv_drop_last(sv, 2);
        MT_CHECK_THAT(sv_eq_cstr(r, "abcd"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
    }
    {
        StringView sv = sv_from_cstr("hello");
        StringView r  = sv_drop_last(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(r, "hello"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "hello"));
    }
}

MT_DEFINE_TEST(drop_last_exact_and_empty)
{
    {
        StringView sv = sv_from_cstr("xyz");
        StringView r  = sv_drop_last(sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(r, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, "xyz"));
    }
    {
        StringView sv = sv_from_cstr("");
        StringView r  = sv_drop_last(sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(r, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(drop_last_equivalence_with_substr)
{
    StringView sv = sv_from_cstr("abcdef");
    for (size_t n = 0; n <= sv.length; ++n) {
        StringView a = sv_drop_last(sv, n);
        StringView b = sv_substr(sv, 0, sv.length - n);
        MT_CHECK_THAT(a.begin == b.begin && a.length == b.length);
    }
}

MT_DEFINE_TEST(drop_last_pairs_with_take_last)
{
    StringView sv = sv_from_cstr("abcdef");

    StringView head4 = sv_drop_last(sv, 2);
    StringView tail2 = sv_take_last(sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(head4, "abcd"));
    MT_CHECK_THAT(sv_eq_cstr(tail2, "ef"));

    StringView head6 = sv_drop_last(sv, 6);
    StringView tail6 = sv_take_last(sv, 6);
    MT_CHECK_THAT(sv_eq_cstr(head6, ""));
    MT_CHECK_THAT(sv_eq_cstr(tail6, "abcdef"));


    MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
}

MT_DEFINE_TEST(drop_last_does_not_mutate)
{
    StringView sv = sv_from_cstr("abcdef");
    (void)sv_drop_last(sv, 1);
    (void)sv_drop_last(sv, 3);
    MT_CHECK_THAT(sv_eq_cstr(sv, "abcdef"));
}

MT_DEFINE_TEST(take_and_consume_basic)
{
    StringView sv = sv_from_cstr("abcdef");

    StringView t = sv_take_and_consume(&sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(t, "ab"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "cdef"));

    t = sv_take_and_consume(&sv, 3);
    MT_CHECK_THAT(sv_eq_cstr(t, "cde"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "f"));
}

MT_DEFINE_TEST(take_and_consume_zero_and_exact)
{
    {
        StringView sv = sv_from_cstr("data");
        StringView t  = sv_take_and_consume(&sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(t, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, "data"));
    }
    {
        StringView sv = sv_from_cstr("xyz");
        StringView t  = sv_take_and_consume(&sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(t, "xyz"));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(take_and_consume_chain)
{
    StringView sv = sv_from_cstr("abc");
    StringView a = sv_take_and_consume(&sv, 1);
    StringView b = sv_take_and_consume(&sv, 1);
    StringView c = sv_take_and_consume(&sv, 1);

    MT_CHECK_THAT(sv_eq_cstr(a, "a"));
    MT_CHECK_THAT(sv_eq_cstr(b, "b"));
    MT_CHECK_THAT(sv_eq_cstr(c, "c"));
    MT_CHECK_THAT(sv_eq_cstr(sv, ""));
}

MT_DEFINE_TEST(drop_and_consume_basic)
{
    StringView sv = sv_from_cstr("--option");

    StringView r = sv_drop_and_consume(&sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(r, "option"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "option"));
}

MT_DEFINE_TEST(drop_and_consume_zero_and_exact)
{
    {
        StringView sv = sv_from_cstr("hello");
        StringView r  = sv_drop_and_consume(&sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(r, "hello"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "hello"));
    }
    {
        StringView sv = sv_from_cstr("xyz");
        StringView r  = sv_drop_and_consume(&sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(r, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(drop_and_consume_chain)
{
    StringView sv = sv_from_cstr("abcdef");
    (void)sv_drop_and_consume(&sv, 1);
    MT_CHECK_THAT(sv_eq_cstr(sv, "bcdef"));
    StringView r = sv_drop_and_consume(&sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(r, "def"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "def"));
}

MT_DEFINE_TEST(take_last_and_consume_basic)
{
    StringView sv = sv_from_cstr("abcdef");

    StringView t = sv_take_last_and_consume(&sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(t, "ef"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "abcd"));

    t = sv_take_last_and_consume(&sv, 3);
    MT_CHECK_THAT(sv_eq_cstr(t, "bcd"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "a"));
}

MT_DEFINE_TEST(take_last_and_consume_zero_and_exact)
{
    {
        StringView sv = sv_from_cstr("data");
        StringView t  = sv_take_last_and_consume(&sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(t, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, "data"));
    }
    {
        StringView sv = sv_from_cstr("xyz");
        StringView t  = sv_take_last_and_consume(&sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(t, "xyz"));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(take_last_and_consume_chain)
{
    StringView sv = sv_from_cstr("abcd");
    StringView d = sv_take_last_and_consume(&sv, 1);
    StringView c = sv_take_last_and_consume(&sv, 1);
    StringView b = sv_take_last_and_consume(&sv, 1);
    StringView a = sv_take_last_and_consume(&sv, 1);

    MT_CHECK_THAT(sv_eq_cstr(a, "a"));
    MT_CHECK_THAT(sv_eq_cstr(b, "b"));
    MT_CHECK_THAT(sv_eq_cstr(c, "c"));
    MT_CHECK_THAT(sv_eq_cstr(d, "d"));
    MT_CHECK_THAT(sv_eq_cstr(sv, ""));
}

MT_DEFINE_TEST(drop_last_and_consume_basic)
{
    StringView sv = sv_from_cstr("filename.txt");

    StringView head = sv_drop_last_and_consume(&sv, 4);
    MT_CHECK_THAT(sv_eq_cstr(head, "filename"));
    MT_CHECK_THAT(sv_eq_cstr(sv,   "filename"));
}

MT_DEFINE_TEST(drop_last_and_consume_zero_and_exact)
{
    {
        StringView sv = sv_from_cstr("hello");
        StringView r  = sv_drop_last_and_consume(&sv, 0);
        MT_CHECK_THAT(sv_eq_cstr(r, "hello"));
        MT_CHECK_THAT(sv_eq_cstr(sv, "hello"));
    }
    {
        StringView sv = sv_from_cstr("xyz");
        StringView r  = sv_drop_last_and_consume(&sv, 3);
        MT_CHECK_THAT(sv_eq_cstr(r, ""));
        MT_CHECK_THAT(sv_eq_cstr(sv, ""));
    }
}

MT_DEFINE_TEST(drop_last_and_consume_chain)
{
    StringView sv = sv_from_cstr("abcdef");
    (void)sv_drop_last_and_consume(&sv, 1);
    MT_CHECK_THAT(sv_eq_cstr(sv, "abcde"));
    StringView r = sv_drop_last_and_consume(&sv, 2);
    MT_CHECK_THAT(sv_eq_cstr(r, "abc"));
    MT_CHECK_THAT(sv_eq_cstr(sv, "abc"));
}

MT_DEFINE_TEST(split_first)
{
    {
        StringView sv = sv_from_cstr("");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_first(sv, ' ', &before_delim, &after_delim);
        MT_CHECK_THAT(before_delim.length == 0);
        MT_CHECK_THAT(after_delim.length  == 0);
    }
    {
        StringView sv = sv_from_cstr("foo,bar");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_first(sv, ',', &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar"));
    }
    {
        StringView sv = sv_from_cstr("foo bar");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_first(sv, ' ', &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar"));
    }
    {
        StringView sv = sv_from_cstr("foo bar baz");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_first(sv, ' ', &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar baz"));
    }
}

MT_DEFINE_TEST(split_last)
{
    {
        StringView sv = sv_from_cstr("");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_last(sv, ' ', &before_delim, &after_delim);
        MT_CHECK_THAT(before_delim.length == 0);
        MT_CHECK_THAT(after_delim.length  == 0);
    }
    {
        StringView sv = sv_from_cstr("foo,bar");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_last(sv, ',', &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar"));
    }
    {
        StringView sv = sv_from_cstr("foo bar");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_last(sv, ' ', &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar"));
    }
    {
        StringView sv = sv_from_cstr("foo bar baz");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_last(sv, ' ', &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo bar"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "baz"));
    }
}

MT_DEFINE_TEST(split_once_from)
{
    {
        StringView sv = sv_from_cstr("");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_once_from(sv, ' ', 0, &before_delim, &after_delim);
        MT_CHECK_THAT(before_delim.length == 0);
        MT_CHECK_THAT(after_delim.length  == 0);
    }
    {
        StringView sv = sv_from_cstr("foo,bar");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_once_from(sv, ',', 0, &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar"));
    }
    {
        StringView sv = sv_from_cstr("foo bar");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_once_from(sv, ' ', 0, &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar"));
    }
    {
        StringView sv = sv_from_cstr("foo bar baz");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_once_from(sv, ' ', 0, &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar baz"));
    }
    {
        StringView sv = sv_from_cstr("foo bar baz");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_once_from(sv, ' ', 3, &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "bar baz"));
    }
    {
        StringView sv = sv_from_cstr("foo bar baz");
        StringView before_delim = sv_empty();
        StringView after_delim  = sv_empty();
        sv_split_once_from(sv, ' ', 4, &before_delim, &after_delim);
        MT_CHECK_THAT(sv_eq_cstr(before_delim, "foo bar"));
        MT_CHECK_THAT(sv_eq_cstr(after_delim, "baz"));
    }
}

MT_DEFINE_TEST(to_cstr)
{
    StringView sv = sv_from_cstr("Hello World");
    {
        char buf[12];
        (void)sv_to_cstr(sv, buf, 12);
        MT_ASSERT_THAT(buf[11] == '\0');
        MT_CHECK_THAT(strcmp(buf, "Hello World") == 0);
    }
    {
        char buf[11];
        (void)sv_to_cstr(sv, buf, 11);
        MT_ASSERT_THAT(buf[10] == '\0');
        MT_CHECK_THAT(strcmp(buf, "Hello Worl") == 0);
    }
}

MT_DEFINE_TEST(to_uint64_basic)
{
    {
        StringView sv = sv_from_cstr("0");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(ok && x == 0ULL);
    }
    {
        StringView sv = sv_from_cstr("12345");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(ok && x == 12345ULL);
    }
    {   // leading plus
        StringView sv = sv_from_cstr("+42");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(ok && x == 42ULL);
    }
}

MT_DEFINE_TEST(to_int64_basic)
{
    {
        StringView sv = sv_from_cstr("0");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(ok && x == 0);
    }
    {
        StringView sv = sv_from_cstr("12345");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(ok && x == 12345);
    }
    {
        StringView sv = sv_from_cstr("-98765");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(ok && x == -98765);
    }
    {   // leading plus
        StringView sv = sv_from_cstr("+42");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(ok && x == 42);
    }
    {   // sign only is invalid
        StringView sv = sv_from_cstr("-");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_uint64_limits)
{
    {
        char buf[64];
        snprintf(buf, sizeof buf, "%" PRIu64, UINT64_MAX);
        StringView sv = sv_from_cstr(buf);
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(ok && x == UINT64_MAX);
    }
    {   // overflow: UINT64_MAX + 1
        StringView sv = sv_from_cstr("18446744073709551616"); // 2^64
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // negative not allowed for uint64
        StringView sv = sv_from_cstr("-1");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_int64_limits)
{
    {
        char buf[64];
        snprintf(buf, sizeof buf, "%" PRId64, INT64_MAX);
        StringView sv = sv_from_cstr(buf);
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(ok && x == INT64_MAX);
    }
    {
        char buf[64];
        snprintf(buf, sizeof buf, "%" PRId64, INT64_MIN);
        StringView sv = sv_from_cstr(buf);
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(ok && x == INT64_MIN);
    }
    {   // overflow positive: INT64_MAX + 1
        StringView sv = sv_from_cstr("9223372036854775808");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // overflow negative: INT64_MIN - 1
        StringView sv = sv_from_cstr("-9223372036854775809");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_uint64_format_errors)
{
    {   // empty string
        StringView sv = sv_from_cstr("");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // non-digit characters
        StringView sv = sv_from_cstr("12x3");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // whitespace not allowed (strict)
        StringView sv = sv_from_cstr(" 42");
        uint64_t x; bool ok = sv_to_uint64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_int64_format_errors)
{
    {   // empty string
        StringView sv = sv_from_cstr("");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // non-digit characters
        StringView sv = sv_from_cstr("12x3");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // whitespace not allowed (strict)
        StringView sv = sv_from_cstr(" 42");
        int64_t x; bool ok = sv_to_int64(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_long_basic)
{
    {
        StringView sv = sv_from_cstr("0");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(ok && x == 0);
    }
    {
        StringView sv = sv_from_cstr("12345");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(ok && x == 12345L);
    }
    {
        StringView sv = sv_from_cstr("-98765");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(ok && x == -98765L);
    }
    {   // leading plus
        StringView sv = sv_from_cstr("+42");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(ok && x == 42L);
    }
    {   // sign only is invalid
        StringView sv = sv_from_cstr("-");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_long_limits)
{
    {
        char buf[64];
        snprintf(buf, sizeof buf, "%ld", LONG_MAX);
        StringView sv = sv_from_cstr(buf);
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(ok && x == LONG_MAX);
    }
    {
        char buf[64];
        snprintf(buf, sizeof buf, "%ld", LONG_MIN);
        StringView sv = sv_from_cstr(buf);
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(ok && x == LONG_MIN);
    }
    {   // overflow positive
        StringView sv = sv_from_cstr("9999999999999999999999");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // overflow negative
        StringView sv = sv_from_cstr("-9999999999999999999999");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_long_format_errors)
{
    {   // empty string
        StringView sv = sv_from_cstr("");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // non-digit characters
        StringView sv = sv_from_cstr("12x3");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // whitespace not allowed (strict)
        StringView sv = sv_from_cstr(" 42");
        long x; bool ok = sv_to_long(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

static inline bool
double_eq_epsilon(double a, double b, double epsilon)
{
    double c = a - b;
    return (c >= -epsilon && c <= epsilon);
}


MT_DEFINE_TEST(to_double_basic)
{
    {
        StringView sv = sv_from_cstr("0");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.0, 1e-15));
    }
    {
        StringView sv = sv_from_cstr("123");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 123.0, 1e-15));
    }
    {
        StringView sv = sv_from_cstr("-42");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, -42.0, 1e-15));
    }
    {
        StringView sv = sv_from_cstr("3.14159");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 3.14159, 1e-12));
    }
    {   // no int part, fraction only
        StringView sv = sv_from_cstr(".5");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.5, 1e-15));
    }
    {   // sign + fraction only
        StringView sv = sv_from_cstr("+.5");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.5, 1e-15));
    }
    {   // dot must be followed by at least one digit
        StringView sv = sv_from_cstr("1.");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_double_exponent)
{
    {
        StringView sv = sv_from_cstr("1e3");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 1000.0, 1e-12));
    }
    {
        StringView sv = sv_from_cstr("1e-3");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.001, 1e-15));
    }
    {
        StringView sv = sv_from_cstr("-.5e1");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, -5.0, 1e-12));
    }
    {   // trailing junk not allowed
        StringView sv = sv_from_cstr("1e3x");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_double_limits)
{
    {   // largest "easy" value should work
        StringView sv = sv_from_cstr("1e308");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && x > 0.0);
    }
    {   // overflow should fail
        StringView sv = sv_from_cstr("1e309");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // overflow with mantissa
        StringView sv = sv_from_cstr("1.8e308");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // strong underflow -> success with signed zero
        StringView sv = sv_from_cstr("1e-400");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.0, 0.0));
    }
}

MT_DEFINE_TEST(to_double_format_errors)
{
    {   // empty
        StringView sv = sv_from_cstr("");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // sign only
        StringView sv = sv_from_cstr("+");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // junk
        StringView sv = sv_from_cstr("12x");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
    {   // whitespace not allowed
        StringView sv = sv_from_cstr(" 1");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(!ok);
    }
}

MT_DEFINE_TEST(to_double_precision_paths)
{
    {   // >19 fractional digits path
        StringView sv = sv_from_cstr("1.00000000000000000001"); // 20 frac digits
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 1.0, 1e-18));
    }
    {   // many leading fractional zeros
        StringView sv = sv_from_cstr("0.0000000000000000001234");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && x > 0.0);
    }
}

MT_DEFINE_TEST(to_double_signed_zero)
{
    {
        StringView sv = sv_from_cstr("-0");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.0, 0.0) && signbit(x));
    }
    {
        StringView sv = sv_from_cstr("-0.0");
        double x; bool ok = sv_to_double(sv, &x);
        MT_CHECK_THAT(ok && double_eq_epsilon(x, 0.0, 0.0) && signbit(x));
    }
}


#if defined(__cplusplus) && __cplusplus >= 201703L  && defined(SV_STD_SV_CONV)
#define TEST_STD_SV
MT_DEFINE_TEST(sv_from_std_sv)
{
    std::string_view stdsv("hello world");
    StringView sv = sv_from_std_sv(stdsv);
    MT_CHECK_THAT(sv_eq_cstr(sv, "hello world"));
}
MT_DEFINE_TEST(sv_to_std_sv)
{
    StringView sv = SV_LIT("hello world");
    std::string_view stdsv = sv_to_std_sv(sv);
    MT_CHECK_THAT(stdsv == "hello world");
}
#endif

int
main(void)
{
    MT_INIT();

    MT_RUN_TEST(empty);
    MT_RUN_TEST(from_cstr);
    MT_RUN_TEST(from_parts);
    MT_RUN_TEST(lit);

    MT_RUN_TEST(at);
    MT_RUN_TEST(first);
    MT_RUN_TEST(last);
    MT_RUN_TEST(substr);

    MT_RUN_TEST(eq);

    MT_RUN_TEST(eq_cstr_basic);
    MT_RUN_TEST(eq_cstr_empty_and_null);
    MT_RUN_TEST(eq_cstr_case_sensitive);
    MT_RUN_TEST(eq_cstr_embedded_nul_behavior);
    MT_RUN_TEST(eq_cstr_subview);

    MT_RUN_TEST(eq_cstr_n_basic);
    MT_RUN_TEST(eq_cstr_n_null_cstr);
    MT_RUN_TEST(eq_cstr_n_length_mismatch);
    MT_RUN_TEST(eq_cstr_n_binary_bytes);
    MT_RUN_TEST(eq_cstr_n_subview);
    MT_RUN_TEST(eq_cstr_n_case_sensitive);
    MT_RUN_TEST(eq_cstr_vs_eq_cstr_n_equivalence);

    MT_RUN_TEST(is_empty);
    MT_RUN_TEST(starts_with);
    MT_RUN_TEST(ends_with);

    MT_RUN_TEST(find_char);
    MT_RUN_TEST(rfind_char);
    MT_RUN_TEST(find_char_from);
    MT_RUN_TEST(rfind_char_from);
    MT_RUN_TEST(find_substr);
    MT_RUN_TEST(rfind_substr);
    MT_RUN_TEST(find_substr_from);
    MT_RUN_TEST(rfind_substr_from);
    MT_RUN_TEST(find_substr_cstr);

    MT_RUN_TEST(contains);
    MT_RUN_TEST(contains_cstr);

    MT_RUN_TEST(trim_left);
    MT_RUN_TEST(trim_right);
    MT_RUN_TEST(trim);

    MT_RUN_TEST(take_basic);
    MT_RUN_TEST(take_zero_and_empty);
    MT_RUN_TEST(take_exact_length);
    MT_RUN_TEST(take_equivalence_with_substr);
    MT_RUN_TEST(take_nested_chaining_does_not_mutate);

    MT_RUN_TEST(drop_basic);
    MT_RUN_TEST(drop_exact_and_empty);
    MT_RUN_TEST(drop_equivalence_with_substr);
    MT_RUN_TEST(drop_pairs_with_take);
    MT_RUN_TEST(drop_does_not_mutate);

    MT_RUN_TEST(drop_last_basic);
    MT_RUN_TEST(drop_last_exact_and_empty);
    MT_RUN_TEST(drop_last_equivalence_with_substr);
    MT_RUN_TEST(drop_last_pairs_with_take_last);
    MT_RUN_TEST(drop_last_does_not_mutate);

    MT_RUN_TEST(take_last_basic);
    MT_RUN_TEST(take_last_exact_and_empty);
    MT_RUN_TEST(take_last_equivalence_with_substr);
    MT_RUN_TEST(take_last_pairs_with_drop_last);
    MT_RUN_TEST(take_last_does_not_mutate);

    MT_RUN_TEST(take_and_consume_basic);
    MT_RUN_TEST(take_and_consume_zero_and_exact);
    MT_RUN_TEST(take_and_consume_chain);
    MT_RUN_TEST(drop_and_consume_basic);
    MT_RUN_TEST(drop_and_consume_zero_and_exact);
    MT_RUN_TEST(drop_and_consume_chain);
    MT_RUN_TEST(take_last_and_consume_basic);
    MT_RUN_TEST(take_last_and_consume_zero_and_exact);
    MT_RUN_TEST(take_last_and_consume_chain);
    MT_RUN_TEST(drop_last_and_consume_basic);
    MT_RUN_TEST(drop_last_and_consume_zero_and_exact);
    MT_RUN_TEST(drop_last_and_consume_chain);

    MT_RUN_TEST(split_first);
    MT_RUN_TEST(split_last);
    MT_RUN_TEST(split_once_from);

    MT_RUN_TEST(to_cstr);

    MT_RUN_TEST(to_uint64_basic);
    MT_RUN_TEST(to_int64_basic);
    MT_RUN_TEST(to_uint64_limits);
    MT_RUN_TEST(to_int64_limits);
    MT_RUN_TEST(to_uint64_format_errors);
    MT_RUN_TEST(to_int64_format_errors);
    MT_RUN_TEST(to_long_basic);
    MT_RUN_TEST(to_long_limits);
    MT_RUN_TEST(to_long_format_errors);
    MT_RUN_TEST(to_double_basic);
    MT_RUN_TEST(to_double_exponent);
    MT_RUN_TEST(to_double_limits);
    MT_RUN_TEST(to_double_format_errors);
    MT_RUN_TEST(to_double_precision_paths);
    MT_RUN_TEST(to_double_signed_zero);

#ifdef TEST_STD_SV
    MT_RUN_TEST(sv_from_std_sv);
    MT_RUN_TEST(sv_to_std_sv);
#endif

    MT_PRINT_SUMMARY();
    return MT_EXIT_CODE;
}
