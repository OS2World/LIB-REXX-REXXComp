
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <io.h>
#include "lzss.h"

int main(int argc, char *argv[])
{
  char         *s;
  FILE         *infile, *outfile;    /* input & output files */
  char         *in_buffer = NULL;    /* buffer for file I/O operations */
  char         *out_buffer = NULL;   /* buffer for file I/O operations */
  char          bTest='n';
  unsigned long ulCRC=0;
  unsigned long ulCompFileSize;
  unsigned long ulTestCRC=0;

   if (argc < 3)
   {
      printf("'lzss e file1 file2' encodes file1 into file2.\n"
          "'lzss d file2 file1' decodes file2 into file1.\n"
          "'lzss t file2' tests the integrity of file2.\n");
      return EXIT_FAILURE;
   }
   if (s = argv[1], s[1] || strpbrk(s, "TtDEde") == NULL)
   {
      printf("??? %s\n", s);
      return EXIT_FAILURE;
   }
   if (s = argv[2], (infile  = fopen(s, "rb")) == NULL)
   {
      printf("??? %s\n", s);
      return EXIT_FAILURE;
   }
   if (toupper(*argv[1]) != 'T')
   {
      if (s = argv[3], (outfile = fopen(s, "wb")) == NULL)
      {
         printf("??? %s\n", s);
         return EXIT_FAILURE;
      }
       setvbuf(outfile,out_buffer,_IOFBF,32000);
   }
   else
   {
      /* if test mode then open output stream as nul */
      outfile = fopen("nul", "wb");
      bTest='y';
   }
   setvbuf(infile,in_buffer,_IOFBF,32000);
   setvbuf(stdout,NULL,_IONBF,0);
   if (toupper(*argv[1]) == 'E')
   {
      /* write out a blank crc to the first 4 bytes of outfile */
      fwrite(&ulCRC,sizeof(unsigned long),1,outfile);
      Encode(infile,outfile,&ulCRC,&ulCompFileSize);
      /* write out the real crc to the first 4 bytes of outfile */
      fseek(outfile,0L,SEEK_SET);
      fwrite(&ulCRC,sizeof(unsigned long),1,outfile);
   }
   else
   {
      /* read the crc from the first 4 bytes of infile */
      fread(&ulTestCRC,sizeof(unsigned long),1,infile);
      Decode(infile,outfile,&ulCRC);
      if (ulTestCRC != ulCRC)
      {
         printf("CRC Error!!! \n");
         printf("Old CRC: %10lx \n", ulTestCRC);
         printf("New CRC: %10lx \n ", ulCRC);
      }
      else
         if (bTest == 'y')
               printf("File Ok!!! \n");
   }
   fclose(infile);
   fclose(outfile);
   return EXIT_SUCCESS;
}
