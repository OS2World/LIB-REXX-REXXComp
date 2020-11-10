
/* REXX return code errors */
#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

/* string passed back to REXX */
#define NO_ERROR                 "0"
#define INPUT_FILE_OPEN_ERROR    "1"
#define OUTPUT_FILE_OPEN_ERROR   "2"
#define FILE_OPEN_ERROR          "3"
#define SPACE_ERROR              "4"
#define MEM_ERROR                "5"
#define ZERO_LEN_FILE            "6"
#define CRC_ERROR                "7"
#define SIZE_ERROR               "8"
#define UNKNOWN_ERROR            "99"

/* return codes from the compression routine */
#define RETURN_OK              0
#define RETURN_SPACE_ERROR     1
#define RETURN_MEM_ERROR       2
#define RETURN_ZERO_LEN_FILE   3
#define RETURN_SIZE_ERROR      4

#define BUILDRXSTRING(t, s) { \
  strcpy((t)->strptr,(s));\
  (t)->strlength = strlen((s)); \
}

static PSZ  RxFncTable[] =
   {
      "Compress",
      "DeCompress",
      "LoadCompressionFuncs",
      "UnLoadCompressionFuncs"
   };
RexxFunctionHandler Compress;
RexxFunctionHandler DeCompress;
RexxFunctionHandler LoadCompressionFuncs;
RexxFunctionHandler UnLoadCompressionFuncs;
