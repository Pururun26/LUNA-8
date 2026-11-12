#pragma once
#include <stdint.h>

void    luna8_save_open(const char* cart_path);
void    luna8_save_close(void);
void    luna8_save_dset(int slot, int64_t value);
int64_t luna8_save_dget(int slot);