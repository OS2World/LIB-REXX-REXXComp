#include <stdio.h>      /* needed for fprintf and constants */
#include "crc32.h"

void UpdateCRC32(char c,unsigned long *crccode)
{
/*
 * Copyright (C) 1986 Gary S. Brown.  You may use this program, or
 * code or tables extracted from it, as desired without restriction.
 */

    *crccode = crc_32_tab[(int) ((*crccode) ^ (c)) & 0xff] ^
         (((*crccode) >> 8) & 0x00FFFFFFL);

return;
}

