#include <Pos.hpp>
#include "tests.hpp"

using namespace tafl;

TEST_CASE("Positions work correctly")
{
    Pos x1y2 = {1,2};
    Pos x1y0 = {1,0};
    Pos x8y8 = {8,8};
    Pos x7y8 = {7,8};

    Pos x1y18 = {1,18};
    Pos x18y17 = {18,17};

    REQUIRE(x1y2.flatten(9) == 2*9 + 1);
    REQUIRE(x1y0.flatten(9) == 0*9 + 1);

    REQUIRE(x8y8.flatten(18) == 8*18 + 8);

    REQUIRE(x1y0 < x1y2);
    REQUIRE(x7y8 < x8y8);

    REQUIRE(x18y17 < x1y18);
}
