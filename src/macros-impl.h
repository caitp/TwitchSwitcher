// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

// Commonly needed preprocessor utilities
#define TSW_CAT(LEFT, RIGHT) TSW_CAT_TUPLE((LEFT, RIGHT))
#define TSW_CAT_TUPLE(ARGS) TSW_CAT_ ## ARGS

#ifdef _MSC_VER
#define TSW_CAT_(LEFT, RIGHT) TSW_CAT_MSVC_(~, LEFT ## RIGHT)
#define TSW_CAT_MSVC_(OP, RET) RET
#else
#define TSW_CAT_(LEFT, RIGHT) LEFT ## RIGHT
#endif

#define TSW_XCAT(LEFT, RIGHT) TSW_CAT(LEFT, RIGHT)
#define TSW_EXPAND(X) X

// TSW_ARGC() will return a minimum of `1`, for details on the explanation, see
// https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/

#if defined(_MSC_VER)
// Due to an MSVC bug re: macro expansion, TSW_ARGC_SEQ_N_ and RSEQ_N_ are implemented inline
// here. Remember to update both copies in the unlikely event they need to change.

#define TSW_ARGC(...) TSW_CAT(TSW_ARGC_SEQ_N_(__VA_ARGS__, \
    16, 15, 14, 13, 12, 11, 10, 9, \
     8, 7,  6,  5,  4,  3,  2,  1, \
     0),)

#else
#define TSW_ARGC(...) TSW_ARGC_(__VA_ARGS__, TSW_ARGC_RSEQ_N_())
#endif

#define TSW_ARGC_(...) TSW_ARGC_SEQ_N_(__VA_ARGS__)

// With  N arguments, return the value of the Nth. For now, limited to
// 50, as 50 is a pretty high upper limit for this sort of thing.
#define TSW_ARGC_SEQ_N_( \
    _01, _02, _03, _04, _05, _06, _07, _08, _09, _10, \
    _11, _12, _13, _14, _15, _16, N, ...) N

// For use with the above method, pads the arg count with numeric values
// to be returned by TSW_ARGC(__VA_ARGS__). This will provide a useful
// argument count so long as there are fewer than N arguments passed in.
#define TSW_ARGC_RSEQ_N_() \
    16, 15, 14, 13, 12, 11, 10,  9, \
     8,  7,  6,  5,  4,  3,  2,  1, \
     0

#define TSW_GET_MACRO_N_(MACRO, ...) TSW_XCAT(TSW_XCAT(MACRO, _), TSW_XCAT(TSW_ARGC(__VA_ARGS__), _))

#define TSW_APPLY_(MACRO, ...) MACRO(__VA_ARGS__)

#define TSW_FOREACH_1_(M, _1) M(_1)
#define TSW_FOREACH_2_(M, _1, _2) M(_1) ##M(_2)
#define TSW_FOREACH_3_(M, _1, _2, _3) M(_1) ## M(_2) ## M(_3)
#define TSW_FOREACH_4_(M, _1, _2, _3, _4) M(_1) ## M(_2) ## M(_3) ## M(_4)
#define TSW_FOREACH_5_(M, _1, _2, _3, _4, _5) M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5)
#define TSW_FOREACH_6_(M, _1, _2, _3, _4, _5, _6) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6)
#define TSW_FOREACH_7_(M, _1, _2, _3, _4, _5, _6, _7) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7)
#define TSW_FOREACH_8_(M, _1, _2, _3, _4, _5, _6, _7, _8) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8)
#define TSW_FOREACH_9_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9)
#define TSW_FOREACH_10_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10)
#define TSW_FOREACH_11_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10) \
     M(_11)
#define TSW_FOREACH_12_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10) \
     M(_11) ## M(_12)
#define TSW_FOREACH_13_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10) \
     M(_11) ## M(_12) ## M(_13)
#define TSW_FOREACH_14_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10) \
     M(_11) ## M(_12) ## M(_13) ## M(_14)
#define TSW_FOREACH_15_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10) \
     M(_11) ## M(_12) ## M(_13) ## M(_14) ## M(_15)
#define TSW_FOREACH_16_(M, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, \
                        _16) \
     M(_1) ## M(_2) ## M(_3) ## M(_4) ## M(_5) ## M(_6) ## M(_7) ## M(_8) ## M(_9) ## M(_10) \
     M(_11) ## M(_12) ## M(_13) ## M(_14) ## M(_15) ## M(_16)

#define TSW_FOREACH(MACRO, ...) TSW_APPLY_(TSW_GET_MACRO_N_(TSW_FOREACH, __VA_ARGS__), MACRO, __VA_ARGS__)
