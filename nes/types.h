#pragma once

#include <numeric>

// I really like Rust's type names
// For doing work like this I think it keeps it much clearer than any other convention

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

static_assert(sizeof(u8) == 1, "");
static_assert(sizeof(u16) == 2, "");
static_assert(sizeof(u32) == 4, "");
static_assert(sizeof(u64) == 8, "");
static_assert(std::numeric_limits<u8>::is_signed == false, "");
static_assert(std::numeric_limits<u16>::is_signed == false, "");
static_assert(std::numeric_limits<u32>::is_signed == false, "");
static_assert(std::numeric_limits<u64>::is_signed == false, "");

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long int i64;

static_assert(sizeof(i8) == 1, "");
static_assert(sizeof(i16) == 2, "");
static_assert(sizeof(i32) == 4, "");
static_assert(sizeof(i64) == 8, "");
static_assert(std::numeric_limits<i8>::is_signed == true, "");
static_assert(std::numeric_limits<i16>::is_signed == true, "");
static_assert(std::numeric_limits<i32>::is_signed == true, "");
static_assert(std::numeric_limits<i64>::is_signed == true, "");
