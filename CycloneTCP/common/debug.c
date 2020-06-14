/**
 * @file debug.c
 * @brief Debugging facilities
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.9.6
 **/

//Dependencies
#include "debug.h"
#include "os_debug.h"

/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/

void debugDisplayArray(FILE *stream,
   const char_t *prepend, const void *data, size_t length)
{
   uint_t i;

   //Dump the contents of the array
   for(i = 0; i < length; i++)
   {
      //Beginning of a new line?
      if((i % 16) == 0)
      {
         TRACE_PRINTF("%s", prepend);
      }

      //Display current data byte
      TRACE_PRINTF("%02" PRIX8 " ", *((const uint8_t *) data + i));

      //End of current line?
      if((i % 16) == 15 || i == (length - 1))
      {
         TRACE_PRINTF("\r\n");
      }
   }
}

#define ITM_PORT 0
#if defined (ITM_PORT) && (ITM_PORT == 1)
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))  
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))  
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))  
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))  
#define TRCENA          0x01000000
#endif

int_t fputc(int_t c, FILE *stream)
{
   //Standard output or error output?
   if(stream == stdout || stream == stderr)
   {
      //Character to be written
      uint8_t ch = c;

      //Transmit data
#if defined (ITM_PORT) && (ITM_PORT == 1)
        if (DEMCR & TRCENA) {
        while (ITM_Port32(0) == 0);
        ITM_Port8(0) = ch;
        }
        HAL_UART_Transmit(&UART_Handle, &ch, 1, HAL_MAX_DELAY);
#else
      rt_hw_console_output((char *)&ch);
#endif
      //On success, the character written is returned
      return c;
   }
   //Unknown output?
   else
   {
      //If a writing error occurs, EOF is returned
      return EOF;
   }
}