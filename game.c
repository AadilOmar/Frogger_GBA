//TODO
//when logs/lilys go offscreen, their col doesnt reset to 0. Must fix in orderto do collision check


#include <stdio.h>
#include "font.c"
#include "frog.c"
#include "lily.c"
#include "logPic.c"
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
const short lilyWidth = 12;
const short lilyHeight = 12;

extern const unsigned char fontdata_6x8[12288];
unsigned short *videoBuffer = (unsigned short *)0x6000000;

// Prototype
void setPixel(int row, int col, u16 color);
void drawRect(int row, int col, int height, int width, u16 color);
void delay(int n);
void boundsCheck();
void WaitForVblank();
void drawChar(int row, int col, char ch, u16 color);
void drawString(int row, int col, char *s, u16 color);
void drawStartGameString();
void drawFrog();
void drawLog();
void drawLily();
int checkCollision();
int onLilyPad();
void reset();


typedef struct{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;
}Frog;


typedef struct{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;	
}Log;

typedef struct{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;	
}Lily;

int main()
{
	Frog frog1;
	frog1.height = frogHeight;
	frog1.width = frogWidth;
	frog1.col = 125;
	frog1.row = 150;
	Frog *frogPntr = &frog1;

	Log logArray[5];
	for(int x=0;x<5;x++){
		logArray[x].row = 100;
		logArray[x].col = 50*x;
		logArray[x].height = logHeight;
		logArray[x].width = logWidth;
	}

	Lily lilyArray[5];
	for(int x=0;x<5;x++){
		lilyArray[x].row = 85;
		lilyArray[x].col = 50*x;
		lilyArray[x].height = lilyHeight;
		lilyArray[x].width = lilyWidth;
	}

	REG_DISPCTL = MODE3 | BG2_ENABLE;

	while(1) // Game Loop
	{
		if(gameMode==0){
			drawStartGameString();
		}
		else if (gameMode==2){
			drawStartGameString();
			// drawEndGameThing();
			// drawString(20, 20, "mah niga", BLUE);
		}
		else{ //gameMode = 1
			if(KEY_DOWN_NOW(BUTTON_UP))
			{
				frog1.row--;
			}
			if(KEY_DOWN_NOW(BUTTON_DOWN))
			{
				frog1.row++;
			}
			if(KEY_DOWN_NOW(BUTTON_LEFT))
			{
				frog1.col--;
			}
			if(KEY_DOWN_NOW(BUTTON_RIGHT))
			{
				frog1.col++;
			}	
			//makes logs go forwards and lilys go backwards
			for(int x=0;x<5;x++){
				logArray[x].col++;
				drawLog(&logArray[x]);
			}
			for(int x=0;x<5;x++){
				lilyArray[x].col--;
				drawLily(&lilyArray[x]);
			}
			WaitForVblank();
			drawFrog(frogPntr);
			boundsCheck(&lilyArray,&logArray);			
			if(checkCollision(frogPntr,&logArray)){
				gameMode = 2;
			}
			if(onLilyPad(frogPntr,&lilyArray)){
				frog1.col--;
			}

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

int onLilyPad(Frog *fpntr, Lily *lpntr){
	for (int x=0;x<5;x++){
		int frogX = fpntr->col;	
		int frogY = fpntr->row;	
		int frogW = fpntr->width;	
		int frogH = fpntr->height;	
		int lilyX = (*lpntr).col;	
		int lilyY = (*lpntr).row;	
		int lilyW = (*lpntr).width;	
		int lilyH = (*lpntr).height;	
		if ((frogX < (lilyX + lilyW)) && ((frogX + frogW) > lilyX) && (frogY < (lilyY + lilyH)) && ((frogH + frogY) > lilyY)) {
			return 1;
		}
		lpntr++;				
	}	
	return 0;
}

int checkCollision(Frog *fpntr, Log *lpntr){
	for (int x=0;x<5;x++){
		int frogX = fpntr->col;	
		int frogY = fpntr->row;	
		int frogW = fpntr->width;	
		int frogH = fpntr->height;	
		int logX = (*lpntr).col;	
		int logY = (*lpntr).row;	
		int logW = (*lpntr).width;	
		int logH = (*lpntr).height;	
		if ((frogX < (logX + logW)) && ((frogX + frogW) > logX) && (frogY < (logY + logH)) && ((frogH + frogY) > logY)) {
			// gameMode = 2;
			return 1;
		}
		lpntr++;				
	}	
	return 0;
}

void boundsCheck(Lily *lilyPntr, Log *logPntr)
{
	for (int x=0;x<5;x++){
		if((*logPntr).col > MAX_WIDTH){
			(*logPntr).col = 0;
		}	
		if((*lilyPntr).col > MAX_WIDTH){
			(*lilyPntr).col = 0;
		}	
		if((*logPntr).col < 0){
			(*logPntr).col = MAX_WIDTH;
		}	
		if((*lilyPntr).col < 0){
			(*lilyPntr).col = MAX_WIDTH;
		}
		lilyPntr++;				
		logPntr++;				
	}
}

void WaitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

void drawStartGameString(){
	if(KEY_DOWN_NOW(BUTTON_START)){
		gameMode = 1;
		reset();
	}else{
		drawString(20, 20, "GAME OF FROGGER. CLICK TO PLAY", RED);		
		drawString(MAX_HEIGHT/2+5, MAX_WIDTH/2-40, "PUSH START", RED);
	}
}

void reset(){
	drawRect(0,0,160,240,BLACK);
	main();
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

void drawFrog(Frog *fptr)
{
	drawRect(fptr->oldrow,fptr->oldcol,fptr->height,fptr->width, BLACK);				
	int index = 0;
	int r,c;
	for(r=0; r<frogHeight; r++)
	{
		for(c=0; c<frogWidth; c++)
		{
			setPixel(r+fptr->row, c+fptr->col, frog[index]);
			index+=1;
		}
	}
	fptr->oldrow = fptr->row;
	fptr->oldcol = fptr->col;
}

void drawLog(Log *lptr)
{
	drawRect(lptr->oldrow,lptr->oldcol,lptr->height,lptr->width, BLACK);				
	int index = 0;
	int r,c;
	for(r=0; r<logHeight; r++)
	{
		for(c=0; c<logWidth; c++)
		{
			setPixel(r+lptr->row, c+lptr->col, logPic[index]);
			index+=1;
		}
	}
	lptr->oldrow = lptr->row;
	lptr->oldcol = lptr->col;
}

void drawLily(Lily *lptr)
{
	drawRect(lptr->oldrow,lptr->oldcol,lptr->height,lptr->width, BLACK);				
	int index = 0;
	int r,c;
	for(r=0; r<lilyHeight; r++)
	{
		for(c=0; c<lilyWidth; c++)
		{
			setPixel(r+lptr->row, c+lptr->col, lily[index]);
			index+=1;
		}
	}
	lptr->oldrow = lptr->row;
	lptr->oldcol = lptr->col;
}