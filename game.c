//TODO
//when logs/lilys go offscreen, their col doesnt reset to 0. Must fix in orderto do collision check

// #include "mylib.c"
#include <stdio.h>
#include "font.c"
#include "frog.c"
#include "lily.c"
#include "logPic.c"
#include "car.c"
#include "truck.c"

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
#define GREY COLOR(10,10,10)
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

const short frogWidth = 8;
const short frogHeight = 8;
const short logWidth = 25;
const short logHeight = 15;
const short lilyWidth = 15;
const short lilyHeight = 15;
const short carWidth = 20;
const short carHeight = 15;
const short truckWidth = 25;
const short truckHeight = 15;

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
void drawNewFrog();
void drawLog();
void drawLily();
void drawCar();
void drawTruck();
int checkCollision();
int checkEndSlotCollision();
int onLog();
int onLilyPad();
int onRaft();
void reset();
void drawSafeAreas();
void drawHoles();
int outOfBounds();
void printLives(int lives);

typedef struct{
	int width;
	int height;
	int col;
	int row;
	int occupied;
}EndSlot;

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

typedef struct{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;	
}Car;

typedef struct{
	int width;
	int height;
	int col;
	int row;
	int oldrow;
	int oldcol;	
}Truck;


int main()
{
	Frog frog1;
	frog1.height = frogHeight;
	frog1.width = frogWidth;
	frog1.col = 125;
	frog1.row = 150;
	Frog *frogPntr = &frog1;

	EndSlot slotArray[4];
	int offset = 32;
	for(int x=0;x<4;x++){
		slotArray[x].row = 10;
		slotArray[x].col = 20*x+offset;
		slotArray[x].height = logHeight;
		slotArray[x].width = logWidth;
		offset+=32;
		slotArray[x].occupied = 0;
	}
	
	Log logArray[4];
	for(int x=0;x<4;x++){
		logArray[x].row = 20;
		logArray[x].col = 60*x;
		logArray[x].height = logHeight;
		logArray[x].width = logWidth;
	}
	Log logArray2[4];
	for(int x=0;x<4;x++){
		logArray2[x].row = 50;
		logArray2[x].col = 60*x+10;
		logArray2[x].height = logHeight;
		logArray2[x].width = logWidth;
	}

	Lily lilyArray[4];
	for(int x=0;x<4;x++){
		lilyArray[x].row = 35;
		lilyArray[x].col = 60*x;
		lilyArray[x].height = lilyHeight;
		lilyArray[x].width = lilyWidth;
	}
	Lily lilyArray2[4];
	for(int x=0;x<4;x++){
		lilyArray2[x].row = 65;
		lilyArray2[x].col = 60*x+10;
		lilyArray2[x].height = lilyHeight;
		lilyArray2[x].width = lilyWidth;
	}

	Car carArray[4];
	for(int x=0;x<4;x++){
		carArray[x].row = 90;
		carArray[x].col = 60*x;
		carArray[x].height = carHeight;
		carArray[x].width = carWidth;
	}

	Car carArray2[4];
	for(int x=0;x<4;x++){
		carArray2[x].row = 120;
		carArray2[x].col = 60*x+10;
		carArray2[x].height = carHeight;
		carArray2[x].width = carWidth;
	}

	Truck truckArray[4];
	for(int x=0;x<4;x++){
		truckArray[x].row = 105;
		truckArray[x].col = 60*x;
		truckArray[x].height = truckHeight;
		truckArray[x].width = truckWidth;
	}
	Truck truckArray2[4];
	for(int x=0;x<4;x++){
		truckArray2[x].row = 135;
		truckArray2[x].col = 60*x+10;
		truckArray2[x].height = truckHeight;
		truckArray2[x].width = truckWidth;
	}
	int lives = 3;
	// int frogscompleted = 0;

	REG_DISPCTL = MODE3 | BG2_ENABLE;

	while(1) // Game Loop
	{
		if(KEY_DOWN_NOW(BUTTON_SELECT)){
			gameMode = 0;
			break;
		}
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
			// drawRect(20,0,60,240,BLUE); //makes everything flash

			//makes logs go forwards and lilys go backwards
			for(int x=0;x<4;x++){
				logArray[x].col++;
				logArray2[x].col++;				
				drawLog(&logArray[x]);
				drawLog(&logArray2[x]);		
				lilyArray[x].col--;
				drawLily(&lilyArray[x]);
				lilyArray2[x].col--;
				drawLily(&lilyArray2[x]);			
			}
			for(int x=0;x<4;x++){
				carArray[x].col--;
				drawCar(&carArray[x]);
				carArray2[x].col--;
				drawCar(&carArray2[x]);				
				if(carArray[x].col == 0){
					carArray[x].col = 240;
				}
				if(carArray2[x].col == 0){
					carArray2[x].col = 240;
				}
				truckArray[x].col++;
				drawTruck(&truckArray[x]);
				truckArray2[x].col++;
				drawTruck(&truckArray2[x]);				
				if(truckArray[x].col == 240){
					truckArray[x].col = 0;
				}
				if(truckArray2[x].col == 240){
					truckArray2[x].col = 0;
				}
			}
			// if((!onRaft(frogPntr, &lilyArray, &logArray)&&((frog1.row+4)<80))&&(!onRaft(frogPntr, &lilyArray2, &logArray2)&&((frog1.row+4)<80))){
			// 	lives--;
			// 	frog1.col = 125;
			// 	frog1.row = 150;
			// 	// reset();
			// 	drawRect(0,0,20,20,GREEN);
			// }

			if(outOfBounds(frogPntr,&slotArray) && frog1.row>=20){
				lives--;
				drawRect(0,0,10,100,BLACK);
				frog1.col = 125;
				frog1.row = 150;
			}
			//not out of bounds or row<20
			else if (frog1.row<20){
				if(checkEndSlotCollision(frogPntr,&slotArray)==1){
					drawRect(0,0,10,10,YELLOW);
				}
				else if (checkEndSlotCollision(frogPntr,&slotArray)==0){
					drawRect(0,0,10,10,RED);	
					drawRect(0,0,10,100,BLACK);
					lives--;
					frog1.col = 125;
					frog1.row = 150;					
				}
				else{
					frog1.col = 125;
					frog1.row = 150;						
					drawRect(0,0,10,10,MAGENTA);										
				}
			}

			drawHoles(&slotArray);
			drawSafeAreas();						
			WaitForVblank();
			drawFrog(frogPntr);
			boundsCheck(&lilyArray,&logArray);		
			boundsCheck(&lilyArray2,&logArray2);		
			printLives(lives);
			//make two if statements here so the mehtod code doesnt get too crazy. Done	
			// if(checkCollision(frogPntr,&carArray) || checkCollision(frogPntr,&carArray2)){
			// 	frog1.col = 125;
			// 	frog1.row = 150;
			// 	lives--;
			// drawRect(0,0,10,100,BLACK);

			// }
			// 	if(lives==0){
			// 		gameMode = 2;
			// 	}	
			if(onLilyPad(frogPntr, &lilyArray)||onLilyPad(frogPntr, &lilyArray2)){
				frog1.col--;
			}
			if(onLog(frogPntr, &logArray)||onLog(frogPntr, &logArray2)){
				frog1.col++;
			}

		}	
	}
}

void printLives(int lives){
	if(lives == 3){
		drawString(0,0,"Lives: 3",RED);
	}
	else if(lives == 2){
		drawString(0,0,"Lives: 2",RED);
	}
	else if (lives == 1){
		drawString(0,0,"Lives: 1",RED);
	}
	else{
		drawString(0,0,"Lives: 0",RED);
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

int onLog(Frog *fpntr, Log *logPntr){
	int frogX = fpntr->col;	
	int frogY = fpntr->row;	
	int frogW = fpntr->width;	
	int frogH = fpntr->height;		
	int logX = (*logPntr).col;	
	int logY = (*logPntr).row;	
	int logW = (*logPntr).width;	
	int logH = (*logPntr).height;

	for (int x=0;x<4;x++){
		if ((frogX < (logX + logW)) && ((frogX + frogW) > logX) && (frogY < (logY + logH)) && ((frogH + frogY) > logY)) {
			return 1;
		}
		logPntr++;		
	}	
	return 0;
}

int onLilyPad(Frog *fpntr, Lily *lpntr){
	int frogX = fpntr->col;	
	int frogY = fpntr->row;	
	int frogW = fpntr->width;	
	int frogH = fpntr->height;	
	int lilyX = (*lpntr).col;	
	int lilyY = (*lpntr).row;	
	int lilyW = (*lpntr).width;	
	int lilyH = (*lpntr).height;
	for (int x=0;x<4;x++){			
		if ((frogX < (lilyX + lilyW)) && ((frogX + frogW) > lilyX) && (frogY < (lilyY + lilyH)) && ((frogH + frogY) > lilyY)) {
			return 1;
		}
		lpntr++;				
	}	
	return 0;
}

int onRaft(Frog *fpntr, Lily *lpntr, Log *lgpntr){
	int frogX = fpntr->col+2;	
	int frogY = fpntr->row+2;	
	int frogW = fpntr->width-4;	
	int frogH = fpntr->height-4;	
	int lilyX = (*lpntr).col;	
	int lilyY = (*lpntr).row;	
	int lilyW = (*lpntr).width;	
	int lilyH = (*lpntr).height;
	int logX = (*lgpntr).col;	
	int logY = (*lgpntr).row;	
	int logW = (*lgpntr).width;	
	int logH = (*lgpntr).height;

	for (int x=0;x<4;x++){
		if (((frogX < (lilyX + lilyW)) && ((frogX + frogW) > lilyX) && (frogY < (lilyY + lilyH)) && ((frogH + frogY) > lilyY))) {
			return 1;
		}
		else if (((frogX < (logX + logW)) && ((frogX + frogW) > logX) && (frogY < (logY + logH)) && ((frogH + frogY) > logY))) {
			return 1;
		}		
		lpntr++;
		lgpntr++;				
	}	
	return 0;
}



int outOfBounds(Frog *fpntr, EndSlot *spntr){

	if(fpntr->col >= MAX_WIDTH-4 || (fpntr->col+fpntr->width) <= 4 || fpntr->row >= (MAX_HEIGHT-4)){
		return 1;
	}	
	return 0;
}

int checkEndSlotCollision(Frog *fpntr, EndSlot *spntr){

	// int slotH = spntr->height;	
	for (int x=0;x<4;x++){
		int frogX = fpntr->col;	
		int frogY = fpntr->row;	
		int frogW = fpntr->width;	
		// int frogH = fpntr->height;
		int slotX = spntr->col;	
		int slotY = spntr->row;	
		int slotW = spntr->width;
		int slotH = spntr->height;	
		// spntr->occupied = 1;

		if(frogY == slotY+1){
			//success
			(*spntr).occupied = 1;
			spntr->occupied = 1;
			return 2;
		}
		if(frogX >= slotX && ((frogX+frogW) <= (slotX+slotW)) && (frogY-5)<(slotY+slotH)){
			//is in slot
			return 1;
		}
	spntr++;
	}
	return 0;
}

int checkCollision(Frog *fpntr, Car *cpntr, Truck *tpntr){
	int frogX = fpntr->col;	
	int frogY = fpntr->row;	
	int frogW = fpntr->width;	
	int frogH = fpntr->height;	
	int carX = (*cpntr).col;	
	int carY = (*cpntr).row;	
	int carW = (*cpntr).width;	
	int carH = (*cpntr).height;
	int truckX = (*tpntr).col;	
	int truckY = (*tpntr).row;	
	int truckW = (*tpntr).width;	
	int truckH = (*tpntr).height;	
	for (int x=0;x<4;x++){
		
		if ((frogX < (carX + carW)) && ((frogX + frogW) > carX) && (frogY < (carY + carH)) && ((frogH + frogY) > carY)) {
			// gameMode = 2;
			return 1;
		}
		if ((frogX < (truckX + truckW)) && ((frogX + frogW) > truckX) && (frogY < (truckY + truckH)) && ((frogH + frogY) > truckY)) {
			// gameMode = 2;
			return 1;
		}
		cpntr++;
		tpntr++;							
	}	
	return 0;
}

void boundsCheck(Lily *lilyPntr, Log *logPntr)
{
	for (int x=0;x<4;x++){
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

void drawHoles(EndSlot *slotPntr){
	for (int x=0;x<4;x++){
		if(slotPntr->occupied==0){
			drawRect(10,slotPntr->col,10,20,GREY);
		}
		else{
			drawRect(10,slotPntr->col,10,20,BLUE);			
		}
		slotPntr++;
	}
	drawRect(10,0,10,32,BLACK);
	drawRect(10,52,10,32,BLACK);
	drawRect(10,104,10,32,BLACK);
	drawRect(10,156,10,32,BLACK);
	drawRect(10,208,10,32,BLACK);
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

void drawSafeAreas(){
	drawRect(150,0,10,240,GREY);
	drawRect(80,0,10,240,GREY);
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
void drawNewFrog(Frog *fptr)
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

void drawCar(Car *cptr)
{
	drawRect(cptr->oldrow,cptr->oldcol,cptr->height,cptr->width, BLACK);				
	int index = 0;
	int r,c;
	for(r=0; r<carHeight; r++)
	{
		for(c=0; c<carWidth; c++)
		{
			setPixel(r+cptr->row, c+cptr->col, car[index]);
			index+=1;
		}
	}
	cptr->oldrow = cptr->row;
	cptr->oldcol = cptr->col;
}

void drawTruck(Truck *tptr)
{
	drawRect(tptr->oldrow,tptr->oldcol,tptr->height,tptr->width, BLACK);				
	int index = 0;
	int r,c;
	for(r=0; r<truckHeight; r++)
	{
		for(c=0; c<truckWidth; c++)
		{
			setPixel(r+tptr->row, c+tptr->col, truck[index]);
			index+=1;
		}
	}
	tptr->oldrow = tptr->row;
	tptr->oldcol = tptr->col;
}