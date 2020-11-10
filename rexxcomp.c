#define  INCL_REXXSAA
#define  _DLL
#define  _MT
#include <os2.h>
#include <rexxsaa.h>
#include <stdio.h>
#include <string.h>
#include "rexxcomp.h"

/*************************************************************************
* Function:  Compress                                                    *
*                                                                        *
* Syntax:    call Compress infilename outfilename [,mode]                *
*                                                                        *
* Params:    infilename  - name of file to be compressed                 *
*            outfilename - name of compressed file
*            mode        - 'F' - fail if output file exists
                         - default is overwrite
*                                                                        *
* Return:    NO_ERROR
*            SPACE_ERROR
*            MEM_ERROR
*            ZERO_LEN_FILE
*************************************************************************/

ULONG Compress(CHAR *name, ULONG numargs, RXSTRING args[],
                       CHAR *queuename, RXSTRING *retstr)
{

  PSZ  pszInfile;
  PSZ  pszOutfile;
  PSZ  opts;
  BOOL bFail=TRUE;
  FILE *infile;
  FILE *outfile;
  unsigned long ulCRC=0;
  unsigned long ulCompFileSize;
  unsigned long ulTestCRC=0;
  int rc;

  if (numargs < 2 || numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;            /* raise an error             */

  if (numargs == 3 && !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;            /* raise an error             */

  pszInfile = args[0].strptr;             /* get infile name            */
  pszOutfile = args[1].strptr;           /* get outfile name           */

  if (numargs == 3)
  {                  /* process options            */
    opts = args[3].strptr;             /* point to the option        */
    if (strcmpi(opts, "F")==0)
      bFail = TRUE;
  }

  /* now open the files */
   if ((infile  = fopen(pszInfile, "rb")) == NULL)
   {
      BUILDRXSTRING(retstr, INPUT_FILE_OPEN_ERROR);
      return VALID_ROUTINE;
   }

  /* if fail if exists is specified then try r+w mode  */
  /* it will fail if the file does not exist           */
  if (bFail)
  {
     outfile = fopen(pszOutfile, "rb+");
     /* if the file is not found then create it */
     if (outfile)
     {
        /* the file exists so we don't want to overwrite it */
        fclose(outfile);
        BUILDRXSTRING(retstr, OUTPUT_FILE_OPEN_ERROR);
        return VALID_ROUTINE;
     }
  }
  outfile = fopen(pszOutfile, "wb");
  /* if the file cannot be opened then return error */
  if (!outfile)
  {
     BUILDRXSTRING(retstr, OUTPUT_FILE_OPEN_ERROR);
     return VALID_ROUTINE;
  }

  /* now call the compression routine                      */

  /* write out a blank crc to the first 4 bytes of outfile */
  fwrite(&ulCRC,sizeof(unsigned long),1,outfile);
  /* compress the file                                     */
  rc = Encode(infile,outfile,&ulCRC,&ulCompFileSize);

  /* check the return code          */
  switch (rc)
  {
     case  RETURN_OK :
        /* write out the real crc to the first 4 bytes of outfile */
        fseek(outfile,0L,SEEK_SET);
        fwrite(&ulCRC,sizeof(unsigned long),1,outfile);
        BUILDRXSTRING(retstr, NO_ERROR);
     break;

     case  RETURN_SPACE_ERROR :
        BUILDRXSTRING(retstr, SPACE_ERROR);
     break;

     case  RETURN_MEM_ERROR :
        BUILDRXSTRING(retstr, MEM_ERROR);
     break;

     case  RETURN_ZERO_LEN_FILE :
        BUILDRXSTRING(retstr, ZERO_LEN_FILE);
     break;

     case  RETURN_SIZE_ERROR :
        BUILDRXSTRING(retstr, SIZE_ERROR);
     break;

     default:
        BUILDRXSTRING(retstr, UNKNOWN_ERROR);
     break;
  }

  fclose(infile);
  fclose(outfile);
  /* delete output file if someone went wrong */
  if (rc)
     remove(outfile);
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  DeCompress                                                    *
*                                                                        *
* Syntax:    call DeCompress infilename outfilename [,mode]                *
*                                                                        *
* Params:    infilename  - name of the compressed file
*            outfilename - name of decompressed file
*            mode        - 'F' - fail if output file exists
                         - default is overwrite
*                                                                        *
* Return:    NO_ERROR
*            SPACE_ERROR
*            MEM_ERROR
*            CRC_ERROR
*
*----------------------------------------------------------------------
*
* Function:  TestCompressedFile                                                    *
*
* Syntax:    call TestCompressedFile infilename
*
* Params:    infilename  - name of the compressed file
*
* Return:    NO_ERROR
*            SPACE_ERROR  - shouldn't happen since outfile is nul!
*            MEM_ERROR
*            CRC_ERROR
*----------------------------------------------------------------------
*  Note:
*        This function is the entry point for both REXX procedures
*        DeCompress and TestCompressedFile.   The name
*        of the called function is checked and if it
*        it is TestCompressedFile then nul is used for
*        outputfile
*************************************************************************/

ULONG DeCompress(CHAR *name, ULONG numargs, RXSTRING args[],
                       CHAR *queuename, RXSTRING *retstr)
{

   PSZ  pszInfile;
   PSZ  pszOutfile;
   PSZ  opts;
   BOOL bFail=FALSE;
   FILE *infile;
   FILE *outfile;
   unsigned long ulCRC=0;
   unsigned long ulCompFileSize;
   unsigned long ulTestCRC=0;
   int rc;

   /* if called routine is DeCompress then handle the output */
   /* file and the options                                   */
   if (!(strcmpi(name,"DeCompress")))
   {
      if (numargs < 2 || numargs > 3 ||
          !RXVALIDSTRING(args[0]) ||
          !RXVALIDSTRING(args[1]))
        return INVALID_ROUTINE;            /* raise an error             */

      if (numargs == 3 && !RXVALIDSTRING(args[2]))
        return INVALID_ROUTINE;            /* raise an error             */

      pszOutfile = args[1].strptr;           /* get outfile name           */

      if (numargs == 3)
      {                  /* process options            */
        opts = args[3].strptr;             /* point to the option        */
        if (strcmpi(opts, "F")==0)
          bFail = TRUE;   /* fail if file exists */
      }
   }
   else
   {
      /* if called routine is TestCompressedFile then  */
      /*  output file is nul */
      if (numargs !=1 || !RXVALIDSTRING(args[0]))
         return INVALID_ROUTINE;            /* raise an error         */
      pszOutfile = "nul";
   }


   pszInfile = args[0].strptr;             /* get infile name            */
   /* now open the files */
   if ((infile  = fopen(pszInfile, "rb")) == NULL)
   {
      BUILDRXSTRING(retstr, INPUT_FILE_OPEN_ERROR);
      return VALID_ROUTINE;
   }

   /* if fail if exists is specified return error       */
   /* file already exists                               */
   if (bFail)
   {
      if (-1 != _access(pszOutfile,00))
      {
         fclose(infile);
         BUILDRXSTRING(retstr, OUTPUT_FILE_OPEN_ERROR);
         return VALID_ROUTINE;
      }
   }
   outfile = fopen(pszOutfile, "wb");
   /* if the file cannot be opened then return error */
   if (!outfile)
   {
      fclose(infile);
      BUILDRXSTRING(retstr, OUTPUT_FILE_OPEN_ERROR);
      return VALID_ROUTINE;
   }

   /* now call the decompression routine                      */

   /* read the crc from the first 4 bytes of infile */
   fread(&ulTestCRC,sizeof(unsigned long),1,infile);
   /* decompress the file                                     */
   rc = Decode(infile,outfile,&ulCRC);

   /* check the return code          */
   switch (rc)
   {
      case  RETURN_OK :
         /* compare the CRC's                        */
         if (ulTestCRC != ulCRC)
            BUILDRXSTRING(retstr, CRC_ERROR)
         else
            BUILDRXSTRING(retstr, NO_ERROR);
      break;

      case  RETURN_SPACE_ERROR :
         BUILDRXSTRING(retstr, SPACE_ERROR);
      break;

      case  RETURN_MEM_ERROR :
         BUILDRXSTRING(retstr, MEM_ERROR);
       break;

      default:
         BUILDRXSTRING(retstr, UNKNOWN_ERROR);
      break;
   }

   fclose(infile);
   fclose(outfile);
   return VALID_ROUTINE;
}

/*************************************************************************
* Function:  LoadCompressionFuncs                                        *
*                                                                        *
* Syntax:    call LoadCompressionFuncs                                   *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/
ULONG LoadCompressionFuncs(CHAR *name, ULONG numargs, RXSTRING args[],
                           CHAR *queuename, RXSTRING *retstr)
{
   INT    entries;                      /* Num of entries             */
   INT    j;                            /* Counter                    */

   retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
   if (numargs > 0)
      return INVALID_ROUTINE;

   entries = sizeof(RxFncTable)/sizeof(PSZ);

   /* register the functions that are named the same */
   /* as their entry point */
   for (j = 0; j < entries; j++)
   {
      RexxRegisterFunctionDll(RxFncTable[j],
                              "REXXCOMP",
                               RxFncTable[j]);
   }
   /* now register TestCompressedFile that uses the entry point */
   /* DeCompress                                                */
   RexxRegisterFunctionDll("TestCompressedFile",
                           "REXXCOMP",
                           "DeCompress");
   return VALID_ROUTINE;
}


/*************************************************************************
* Function:  UnLoadCompressionFuncs                                        *
*                                                                        *
* Syntax:    call UnLoadCompressionFuncs                                   *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

ULONG UnLoadCompressionFuncs(CHAR *name, ULONG numargs, RXSTRING args[],
                          CHAR *queuename, RXSTRING *retstr)
{
   INT     entries;                     /* Num of entries             */
   INT     j;                           /* Counter                    */

   if (numargs != 0)                    /* no arguments for this      */
      return INVALID_ROUTINE;            /* raise an error             */

   retstr->strlength = 0;               /* return a null string result*/

   entries = sizeof(RxFncTable)/sizeof(PSZ);

   for (j = 0; j < entries; j++)
      RexxDeregisterFunction(RxFncTable[j]);

   /* now deregister TestCommpressedFile */
   RexxDeregisterFunction("TestCompressedFile");

   return VALID_ROUTINE;                /* no error on call           */
}

