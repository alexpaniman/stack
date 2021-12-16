#pragma once

#include <cstdint>
#include <stdint.h>

uint32_t  int_hash(const int   number);
uint32_t char_hash(const char  symbol); 
uint32_t  str_hash(const char* string);

uint32_t combine_hash(uint32_t lhs, uint32_t rhs);
