#pragma once

// Tiny zero-dependency test registry used by the offline harness.

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace vgtest
{

struct TestFailure : std::runtime_error
{
    explicit TestFailure (const std::string& what) : std::runtime_error (what) {}
};

struct TestCase
{
    std::string name;
    std::function<void()> fn;
};

std::vector<TestCase>& registry();

struct Registrar
{
    Registrar (const char* name, std::function<void()> fn)
    {
        registry().push_back ({ name, std::move (fn) });
    }
};

void fail (const char* file, int line, const std::string& message);
void failClose (const char* file, int line, double actual, double expected, double tolerance,
                const char* actualExpr, const char* expectedExpr);
int runAll();

} // namespace vgtest

#define VG_TEST(name)                                                          \
    static void name();                                                        \
    static ::vgtest::Registrar vg_reg_##name (#name, name);                    \
    static void name()

#define VG_CHECK(cond)                                                         \
    do                                                                         \
    {                                                                          \
        if (! (cond))                                                          \
            ::vgtest::fail (__FILE__, __LINE__, "CHECK failed: " #cond);       \
    } while (false)

#define VG_CHECK_CLOSE(actual, expected, tolerance)                            \
    do                                                                         \
    {                                                                          \
        ::vgtest::failClose (__FILE__, __LINE__, (double) (actual),            \
                             (double) (expected), (double) (tolerance),        \
                             #actual, #expected);                              \
    } while (false)
