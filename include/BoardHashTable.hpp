#pragma once

#include <array>
#include <unordered_map>

namespace tafl
{

template <unsigned N>
class BoardHashTable
{
public:
    bool has(uint64_t board) const
    {
        // For now
        auto hash = board;

        // Check the two slots
        return m_table[hash % N * 2] == board || m_table[hash % N * 2 + 1] == board;
    }

    void insert(uint64_t board)
    {
        auto hash = board;

        if (m_table[hash % N * 2] == 0)
        {
            m_table[hash % N * 2] = board;
        }
        else if (m_table[hash % N * 2 + 1] == 0)
        {
            m_table[hash % N * 2 + 1] = board;
        }
    }

private:
    std::array<uint64_t, N * 2> m_table {0};
};

} // namespace tafl
