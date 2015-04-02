//TODO
//fix slots. Always changing the first one instead of current slot
//add timing and dma


// #include "mylib.c"
#include <stdio.h>
#include "font.c"
#include "frog.c"
#include "lily.c"
#include "logPic.c"
#include "car.c"
#include "truck.c"


typedef unsigned short u16;
typedef unsigned int u32;

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

typedef struct
{								// ***********************************************************
	 const volatile void *src;	// We mark this as const which means we can assign it const
	 volatile void *dst;		// things without the compiler yelling but we can also pass it
	 volatile u32 cnt;		// things that are not const!
								// ***********************************************************
} DMAREC;
#define DMA ((volatile DMAREC *)0x040000B0)

#define DMA_DESTINATION_INCREMENT (0 << 21)
#define DMA_DESTINATION_DECREMENT (1 << 21)
#define DMA_DESTINATION_FIXED (2 << 21)
#define DMA_DESTINATION_RESET (3 << 21)

#define DMA_SOURCE_INCREMENT (0 << 23)
#define DMA_SOURCE_DECREMENT (1 << 23)
#define DMA_SOURCE_FIXED (2 << 23)

#define DMA_REPEAT (1 << 25)

#define DMA_16 (0 << 26)
#define DMA_32 (1 << 26)

#define DMA_NOW (0 << 28)
#define DMA_AT_VBLANK (1 << 28)
#define DMA_AT_HBLANK (2 << 28)
#define DMA_AT_REFRESH (3 << 28)

#define DMA_IRQ (1 << 30)
#define DMA_ON (1 << 31)

#define START_ON_FIFO_EMPTY 0x30000000

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
void drawFrogBetter();
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
void drawBlackHoles();
int outOfBounds();
void printLives(int lives);
void drawImage3(int r, int c, int width, int height, const u16* image);


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
	frog1.row = 152;
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

	drawBlackHoles();
	printLives(lives);

	while(1) // Game Loop
	{
		if(frog1.row >=142 || (frog1.row>=72&&frog1.row<90) || (frog1.row>=10&&frog1.row<20)){	
			drawHoles(&slotArray);
			drawSafeAreas();
		}
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
		}
		else{ //gameMode = 1
			if(KEY_DOWN_NOW(BUTTON_UP))
			{
				frog1.row-=2;
			}
			if(KEY_DOWN_NOW(BUTTON_DOWN))
			{
				frog1.row+=2;
			}
			if(KEY_DOWN_NOW(BUTTON_LEFT))
			{
				frog1.col-=2;
			}
			if(KEY_DOWN_NOW(BUTTON_RIGHT))
			{
				frog1.col+=2;
			}	

			//makes logs go forwards and lilys go backwards
			for(int x=0;x<4;x++){
				logArray[x].col++;
				logArray2[x].col+=2;				
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
				carArray2[x].col-=2;
				drawCar(&carArray2[x]);				
				truckArray[x].col++;
				drawTruck(&truckArray[x]);
				truckArray2[x].col++;
				drawTruck(&truckArray2[x]);				
			}

			// if(frog1.row<79 && frog1.row>20){
			// 	if((!onRaft(frogPntr, &lilyArray, &logArray)&&((frog1.row+4)<80))&&(!onRaft(frogPntr, &lilyArray2, &logArray2)&&((frog1.row+4)<80))){
			// 		lives--;
			// 		frog1.col = 125;
			// 		frog1.row = 150;
			// 		// reset();
			// 		drawRect(0,0,20,20,GREEN);
			// 	}
			// }
			if(outOfBounds(frogPntr,&slotArray) && frog1.row>=20){
				lives--;
				printLives(lives);
				frog1.col = 125;
				frog1.row = 152;
			}
			//not out of bounds or row<20
			else if (frog1.row<20){
				int status = checkEndSlotCollision(frogPntr,&slotArray); 
				if(status==1){
					// drawRect(50,50,10,10,YELLOW);
				}
				else if(status==0){
					//loses life
					drawRect(50,50,10,10,RED);	
					lives--;
					frog1.col = 125;
					frog1.row = 152;
					printLives(lives);
				}
				else{
					// drawHoles();
					//success
					frog1.col = 125;
					frog1.row = 152;						
					drawRect(50,50,10,10,MAGENTA);										
				}
			}
						
			WaitForVblank();
			boundsCheck(&lilyArray,&logArray, &carArray, &truckArray);		
			boundsCheck(&lilyArray2,&logArray2, &carArray2, &truckArray2);			
			// make two if statements here so the mehtod code doesnt get too crazy. Done	
			// if(frog1.row>80){
			// 	if(checkCollision(frogPntr,&carArray,&truckArray) || checkCollision(frogPntr,&carArray2,&truckArray2)){
			// 		drawRect(0,0,10,10,MAGENTA);
			// 		frog1.col = 125;
			// 		frog1.row = 152;
			// 		lives--;
			// 		printLives(lives);
			// 		drawRect(0,0,10,100,BLACK);

			// 	}
			// }
				if(lives==0){
					gameMode = 2;
				}	
			if(onLilyPad(frogPntr, &lilyArray)||(onLilyPad(frogPntr, &lilyArray2))){
				frog1.col--;
				drawRect(230,150,10,10,YELLOW);
			}				
			else if(onLog(frogPntr, &logArray)){
				drawRect(230,150,10,10,RED);				
				frog1.col++;
			}
			else if(onLog(frogPntr, &logArray2)){
				drawRect(230,150,10,10,BLUE);								
				frog1.col+=2;
			}

		drawFrogBetter(frogPntr);
		}
	}
}


void drawImage3(int c, int r, int width, int height, const u16* image)
{	
	for (int i = 0; i < height; i++) {
		DMA[3].src = &image[OFFSET(i, 0, width)];
		DMA[3].dst = videoBuffer + OFFSET(r + i, c, 240);
		DMA[3].cnt = width | DMA_ON;
	}
}

void printLives(int lives){
	drawRect(0,0,10,240,BLACK);
	if(lives == 3){
		drawString(0,180,"Lives: 3",RED);
	}
	else if(lives == 2){
		drawString(0,180,"Lives: 2",RED);
	}
	else if (lives == 1){
		drawString(0,180,"Lives: 1",RED);
	}
	else{
		drawString(0,180,"Lives: 0",RED);
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

	for (int x=0;x<4;x++){
		int frogX = fpntr->col;	
		int frogY = fpntr->row;	
		int frogW = fpntr->width;	
		int frogH = fpntr->height;		
		int logX = (*logPntr).col;	
		int logY = (*logPntr).row;	
		int logW = (*logPntr).width;	
		int logH = (*logPntr).height;

		if ((frogX < (logX + logW)) && ((frogX + frogW) > logX) && (frogY < (logY + logH)) && ((frogH + frogY) > logY)) {
			return 1;
		}
		logPntr++;		
	}	
	return 0;
}

int onLilyPad(Frog *fpntr, Lily *lpntr){

	for (int x=0;x<4;x++){	
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

int onRaft(Frog *fpntr, Lily *lpntr, Log *lgpntr){
	
	for (int x=0;x<4;x++){
		int frogX = fpntr->col+3;	
		int frogY = fpntr->row+3;	
		int frogW = fpntr->width-6;	
		int frogH = fpntr->height-6;	
		int lilyX = (*lpntr).col;	
		int lilyY = (*lpntr).row;	
		int lilyW = (*lpntr).width;	
		int lilyH = (*lpntr).height;
		int logX = (*lgpntr).col;	
		int logY = (*lgpntr).row;	
		int logW = (*lgpntr).width;	
		int logH = (*lgpntr).height;

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
	int frogX = fpntr->col;	
	int frogY = fpntr->row;	
	int frogW = fpntr->width;

	for (int x=0;x<4;x++){
	
		int slotX = (spntr+x)->col;	
		int slotY = (spntr+x)->row;	
		int slotW = (spntr+x)->width;
		int slotH = (spntr+x)->height;	

		if(frogY <= slotY+2 && frogX >= slotX && ((frogX+frogW) <= (slotX+slotW)) && (frogY-5)<(slotY+slotH)){
			//success- completely in slot
			(spntr+x)->occupied = 1;
			return 2;
		}
		if(frogX >= slotX && ((frogX+frogW) <= (slotX+slotW)) && (frogY-5)<(slotY+slotH)){
			//is sort of in slot
			return 1;
		}
	}
	return 0;
}

int checkCollision(Frog *fpntr, Car *cpntr, Truck *tpntr){

	for (int x=0;x<4;x++){
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
		if ((frogX < (carX + carW)) && ((frogX + frogW) > carX) && (frogY < (carY + carH)) && ((frogH + frogY) > carY)) {
			return 1;
		}
		if ((frogX < (truckX + truckW)) && ((frogX + frogW) > truckX) && (frogY < (truckY + truckH)) && ((frogH + frogY) > truckY)) {
			return 1;
		}
		cpntr++;
		tpntr++;							
	}	
	return 0;
}

void boundsCheck(Lily *lilyPntr, Log *logPntr, Car *cpntr, Truck *tptr)
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

		if((*tptr).col > MAX_WIDTH){
			(*tptr).col = 0;
		}	
		if((*cpntr).col > MAX_WIDTH){
			(*cpntr).col = 0;
		}	
		if((*tptr).col < 0){
			(*tptr).col = MAX_WIDTH;
		}	
		if((*cpntr).col < 0){
			(*cpntr).col = MAX_WIDTH;
		}
		lilyPntr++;				
		logPntr++;	
		tptr++;
		cpntr++;	
	}
}

void drawHoles(EndSlot *slotPntr){
	
	for (int x=0;x<4;x++){
		if((slotPntr+x)->occupied==0){
			drawRect(10,(slotPntr+x)->col,10,20,GREY);
		}
		else{
			drawRect(150,30,10,10,GREEN);
			drawRect(10,(slotPntr+x)->col,10,20,BLACK);			
		}
		// slotPntr++;
	}

	// EndSlot *s1 = slotPntr;
	// EndSlot *s2 = slotPntr+1;
	// EndSlot *s3 = slotPntr+2;
	// EndSlot *s4 = slotPntr+3;

	// if(s1->occupied==1){
	// 	drawRect(150,0,10,10,GREEN);
	// }
	// else{
	// 	drawRect(10,s1->col,10,20,GREY);		
	// }
	// if(s2->occupied==1){
	// 	drawRect(150,40,10,10,GREEN);		
	// }
	// else{
	// 	drawRect(10,s2->col,10,20,GREY);		
	// }	
	// if(s3->occupied==1){
	// 	drawRect(150,80,10,10,GREEN);
	// }
	// else{
	// 	drawRect(10,s3->col,10,20,GREY);		
	// }	
	// if(s4->occupied==1){
	// 	drawRect(150,160,10,10,GREEN);		
	// }
	// else{
	// 	drawRect(10,s4->col,10,20,GREY);		
	// }	
	// s1->occupied = 0;


}
void drawBlackHoles(){
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
	drawRect(150,0,1,240,GREY);
	drawRect(80,0,1,240,GREY);
	drawRect(90,0,1,240,GREY);
	drawRect(10,0,1,240,GREY);	

}

void drawFrog(Frog *fptr)
{
	drawRect(fptr->oldrow,fptr->oldcol,fptr->height,fptr->width, BLACK);				
	drawImage3(fptr->col,fptr->row,fptr->width,fptr->height, frog);	
	fptr->oldrow = fptr->row;
	fptr->oldcol = fptr->col;
}

void drawFrogBetter(Frog *fptr)
{
	drawRect(fptr->oldrow,fptr->oldcol,fptr->height,fptr->width, BLACK);				
	// drawImage3(fptr->col,fptr->row,fptr->width,fptr->height, frog);	
	drawRect(fptr->row,fptr->col,fptr->height,fptr->width, GREEN);				
	fptr->oldrow = fptr->row;
	fptr->oldcol = fptr->col;
}


void drawLog(Log *lptr)
{
	drawRect(lptr->oldrow,lptr->oldcol,lptr->height,lptr->width, BLACK);				
	drawImage3(lptr->col,lptr->row,lptr->width,lptr->height, logPic);
	lptr->oldrow = lptr->row;
	lptr->oldcol = lptr->col;
}

void drawLily(Lily *lptr)
{
	drawRect(lptr->oldrow,lptr->oldcol,lptr->height,lptr->width, BLACK);				
	drawImage3(lptr->col,lptr->row,lptr->width,lptr->height, lily);	
	lptr->oldrow = lptr->row;
	lptr->oldcol = lptr->col;
}

void drawCar(Car *cptr)
{
	drawRect(cptr->oldrow,cptr->oldcol,cptr->height,cptr->width, BLACK);				
	drawImage3(cptr->col,cptr->row,cptr->width,cptr->height, car);
	cptr->oldrow = cptr->row;
	cptr->oldcol = cptr->col;
}

void drawTruck(Truck *tptr)
{
	drawRect(tptr->oldrow,tptr->oldcol,tptr->height,tptr->width, BLACK);				
	drawImage3(tptr->col,tptr->row,tptr->width,tptr->height, truck);	
	tptr->oldrow = tptr->row;
	tptr->oldcol = tptr->col;
}