#include "font.c"
#include "frog.c"
// #include "text.c"
// #include "text.h"
typedef unsigned short u16;
#define MAX_WIDTH 240
#define MAX_HEIGHT 160


#define SCANLINECOUNTER  (*(volatile unsigned short *)0x4000006)
#define REG_DISPCTL *(unsigned short *)0x4000000
#define MODE3 3
#define BG2_ENABLE (1<<10)
#define COLOR(r, g, b) ((r) | (g)<<5 | (b)<<10)
#define RED COLOR(31,0,0)
#define GREEN COLOR(0,31,0)
#define BLUE COLOR(0,0,31)
#define CYAN COLOR(0,31,31)
#define MAGENTA COLOR(31, 0,31)
#define YELLOW COLOR(31,31,0)
#define WHITE COLOR(31,31,31)
#define BLACK 0

#define OFFSET(r, c, numcols) ((r)*(numcols)+(c))

// Buttons
#define BUTTON_A		(1<<0)
#define BUTTON_B		(1<<1)
#define BUTTON_SELECT	(1<<2)
#define BUTTON_START	(1<<3)
#define BUTTON_RIGHT	(1<<4)
#define BUTTON_LEFT		(1<<5)
#define BUTTON_UP		(1<<6)
#define BUTTON_DOWN		(1<<7)
#define BUTTON_R		(1<<8)
#define BUTTON_L		(1<<9)

#define KEY_DOWN_NOW(key)  (~(BUTTONS) & key)
#define BUTTONS *(volatile unsigned int *)0x4000130

char gameMode = 0;
const short frogWidth = 10;
const short frogHeight = 10;
const short logWidth = 20;
const short logHeight = 6;
const short lilyWidth = 10;
const short lilyHeight = 10;

extern const unsigned char fontdata_6x8[12288];
unsigned short *videoBuffer = (unsigned short *)0x6000000;

// Prototype
void setPixel(int row, int col, u16 color);
void drawRect(int row, int col, int height, int width, u16 color);
void delay(int n);
int boundsCheck(int *var, int bound, int *delta, int size);
void WaitForVblank();
void drawChar(int row, int col, char ch, u16 color);
void drawString(int row, int col, char *s, u16 color);
void drawStartGameString();
void drawFrog(int r, int c, int width, int height);
void drawLog(int r, int c, int width, int height);
void drawLily(int r, int c, int width, int height);


struct frog{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;
};

struct log{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;	
};

struct lily{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;	
};

int main()
{
	struct frog frog1;
	frog1.height = frogHeight;
	frog1.width = frogWidth;
	frog1.col = 125;
	frog1.row = 150;
	REG_DISPCTL = MODE3 | BG2_ENABLE;
	int row = 150;
	int col = 125;
	int oldrow = row;
	int oldcol = col;
	int rd = 1;
	int cd= 1;
	int size = 10;
	int oldsize = size;
	int score = 0;
	while(1) // Game Loop
	{
		if(gameMode==0){
			drawStartGameString();
		}
		else if (gameMode==2){//actual game starts
			// drawEndGameThing();
			// drawString(20, 20, "mah niga", BLUE);

		}
		else{ //gameMode = 1
			if(KEY_DOWN_NOW(BUTTON_UP))
			{
				row--;
			}
			if(KEY_DOWN_NOW(BUTTON_DOWN))
			{
				row++;
			}
			if(KEY_DOWN_NOW(BUTTON_LEFT))
			{
				col--;
			}
			if(KEY_DOWN_NOW(BUTTON_RIGHT))
			{
				col++;
			}			
			if(boundsCheck(&row, 159, &rd, size))
			{
				score++;
			}
			boundsCheck(&col, 239, &cd, size);
			WaitForVblank();
			drawRect(oldrow, oldcol, size, size, BLACK);			
			drawFrog(row, col, size, size);
			oldrow = row;
			oldcol = col;
			oldsize = size;
		}	
	}
}

void setPixel(int row, int col, u16 color)
{
	videoBuffer[OFFSET(row, col, 240)] = color;
}

void drawRect(int row, int col, int height, int width, unsigned short color)
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
// 		if(*var < 0)
// 		{
// 			*var = 0;
// 			*delta = -*delta;
// 			return -1;
// 		}
// 		if(*var > bound-size+1)
// 		{
// 			*var = bound-size+1;
// 			*delta = -*delta;
// 		}
		return 0;
}

void WaitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

void drawStartGameString(){
	if(KEY_DOWN_NOW(BUTTON_START)){
		gameMode = 1;
		//make entire screen black to start off(remove text/button)
		drawRect(0,0,160,240,BLACK);
	}else{
		drawString(20, 20, "GAME OF FROGGER. CLICK TO PLAY", RED);		
		drawString(MAX_HEIGHT/2+5, MAX_WIDTH/2-40, "PUSH START", RED);
	}
}

void drawChar(int row, int col, char ch, u16 color)
{
	int r,c;
	for(r=0; r<8; r++)
	{
		for(c=0; c<6; c++)
		{
			if(fontdata_6x8[OFFSET(r, c, 6) + ch*48])
			{
				setPixel(r+row, c+col, color);
			}
		}
	}
}

void drawString(int row, int col, char *s, u16 color)
{
	while(*s)
	{
		drawChar(row, col, *s++, color);
		col += 6;
	}
}

void drawFrog(int row, int col, int width, int height)
{
	int index = 0;
	int r,c;
	for(r=0; r<width; r++)
	{
		for(c=0; c<height; c++)
		{
			setPixel(r+row, c+col, frog[index]);
			index+=1;
		}
	}
}

void drawLog(int row, int col, int width, int height)
{
	int index = 0;
	int r,c;
	for(r=0; r<width; r++)
	{
		for(c=0; c<height; c++)
		{
			setPixel(r+row, c+col, frog[index]);
			index+=1;
		}
	}
}

void drawLily(int row, int col, int width, int height)
{
	int index = 0;
	int r,c;
	for(r=0; r<width; r++)
	{
		for(c=0; c<height; c++)
		{
			setPixel(r+row, c+col, frog[index]);
			index+=1;
		}
	}
}