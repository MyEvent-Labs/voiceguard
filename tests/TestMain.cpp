#include "TestRegistry.h"

#include <cmath>
#include <iostream>

namespace vgtest
{

std::vector<TestCase>& registry()
{
    static std::vector<TestCase> r;
    return r;
}

void fail (const char* file, int line, const std::string& message)
{
    throw TestFailure (std::string (file) + ":" + std::to_string (line) + ": " + message);
}

void failClose (const char* file, int line, double actual, double expected, double tolerance,
                const char* actualExpr, const char* expectedExpr)
{
    const double diff = std::abs (actual - expected);

    if (! (diff <= tolerance))
        throw TestFailure (std::string (file) + ":" + std::to_string (line) + ": expected "
                           + expectedExpr + " (" + std::to_string (expected) + ") within "
                           + std::to_string (tolerance) + " of " + actualExpr + " ("
                           + std::to_string (actual) + "), diff=" + std::to_string (diff));
}

int runAll()
{
    int failed = 0;

    for (auto& test : registry())
    {
        try
        {
            test.fn();
            std::cout << "[PASS] " << test.name << "\n";
        }
        catch (const std::exception& e)
        {
            ++failed;
            std::cout << "[FAIL] " << test.name << " : " << e.what() << "\n";
        }
    }

    const int passed = (int) registry().size() - failed;
    std::cout << ((failed == 0) ? "OK " : "FAILED ") << passed << "/" << registry().size() << "\n";
    return failed == 0 ? 0 : 1;
}

} // namespace vgtest

int main()
{
    return vgtest::runAll();
}
