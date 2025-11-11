#pragma once

enum class COLOR {
    none = -1,
    white, black,
    both
};

enum class SQUARE {
  none = -1,
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8
};

enum class PIECE{
    // pawn(Pp), rook(Rr), knight(Nn), bishop(Bb), queen(Qq), king(Kk)
    // upperCase white; lowerCase black
    P, R, N, B, Q, K,
    p, r, n, b, q, k
};

enum class MoveType{
    quiet_move = 0,
    double_pawn_push = 1,
    king_castle = 2,
    queen_castle = 3,
    capture = 4,
    en_passant_capture = 5,
    knight_promotion = 8,
    bishop_promotion = 9,
    rook_promotion = 10,
    queen_promotion = 11,
    knight_promo_capture = 12,
    bishop_promo_capture = 13,
    rook_promo_capture = 14,
    queen_promo_capture = 15
};
