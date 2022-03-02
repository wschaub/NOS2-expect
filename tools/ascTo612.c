/*
 * asc2612.c - convert Ascii to 6 in 12 bit CDC Display code.
 *
 * Copyright 1998,1999,2000,2003 - G. J. van der Grinten
 * All rights reserved by G. J. van der Grinten
 *                        Pater L.A. Bleysstraat 7
 *                        5684 TR Best
 *                        Netherlands
 *
 * INPUT - an ascii file
 * OUTPUT - CDC display code file
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

unsigned char ibuf[1024];

char line[15];
int nibble  = 0;
int state   = 0;
int addFill = 0;
int charPosition = 0;

FILE *outputFile;
char outputLine[120];
int next;

void dumplin( char *line, int size);
void dumper( char *memory, int size);
void storeChar(char chr);
void clearLine();
void putChar(char chr);
void endLine();

/* display to ascii table */
char dis2asc[] = {
    ':','A','B','C','D','E','F','G',
    'H','I','J','K','L','M','N','O',
    'P','Q','R','S','T','U','V','W',
    'X','Y','Z','0','1','2','3','4',
    '5','6','7','8','9','+','-','*',
    '/','(',')','$','=',' ',',','.',
    '#','[',']','%','"','_','!','&',
    '\'','?','<','>','@','\\','^',';'
   };

/* ascii to display table:
   00-077 - display character
   01XX   - 74 XX
   02XX   - 76 XX
   0300   -  end of line processing
   0377   - ignore character
 */

int asc2dis6in12_63[] = {

/* 00   NUL  SOH   STX   ETX   EOT   ENQ   ACK   BEL */
      0240, 0241, 0242, 0243, 0244, 0245, 0246, 0247,

/* 08   BS   HT    LF    VT    FF    CR    SO    SI  */
      0250, 0251, 0252, 0253, 0254, 0255, 0256, 0257,

/* 10   DLE  DC1   DC2   DC3   DC4   NAK   SYN   ETB */
      0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,

/* 18   CAN  EM    SUB   ESC   FS    GS    RS    US  */
      0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,

/* 20   BLANK !     "     #     $     %     &     '  */
       055,  066,  064,  060,  053,  063,  067,  070,

/* 28   (     )     *     +     ,     -     .     /  */
       051,  052,  047,  045,  056,  046,  057,  050,

/* 30   0     1     2     3     4     5     6     7  */
       033,  034,  035,  036,  037,  040,  041,  042,

/* 38   8     9     :     ;     <     =     >     ?  */
       043,  044, 0104,  077,  072,  054,  073,  071,

/* 40   @     A     B     C     D     E     F     G  */
      0101,  001,  002,  003,  004,  005,  006,  007,

/* 48   H     I     J     K     L     M     N     O  */
       010,  011,  012,  013,  014,  015,  016,  017,

/* 50   P     Q     R     S     T     U     V     W  */
       020,  021,  022,  023,  024,  025,  026,  027,

/* 58   X     Y     Z     [     \     ]     ^     _  */
       030,  031,  032,  061,  075,  062, 0104,  065,

/* 60   `     a     b     c     d     e     f     g  */
      0107, 0201, 0202, 0203, 0204, 0205, 0206, 0207,

/* 68   h     i     j     k     l     m     n     o  */
      0210, 0211, 0212, 0213, 0214, 0215, 0216, 0217,

/* 70   p     q     r     s     t     u     v     w  */
      0220, 0221, 0222, 0223, 0224, 0225, 0226, 0227,

/* 78   x     y     z     {     |     }     ~    DEL */
      0230, 0231, 0232, 0233, 0234, 0235, 0236, 0237,
};

int asc2dis6in12_64[] = {

/* 00   NUL  SOH   STX   ETX   EOT   ENQ   ACK   BEL */
      0240, 0241, 0242, 0243, 0244, 0245, 0246, 0247,

/* 08   BS   HT    LF    VT    FF    CR    SO    SI  */
      0250, 0251, 0252, 0253, 0254, 0255, 0256, 0257,

/* 10   DLE  DC1   DC2   DC3   DC4   NAK   SYN   ETB */
      0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,

/* 18   CAN  EM    SUB   ESC   FS    GS    RS    US  */
      0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,

/* 20   BLANK !     "     #     $     %     &     '  */
       055,  066,  064,  060,  053,  063,  067,  070,

/* 28   (     )     *     +     ,     -     .     /  */
       051,  052,  047,  045,  056,  046,  057,  050,

/* 30   0     1     2     3     4     5     6     7  */
       033,  034,  035,  036,  037,  040,  041,  042,

/* 38   8     9     :     ;     <     =     >     ?  */
       043,  044, 0104,  077,  072,  054,  073,  071,

/* 40   @     A     B     C     D     E     F     G  */
      0101,  001,  002,  003,  004,  005,  006,  007,

/* 48   H     I     J     K     L     M     N     O  */
       010,  011,  012,  013,  014,  015,  016,  017,

/* 50   P     Q     R     S     T     U     V     W  */
       020,  021,  022,  023,  024,  025,  026,  027,

/* 58   X     Y     Z     [     \     ]     ^     _  */
       030,  031,  032,  061,  075,  062, 0104,  065,

/* 60   `     a     b     c     d     e     f     g  */
      0107, 0201, 0202, 0203, 0204, 0205, 0206, 0207,

/* 68   h     i     j     k     l     m     n     o  */
      0210, 0211, 0212, 0213, 0214, 0215, 0216, 0217,

/* 70   p     q     r     s     t     u     v     w  */
      0220, 0221, 0222, 0223, 0224, 0225, 0226, 0227,

/* 78   x     y     z     {     |     }     ~    DEL */
      0230, 0231, 0232, 0233, 0234, 0235, 0236, 0237,
};

void dumper( char *memory, int size)
{
    long addr;
    char *sixpack;
    char *inbuf;
    int i,j,k;

    i = size * 4 / 3 + 3;
    sixpack = (char *)malloc(i);
    inbuf = memory;
    addr = 0L;
    i = size > 39 ? 40 : size;
    k = 0;
    for( j = 0 ; j < i ; j++) {
        switch( k & 3) {
            case 0 :
                 sixpack[k] = (inbuf[j] >> 2) & 0x3f;
                 k++;
                 sixpack[k] = (inbuf[j] & 0x03)  << 4;
                 break;
            case 1 :
                 sixpack[k] = ((inbuf[j] >> 4) & 0x0f) | sixpack[k];
                 k++;
                 sixpack[k] = (inbuf[j] << 2) & 0x3c;
                 break;
            case 2 :
                 sixpack[k] = ((inbuf[j] >> 6) & 0x03) | sixpack[k];
                 k++;
                 sixpack[k] = inbuf[j] & 0x3f;
                 k++;
                 break;
            case 3 : fprintf(stderr," Illegal state %d %d %d\n",i,j,k);

                 break;
        }
    }
    dumplin(sixpack,k);
    free(sixpack);
}


void dumplin( char *line, int size)
{
    int i;

    for( i = 0; i < size; i++) {
        fprintf(stderr,"%c",dis2asc[line[i]]);
#if 0
        outputLine[next++] =dis2asc[line[i]];
#endif
    }
}

void storeChar(char chr)
{
    switch( state & 3) {
        case 0 :
             line[nibble] = chr << 2;
             state++;
             break;
        case 1 :
             line[nibble] = line[nibble] | (chr & 0x030)  >> 4;
             nibble++;
             line[nibble] = (chr & 0x0f) << 4;
             state++;
             break;
        case 2 :
             line[nibble] = line[nibble] | (chr & 0x3c) >> 2;
             nibble++;
             line[nibble] = (chr & 0x03) << 6;
             state++;
             break;
        case 3 :
             line[nibble] = line[nibble] | (chr & 0x3f);
             nibble++;
             state = 0;
             break;
    }
    charPosition++;
}

void clearLine()
{
    memset(line, 0, sizeof line);
    nibble = 0;
    state = 0;
    charPosition = 0;
}

void putChar(char chr)
{
    addFill = (chr == '\0') ? 1 : 0;
    chr &= 077;
    storeChar(chr);
    if (charPosition == 20 ) {
        fwrite(line , 15, 1, outputFile);
        clearLine();
    }
}

void endLine()
{
    int i;

    if ( addFill == 1) {
        putChar(055);  /* add a space to protect the colon */
    }
    switch(charPosition) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            while(charPosition != 10) {
                putChar(0300);
            }
            break;
        case 9:
        case 10:
            while(charPosition != 10) {
                putChar(0300);
            }
            for (i = 0; i < 10; i++)
                putChar(0300);
            break;

        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            while(charPosition != 0) {
                putChar(0300);
            }
            break;
        case 19:
        case 20:
        case 0:
            while(charPosition != 0) {
                putChar(0300);
            }
            for (i = 0; i < 10; i++)
                putChar(0300);
            break;
    }
}

int main(int argc, char **argv)
{
    FILE *inputFile;
    char *p1, *p2, ch;
    int cdc;

    for (nibble = 0 ; nibble < 16; nibble++) {
        line[nibble] = 0;
    }
    nibble = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: ascTo612 ascii_source output_CDC_6_in_12\n");
        exit(1);
        }

    if ((inputFile = fopen(argv[1], "rb")) == NULL) {
        perror("open input file");
        exit(1);
    }

    if ((outputFile = fopen(argv[2], "wb")) == NULL) {
        perror("open output file");
        exit(1);
    }

    while (fgets( ibuf, sizeof(ibuf) -1, inputFile) != NULL) {
        p1 = ibuf;
        while(1) {
            ch = *p1++;
            ch &= 0x7f; /* strip parity */
            if ( ch == 0) { /* end of line */
                endLine();
                break;
            }
            if (ch == '\015' ) continue; /* skip cr */
            if (ch == '\012' ) continue; /* skip lf */
            cdc = asc2dis6in12_64[ch];
            if (cdc > 077) {
                if (cdc > 0177) {
                    putChar(076);
                } else {
                    putChar(074);
                }
                putChar((char)(cdc & 077));
            } else {
                putChar((char)(cdc));
            }
        }
    }
    if ( charPosition != 0) {
        fwrite(line , 8, 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);
    return(0);
}


