#include "BoardHashTable.hpp"

#include "tests.hpp"

using namespace tafl;


TEST_CASE("An empty BoardHashTable has no elements")
{
    BoardHashTable<10> table;

    REQUIRE_FALSE(table.has(1));
}


TEST_CASE("The BoardHashTable can insert elements, until it's full")
{
    BoardHashTable<1> table;

    // 0 is not a valid board, since there are no pieces then
    table.insert(1);
    REQUIRE(table.has(1));
    table.insert(2);
    REQUIRE(table.has(2));

    table.insert(3);
    REQUIRE_FALSE(table.has(3));
}
