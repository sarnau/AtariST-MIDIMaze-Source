/************************************************************
 *** Misc ASCII character output
 ************************************************************/
#include "GLOBALS.H"


/************************************************************
 *** Print an integer number into the console
 ************************************************************/
void bconout_dec_number(int num) {
    if(num < 0) {
        Bconout(CON, '-');
        num = -num;
    }
    if(num > 9)
        bconout_dec_number(num/10);
    Bconout(CON, num%10+'0');
}

/************************************************************
 *** Print a string into the console
 ************************************************************/
void bconout_string(const char *str) {
    while(*str != 0) {
        Bconout(CON, *str++);
    }
}
