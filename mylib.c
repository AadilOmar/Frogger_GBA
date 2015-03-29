// #include "mylib.h"

unsigned short *videoBuffer = (unsigned short *)0x6000000;


void setPixel(int row, int col, u16 color)
{
	videoBuffer[OFFSET(row, col, 240)] = color;
}

void drawRect(int row, int col, int height, int width, u16 color)
{
	for(int r=0; r<height; r++)
	{
		for(int c=0; c<width; c++)
		{
			setPixel(row+r, col+c, color);
		}
	}
}


int boundsCheck(int *var, int bound, int *delta, int size)
{
		if(*var < 0)
		{
			*var = 0;
			*delta = -*delta;
			return 1;
		}
		if(*var > bound-size+1)
		{
			*var = bound-size+1;
			*delta = -*delta;
		}
		return 0;

}

void WaitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

