#include "GLOBALS.H"

int Fopen( const char *filename, int mode )
{
}

int Fclose( int handle )
{
}

long Fread( int handle, long count, void *buf )
{
}

int Dgetdrv( void )
{
}

int Dgetpath( char *path, int driveno )
{
}


int Bconstat( int dev )
{
}

long Bconin( int dev )
{
}

void Bconout( int dev, int c )
{
}

void *Physbase( void )
{
}

void *Logbase( void )
{
}

int     Getrez( void )
{
}

void    Setscreen( void *laddr, void *paddr, int rez )
{
}
	
void    Setpalette( void *pallptr )
{
}

int     Setcolor( int colornum, int color )
{
}
	
void    Vsync( void )
{
}

char    Giaccess( char data, int regno )
{
}

void    *Dosound( void *buf )
{
}

long    Supexec( void (*func)(void) )
{
}

long    Random( void )
{
}

KBDVBASE *Kbdvbase( void )
{
}

void    Initmouse( int type, const char *par, void (*mousevec)(void) )
{
}



