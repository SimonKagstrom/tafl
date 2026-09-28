#pragma once

#include <array>
#include <optional>
#include <unordered_map>
#include <fstream>

namespace tafl
{

template <unsigned N>
class BoardHashTable
{
public:
    struct Entry
    {
        uint64_t checksum {0};

        float whiteWins;
        float blackWins;
    };

    std::optional<Entry> get(uint64_t board) const
    {
        // For now
        auto hash = board;

        // Check the two slots
        if (m_table[hash % N * 2].checksum == board)
        {
            return m_table[hash % N * 2];
        }
        else if (m_table[hash % N * 2 + 1].checksum == board)
        {
            return m_table[hash % N * 2 + 1];
        }

        return std::nullopt;
    }

    void insert(uint64_t board, float whiteWins, float blackWins)
    {
        auto hash = board;

        auto &slot_0 = m_table[hash % N * 2];
        auto &slot_1 = m_table[hash % N * 2 + 1];

        if (slot_0.checksum == board)
        {
            slot_0 = {board, whiteWins, blackWins};
            return;
        }
        else if (slot_1.checksum == board)
        {
            slot_1 = {board, whiteWins, blackWins};
            return;
        }

        if (slot_0.checksum == 0)
        {
            slot_0 = {board, whiteWins, blackWins};
        }
        else if (slot_1.checksum == 0)
        {
            slot_1 = {board, whiteWins, blackWins};
        }
    }

    void merge(const BoardHashTable& other)
    {
        for (const auto& e : other.m_table)
        {
            if (e.checksum != 0)
            {
                insert(e.checksum, e.whiteWins, e.blackWins);
            }
        }
    }

    void fillFromFile(std::istream& file)
    {
        do
        {
            Entry e;
            file.read(reinterpret_cast<char*>(&e), sizeof(Entry));
            if (file)
            {
                insert(e.checksum, e.whiteWins, e.blackWins);
            }
        } while (file);
    }

    void writeToFile(std::ostream& file) const
    {
        for (const auto& e : m_table)
        {
            if (e.checksum != 0)
            {
                file.write(reinterpret_cast<const char*>(&e), sizeof(Entry));
            }
        }
    }

private:
    std::array<Entry, N * 2> m_table {0};
};

} // namespace tafl
