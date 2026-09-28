#include "BoardHashTable.hpp"
#include "tests.hpp"

using namespace tafl;


TEST_CASE("An empty BoardHashTable has no elements")
{
    BoardHashTable<10> table;

    REQUIRE_FALSE(table.get(1));
}

TEST_CASE("Entries in the BoardHashTable are correctly stored")
{
    BoardHashTable<1> table;

    table.insert(1, 0.1, 0.7);
    auto entry = table.get(1);
    REQUIRE(entry);
    REQUIRE(entry->whiteWins == doctest::Approx(0.1));
    REQUIRE(entry->blackWins == doctest::Approx(0.7));
}

TEST_CASE("The BoardHashTable can insert elements, until it's full")
{
    BoardHashTable<1> table;

    // 0 is not a valid board, since there are no pieces then
    table.insert(1, 0.5, 0.5);
    REQUIRE(table.get(1));
    table.insert(2, 0.5, 0.6);
    REQUIRE(table.get(2));
    REQUIRE(table.get(2)->blackWins == doctest::Approx(0.6));

    // Re-insert
    table.insert(2, 0.9, 0.8);
    REQUIRE(table.get(2));
    REQUIRE(table.get(2)->blackWins == doctest::Approx(0.8));

    table.insert(3, 0.5, 0.5);
    REQUIRE_FALSE(table.get(3));
}

TEST_CASE("two BoardHashTables can be merged")
{
    BoardHashTable<2> table_1, table_2;

    table_1.insert(1, 0.5, 0.7);
    table_2.insert(2, 0.6, 0.8);

    REQUIRE(table_2.get(2));
    REQUIRE_FALSE(table_1.get(2));

    table_1.merge(table_2);
    REQUIRE(table_1.get(1));
    REQUIRE(table_1.get(2));
}

TEST_CASE("BoardHashTable can be written and read from disk")
{
    using HT = BoardHashTable<2>;

    std::array<HT::Entry, 2> entries = {
        HT::Entry {1, 0.5, 0.7},
        HT::Entry {2, 0.6, 0.8},
    };

    THEN("the entries can be read from disk")
    {
        HT from_disk;
        std::stringstream instream;
        instream.write(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(HT::Entry));
        from_disk.fillFromFile(instream);

        auto entry_1 = from_disk.get(1);
        auto entry_2 = from_disk.get(2);
        REQUIRE_FALSE(from_disk.get(3));

        REQUIRE(entry_1);
        REQUIRE(entry_2);
        REQUIRE(entry_1->whiteWins == doctest::Approx(0.5));
        REQUIRE(entry_1->blackWins == doctest::Approx(0.7));
        REQUIRE(entry_2->whiteWins == doctest::Approx(0.6));
        REQUIRE(entry_2->blackWins == doctest::Approx(0.8));
    }

    THEN("a hash table can be written to disk")
    {
        std::array<HT::Entry, 2> read_entries;

        HT to_disk, from_disk;

        to_disk.insert(1, 0.5, 0.7);
        to_disk.insert(2, 0.6, 0.8);

        std::stringstream outstream;
        to_disk.writeToFile(outstream);
        outstream.read(reinterpret_cast<char*>(read_entries.data()), read_entries.size() * sizeof(HT::Entry));  

        // Read back into a second table
        std::stringstream instream;
        instream.write(reinterpret_cast<char*>(entries.data()), entries.size() * sizeof(HT::Entry));

        from_disk.fillFromFile(instream);
        auto entry_1 = from_disk.get(1);
        auto entry_2 = from_disk.get(2);

        REQUIRE(entry_1);
        REQUIRE(entry_2);
        REQUIRE(entry_1->whiteWins == doctest::Approx(0.5));
        REQUIRE(entry_1->blackWins == doctest::Approx(0.7));
        REQUIRE(entry_2->whiteWins == doctest::Approx(0.6));
        REQUIRE(entry_2->blackWins == doctest::Approx(0.8));
    }
}
