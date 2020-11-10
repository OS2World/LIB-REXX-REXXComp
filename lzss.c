/*--------------------------------------------------------------
        LZSS.C -- A Data Compression Program

        4/6/1989 Haruhiko Okumura
        Use, distribute, and modify this program freely.
        Please send me your improved versions.
                PC-VAN          SCIENCE
                NIFTY-Serve     PAF01022
                CompuServe      74050,1022
-------------------------------------------------------------
   modified:
         7/30/93 Robert Mahoney - rmahoney@bix.com
         - made all variables local - no globals
         - removed main
         - ifdef'd code to be silent if compiled into a DLL
         - added CRC checking
         - added space and memory error checking
         - when compressed file gets larger then input error is returned
         - all comments are Haruhiko's
         - Haruhiko's code is over my head so I didn't touch that!
--------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <io.h>
#include "lzss.h"


void InitTree(int rson[],int dad[])  /* initialize trees */
{
   int  i;

   /* For i = 0 to N - 1, rson[i] and lson[i] will be the right and   */
   /* left children of node i.  These nodes need not be initialized.  */
   /* Also, dad[i] is the parent of node i.  These are initialized to */
   /* NIL (= N), which stands for 'not used.'                         */
   /* For i = 0 to 255, rson[N + i + 1] is the root of the tree       */
   /* for strings that begin with character i.  These are initialized */
   /* to NIL.  Note there are 256 trees.                              */

   for (i = N + 1; i <= N + 256; i++)
      rson[i] = NIL;
   for (i = 0; i < N; i++)
      dad[i] = NIL;
}

void InsertNode(int r,UCHAR text_buf[],int *match_position, int *match_length,
int lson[],int rson[],int dad[] )
/* Inserts string of length F, text_buf[r..r+F-1], into one of the     */
/* trees (text_buf[r]'th tree) and returns the longest-match position  */
/* and length via the global variables match_position and match_length.*/
/* If match_length = F, then removes the old node in favor of the new  */
/* one, because the old one will be deleted sooner.                    */
/* Note r plays double role, as tree node and position in buffer.      */
{
   int  i, p, cmp;
   unsigned char  *key;

   cmp = 1;
   key = &text_buf[r];
   p = N + 1 + key[0];
   rson[r] = lson[r] = NIL;
   *match_length = 0;
   for ( ; ; )
   {
      if (cmp >= 0)
      {
         if (rson[p] != NIL) p = rson[p];
         else
         {
            rson[p] = r;
            dad[r] = p;
            return;
         }
      }
      else
      {
         if (lson[p] != NIL) p = lson[p];
         else
         {
            lson[p] = r;
            dad[r] = p;
            return;
         }
      }
      for (i = 1; i < F; i++)
         if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
      if (i > *match_length)
      {
         *match_position = p;
         if ((*match_length = i) >= F)  break;
      }
   }
   dad[r] = dad[p];
   lson[r] = lson[p];
   rson[r] = rson[p];
   dad[lson[p]] = r;
   dad[rson[p]] = r;
   if (rson[dad[p]] == p)
      rson[dad[p]] = r;
   else
      lson[dad[p]] = r;
   dad[p] = NIL;  /* remove p */
}

/* deletes node p from tree */
void DeleteNode(int p,
int lson[],int rson[],int dad[] )
{
   int  q;

   if (dad[p] == NIL)
      return;  /* not in tree */
   if (rson[p] == NIL)
      q = lson[p];
   else
      if (lson[p] == NIL)
         q = rson[p];
      else
      {
         q = lson[p];
         if (rson[q] != NIL)
         {
            do
            {
               q = rson[q];
            }
            while (rson[q] != NIL);
            rson[dad[q]] = lson[q];
            dad[lson[q]] = dad[q];
            lson[q] = lson[p];
            dad[lson[p]] = q;
         }
         rson[q] = rson[p];
         dad[rson[p]] = q;
      }
   dad[q] = dad[p];
   if (rson[dad[p]] == p)
      rson[dad[p]] = q;
   else
      lson[dad[p]] = q;
   dad[p] = NIL;
}

int Encode(FILE *infile,FILE *outfile,unsigned long *ulCRC,unsigned long *ulCompFileSize)
{
   int  i, c, len, r, s, last_match_length, code_buf_ptr;
   unsigned char  code_buf[17], mask;
   /* textsize - input file byte counter */
   /* codesize - compressed file byte counter  */
   unsigned long int textsize = 0,codesize = 0;
   /* inputsize - size of input file    */
   unsigned long int inputsize = 0;
#ifdef FOR_EXE
   /* counter for reporting progress every 1K bytes */
   unsigned long int printcount = 0;
#endif

   /* of longest match.  These are      */
   /*set by the InsertNode() procedure. */
   int     match_position, match_length;
   /* left & right children &                          */
   /* parents -- These constitute binary search trees. */
   int     *lson, *rson, *dad;
   /* ring buffer of size N, */
   /* with extra F-1 bytes to facilitate string comparison */
   unsigned char *text_buf;

   inputsize = filelength(fileno(infile));
   *ulCRC=0xffffffff;
   lson = (int *)calloc( (N + 1),sizeof(int));
   rson = (int *)calloc( (N + 257),sizeof(int));
   dad = (int *)calloc( (N + 1),sizeof(int));
   text_buf = (char *)calloc( (N + F - 1),sizeof(char));

   if(text_buf == NULL || lson == NULL || rson == NULL || dad == NULL)
      return(RETURN_MEM_ERROR);


   /* initialize trees */
   InitTree(rson,dad);
   /* code_buf[1..16] saves eight units of code, and                  */
   /* code_buf[0] works as eight flags, "1" representing that the unit*/
   /* is an unencoded letter (1 byte), "0" a position-and-length pair*/
   /* (2 bytes).  Thus, eight units require at most 16 bytes of code. */
   code_buf[0] = 0;
   code_buf_ptr = mask = 1;
   s = 0;
   r = N - F;
   /* Clear the buffer with */
   /* any character that will appear often. */
   for (i = s; i < r; i++) text_buf[i] = ' ';
   for (len = 0; len < F && (c = getc(infile)) != EOF; len++)
   {
      /* Read F bytes into the last F bytes of the buffer */
      text_buf[r + len] = (unsigned char) c;
      UpdateCRC32(c,ulCRC);
   }
   /* text of size zero */
   if ((textsize = len) == 0) return(RETURN_ZERO_LEN_FILE);
   for (i = 1; i <= F; i++) InsertNode(r - i,text_buf,&match_position,&match_length,
   lson,rson,dad );
   InsertNode(r,text_buf,&match_position,&match_length,
              lson,rson,dad );
   do
   {
      /* match_length may be spuriously long near the end of text. */
      if (match_length > len) match_length = len;
      if (match_length <= THRESHOLD)
      {
         /* Not long enough match.  Send one byte. */
         match_length = 1;
         /* 'send one byte' flag */
         code_buf[0] |= mask;
         /* Send uncoded. */
         code_buf[code_buf_ptr++] = text_buf[r];
      }
      else
      {
         code_buf[code_buf_ptr++] = (unsigned char) match_position;
         code_buf[code_buf_ptr++] = (unsigned char)
            (((match_position >> 4) & 0xf0)
                | (match_length - (THRESHOLD + 1)));
                /* Send position and */
                /* length pair. Note match_length > THRESHOLD. */
      }
      /* Shift mask left one bit. */
      if ((mask <<= 1) == 0)
      {
         /* Send at most 8 units of */
         /* code together */
         for (i = 0; i < code_buf_ptr; i++)
            if(putc(code_buf[i], outfile)==EOF)
            {
               free(lson);
               free(rson);
               free(dad);
               free(text_buf);
               return(RETURN_SPACE_ERROR);
            }
         codesize += code_buf_ptr;
         /* if the compressed file is larger than input file */
         if(codesize > inputsize)
         {
            free(lson);
            free(rson);
            free(dad);
            free(text_buf);
            return(RETURN_SIZE_ERROR);
         }
         code_buf[0] = 0;
         code_buf_ptr = mask = 1;
      }
      last_match_length = match_length;
      for (i = 0; i < last_match_length &&
      (c = getc(infile)) != EOF; i++)
      {
         /* Delete old strings and */
          DeleteNode(s,lson,rson,dad );
         /* read new bytes */
         UpdateCRC32(c,ulCRC);
         text_buf[s] = (unsigned char) c;
         /* If the position is                               */
         /* near the end of buffer, extend the buffer to make */
         /* string comparison easier. */
         if (s < F - 1) text_buf[s + N] = (unsigned char) c;
         /* Since this is a ring buffer, increment the position modulo N. */
         s = (s + 1) & (N - 1);
         r = (r + 1) & (N - 1);
         /* Register the string in text_buf[r..r+F-1] */
         InsertNode(r,text_buf,&match_position,&match_length,
                    lson,rson,dad );
      }
#ifdef FOR_EXE
      /* Reports progress each time the textsize exceeds */
      /* multiples of 1024. */
      if ((textsize += i) > printcount)
      {
         printf("%12ld\r", textsize);
         printcount += 1024;
      }
#endif
      while (i++ < last_match_length)
      {
         /* After the end of text, */
         DeleteNode(s,
         /* no need to read, but */
         lson,rson,dad );
         s = (s + 1) & (N - 1);
         r = (r + 1) & (N - 1);
         if (--len) InsertNode(r,text_buf,&match_position,&match_length,
         lson,rson,dad );
      }
   }
   /* until length of string to be processed is zero */
   while (len > 0);
   /* Send remaining code. */
   if (code_buf_ptr > 1)
   {
      for (i = 0; i < code_buf_ptr; i++)
          if(putc(code_buf[i], outfile)==EOF)
          {
             free(lson);
             free(rson);
             free(dad);
             free(text_buf);
             return(RETURN_SPACE_ERROR);
          }
      codesize += code_buf_ptr;
      /* if the compressed file is larger than input file */
      if(codesize > inputsize)
      {
         free(lson);
         free(rson);
         free(dad);
         free(text_buf);
         return(RETURN_SIZE_ERROR);
      }
   }
   *ulCRC^=0xffffffff;
#ifdef FOR_EXE
   /* Encoding is done. */
   printf("In : %ld bytes\n", textsize);
   printf("Out: %ld bytes\n", codesize);
   printf("Out/In: %.3f\n", (double)codesize / textsize);
   printf("Percent: %.3f\n", (double)(textsize - codesize) / textsize);
   printf("CRC: %10lx", *ulCRC);
#endif
   free(lson);
   free(rson);
   free(dad);
   free(text_buf);
   return(RETURN_OK);
}

/* Just the reverse of Encode(). */
int Decode(FILE *infile, FILE *outfile,unsigned long *ulCRC)
{
   int  i, j, k, r, c;
   unsigned int  flags;
   unsigned char *text_buf;
   /*ring buffer of size N, */
   /* with extra F-1 bytes to facilitate string comparison */

   *ulCRC=0xffffffff;
   text_buf = (char *)calloc( (N+F-1) ,sizeof(char));
   if(text_buf == NULL)
      return(RETURN_MEM_ERROR);

   for (i = 0; i < N - F; i++) text_buf[i] = ' ';
   r = N - F;
   flags = 0;
   for ( ; ; )
   {
      if (((flags >>= 1) & 256) == 0)
      {
         if ((c = getc(infile)) == EOF) break;
         flags = c | 0xff00;
         /* uses higher byte cleverly */
         /* to count eight */
      }
      if (flags & 1)
      {
         if ((c = getc(infile)) == EOF) break;
         if (putc(c, outfile)==EOF)
         {
            free(text_buf);
            return(RETURN_SPACE_ERROR);
         }
         UpdateCRC32(c,ulCRC);
         text_buf[r++] = (unsigned char) c;
         r &= (N - 1);
      }
      else
      {
         if ((i = getc(infile)) == EOF) break;
         if ((j = getc(infile)) == EOF) break;
         i |= ((j & 0xf0) << 4);
         j = (j & 0x0f) + THRESHOLD;
         for (k = 0; k <= j; k++)
         {
            c = text_buf[(i + k) & (N - 1)];
            if (putc(c, outfile)==EOF)
            {
               free(text_buf);
               return(RETURN_SPACE_ERROR);
            }
            UpdateCRC32(c,ulCRC);
            text_buf[r++] = (unsigned char) c;
            r &= (N - 1);
         }
      }
   }
   *ulCRC^=0xffffffff;
   free(text_buf);

   return(RETURN_OK);
}

