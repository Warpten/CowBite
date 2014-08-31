//2XSAI.C -- Heavily modified, based on a heavily modified version I found elsewhere.
//The algorithm is still the same as Kreed's original.
//The idea behind my modifications was to make it more compatible (the version I found
//was based on Allegro, which I don't want to use)
//I also tried to "optimize" it by removing some muls, memory allocation, and using
//shifts instead and pointer arithmetic instead
//. . . this didn't have much of an effect, sadly, since most of the overhead is
//in routines which are already pretty darned fast.
//Thanks to Kreed for having such ingenious coding skills and letting me port it
//to CowBite!

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//#include "allegro.h"
#include "2xsai.h"
#include "Constants.h"		//This is to keep the style somewhat similar
							//to the rest of the emulator

//#define uint32 unsigned long
//#define uint16 unsigned short
//#define u8 unsigned char



static u32 colorMask = 0xF7DEF7DE;
static u32 lowPixelMask = 0x08210821;
static u32 qcolorMask = 0xE79CE79C;
static u32 qlowpixelMask = 0x18631863;
static u32 redblueMask = 0xF81F;
static u32 greenMask = 0x7E0;
static int PixelsPerMask = 2;
static int XSAI_Depth;	//The color depth 
static int XSAI_bitWidth;	//The color width
static int XSAI_sourceWidth;
static int XSAI_sourceHeight;
static int XSAI_destWidth;
static int XSAI_destHeight;
static int XSAI_sbpp;
static int XSAI_dbpp;

static unsigned char *src_line[4];
static unsigned char **src_line0;
static unsigned char **src_line1;
static unsigned char **src_line2;
static unsigned char **src_line3;

static unsigned char *dst_line[2];
static unsigned char **dst_line0;
static unsigned char **dst_line1;


int Init_2xSaI(int d, int nSourceWidth, int nSourceHeight)
{

	int minr = 0, ming = 0, minb = 0;
	int i, j;

	//A little global variable added by me
	//Throughout I am assuming that same bit depth for source and dest
	XSAI_Depth = d;
	XSAI_bitWidth = d / 16;	//Divide depth by 16 to get width
	XSAI_sourceWidth = nSourceWidth;
	XSAI_sourceHeight = nSourceHeight;
	XSAI_destWidth = XSAI_sourceWidth << 1;
	XSAI_destHeight = XSAI_sourceHeight << 1;

	/* Get lowest color bit*/
	//(makecol converts automatically to the correct format.  This loop
	//effectively finds out the lowest possible value for r, g, and b.
	//Since I'm not using allegro, I must resort to less savory methods...
	/*
	for (i = 0; i < 255; i++) {
		if (!minr)
			minr = makecol(i, 0, 0);
		if (!ming)
			ming = makecol(0, i, 0);
		if (!minb)
			minb = makecol(0, 0, i);
	}*/
	
	//This basically calculates the max value minus the min value for each color
	/*
	colorMask = (makecol_depth(d, 255, 0, 0) - minr) | (makecol_depth(d, 0, 255, 0) - ming) | (makecol_depth(d, 0, 0, 255) - minb);
	lowPixelMask = minr | ming | minb;
	qcolorMask = (makecol_depth(d, 255, 0, 0) - 3 * minr) | (makecol_depth(d, 0, 255, 0) - 3 * ming) | (makecol_depth(d, 0, 0, 255) - 3 * minb);
	qlowpixelMask = (minr * 3) | (ming * 3) | (minb * 3);
	redblueMask = makecol_depth(d, 255, 0, 255);
	greenMask = makecol_depth(d, 0, 255, 0);
	*/

	//The less savory method
	switch (XSAI_Depth) {
		//Note that 16 bit is just the default case.  Kreed already did these calculations,
		//and I am trusting him.
		case 16:	colorMask = 0xF7DEF7DE;
					lowPixelMask = 0x08210821;
					qcolorMask = 0xE79CE79C;
					qlowpixelMask = 0x18631863;
					redblueMask = 0xF81F;	//Never gets used
					greenMask = 0x7E0;
					break;
		
		case 32:	colorMask = 0xFEFEFEFE;
					lowPixelMask = 0x01010101;
					qcolorMask = 0xFCFCFCFC;
					qlowpixelMask = 0x03030303;
					redblueMask = 0x00FF00FF;	//Never gets used
					greenMask = 0x00FF0000;		//nor does this
					break;
		default:  return 0;

	}



	PixelsPerMask = (d <= 16) ? 2 : 1;
	
	//This basically takes 16 bit values and or's them together,
	//so you can apply a 32 bit mask to two 16 bit values
	if (PixelsPerMask == 2) {
		colorMask |= (colorMask << 16);
		qcolorMask |= (qcolorMask << 16);
		lowPixelMask |= (lowPixelMask << 16);
		qlowpixelMask |= (qlowpixelMask << 16);
	}

	//TRACE("Color Mask:       0x%X\n", colorMask);
	//TRACE("Low Pixel Mask:   0x%X\n", lowPixelMask);
	//TRACE("QColor Mask:      0x%X\n", qcolorMask);
	//TRACE("QLow Pixel Mask:  0x%X\n", qlowpixelMask);

	XSAI_sbpp = XSAI_dbpp = XSAI_Depth;

	//Comment from Tom:  As of this writing I have no idea what adding 7 does for this,
	//unless there is some weird 17 bit graphics mode or something that I don't know about
	XSAI_sbpp = (XSAI_sbpp + 7) / 8;		// turn into bytes per pix

	//Allocating and freeing memory is slow business.  Instead of doing this
	//each iteration, it's better to do it in the init
	//and have a special destructor to free it

	for (i = 0; i < 4; i++) {
		src_line[i] = new unsigned char [(XSAI_sbpp * (XSAI_sourceWidth + 4))];

		if (!src_line[i]) {		/* Out of ram */
			for (j = 0; j < i; j++)
				free(src_line[j]);
			return 0;
		}
		memset(src_line[i], 0, XSAI_sbpp * (XSAI_sourceWidth + 4));
	}

	src_line0 = src_line;
	src_line1 = src_line+1;
	src_line2 = src_line+2;
	src_line3 = src_line+3;

	for (i = 0; i < 2; i++) {
		dst_line[i] = new unsigned char [(XSAI_sbpp * (XSAI_destWidth + 4))];
		//malloc(XSAI_sbpp * XSAI_sourceWidth);

		if (!dst_line[i]) {		/* Out of ram */
			for (j = 0; j < i; j++)
				delete(dst_line[j]);
			return 0;
		}
		memset(dst_line[i], 0, XSAI_sbpp * XSAI_destWidth);
	}

	dst_line0 = dst_line;
	dst_line1 = dst_line+1;
	

	return 1;
}

//This was made by Tom -- it frees all the memory allocated by 2xSai
void Delete_2xSaI () {
//	if (_msize(dst_line[0]) !=0)
		delete dst_line[0];
//	if (_msize(dst_line[1]) !=0)
		delete dst_line[1];
//	if (_msize(src_line[0]) !=0)
		delete src_line[0];
//	if (_msize(src_line[1]) !=0)
		delete src_line[1];
//	if (_msize(src_line[2]) !=0)
		delete src_line[2];
//	if (_msize(src_line[3]) !=0)
		delete src_line[3];
}

static inline int GetResult1(u32 A, u32 B, u32 C, u32 D)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r += 1;
	if (y <= 1)
		r -= 1;
	return r;
}

static inline int GetResult2(u32 A, u32 B, u32 C, u32 D, u32 E)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r -= 1;
	if (y <= 1)
		r += 1;
	return r;
}


static inline int GetResult(u32 A, u32 B, u32 C, u32 D)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C)
		x += 1;
	else if (B == C)
		y += 1;
	if (A == D)
		x += 1;
	else if (B == D)
		y += 1;
	if (x <= 1)
		r += 1;
	if (y <= 1)
		r -= 1;
	return r;
}


static inline u32 INTERPOLATE(u32 A, u32 B)
{
	if (A != B) {
		return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask));
	}
	else
		return A;
}


static inline u32 Q_INTERPOLATE(u32 A, u32 B, u32 C, u32 D)
{
	register u32 x = ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	register u32 y = (A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);
	y = (y >> 2) & qlowpixelMask;
	return x + y;
}


void SuperEagle(u8 * src, u8 * dst)
{

	int i, j;
	int x, y, w, h;
	unsigned long src_addr, dst_addr;
	
	if (!src || !dst)
		return;

	
	/* Read 3 src lines, with line[0] = 0 cause it's outside */
	
	for (i = 1; i < 4; i++) {
		src_addr = (unsigned long)(src + (i - 1 ) * (240 << XSAI_bitWidth ));
		for (j = 0; j < 240 * XSAI_sbpp; j += 4)
			*((unsigned long *) &src_line[i][j + XSAI_sbpp]) = *(u32*)(src_addr + j);

	}


	x = 0, y = 0;

	for (h = 0; h < 160; h++) {
		x = 0;
		for (w = 0; w < 240; w++) {
			unsigned long color4, color5, color6,
				color1, color2, color3,
				colorA0, colorA1, colorA2, colorA3, colorB0, colorB1, colorB2, colorB3, colorS1, colorS2, product1a, product1b, product2a, product2b;
			unsigned short *sbp;
			unsigned long *lbp;

//---------------------------------------     B1 B2 
//                                         4  5  6  S2
//                                         1  2  3  S1
//                                            A1 A2 

			if (PixelsPerMask == 2) {
				sbp = (unsigned short *) (*src_line);
				colorB1 = *(sbp + x + 1);
				colorB2 = *(sbp + x + 2);

				sbp = (unsigned short *) (*src_line1);
				color4 = *(sbp + x);
				color5 = *(sbp + x+ 1);
				color6 = *(sbp + x + 2);
				colorS2 = *(sbp + x + 3);

				sbp = (unsigned short *) (*src_line2);
				color1 = *(sbp + x);
				color2 = *(sbp + x+ 1);
				color3 = *(sbp + x + 2);
				colorS1 = *(sbp + x + 3);

				sbp = (unsigned short *) (*src_line3);
				colorA1 = *(sbp + x+ 1);
				colorA2 = *(sbp + x + 2);
			}
			else {
				lbp = (unsigned long *) (*src_line);
				colorB1 = *(lbp + x + 1);
				colorB2 = *(lbp + x + 2);

				lbp = (unsigned long *) (*src_line1);
				color4 = *(lbp + x);
				color5 = *(lbp + x + 1);
				color6 = *(lbp + x + 2);
				colorS2 = *(lbp + x + 3);

				lbp = (unsigned long *) (*src_line2);
				color1 = *(lbp + x);
				color2 = *(lbp + x + 1);
				color3 = *(lbp + x + 2);
				colorS1 = *(lbp + x + 3);

				lbp = (unsigned long *) (*src_line3);
				colorA1 = *(lbp + x + 1);
				colorA2 = *(lbp + x + 2);
			}

			if (color2 == color6 && color5 != color3) {
				product1b = product2a = color2;

				if ((color1 == color2) || (color6 == colorB2)) {
					product1a = INTERPOLATE(color2, color5);
					product1a = INTERPOLATE(color2, product1a);
				}
				else {
					product1a = INTERPOLATE(color5, color6);
				}

				if ((color6 == colorS2) || (color2 == colorA1)) {
					product2b = INTERPOLATE(color2, color3);
					product2b = INTERPOLATE(color2, product2b);
				}
				else {
					product2b = INTERPOLATE(color2, color3);
				}
			}
			else if (color5 == color3 && color2 != color6) {
				product2b = product1a = color5;

				if ((colorB1 == color5) || (color3 == colorS1)) {
					product1b = INTERPOLATE(color5, color6);
					product1b = INTERPOLATE(color5, product1b);
				}
				else {
					product1b = INTERPOLATE(color5, color6);
				}

				if ((color3 == colorA2) || (color4 == color5)) {
					product2a = INTERPOLATE(color5, color2);
					product2a = INTERPOLATE(color5, product2a);
				}
				else {
					product2a = INTERPOLATE(color2, color3);
				}

			}
			else if (color5 == color3 && color2 == color6) {
				register int r = 0;

				r += GetResult(color6, color5, color1, colorA1);
				r += GetResult(color6, color5, color4, colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0) {
					product1b = product2a = color2;
					product1a = product2b = INTERPOLATE(color5, color6);
				}
				else if (r < 0) {
					product2b = product1a = color5;
					product1b = product2a = INTERPOLATE(color5, color6);
				}
				else {
					product2b = product1a = color5;
					product1b = product2a = color2;
				}
			}
			else {
				product2b = product1a = INTERPOLATE(color2, color6);
				product2b = Q_INTERPOLATE(color3, color3, color3, product2b);
				product1a = Q_INTERPOLATE(color5, color5, color5, product1a);

				product2a = product1b = INTERPOLATE(color5, color3);
				product2a = Q_INTERPOLATE(color2, color2, color2, product2a);
				product1b = Q_INTERPOLATE(color6, color6, color6, product1b);
			}

			if (PixelsPerMask == 2) {
				//*((unsigned long *) (&dst_line[0][x << 2])) = product1a | (product1b << 16);
				*((unsigned long *) ( *dst_line + (x << 2) )) = product1a | (product1b << 16);
				*((unsigned long *) ( *dst_line1 + (x << 2) )) = product2a | (product2b << 16);
			}
			else {
				*((unsigned long *) ( *dst_line + (x << 3) )) = product1a;
				*((unsigned long *) ( *dst_line + (x << 3) + 4 )) = product1b;
				*((unsigned long *) ( *dst_line1 + (x << 3) )) = product2a;
				*((unsigned long *) ( *dst_line1 + (x << 3) + 4 )) = product2b;
			}
			x++;
		}

		/* We're done with one line, so we shift everything up */
		{
			unsigned char *temp = *src_line;
			*src_line = *src_line1;
			*src_line1 = *src_line2;
			*src_line2 = *src_line3;
			*src_line3 = temp;
		}

		/* Read next line */
		if (y + 3 >= XSAI_sourceHeight)
			//memset(src_line[3], 0, XSAI_sbpp * (240 + 4));
			memset(src_line[3], 0, 244 << XSAI_bitWidth );
		else {
			//bmp_select(src);
			//src_addr = bmp_read_line(src, y + 3);
			src_addr = (u32)(src + ((y+3) * (240 << XSAI_bitWidth)));
			for (j = 0; j < 240 << XSAI_bitWidth; j += 4)
				*((u32 *) ( *src_line3 + (j + XSAI_sbpp)) ) = *(u32*)(src_addr + j);
		}

		/* Write the 2 lines */
		//bmp_select(dst);
		//dst_addr = bmp_write_line(dst, y * 2);
		dst_addr = (u32)(dst + y * 2* (480 << XSAI_bitWidth));
		for (j = 0; j < 480 << XSAI_bitWidth; j += 4)
			//bmp_write32(dst_addr + j, *((unsigned long *) &(dst_line[0][j])));
			*(u32 *)(dst_addr + (j)) = *((unsigned long *) &(dst_line[0][j]) );
	
		//What the hell is an unwrite?  I can only guess that write_line
		//advances some kind of internal pointer, and this undoes it . .. 
		//So hopefully taking it out doesn't hurt.
		//bmp_unwrite_line(dst);
		//dst_addr = bmp_write_line(dst, y * 2 + 1);
		dst_addr = (u32)(dst + (y*2 +1) * (480 << XSAI_bitWidth));
		for (j = 0; j < 480 << XSAI_bitWidth; j += 4)
			//bmp_write32(dst_addr + j, *((unsigned long *) &(dst_line[1][j])));
			*(u32 *)(dst_addr + (j )) = *((unsigned long *) ( *dst_line1 + j));
		//bmp_unwrite_line(dst);


		y++;
	}
	
}//End of supereagle

void Super2xSaI(u8* src, u8 * dst)
{

	//int height = XSAI_sourceHeight, width = 240;
	//int destWidth = 240 << 1;
	//int destHeight = XSAI_sourceHeight << 1;

	int i, j;
	int x, y, w, h;
	unsigned long src_addr, dst_addr;
	
	
	if (!src || !dst)
		return;



	
	/* Read 3 src lines, with line[0] = 0 cause it's outside */
	/*
	bmp_select(src);
	for (i = 1; i < 4; i++) {
		src_addr = bmp_read_line(src, i - 1);
		for (j = 0; j < 240 * XSAI_sbpp; j += sizeof(long))
			*((unsigned long *) &src_line[i][j + XSAI_sbpp]) = bmp_read32(src_addr + j);

	}*/

	for (i = 1; i < 4; i++) {
		src_addr = (unsigned long)(src + (i - 1 ) * (240 << XSAI_bitWidth ));
		for (j = 0; j < 240 * XSAI_sbpp; j += 4)
			*((unsigned long *) &src_line[i][j + XSAI_sbpp]) = *(u32*)(src_addr + j);

	}

	x = 0, y = 0;

	for (h = 0; h < XSAI_sourceHeight; h++) {
		x = 0;
		for (w = 0; w < 240; w++) {
			unsigned long color4, color5, color6,
				color1, color2, color3,
				colorA0, colorA1, colorA2, colorA3, colorB0, colorB1, colorB2, colorB3, colorS1, colorS2, product1a, product1b, product2a, product2b;
			unsigned short *sbp;
			unsigned long *lbp;

//---------------------------------------  B0 B1 B2 B3
//                                         4  5  6  S2
//                                         1  2  3  S1
//                                         A0 A1 A2 A3

			if (PixelsPerMask == 2) {
				sbp = (unsigned short *) (*src_line);
				colorB0 = *(sbp + x);
				colorB1 = *(sbp + x+ 1);
				colorB2 = *(sbp + x + 2);
				colorB3 = *(sbp + x + 3);

				sbp = (unsigned short *) (*(src_line + 1));
				color4 = *(sbp + x);
				color5 = *(sbp + x+ 1);
				color6 = *(sbp + x + 2);
				colorS2 = *(sbp + x + 3);

				sbp = (unsigned short *) (*(src_line + 2));
				color1 = *(sbp + x);
				color2 = *(sbp + x+ 1);
				color3 = *(sbp + x + 2);
				colorS1 = *(sbp + x + 3);

				sbp = (unsigned short *) (src_line[3]);
				colorA0 = *(sbp + x);
				colorA1 = *(sbp + x+ 1);
				colorA2 = *(sbp + x + 2);
				colorA3 = *(sbp + x + 3);
			}
			else {
				lbp = (unsigned long *) (*src_line);
				colorB0 = *(lbp + x);
				colorB1 = *(lbp + x + 1);
				colorB2 = *(lbp + x + 2);
				colorB3 = *(lbp + x + 3);

				lbp = (unsigned long *) (*(src_line + 1));
				color4 = *(lbp + x);
				color5 = *(lbp + x + 1);
				color6 = *(lbp + x + 2);
				colorS2 = *(lbp + x + 3);

				lbp = (unsigned long *) (*(src_line + 2));
				color1 = *(lbp + x);
				color2 = *(lbp + x + 1);
				color3 = *(lbp + x + 2);
				colorS1 = *(lbp + x + 3);

				lbp = (unsigned long *) (src_line[3]);
				colorA0 = *(lbp + x);
				colorA1 = *(lbp + x + 1);
				colorA2 = *(lbp + x + 2);
				colorA3 = *(lbp + x + 3);
			}

//--------------------------------------
			if (color2 == color6 && color5 != color3) {
				product2b = color2;
				product1b = color2;
			}
			else if (color5 == color3 && color2 != color6) {
				product2b = color5;
				product1b = color5;
			}
			else if (color5 == color3 && color2 == color6) {
				int r = 0;

				r += GetResult(color6, color5, color1, colorA1);
				r += GetResult(color6, color5, color4, colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0) {
					product2b = color6;
					product1b = color6;
				}
				else if (r < 0) {
					product2b = color5;
					product1b = color5;
				}
				else {
					product2b = INTERPOLATE(color5, color6);
					product1b = product2b;
				}

			}
			else {
				if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
					product2b = Q_INTERPOLATE(color3, color3, color3, color2);
				else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
					product2b = Q_INTERPOLATE(color2, color2, color2, color3);
				else
					product2b = INTERPOLATE(color2, color3);					

				if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
					product1b = Q_INTERPOLATE(color6, color6, color6, color5);
				else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
					product1b = Q_INTERPOLATE(color6, color5, color5, color5);
				else
					product1b = INTERPOLATE(color5, color6);

			}

			if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
				product2a = INTERPOLATE(color2, color5);				
			else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
				product2a = INTERPOLATE(color2, color5);
			else
				product2a = color2;

			if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
				product1a = INTERPOLATE(color2, color5);
			else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
				product1a = INTERPOLATE(color2, color5);
			else
				product1a = color5;

			if (PixelsPerMask == 2) {
				*((unsigned long *) (&dst_line[0][x * 4])) = product1a | (product1b << 16);
				*((unsigned long *) (&dst_line[1][x * 4])) = product2a | (product2b << 16);
			}
			else {
				*((unsigned long *) (&dst_line[0][x * 8])) = product1a;
				*((unsigned long *) (&dst_line[0][x * 8 + 4])) = product1b;
				*((unsigned long *) (&dst_line[1][x * 8])) = product2a;
				*((unsigned long *) (&dst_line[1][x * 8 + 4])) = product2b;
			}
			x++;
		}

		/* We're done with one line, so we shift everything up */
		{
			unsigned char *temp = *src_line;
			*src_line = *(src_line + 1);
			*(src_line + 1) = *(src_line + 2);
			*(src_line + 2) = src_line[3];
			src_line[3] = temp;
		}

		
		/* Read next line */
		if (y + 3 >= XSAI_sourceHeight)
			memset(src_line[3], 0, XSAI_sbpp * (240 + 4));
		else {
			//bmp_select(src);
			//src_addr = bmp_read_line(src, y + 3);
			src_addr = (u32)(src + ((y+3) * (240 << XSAI_bitWidth)));
			for (j = 0; j < 240 * XSAI_sbpp; j += sizeof(long))
				*((u32 *) &(src_line[3][j + XSAI_sbpp])) = *(u32*)(src_addr + j);
		}

		/* Write the 2 lines */
		//bmp_select(dst);
		//dst_addr = bmp_write_line(dst, y * 2);
		dst_addr = (u32)(dst + y * 2* (480 << XSAI_bitWidth));
		for (j = 0; j < 480 * XSAI_sbpp; j += 4)
			//bmp_write32(dst_addr + j, *((unsigned long *) &(dst_line[0][j])));
			*(u32 *)(dst_addr + (j)) = *((unsigned long *) &(dst_line[0][j]));
	
		//What the hell is an unwrite?  I can only guess that write_line
		//advances some kind of internal pointer, and this undoes it . .. 
		//So hopefully taking it out doesn't hurt.
		//bmp_unwrite_line(dst);
		//dst_addr = bmp_write_line(dst, y * 2 + 1);
		dst_addr = (u32)(dst + (y*2 +1) * (480 << XSAI_bitWidth));
		for (j = 0; j < 480 * XSAI_sbpp; j += 4)
			//bmp_write32(dst_addr + j, *((unsigned long *) &(dst_line[1][j])));
			*(u32 *)(dst_addr + (j )) = *((unsigned long *) &(dst_line[1][j]));
		//bmp_unwrite_line(dst);


		y++;
	}
	
}