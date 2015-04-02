#include "mylib.h"

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