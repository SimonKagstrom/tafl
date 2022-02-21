#pragma once

#include <IBoard.hpp>

namespace tafl
{

class Board : public IBoard
{
public:
    Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>> &pieces);

    unsigned getBoardDimension() const override;

    std::optional<Piece::Type> pieceAt(const Pos &pos) const override;

    const std::vector<Piece> getPieces(const Color &which) const override;

    virtual void move(Move move) override;

    virtual Color getTurn() const override;

    virtual void setTurn(Color which) override;

    virtual std::optional<Color> getWinner() const override;

private:
    void scanCaptures();

    std::optional<Color> pieceColorAt(const Pos &pos) const;

    const unsigned m_dimensions;
    Color m_turn{Color::White};
    std::map<Pos, Piece> m_pieces;

    std::optional<Color> m_winner;
};

}