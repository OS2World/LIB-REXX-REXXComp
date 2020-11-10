int  main(int argc, char *argv[]);
int  Encode(FILE *infile,FILE *outfile,unsigned long *ulCRC,unsigned long *ulCompFileSize);
int  Decode(FILE *infile, FILE *outfile,unsigned long *ulCRC);
