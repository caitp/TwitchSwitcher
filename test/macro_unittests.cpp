// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <gtest/gtest.h>
#include "macros-impl.h"

TEST(TSW_MACROS, ARGC) {
    EXPECT_EQ(1, TSW_ARGC(1));
    EXPECT_EQ(2, TSW_ARGC(2, 1));
    EXPECT_EQ(3, TSW_ARGC(3, 2, 1));
    EXPECT_EQ(4, TSW_ARGC(4, 3, 2, 1));
    EXPECT_EQ(5, TSW_ARGC(5, 4, 3, 2, 1));
    EXPECT_EQ(6, TSW_ARGC(6, 5, 4, 3, 2, 1));
    EXPECT_EQ(7, TSW_ARGC(7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(8, TSW_ARGC(8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(9, TSW_ARGC(9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(10, TSW_ARGC(10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(11, TSW_ARGC(11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(12, TSW_ARGC(12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(13, TSW_ARGC(13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(14, TSW_ARGC(14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(15, TSW_ARGC(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    EXPECT_EQ(16, TSW_ARGC(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
}

TEST(TSW_MACROS, GET_MACRO_N_) {
#define PASTE(X) X

    const int EXP_1_ = 1;
    EXPECT_EQ(1, TSW_GET_MACRO_N_(EXP, A));

#define EXP_1_(A) 1400
    EXPECT_EQ(1400, TSW_GET_MACRO_N_(EXP, A)(A));
#undef EXP_1_

    const int EXP_2_ = 2;
    EXPECT_EQ(2, TSW_GET_MACRO_N_(EXP, A, B));

    const int AB = 2700;
#define EXP_2_(M, X, ...) M(X) ## M(__VA_ARGS__)
    EXPECT_EQ(2700, TSW_GET_MACRO_N_(EXP, A, B)(PASTE, A, B));
#undef EXP_2_

    const int EXP_3_ = 3;
    EXPECT_EQ(3, TSW_GET_MACRO_N_(EXP, A, B, C));


    const int ABC = 3500;
#define EXP_3_(M, X, Y, Z) M(X) ## M(Y) ## M(Z)
    EXPECT_EQ(3500, TSW_GET_MACRO_N_(EXP, A, B, C)(PASTE, A, B, C));
#undef EXP_3_

    const int EXP_4_ = 4;
    EXPECT_EQ(4, TSW_GET_MACRO_N_(EXP, A, B, C, D));

    const int ABCD = 4800;
#define EXP_4_(M, X, Y, Z, Q) M(X) ## M(Y) ## M(Z) ## M(Q)
    EXPECT_EQ(4800, TSW_GET_MACRO_N_(EXP, A, B, C, D)(PASTE, A, B, C, D));
#undef EXP_4_

    const int EXP_5_ = 5;
    EXPECT_EQ(5, TSW_GET_MACRO_N_(EXP, A, B, C, D, E));

    const int ABCDE = 5200;
#define EXP_5_(M, X, Y, Z, Q, R) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R)
    EXPECT_EQ(5200, TSW_GET_MACRO_N_(EXP, A, B, C, D, E)(PASTE, A, B, C, D, E));
#undef EXP_5_

    const int EXP_6_ = 6;
    EXPECT_EQ(6, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F));

    const int ABCDEF = 5200;
#define EXP_6_(M, X, Y, Z, Q, R, S) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S)
    EXPECT_EQ(5200, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F)(PASTE, A, B, C, D, E, F));
#undef EXP_6_

    const int EXP_7_ = 7;
    EXPECT_EQ(7, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G));

    const int ABCDEFG = 5900;
#define EXP_7_(M, X, Y, Z, Q, R, S, T) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T)
    EXPECT_EQ(5900, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G)(PASTE, A, B, C, D, E, F, G));
#undef EXP_7_

    const int EXP_8_ = 8;
    EXPECT_EQ(8, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H));

    const int ABCDEFGH = 6300;
#define EXP_8_(M, X, Y, Z, Q, R, S, T, U) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U)
    EXPECT_EQ(6300, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H)(PASTE, A, B, C, D, E, F, G, H));
#undef EXP_8_

    const int EXP_9_ = 9;
    EXPECT_EQ(9, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I));

const int ABCDEFGHI = 6700;
#define EXP_9_(M, X, Y, Z, Q, R, S, T, U, V) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## M(V)
    EXPECT_EQ(6700, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I)(PASTE, A, B, C, D, E, F, G, H, I));
#undef EXP_9_

    const int EXP_10_ = 10;
    EXPECT_EQ(10, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J));

const int ABCDEFGHIJ = 9100;
#define EXP_10_(M, X, Y, Z, Q, R, S, T, U, V, W) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## M(V) ## M(W)
    EXPECT_EQ(9100, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J)(PASTE, A, B, C, D, E, F, G, H, I, J));
#undef EXP_10_

    const int EXP_11_ = 11;
    EXPECT_EQ(11, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K));

const int ABCDEFGHIJK = 1400;
#define EXP_11_(M, X, Y, Z, Q, R, S, T, U, V, W, A) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## M(V) ## M(W) ## M(A)
    EXPECT_EQ(1400, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K)(PASTE, A, B, C, D, E, F, G, H, I, J, K));
#undef EXP_11_

    const int EXP_12_ = 12;
    EXPECT_EQ(12, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L));

const int ABCDEFGHIJKL = 1700;
#define EXP_12_(M, X, Y, Z, Q, R, S, T, U, V, W, A, B) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## \
                                                       M(V) ## M(W) ## M(A) ## M(B)
    EXPECT_EQ(1700, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L)(PASTE, A, B, C, D, E, F, G, H, I, J, K, L));
#undef EXP_12_

    const int EXP_13_ = 13;
    EXPECT_EQ(13, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M));

const int ABCDEFGHIJKLM = 2300;
#define EXP_13_(M, X, Y, Z, Q, R, S, T, U, V, W, A, B, C) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## \
                                                          M(V) ## M(W) ## M(A) ## M(B) ## M(C)
    EXPECT_EQ(2300, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M)(PASTE, A, B, C, D, E, F, G, H, I, J, K, L, M));
#undef EXP_13_

    const int EXP_14_ = 14;
    EXPECT_EQ(14, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M, N));

const int ABCDEFGHIJKLMN = -74;
#define EXP_14_(M, X, Y, Z, Q, R, S, T, U, V, W, A, B, C, D) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## \
                                                             M(V) ## M(W) ## M(A) ## M(B) ## M(C) ## M(D)
    EXPECT_EQ(-74, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M, N)(PASTE, A, B, C, D, E, F, G, H, I, J, K, L, M, N));
#undef EXP_14_

    const int EXP_15_ = 15;
    EXPECT_EQ(15, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O));

const int ABCDEFGHIJKLMNO = 354;
#define EXP_15_(M, X, Y, Z, Q, R, S, T, U, V, W, A, B, C, D, E) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## \
                                                             M(V) ## M(W) ## M(A) ## M(B) ## M(C) ## M(D) ## M(E)
    EXPECT_EQ(354, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O)(PASTE, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O));
#undef EXP_15_


    const int EXP_16_ = 16;
    EXPECT_EQ(16, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P));

const int ABCDEFGHIJKLMNOP = 7988;
#define EXP_16_(M, X, Y, Z, Q, R, S, T, U, V, W, A, B, C, D, E, F) M(X) ## M(Y) ## M(Z) ## M(Q) ## M(R) ## M(S) ## M(T) ## M(U) ## \
                                                                   M(V) ## M(W) ## M(A) ## M(B) ## M(C) ## M(D) ## M(E) ## M(F)
    EXPECT_EQ(7988, TSW_GET_MACRO_N_(EXP, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P)(PASTE, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P));
#undef EXP_16_

#undef PASTE
}

TEST(TSW_MACROS, FOREACH) {
#define STRINGIFY(X) #X
    EXPECT_STREQ("1", TSW_FOREACH(STRINGIFY, 1));
    EXPECT_STREQ("12", TSW_FOREACH(STRINGIFY, 1, 2));
    EXPECT_STREQ("123", TSW_FOREACH(STRINGIFY, 1, 2, 3));
    EXPECT_STREQ("1234", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4));
    EXPECT_STREQ("12345", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5));
    EXPECT_STREQ("123456", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6));
    EXPECT_STREQ("1234567", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7));
    EXPECT_STREQ("12345678", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8));
    EXPECT_STREQ("123456789", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_STREQ("123456789A", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A));
    EXPECT_STREQ("123456789AB", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B));
    EXPECT_STREQ("123456789ABC", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C));
    EXPECT_STREQ("123456789ABCD", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D));
    EXPECT_STREQ("123456789ABCDE", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E));
    EXPECT_STREQ("123456789ABCDEF", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F));
    EXPECT_STREQ("123456789ABCDEFG", TSW_FOREACH(STRINGIFY, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F, G));
#undef STRINGIFY
}
