typedef short int16_t;

extern int16_t contrl[];
extern int16_t intin[];
extern int16_t ptsin[];
extern int16_t intout[];
extern int16_t ptsout[];
extern VDIPB pblock;

#define i_ptsin(ptr) pblock.ptsin = ptr
#define i_intin(ptr) pblock.intin = ptr
#define i_intout(ptr) pblock.intout = ptr
#define i_ptsout(ptr) pblock.ptsout = ptr


void vdi(void);

short vec_len(short dx, short dy);
