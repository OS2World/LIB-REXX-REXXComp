#define N                4096   /* size of ring buffer */
#define F                  18   /* upper limit for match_length */
#define THRESHOLD           2   /* encode string into position and length
                                    if match_length is greater than this */
#define NIL              N       /* index for root of binary search trees */

#define UCHAR unsigned char
#define RETURN_OK             0
#define RETURN_SPACE_ERROR    1
#define RETURN_MEM_ERROR      2
#define RETURN_ZERO_LEN_FILE  3
#define RETURN_SIZE_ERROR     4

#ifdef USE_MAIN
int main(int argc, char *argv[]);
#endif

void InitTree(int rson[],int dad[]);
void InsertNode(int r,UCHAR text_buf[],int *match_position, int *match_length,
                int lson[],int rson[],int dad[] );
void DeleteNode(int p,int  lson[],int rson[],int dad[] );
int  Encode(FILE *infile,FILE *outfile,unsigned long *ulCRC,unsigned long *ulCompFileSize);
int  Decode(FILE *infile, FILE *outfile,unsigned long *ulCRC);
void upDateCRC32(UCHAR c,unsigned long *ulCRC);
