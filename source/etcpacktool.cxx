//// etcpack v2.74
//// 
//// NO WARRANTY 
//// 
//// BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE THE PROGRAM IS PROVIDED
//// "AS IS". ERICSSON MAKES NO REPRESENTATIONS OF ANY KIND, EXTENDS NO
//// WARRANTIES OR CONDITIONS OF ANY KIND; EITHER EXPRESS, IMPLIED OR
//// STATUTORY; INCLUDING, BUT NOT LIMITED TO, EXPRESS, IMPLIED OR
//// STATUTORY WARRANTIES OR CONDITIONS OF TITLE, MERCHANTABILITY,
//// SATISFACTORY QUALITY, SUITABILITY AND FITNESS FOR A PARTICULAR
//// PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
//// PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME
//// THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION. ERICSSON
//// MAKES NO WARRANTY THAT THE MANUFACTURE, SALE, OFFERING FOR SALE,
//// DISTRIBUTION, LEASE, USE OR IMPORTATION UNDER THE LICENSE WILL BE FREE
//// FROM INFRINGEMENT OF PATENTS, COPYRIGHTS OR OTHER INTELLECTUAL
//// PROPERTY RIGHTS OF OTHERS, AND THE VALIDITY OF THE LICENSE IS SUBJECT
//// TO YOUR SOLE RESPONSIBILITY TO MAKE SUCH DETERMINATION AND ACQUIRE
//// SUCH LICENSES AS MAY BE NECESSARY WITH RESPECT TO PATENTS, COPYRIGHT
//// AND OTHER INTELLECTUAL PROPERTY OF THIRD PARTIES.
//// 
//// FOR THE AVOIDANCE OF DOUBT THE PROGRAM (I) IS NOT LICENSED FOR; (II)
//// IS NOT DESIGNED FOR OR INTENDED FOR; AND (III) MAY NOT BE USED FOR;
//// ANY MISSION CRITICAL APPLICATIONS SUCH AS, BUT NOT LIMITED TO
//// OPERATION OF NUCLEAR OR HEALTHCARE COMPUTER SYSTEMS AND/OR NETWORKS,
//// AIRCRAFT OR TRAIN CONTROL AND/OR COMMUNICATION SYSTEMS OR ANY OTHER
//// COMPUTER SYSTEMS AND/OR NETWORKS OR CONTROL AND/OR COMMUNICATION
//// SYSTEMS ALL IN WHICH CASE THE FAILURE OF THE PROGRAM COULD LEAD TO
//// DEATH, PERSONAL INJURY, OR SEVERE PHYSICAL, MATERIAL OR ENVIRONMENTAL
//// DAMAGE. YOUR RIGHTS UNDER THIS LICENSE WILL TERMINATE AUTOMATICALLY
//// AND IMMEDIATELY WITHOUT NOTICE IF YOU FAIL TO COMPLY WITH THIS
//// PARAGRAPH.
//// 
//// IN NO EVENT WILL ERICSSON, BE LIABLE FOR ANY DAMAGES WHATSOEVER,
//// INCLUDING BUT NOT LIMITED TO PERSONAL INJURY, ANY GENERAL, SPECIAL,
//// INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN
//// CONNECTION WITH THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
//// NOT LIMITED TO LOSS OF PROFITS, BUSINESS INTERUPTIONS, OR ANY OTHER
//// COMMERCIAL DAMAGES OR LOSSES, LOSS OF DATA OR DATA BEING RENDERED
//// INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF
//// THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) REGARDLESS OF THE
//// THEORY OF LIABILITY (CONTRACT, TORT OR OTHERWISE), EVEN IF SUCH HOLDER
//// OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//// 
//// (C) Ericsson AB 2005-2013. All Rights Reserved.
//// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include <time.h>
#include <sys/timeb.h>
#include "image.h"
#include "../etcpack.h"

// Typedefs
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;

#define EXHAUSTIVE_CODE_ACTIVE 1 // must be the same value as in etcpack.cxx

// The ETC2 package of codecs includes the following codecs:
//
// codec                                             enum
// --------------------------------------------------------
// GL_COMPRESSED_R11_EAC                            0x9270
// GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
// GL_COMPRESSED_RG11_EAC                           0x9272
// GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
// GL_COMPRESSED_RGB8_ETC2                          0x9274
// GL_COMPRESSED_SRGB8_ETC2                         0x9275
// GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
// GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
// GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
// GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279
// 
// The older codec ETC1 is not included in the package 
// GL_ETC1_RGB8_OES                                 0x8d64
// but since ETC2 is backwards compatible an ETC1 texture can
// be decoded using the RGB8_ETC2 enum (0x9274)
// 
// In a PKM-file, the codecs are stored using the following identifiers
// 
// identifier                         value               codec
// --------------------------------------------------------------------
// ETC1_RGB_NO_MIPMAPS                  0                 GL_ETC1_RGB8_OES
// ETC2PACKAGE_RGB_NO_MIPMAPS           1                 GL_COMPRESSED_RGB8_ETC2
// ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD      2, not used       -
// ETC2PACKAGE_RGBA_NO_MIPMAPS          3                 GL_COMPRESSED_RGBA8_ETC2_EAC
// ETC2PACKAGE_RGBA1_NO_MIPMAPS         4                 GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
// ETC2PACKAGE_R_NO_MIPMAPS             5                 GL_COMPRESSED_R11_EAC
// ETC2PACKAGE_RG_NO_MIPMAPS            6                 GL_COMPRESSED_RG11_EAC
// ETC2PACKAGE_R_SIGNED_NO_MIPMAPS      7                 GL_COMPRESSED_SIGNED_R11_EAC
// ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS     8                 GL_COMPRESSED_SIGNED_RG11_EAC
//
// In the code, the identifiers are not always used strictly. For instance, the
// identifier ETC2PACKAGE_R_NO_MIPMAPS is sometimes used for both the unsigned
// (GL_COMPRESSED_R11_EAC) and signed (GL_COMPRESSED_SIGNED_R11_EAC) version of 
// the codec.
// 
enum {ETC1_RGB_NO_MIPMAPS,ETC2PACKAGE_RGB_NO_MIPMAPS,ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD,ETC2PACKAGE_RGBA_NO_MIPMAPS,ETC2PACKAGE_RGBA1_NO_MIPMAPS,ETC2PACKAGE_R_NO_MIPMAPS,ETC2PACKAGE_RG_NO_MIPMAPS,ETC2PACKAGE_R_SIGNED_NO_MIPMAPS,ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS,ETC2PACKAGE_sRGB_NO_MIPMAPS,ETC2PACKAGE_sRGBA_NO_MIPMAPS,ETC2PACKAGE_sRGBA1_NO_MIPMAPS};
enum {MODE_COMPRESS, MODE_UNCOMPRESS, MODE_PSNR};
enum {SPEED_SLOW, SPEED_FAST, SPEED_MEDIUM};
enum {METRIC_PERCEPTUAL, METRIC_NONPERCEPTUAL};
enum {CODEC_ETC, CODEC_ETC2};

// Global variables
static int mode = MODE_COMPRESS;
static int speed = SPEED_FAST;
static int metric = METRIC_PERCEPTUAL;
static int codec = CODEC_ETC2;
static int format = ETC2PACKAGE_RGB_NO_MIPMAPS;
static int verbose = true;
static int formatSigned = 0;
static int ktxFile=0;
static bool first_time_message = true;

static int scramble[4] = {3, 2, 0, 1};
static int unscramble[4] = {2, 3, 1, 0};

typedef struct KTX_header_t
{
	unsigned char identifier[12];
	unsigned int endianness;
	unsigned int glType;
	unsigned int glTypeSize;
	unsigned int glFormat;
	unsigned int glInternalFormat;
	unsigned int glBaseInternalFormat;
	unsigned int pixelWidth;
	unsigned int pixelHeight;
	unsigned int pixelDepth;
	unsigned int numberOfArrayElements;
	unsigned int numberOfFaces;
	unsigned int numberOfMipmapLevels;
	unsigned int bytesOfKeyValueData;
} 
KTX_header;
#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }

#define KTX_ENDIAN_REF      (0x04030201)
#define KTX_ENDIAN_REF_REV  (0x01020304)

enum {GL_R=0x1903,GL_RG=0x8227,GL_RGB=0x1907,GL_RGBA=0x1908};
#define GL_SRGB                                          0x8C40
#define GL_SRGB8                                         0x8C41
#define GL_SRGB8_ALPHA8                                  0x8C43
#define GL_ETC1_RGB8_OES                                 0x8d64
#define GL_COMPRESSED_R11_EAC                            0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
#define GL_COMPRESSED_RG11_EAC                           0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
#define GL_COMPRESSED_RGB8_ETC2                          0x9274
#define GL_COMPRESSED_SRGB8_ETC2                         0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279

int ktx_identifier[] = KTX_IDENTIFIER_REF;

// Tests if a file exists.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
bool fileExist(const char *filename)
{
	FILE *f=NULL;
	if((f=fopen(filename,"rb"))!=NULL)
	{
		fclose(f);
		return true;
	}
	return false;
}

// Expand source image so that it is divisible by a factor of four in the x-dimension.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
bool expandToWidthDivByFour(uint8_t *&img, int width, int height, int &expandedwidth, int &expandedheight, int bitrate)
{
	int wdiv4;
	int xx, yy;
	uint8_t *newimg;

	wdiv4 = width /4;
	if( !(wdiv4 *4 == width) )
	{
     	expandedwidth = (wdiv4 + 1)*4;
		expandedheight = height;
	    newimg=(uint8_t*) malloc(3*expandedwidth*expandedheight*bitrate/8);
		if(!newimg)
		{
			printf("Could not allocate memory to expand width\n");
			return false;
		}

		// First copy image
		for(yy = 0; yy<height; yy++)
		{
			for(xx = 0; xx < width; xx++)
			{
				//we have 3*bitrate/8 bytes for each pixel..
				for(int i=0; i<3*bitrate/8; i++) 
				{
					newimg[(yy * expandedwidth+ xx)*3*bitrate/8 + i] = img[(yy * width+xx)*3*bitrate/8 + i];

				}
			}
		}

		// Then make the last column of pixels the same as the previous column.

		for(yy = 0; yy< height; yy++)
		{
			for(xx = width; xx < expandedwidth; xx++)
			{
				for(int i=0; i<3*bitrate/8; i++) 
				{
					newimg[(yy * expandedwidth+xx)*3*bitrate/8 + i] = img[(yy * width+(width-1))*3*bitrate/8 + i];
				}
			}
		}

		// Now free the old image
		free(img);

		// Use the new image
		img = newimg;

		return true;
	}
	else
	{
		printf("Image already of even width\n");
		expandedwidth = width;
		expandedheight = height;
		return false;
	}
}

// Expand source image so that it is divisible by a factor of four in the y-dimension.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
bool expandToHeightDivByFour(uint8_t *&img, int width, int height, int &expandedwidth, int &expandedheight, int bitrate)
{
	int hdiv4;
	int xx, yy;
	int numlinesmissing;
	uint8_t *newimg;

	hdiv4 = height/4;

	if( !(hdiv4 * 4 == height) )
	{
		expandedwidth = width;
		expandedheight = (hdiv4 + 1) * 4;
		numlinesmissing = expandedheight - height;
		newimg=(uint8_t*)malloc(3*expandedwidth*expandedheight*bitrate/8);
		if(!newimg)
		{
			printf("Could not allocate memory to expand height\n");
			return false;
		}
		
		// First copy image. No need to reformat data.

		for(xx = 0; xx<3*width*height*bitrate/8; xx++)
			newimg[xx] = img[xx];

		// Then copy up to three lines.

		for(yy = height; yy < height + numlinesmissing; yy++)
		{
			for(xx = 0; xx<width; xx++)
			{
				for(int i=0; i<3*bitrate/8; i++) 
				{
					newimg[(yy*width+xx)*3*bitrate/8 + i] = img[((height-1)*width+xx)*3*bitrate/8 + i];
				}
			}
		}

		// Now free the old image;
		free(img);

		// Use the new image:
		img = newimg;

		return true;

	}
	else
	{
		printf("Image height already divisible by four.\n");
		expandedwidth = width;
		expandedheight = height;
		return true;
	}
}

// Find the position of a file extension such as .ppm or .pkm
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
int find_pos_of_extension(char *src)
{
	int q=strlen(src);
	while(q>=0)		// find file name extension
	{
		if(src[q]=='.') break;
		q--;
	}
	if(q<0) 
		return -1;
	else
		return q;
}

// Read source file. Does conversion if file format is not .ppm.
// Will expand file to be divisible by four in the x- and y- dimension.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
bool readSrcFile(char *filename,uint8_t *&img,int &width,int &height, int &expandedwidth, int &expandedheight)
{
	int w1,h1;
	int wdiv4, hdiv4;
	char str[255];


	// Delete temp file if it exists.
	if(fileExist("tmp.ppm"))
	{
		sprintf(str, "del tmp.ppm\n");
		system(str);
	}

	int q = find_pos_of_extension(filename);
	if(!strcmp(&filename[q],".ppm")) 
	{
		// Already a .ppm file. Just copy. 
		sprintf(str,"copy %s tmp.ppm \n", filename);
		printf("Copying source file:%s to tmp.ppm\n", filename);
	}
	else
	{
		// Converting from other format to .ppm 
		// 
		// Use your favorite command line image converter program,
		// for instance Image Magick. Just make sure the syntax can
		// be written as below:
		// 
		// C:\convert.exe source.jpg dest.ppm
		//
		sprintf(str,"convert %s tmp.ppm\n", filename);
		printf("Converting source file from %s to .ppm\n", filename);
	}
	// Execute system call
	system(str);

	int bitrate=8;
	if(format==ETC2PACKAGE_RG_NO_MIPMAPS)
		bitrate=16;
	if(!fReadPPM("tmp.ppm",w1,h1,img,bitrate))
    {
        printf("Could not read tmp.ppm file\n");
        exit(1);
    }
	width=w1;
	height=h1;
	system("del tmp.ppm");

	// Width must be divisible by 4 and height must be
	// divisible by 4. Otherwise, we will expand the image

	wdiv4 = width / 4;
	hdiv4 = height / 4;

	expandedwidth = width;
	expandedheight = height;

	if( !(wdiv4 * 4 == width) )
	{
		printf(" Width = %d is not divisible by four... ", width);
		printf(" expanding image in x-dir... ");
		if(expandToWidthDivByFour(img, width, height, expandedwidth, expandedheight,bitrate))
		{
			printf("OK.\n");
		}
		else
		{
			printf("\n Error: could not expand image\n");
			return false;
		}
	}
	if( !(hdiv4 * 4 == height))
	{
		printf(" Height = %d is not divisible by four... ", height);
		printf(" expanding image in y-dir...");
		if(expandToHeightDivByFour(img, expandedwidth, height, expandedwidth, expandedheight,bitrate))
		{
			printf("OK.\n");
		}
		else
		{
			printf("\n Error: could not expand image\n");
			return false;
		}
	}
	if(!(expandedwidth == width && expandedheight == height))
	   printf("Active pixels: %dx%d. Expanded image: %dx%d\n",width,height,expandedwidth,expandedheight);
	return true;
}

// Reads a file without expanding it to be divisible by 4.
// Is used when doing PSNR calculation between two files.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
bool readSrcFileNoExpand(char *filename,uint8_t *&img,int &width,int &height)
{
	int w1,h1;
	char str[255];


	// Delete temp file if it exists.
	if(fileExist("tmp.ppm"))
	{
		sprintf(str, "del tmp.ppm\n");
		system(str);
	}


	int q = find_pos_of_extension(filename);
	if(!strcmp(&filename[q],".ppm")) 
	{
		// Already a .ppm file. Just copy. 
		sprintf(str,"copy %s tmp.ppm \n", filename);
		printf("Copying source file:%s to tmp.ppm\n", filename);
	}
	else
	{
		// Converting from other format to .ppm 
		// 
		// Use your favorite command line image converter program,
		// for instance Image Magick. Just make sure the syntax can
		// be written as below:
		// 
		// C:\convert.exe source.jpg dest.ppm
		//
		sprintf(str,"convert %s tmp.ppm\n", filename);
//		printf("Converting source file from %s to .ppm\n", filename);
	}
	// Execute system call
	system(str);

	if(fReadPPM("tmp.ppm",w1,h1,img,8))
	{
		width=w1;
		height=h1;
		system("del tmp.ppm");

		return true;
	}
	return false;
}

// Parses the arguments from the command line.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void readArguments(int argc,char *argv[],char* src,char *dst)
{
	int q;

	//new code!! do this in a more nicer way!
	bool srcfound=false,dstfound=false;
	for(int i=1; i<argc; i++) 
	{
		//loop through the arguments!
		//first check for flags..
		if(argv[i][0]=='-') 
		{
			if(i==argc-1) 
			{
				printf("flag missing argument: %s!\n", argv[i]);
				exit(1);
			}
			//handle speed flag
			if(!strcmp(argv[i],"-s"))  
			{
				// We have argument -s. Now check for slow, medium or fast.
				if(!strcmp(argv[i+1],"slow")) 
					speed = SPEED_SLOW;
				else if(!strcmp(argv[i+1],"medium")) 
					speed = SPEED_MEDIUM;
				else if(!strcmp(argv[i+1],"fast")) 
					speed = SPEED_FAST;
				else 
				{
					printf("Error: %s not part of flag %s\n",argv[i+1], argv[i]);
					exit(1);
				}
			}
			//handle verbose flag
			else if(!strcmp(argv[i],"-v"))  
			{
				// We have argument -s. Now check for slow, medium or fast.
				if(!strcmp(argv[i+1],"off")) 
					verbose = false;
				else if(!strcmp(argv[i+1],"on")) 
					verbose = true;
				else 
				{
					printf("Error: %s not part of flag %s\n",argv[i+1], argv[i]);
					exit(1);
				}
			}
			//error metric flag
			else if(!strcmp(argv[i],"-e")) 	
			{
				// We have argument -e. Now check for perceptual or nonperceptual
				if(!strcmp(argv[i+1],"perceptual")) 
					metric = METRIC_PERCEPTUAL;
				else if(!strcmp(argv[i+1],"nonperceptual")) 
					metric = METRIC_NONPERCEPTUAL;
				else 
				{
					printf("Error: %s not part of flag %s\n",argv[i+1], argv[i]);
					exit(1);
				}
			}
			//codec flag
			else if(!strcmp(argv[i],"-c")) 
			{
				// We have argument -c. Now check for perceptual or nonperceptual
				if(!strcmp(argv[i+1],"etc") || !strcmp(argv[i+1],"etc1"))
					codec = CODEC_ETC;
				else if(!strcmp(argv[i+1],"etc2")) 
					codec = CODEC_ETC2;
				else 
				{
					printf("Error: %s not part of flag %s\n",argv[i+1], argv[i]);
					exit(1);
				}
			}
			//format flag
			else if(!strcmp(argv[i],"-f")) 
			{
				if(!strcmp(argv[i+1],"R"))
					format=ETC2PACKAGE_R_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"RG"))
					format=ETC2PACKAGE_RG_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"R_signed")) 
				{
					format=ETC2PACKAGE_R_NO_MIPMAPS;
					formatSigned=1;
				}
				else if(!strcmp(argv[i+1],"RG_signed")) 
				{
					format=ETC2PACKAGE_RG_NO_MIPMAPS;
					formatSigned=1;
				}
				else if(!strcmp(argv[i+1],"RGB"))
					format=ETC2PACKAGE_RGB_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"sRGB"))
					format=ETC2PACKAGE_sRGB_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"RGBA")||!strcmp(argv[i+1],"RGBA8"))
					format=ETC2PACKAGE_RGBA_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"sRGBA")||!strcmp(argv[i+1],"sRGBA8"))
					format=ETC2PACKAGE_sRGBA_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"RGBA1"))
					format=ETC2PACKAGE_RGBA1_NO_MIPMAPS;
				else if(!strcmp(argv[i+1],"sRGBA1"))
					format=ETC2PACKAGE_sRGBA1_NO_MIPMAPS;
				else 
				{
					printf("Error: %s not part of flag %s\n",argv[i+1], argv[i]);
					exit(1);
				}
			}
			else if(!strcmp(argv[i],"-p")) 
			{
				mode=MODE_PSNR;
				i--; //ugly way of negating the increment of i done later because -p doesn't have an argument.
			}
			else 
			{
				printf("Error: cannot interpret flag %s %s\n",argv[i], argv[i+1]);
				exit(1);
			}
			//don't read the flag argument next iteration..
			i++;
		}
		//this isn't a flag, so must be src or dst
		else 
		{
			if(srcfound&&dstfound) 
			{
				printf("too many arguments! expecting src, dst; found %s, %s, %s\n",src,dst,argv[i]);
				exit(1);
			}
			else if(srcfound) 
			{
				strcpy(dst,argv[i]);
				dstfound=true;
			}
			else 
			{
				strcpy(src,argv[i]);
				srcfound=true;
			}
		}
	}
	if(!srcfound&&dstfound) 
	{
		printf("too few arguments! expecting src, dst\n");
		exit(1);
	}
	if(mode==MODE_PSNR)
		return;
	//check source/destination.. is this compression or decompression?
	q = find_pos_of_extension(src);
	if(q<0) 
	{
		printf("invalid source file: %s\n",src);
		exit(1);
	}

	// If we have etcpack img.pkm img.any

	if(!strncmp(&src[q],".pkm",4)) 
	{
		// First argument is .pkm. Decompress. 
		mode = MODE_UNCOMPRESS;			// uncompress from binary file format .pkm
	}
	else if(!strncmp(&src[q],".ktx",4)) 
	{
		// First argument is .ktx. Decompress. 
		mode = MODE_UNCOMPRESS;			// uncompress from binary file format .pkm
		ktxFile=true;
		printf("decompressing ktx\n");
	}
	else
	{
		// The first argument was not .pkm. The second argument must then be .pkm.
		q = find_pos_of_extension(dst);
		if(q<0) 
		{
			printf("invalid destination file: %s\n",src);
			exit(1);
		}
		if(!strncmp(&dst[q],".pkm",4)) 
		{
			// Second argument is .pkm. Compress. 
			mode = MODE_COMPRESS;			// compress to binary file format .pkm
		}
		else if(!strncmp(&dst[q],".ktx",4)) 
		{
			// Second argument is .ktx. Compress. 
			ktxFile=true;
			mode = MODE_COMPRESS;			// compress to binary file format .pkm
			printf("compressing to ktx\n");
		}
		else 
		{
			printf("source or destination must be a .pkm or .ktx file\n");
			exit(1);
		}
	}
	//do some sanity check stuff..
	if(codec==CODEC_ETC&&format!=ETC2PACKAGE_RGB_NO_MIPMAPS) 
	{
		printf("ETC1 codec only supports RGB format\n");
		exit(1);
	}
	else if(codec==CODEC_ETC)
		format=ETC1_RGB_NO_MIPMAPS;
}

// Read a word in big endian style
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void read_big_endian_2byte_word(unsigned short *blockadr, FILE *f)
{
	uint8_t bytes[2];
	unsigned short block;

	fread(&bytes[0], 1, 1, f);
	fread(&bytes[1], 1, 1, f);

	block = 0;
	block |= bytes[0];
	block = block << 8;
	block |= bytes[1];

	blockadr[0] = block;
}

// Read a word in big endian style
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void read_big_endian_4byte_word(unsigned int *blockadr, FILE *f)
{
	uint8_t bytes[4];
	unsigned int block;

	fread(&bytes[0], 1, 1, f);
	fread(&bytes[1], 1, 1, f);
	fread(&bytes[2], 1, 1, f);
	fread(&bytes[3], 1, 1, f);

	block = 0;
	block |= bytes[0];
	block = block << 8;
	block |= bytes[1];
	block = block << 8;
	block |= bytes[2];
	block = block << 8;
	block |= bytes[3];

	blockadr[0] = block;
}

// Write a word in big endian style
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void write_big_endian_2byte_word(unsigned short *blockadr, FILE *f)
{
	uint8_t bytes[2];
	unsigned short block;

	block = blockadr[0];

	bytes[0] = (block >> 8) & 0xff;
	bytes[1] = (block >> 0) & 0xff;

	fwrite(&bytes[0],1,1,f);
	fwrite(&bytes[1],1,1,f);
}


// Write a word in big endian style
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void write_big_endian_4byte_word(unsigned int *blockadr, FILE *f)
{
	uint8_t bytes[4];
	unsigned int block;

	block = blockadr[0];

	bytes[0] = (block >> 24) & 0xff;
	bytes[1] = (block >> 16) & 0xff;
	bytes[2] = (block >> 8) & 0xff;
	bytes[3] = (block >> 0) & 0xff;

	fwrite(&bytes[0],1,1,f);
	fwrite(&bytes[1],1,1,f);
	fwrite(&bytes[2],1,1,f);
	fwrite(&bytes[3],1,1,f);
}

// Reads alpha data
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void readAlpha(uint8_t* &data, int &width, int &height, int &extendedwidth, int &extendedheight) 
{
	//width and height are already known..?
	uint8_t* tempdata;
	int wantedBitDepth;
	if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
	{
		wantedBitDepth=8;
	}
	else if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
	{
		wantedBitDepth=16;
	}
	else 
	{
		printf("invalid format for alpha reading!\n");
		exit(1);
	}
	fReadPGM("alpha.pgm",width,height,tempdata,wantedBitDepth);
	extendedwidth=4*((width+3)/4);
	extendedheight=4*((height+3)/4);

	if(width==extendedwidth&&height==extendedheight) 
	{
		data=tempdata;
	}
	else 
	{
		data = (uint8_t*)malloc(extendedwidth*extendedheight*wantedBitDepth/8);
		uint8_t last=0;
		uint8_t lastlast=0;
		for(int x=0; x<extendedwidth; x++) 
		{
			for(int y=0; y<extendedheight; y++) 
			{
				if(wantedBitDepth==8) 
				{
					if(x<width&&y<height) 
					{
						last = tempdata[x+y*width];
					}
					data[x+y*extendedwidth]=last;
				}
				else 
				{
					if(x<width&&y<height) 
					{
						last = tempdata[(x+y*width)*2];
						lastlast = tempdata[(x+y*width)*2+1];						
					}
					data[(x+y*extendedwidth)*2]=last;
					data[(x+y*extendedwidth)*2+1]=lastlast;
				}
			}
		}
	}
	if(format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
	{
		for(int x=0; x<extendedwidth; x++) 
		{
			for(int y=0; y<extendedheight; y++) 
			{
				if(data[x+y*extendedwidth]<128)
					data[x+y*extendedwidth]=0;
				else
					data[x+y*extendedwidth]=255;
			}
		}
	}
}

// Calculate weighted PSNR
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
double calculateWeightedPSNR(uint8_t *lossyimg, uint8_t *origimg, int width, int height, double w1, double w2, double w3)
{
	// Note: This calculation of PSNR uses the formula
	//
	// PSNR = 10 * log_10 ( 255^2 / wMSE ) 
	// 
	// where the wMSE is calculated as
	//
	// 1/(N*M) * sum ( ( w1*(R' - R)^2 + w2*(G' - G)^2 + w3*(B' - B)^2) ) 
	//
	// typical weights are  0.299,   0.587,   0.114  for perceptually weighted PSNR and
  //                     1.0/3.0, 1.0/3.0, 1.0/3.0 for nonweighted PSNR

	int x,y;
	double wMSE;
	double PSNR;
	double err;
	wMSE = 0;

	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			err = lossyimg[y*width*3+x*3+0] - origimg[y*width*3+x*3+0];
		    wMSE = wMSE + (w1*(err * err));
			err = lossyimg[y*width*3+x*3+1] - origimg[y*width*3+x*3+1];
		    wMSE = wMSE + (w2*(err * err));
			err = lossyimg[y*width*3+x*3+2] - origimg[y*width*3+x*3+2];
		    wMSE = wMSE + (w3*(err * err));
		}
	}
	wMSE = wMSE / (width * height);
	if(wMSE == 0)
	{
		printf("********************************************************************\n");
		printf("There is no difference at all between image files --- infinite PSNR.\n");
		printf("********************************************************************\n");
	}
	PSNR = 10*log((1.0*255*255)/wMSE)/log(10.0);
	return PSNR;
}

// Calculate unweighted PSNR (weights are (1,1,1))
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
double calculatePSNR(uint8_t *lossyimg, uint8_t *origimg, int width, int height)
{
	// Note: This calculation of PSNR uses the formula
	//
	// PSNR = 10 * log_10 ( 255^2 / MSE ) 
	// 
	// where the MSE is calculated as
	//
	// 1/(N*M) * sum ( 1/3 * ((R' - R)^2 + (G' - G)^2 + (B' - B)^2) ) 
	//
	// The reason for having the 1/3 factor is the following:
	// Presume we have a grayscale image, that is acutally just the red component 
	// of a color image.. The squared error is then (R' - R)^2.
	// Assume that we have a certain signal to noise ratio, say 30 dB. If we add
	// another two components (say green and blue) with the same signal to noise 
	// ratio, we want the total signal to noise ratio be the same. For the
	// squared error to remain constant we must divide by three after adding
	// together the squared errors of the components. 

  return calculateWeightedPSNR(lossyimg, origimg, width, height, (1.0/3.0), (1.0/3.0), (1.0/3.0));
}

// Decompresses a file
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void uncompressFile(char *srcfile, uint8_t* &img, uint8_t *&alphaimg, int& active_width, int& active_height)
{
	FILE *f;
	int width,height;
	unsigned int block_part1, block_part2;
	uint8_t *newimg, *newalphaimg, *alphaimg2;
	unsigned short w, h;
	int xx, yy;
	unsigned char magic[4];
	unsigned char version[2];
	unsigned short texture_type;
    f = fopen(srcfile, "rb");
	if(!f)
    {
        printf("Error: could not open <%s>.\n", srcfile);
        exit(1);
    }
	if(ktxFile) 
	{
		//read ktx header..
		KTX_header header;
		fread(&header,sizeof(KTX_header),1,f);
		//read size parameter, which we don't actually need..
		unsigned int bitsize;
		fread(&bitsize,sizeof(unsigned int),1,f);

		active_width=header.pixelWidth;
		active_height = header.pixelHeight;
		w = ((active_width+3)/4)*4;
		h = ((active_height+3)/4)*4;
		width=w;
		height=h;

		if(header.glInternalFormat==GL_COMPRESSED_SIGNED_R11_EAC) 
		{
			format=ETC2PACKAGE_R_NO_MIPMAPS;
			formatSigned=1;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_R11_EAC) 
		{
			format=ETC2PACKAGE_R_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_SIGNED_RG11_EAC) 
		{
			format=ETC2PACKAGE_RG_NO_MIPMAPS;
			formatSigned=1;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_RG11_EAC) 
		{
			format=ETC2PACKAGE_RG_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_RGB8_ETC2) 
		{
			format=ETC2PACKAGE_RGB_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_SRGB8_ETC2) 
		{
			format=ETC2PACKAGE_sRGB_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_RGBA8_ETC2_EAC) 
		{
			format=ETC2PACKAGE_RGBA_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC) 
		{
			format=ETC2PACKAGE_sRGBA_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2) 
		{
			format=ETC2PACKAGE_RGBA1_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2) 
		{
			format=ETC2PACKAGE_sRGBA1_NO_MIPMAPS;
		}
		else if(header.glInternalFormat==GL_ETC1_RGB8_OES) 
		{
			format=ETC1_RGB_NO_MIPMAPS;
			codec=CODEC_ETC;
		}
		else 
		{
			printf("ktx file has unknown glInternalFormat (not etc compressed)!\n");
			exit(1);
		}
	}
	else 
	{
		// Read magic nunmber
		fread(&magic[0], sizeof(unsigned char), 1, f);
		fread(&magic[1], sizeof(unsigned char), 1, f);
		fread(&magic[2], sizeof(unsigned char), 1, f);
		fread(&magic[3], sizeof(unsigned char), 1, f);
		if(!(magic[0] == 'P' && magic[1] == 'K' && magic[2] == 'M' && magic[3] == ' '))
		{
			printf("\n\n The file %s is not a .pkm file.\n",srcfile);
			exit(1);
		}
	
		// Read version
		fread(&version[0], sizeof(unsigned char), 1, f);
		fread(&version[1], sizeof(unsigned char), 1, f);
		if( version[0] == '1' && version[1] == '0' )
		{

			// Read texture type
			read_big_endian_2byte_word(&texture_type, f);
			if(!(texture_type == ETC1_RGB_NO_MIPMAPS))
			{
				printf("\n\n The file %s (of version %c.%c) does not contain a texture of known format.\n", srcfile, version[0],version[1]);
				printf("Known formats: ETC1_RGB_NO_MIPMAPS.\n");
				exit(1);
			}
		}
		else if( version[0] == '2' && version[1] == '0' )
		{
			// Read texture type
			read_big_endian_2byte_word(&texture_type, f);
			if(texture_type==ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS) 
			{
				texture_type=ETC2PACKAGE_RG_NO_MIPMAPS;
				formatSigned=1;
				//printf("Decompressing 2-channel signed data\n");
			}
			if(texture_type==ETC2PACKAGE_R_SIGNED_NO_MIPMAPS) 
			{
				texture_type=ETC2PACKAGE_R_NO_MIPMAPS;
				formatSigned=1;
				//printf("Decompressing 1-channel signed data\n");
			}
    if(texture_type==ETC2PACKAGE_sRGB_NO_MIPMAPS)
    {
      // The SRGB formats are decoded just as RGB formats -- use RGB format for decompression.
      texture_type=ETC2PACKAGE_RGB_NO_MIPMAPS;
    }
    if(texture_type==ETC2PACKAGE_sRGBA_NO_MIPMAPS)
    {
      // The SRGB formats are decoded just as RGB formats -- use RGB format for decompression.
      texture_type=ETC2PACKAGE_RGBA_NO_MIPMAPS;
    }
    if(texture_type==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
    {
      // The SRGB formats are decoded just as RGB formats -- use RGB format for decompression.
      texture_type=ETC2PACKAGE_sRGBA1_NO_MIPMAPS;
    }
			if(texture_type==ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD) 
			{
				printf("\n\nThe file %s contains a compressed texture created using an old version of ETCPACK.\n",srcfile);
				printf("decompression is not supported in this version.\n");
				exit(1);
			}
			if(!(texture_type==ETC2PACKAGE_RGB_NO_MIPMAPS||texture_type==ETC2PACKAGE_sRGB_NO_MIPMAPS||texture_type==ETC2PACKAGE_RGBA_NO_MIPMAPS||texture_type==ETC2PACKAGE_sRGBA_NO_MIPMAPS||texture_type==ETC2PACKAGE_R_NO_MIPMAPS||texture_type==ETC2PACKAGE_RG_NO_MIPMAPS||texture_type==ETC2PACKAGE_RGBA1_NO_MIPMAPS||texture_type==ETC2PACKAGE_sRGBA1_NO_MIPMAPS))
			{
				printf("\n\n The file %s does not contain a texture of known format.\n", srcfile);
				printf("Known formats: ETC2PACKAGE_RGB_NO_MIPMAPS.\n");
				exit(1);
			}
		}
		else
		{
			printf("\n\n The file %s is not of version 1.0 or 2.0 but of version %c.%c.\n",srcfile, version[0], version[1]);
			printf("Aborting.\n");
			exit(1);
		}
		format=texture_type;
		printf("textype: %d\n",texture_type);
		// ETC2 is backwards compatible, which means that an ETC2-capable decompressor can also handle
		// old ETC1 textures without any problems. Thus a version 1.0 file with ETC1_RGB_NO_MIPMAPS and a 
		// version 2.0 file with ETC2PACKAGE_RGB_NO_MIPMAPS can be handled by the same ETC2-capable decompressor

		// Read how many pixels the blocks make up

		read_big_endian_2byte_word(&w, f);
		read_big_endian_2byte_word(&h, f);
		width = w;
		height = h;

		// Read how many pixels contain active data (the rest are just
		// for making sure we have a 4*a x 4*b size).

		read_big_endian_2byte_word(&w, f);
		read_big_endian_2byte_word(&h, f);
		active_width = w;
		active_height = h;
	}
	printf("Width = %d, Height = %d\n",width, height);
	printf("active pixel area: top left %d x %d area.\n",active_width, active_height);

	if(format==ETC2PACKAGE_RG_NO_MIPMAPS)
		img=(uint8_t*)malloc(3*width*height*2);
	else
		img=(uint8_t*)malloc(3*width*height);
	if(!img)
	{
		printf("Error: could not allocate memory\n");
		exit(0);
	}
	if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_R_NO_MIPMAPS||format==ETC2PACKAGE_RG_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
	{
		//printf("alpha channel decompression\n");
		alphaimg=(uint8_t*)malloc(width*height*2);
		if(!alphaimg)
		{
			printf("Error: could not allocate memory for alpha\n");
			exit(0);
		}
	}
	if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
	{
		alphaimg2=(uint8_t*)malloc(width*height*2);
		if(!alphaimg2)
		{
			printf("Error: could not allocate memory\n");
			exit(0);
		}
	}

	for(int y=0;y<height/4;y++)
	{
		for(int x=0;x<width/4;x++)
		{
			//decode alpha channel for RGBA
			if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS) 
			{
				uint8_t alphablock[8];
				fread(alphablock,1,8,f);
				decompressBlockAlpha(alphablock,alphaimg,width,4*x,4*y);
			}
			//color channels for most normal modes
			if(format!=ETC2PACKAGE_R_NO_MIPMAPS&&format!=ETC2PACKAGE_RG_NO_MIPMAPS) 
			{
				//we have normal ETC2 color channels, decompress these
				read_big_endian_4byte_word(&block_part1,f);
				read_big_endian_4byte_word(&block_part2,f);
				if(format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
					decompressBlockETC21BitAlpha(block_part1, block_part2,img,alphaimg,width,4*x,4*y);
				else
					decompressBlockETC2(block_part1, block_part2,img,width,4*x,4*y);		
			}
			//one or two 11-bit alpha channels for R or RG.
			if(format==ETC2PACKAGE_R_NO_MIPMAPS||format==ETC2PACKAGE_RG_NO_MIPMAPS) 
			{
				uint8_t alphablock[8];
				fread(alphablock,1,8,f);
				decompressBlockAlpha16bit(alphablock,alphaimg,width,4*x,4*y,formatSigned);
			}
			if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
			{
				uint8_t alphablock[8];
				fread(alphablock,1,8,f);
				decompressBlockAlpha16bit(alphablock,alphaimg2,width,4*x,4*y,formatSigned);
			}
		}
	}
	if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
	{
		for(int y=0;y<height;y++)
		{
			for(int x=0;x<width;x++)
			{
				img[6*(y*width+x)]=alphaimg[2*(y*width+x)];
				img[6*(y*width+x)+1]=alphaimg[2*(y*width+x)+1];
				img[6*(y*width+x)+2]=alphaimg2[2*(y*width+x)];
				img[6*(y*width+x)+3]=alphaimg2[2*(y*width+x)+1];
				img[6*(y*width+x)+4]=0;
				img[6*(y*width+x)+5]=0;
			}
		}
	}

	// Ok, and now only write out the active pixels to the .ppm file.
	// (But only if the active pixels differ from the total pixels)

	if( !(height == active_height && width == active_width) )
	{
		if(format==ETC2PACKAGE_RG_NO_MIPMAPS)
			newimg=(uint8_t*)malloc(3*active_width*active_height*2);
		else
			newimg=(uint8_t*)malloc(3*active_width*active_height);
		
		if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_R_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
		{
			newalphaimg = (uint8_t*)malloc(active_width*active_height*2);
		}

		if(!newimg)
		{
			printf("Error: could not allocate memory\n");
			exit(0);
		}
		
		// Convert from total area to active area:

		for(yy = 0; yy<active_height; yy++)
		{
			for(xx = 0; xx< active_width; xx++)
			{
				if(format!=ETC2PACKAGE_R_NO_MIPMAPS&&format!=ETC2PACKAGE_RG_NO_MIPMAPS) 
				{
					newimg[ (yy*active_width)*3 + xx*3 + 0 ] = img[ (yy*width)*3 + xx*3 + 0];
					newimg[ (yy*active_width)*3 + xx*3 + 1 ] = img[ (yy*width)*3 + xx*3 + 1];
					newimg[ (yy*active_width)*3 + xx*3 + 2 ] = img[ (yy*width)*3 + xx*3 + 2];
				}
				else if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
				{
					newimg[ (yy*active_width)*6 + xx*6 + 0 ] = img[ (yy*width)*6 + xx*6 + 0];
					newimg[ (yy*active_width)*6 + xx*6 + 1 ] = img[ (yy*width)*6 + xx*6 + 1];
					newimg[ (yy*active_width)*6 + xx*6 + 2 ] = img[ (yy*width)*6 + xx*6 + 2];
					newimg[ (yy*active_width)*6 + xx*6 + 3 ] = img[ (yy*width)*6 + xx*6 + 3];
					newimg[ (yy*active_width)*6 + xx*6 + 4 ] = img[ (yy*width)*6 + xx*6 + 4];
					newimg[ (yy*active_width)*6 + xx*6 + 5 ] = img[ (yy*width)*6 + xx*6 + 5];
				}
				if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
				{
					newalphaimg[ ((yy*active_width) + xx)*2]   = alphaimg[2*((yy*width) + xx)];
					newalphaimg[ ((yy*active_width) + xx)*2+1] = alphaimg[2*((yy*width) + xx)+1];
				}
				if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
				{
					newalphaimg[ ((yy*active_width) + xx)]   = alphaimg[((yy*width) + xx)];
				}
			}
		}

		free(img);
		img = newimg;
		if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_R_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
		{
			free(alphaimg);
			alphaimg=newalphaimg;
		}
		if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
		{
			free(alphaimg);
			free(alphaimg2);
			alphaimg = NULL;
			alphaimg2 = NULL;
		}
	}
	height=active_height;
	width=active_width;
	fclose(f);
}

// Writes output file 
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void writeOutputFile(char *dstfile, uint8_t* img, uint8_t* alphaimg, int width, int height) 
{
	char str[300];

	if(format!=ETC2PACKAGE_R_NO_MIPMAPS&&format!=ETC2PACKAGE_RG_NO_MIPMAPS) 
	{
		fWritePPM("tmp.ppm",width,height,img,8,false);
		printf("Saved file tmp.ppm \n\n");
	}
	else if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
	{
		fWritePPM("tmp.ppm",width,height,img,16,false);
	}
	if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
		fWritePGM("alphaout.pgm",width,height,alphaimg,false,8);
	if(format==ETC2PACKAGE_R_NO_MIPMAPS)
		fWritePGM("alphaout.pgm",width,height,alphaimg,false,16);

	// Delete destination file if it exists
	if(fileExist(dstfile))
	{
		sprintf(str, "del %s\n",dstfile);	
		system(str);
	}

	int q = find_pos_of_extension(dstfile);
	if(!strcmp(&dstfile[q],".ppm")&&format!=ETC2PACKAGE_R_NO_MIPMAPS) 
	{
		// Already a .ppm file. Just rename. 
		sprintf(str,"move tmp.ppm %s\n",dstfile);
		printf("Renaming destination file to %s\n",dstfile);
	}
	else
	{
		// Converting from .ppm to other file format
		// 
		// Use your favorite command line image converter program,
		// for instance Image Magick. Just make sure the syntax can
		// be written as below:
		// 
		// C:\convert.exe source.ppm dest.jpg
		//
		if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
		{
            // Somewhere after version 6.7.1-2 of ImageMagick the following command gives the wrong result due to a bug. 
			// sprintf(str,"composite -compose CopyOpacity alphaout.pgm tmp.ppm %s\n",dstfile);
            // Instead we read the file and write a tga.

			printf("Converting destination file from .ppm/.pgm to %s with alpha\n",dstfile);
            int rw, rh;
            unsigned char *pixelsRGB;
            unsigned char *pixelsA;
			fReadPPM("tmp.ppm", rw, rh, pixelsRGB, 8);
            fReadPGM("alphaout.pgm", rw, rh, pixelsA, 8);
			fWriteTGAfromRGBandA(dstfile, rw, rh, pixelsRGB, pixelsA, true);
            free(pixelsRGB);
            free(pixelsA);
            sprintf(str,""); // Nothing to execute.
		}
		else if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
		{
			sprintf(str,"convert alphaout.pgm %s\n",dstfile);
			printf("Converting destination file from .pgm to %s\n",dstfile);
		}
		else 
		{
			sprintf(str,"convert tmp.ppm %s\n",dstfile);
			printf("Converting destination file from .ppm to %s\n",dstfile);
		}
	}
	// Execute system call
	system(str);
	
	free(img);
	if(alphaimg!=NULL)
		free(alphaimg);
}

// Calculates the PSNR between two files
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
double calculatePSNRfile(char *srcfile, uint8_t *origimg, uint8_t* origalpha)
{
	uint8_t *alphaimg, *img;
	int active_width, active_height;
	uncompressFile(srcfile,img,alphaimg,active_width,active_height);

	// calculate Mean Square Error (MSE)
	double MSER=0,MSEG=0,MSEA, PSNRR,PSNRG,PSNRA;
	double MSE;
	double wMSE;
	double PSNR=0;
	double wPSNR;
	double err;
	MSE = 0;
	MSEA=0;
	wMSE = 0;
	int width=((active_width+3)/4)*4;
	int numpixels = 0;
	for(int y=0;y<active_height;y++)
	{
		for(int x=0;x<active_width;x++)
		{
			if(format!=ETC2PACKAGE_R_NO_MIPMAPS&&format!=ETC2PACKAGE_RG_NO_MIPMAPS) 
			{
				//we have regular color channels..
				if((format != ETC2PACKAGE_RGBA1_NO_MIPMAPS && format != ETC2PACKAGE_sRGBA1_NO_MIPMAPS) || alphaimg[y*width + x] > 0)
				{
// from etcpack.cxx
#define PERCEPTUAL_WEIGHT_R_SQUARED 0.299
#define PERCEPTUAL_WEIGHT_G_SQUARED 0.587
#define PERCEPTUAL_WEIGHT_B_SQUARED 0.114
                    err = img[y*active_width * 3 + x * 3 + 0] - origimg[y*width * 3 + x * 3 + 0];
					MSE  += ((err * err)/3.0);
					wMSE += PERCEPTUAL_WEIGHT_R_SQUARED * (err*err);
					err = img[y*active_width*3+x*3+1] - origimg[y*width*3+x*3+1];
					MSE  += ((err * err)/3.0);
					wMSE += PERCEPTUAL_WEIGHT_G_SQUARED * (err*err);
					err = img[y*active_width*3+x*3+2] - origimg[y*width*3+x*3+2];
					MSE  += ((err * err)/3.0);
					wMSE += PERCEPTUAL_WEIGHT_B_SQUARED * (err*err);
					numpixels++;
#undef PERCEPTUAL_WEIGHT_R_SQUARED
#undef PERCEPTUAL_WEIGHT_G_SQUARED
#undef PERCEPTUAL_WEIGHT_B_SQUARED
                }
			}
			else if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
			{
				int rorig = (origimg[6*(y*width+x)+0]<<8)+origimg[6*(y*width+x)+1];
				int rnew =  (    img[6*(y*active_width+x)+0]<<8)+    img[6*(y*active_width+x)+1];
				int gorig = (origimg[6*(y*width+x)+2]<<8)+origimg[6*(y*width+x)+3];
				int gnew =  (    img[6*(y*active_width+x)+2]<<8)+    img[6*(y*active_width+x)+3];
				err=rorig-rnew;
				MSER+=(err*err);
				err=gorig-gnew;
				MSEG+=(err*err);
			}
			else if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
			{
				int aorig = (((int)origalpha[2*(y*width+x)+0])<<8)+origalpha[2*(y*width+x)+1];
				int anew =  (((int)alphaimg[2*(y*active_width+x)+0])<<8)+alphaimg[2*(y*active_width+x)+1];
				err=aorig-anew;
				MSEA+=(err*err);
			}
		}
	}
	if(format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
	{
		MSE = MSE / (1.0 * numpixels);
		wMSE = wMSE / (1.0 * numpixels);
		PSNR = 10*log((1.0*255*255)/MSE)/log(10.0);
		wPSNR = 10*log((1.0*255*255)/wMSE)/log(10.0);
		printf("PSNR only calculated on pixels where compressed alpha > 0\n");
		printf("color PSNR: %lf\nweighted PSNR: %lf\n",PSNR,wPSNR);
	}
	else if(format!=ETC2PACKAGE_R_NO_MIPMAPS&&format!=ETC2PACKAGE_RG_NO_MIPMAPS) 
	{
		MSE = MSE / (active_width * active_height);
		wMSE = wMSE / (active_width * active_height);
		PSNR = 10*log((1.0*255*255)/MSE)/log(10.0);
		wPSNR = 10*log((1.0*255*255)/wMSE)/log(10.0);
		if(format == ETC2PACKAGE_RGBA_NO_MIPMAPS || format == ETC2PACKAGE_sRGBA_NO_MIPMAPS)
			printf("PSNR only calculated on RGB, not on alpha\n");
		printf("color PSNR: %lf\nweighted PSNR: %lf\n",PSNR,wPSNR);
	}
	else if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
	{
		MSER = MSER / (active_width * active_height);
		MSEG = MSEG / (active_width * active_height);
		PSNRR = 10*log((1.0*65535*65535)/MSER)/log(10.0);
		PSNRG = 10*log((1.0*65535*65535)/MSEG)/log(10.0);
		printf("red PSNR: %lf\ngreen PSNR: %lf\n",PSNRR,PSNRG);
	}
	else if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
	{
		MSEA = MSEA / (active_width * active_height);
		PSNRA = 10*log((1.0*65535.0*65535.0)/MSEA)/log(10.0);
		printf("PSNR: %lf\n",PSNRA);
	}
	free(img);
	return PSNR;
}

// Compress an image file.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void compressImageFile(uint8_t *img, uint8_t *alphaimg,int width,int height,char *dstfile, int expandedwidth, int expandedheight)
{
	FILE *f;
	int x,y,w,h;
	unsigned int block1, block2;
	unsigned short wi, hi;
	unsigned char magic[4];
	unsigned char version[2];
	unsigned short texture_type=format;
	uint8_t *imgdec;
	uint8_t* alphaimg2;
	imgdec = (unsigned char*) malloc(expandedwidth*expandedheight*3);
	if(!imgdec)
	{
		printf("Could not allocate decompression buffer --- exiting\n");
	}

	magic[0]   = 'P'; magic[1]   = 'K'; magic[2] = 'M'; magic[3] = ' '; 

	if(codec==CODEC_ETC2)
	{
		version[0] = '2'; version[1] = '0';
	}
	else
	{
		version[0] = '1'; version[1] = '0';
	}
    f = fopen(dstfile, "wb");
	if(f)
	{
		w=expandedwidth/4;  w*=4;
		h=expandedheight/4; h*=4;
		wi = w;
		hi = h;
		if(ktxFile) 
		{
			//.ktx file: KTX header followed by compressed binary data.
			KTX_header header;
			//identifier
			for(int i=0; i<12; i++) 
			{
				header.identifier[i]=ktx_identifier[i];
			}
			//endianess int.. if this comes out reversed, all of the other ints will too.
			header.endianness=KTX_ENDIAN_REF;
			
			//these values are always 0/1 for compressed textures.
			header.glType=0;
			header.glTypeSize=1;
			header.glFormat=0;

			header.pixelWidth=width;
			header.pixelHeight=height;
			header.pixelDepth=0;

			//we only support single non-mipmapped non-cubemap textures..
			header.numberOfArrayElements=0;
			header.numberOfFaces=1;
			header.numberOfMipmapLevels=1;

			//and no metadata..
			header.bytesOfKeyValueData=0;
			
			int halfbytes=1;
			//header.glInternalFormat=?
			//header.glBaseInternalFormat=?
			if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_R;
				if(formatSigned)
					header.glInternalFormat=GL_COMPRESSED_SIGNED_R11_EAC;
				else
					header.glInternalFormat=GL_COMPRESSED_R11_EAC;
			}
			else if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
			{
				halfbytes=2;
				header.glBaseInternalFormat=GL_RG;
				if(formatSigned)
					header.glInternalFormat=GL_COMPRESSED_SIGNED_RG11_EAC;
				else
					header.glInternalFormat=GL_COMPRESSED_RG11_EAC;
			}
			else if(format==ETC2PACKAGE_RGB_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_RGB;
				header.glInternalFormat=GL_COMPRESSED_RGB8_ETC2;
			}
			else if(format==ETC2PACKAGE_sRGB_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_SRGB;
				header.glInternalFormat=GL_COMPRESSED_SRGB8_ETC2;
			}
			else if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS) 
			{
				halfbytes=2;
				header.glBaseInternalFormat=GL_RGBA;
				header.glInternalFormat=GL_COMPRESSED_RGBA8_ETC2_EAC;
			}
			else if(format==ETC2PACKAGE_sRGBA_NO_MIPMAPS) 
			{
				halfbytes=2;
				header.glBaseInternalFormat=GL_SRGB8_ALPHA8;
				header.glInternalFormat=GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
			}
			else if(format==ETC2PACKAGE_RGBA1_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_RGBA;
				header.glInternalFormat=GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			else if(format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_SRGB8_ALPHA8;
				header.glInternalFormat=GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
			}
			else if(format==ETC1_RGB_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_RGB;
				header.glInternalFormat=GL_ETC1_RGB8_OES;
			}
			else 
			{
				printf("internal error: bad format!\n");
				exit(1);
			}
			//write header
			fwrite(&header,sizeof(KTX_header),1,f);
			
			//write size of compressed data.. which depend on the expanded size..
			unsigned int imagesize=(w*h*halfbytes)/2;
			fwrite(&imagesize,sizeof(int),1,f);
		}
		else 
		{
			//.pkm file, contains small header..

			// Write magic number
			fwrite(&magic[0], sizeof(unsigned char), 1, f);
			fwrite(&magic[1], sizeof(unsigned char), 1, f);
			fwrite(&magic[2], sizeof(unsigned char), 1, f);
			fwrite(&magic[3], sizeof(unsigned char), 1, f);
		
			// Write version
			fwrite(&version[0], sizeof(unsigned char), 1, f);
			fwrite(&version[1], sizeof(unsigned char), 1, f);

			// Write texture type
			if(texture_type==ETC2PACKAGE_RG_NO_MIPMAPS&&formatSigned) 
			{
				unsigned short temp = ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS;
				write_big_endian_2byte_word(&temp,f);
			}
			else if(texture_type==ETC2PACKAGE_R_NO_MIPMAPS&&formatSigned) 
			{
				unsigned short temp = ETC2PACKAGE_R_SIGNED_NO_MIPMAPS;
				write_big_endian_2byte_word(&temp,f);
			}
			else
				write_big_endian_2byte_word(&texture_type, f);

			// Write binary header: the width and height as unsigned 16-bit words
			write_big_endian_2byte_word(&wi, f);
			write_big_endian_2byte_word(&hi, f);

			// Also write the active pixels. For instance, if we want to compress
			// a 128 x 129 image, we have to extend it to 128 x 132 pixels.
			// Then the wi and hi written above will be 128 and 132, but the
			// additional information that we write below will be 128 and 129,
			// to indicate that it is only the top 129 lines of data in the 
			// decompressed image that will be valid data, and the rest will
			// be just garbage. 

			unsigned short activew, activeh;
			activew = width;
			activeh = height;

			write_big_endian_2byte_word(&activew, f);
			write_big_endian_2byte_word(&activeh, f);
		}
		int totblocks = expandedheight/4 * expandedwidth/4;
		int countblocks = 0;
		double percentageblocks=-1.0;
		double oldpercentageblocks;
		
		if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
		{
			//extract data from red and green channel into two alpha channels.
			//note that the image will be 16-bit per channel in this case.
			alphaimg= (unsigned char*)malloc(expandedwidth*expandedheight*2);
			alphaimg2=(unsigned char*)malloc(expandedwidth*expandedheight*2);
			if(!alphaimg||!alphaimg2) 
			{
				printf("failed allocating space for alpha buffers!\n");
				exit(1);
			}
			for(y=0;y<expandedheight;y++)
			{
				for(x=0;x<expandedwidth;x++)
				{
					alphaimg[2*(y*expandedwidth+x)]=img[6*(y*expandedwidth+x)];
					alphaimg[2*(y*expandedwidth+x)+1]=img[6*(y*expandedwidth+x)+1];
					alphaimg2[2*(y*expandedwidth+x)]=img[6*(y*expandedwidth+x)+2];
					alphaimg2[2*(y*expandedwidth+x)+1]=img[6*(y*expandedwidth+x)+3];
				}
			}
		}
		for(y=0;y<expandedheight/4;y++)
		{
			for(x=0;x<expandedwidth/4;x++)
			{
				countblocks++;
				oldpercentageblocks = percentageblocks;
				percentageblocks = 100.0*countblocks/(1.0*totblocks);
				//compress color channels
				if(codec==CODEC_ETC) 
				{
					if(metric==METRIC_NONPERCEPTUAL) 
					{
						if(speed==SPEED_FAST)
							compressBlockDiffFlipFast(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);
						else
#if EXHAUSTIVE_CODE_ACTIVE
							compressBlockETC1Exhaustive(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);		
#else
							printf("Not implemented in this version\n");
#endif
					}
					else 
					{
						if(speed==SPEED_FAST)
							compressBlockDiffFlipFastPerceptual(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);
						else
#if EXHAUSTIVE_CODE_ACTIVE
							compressBlockETC1ExhaustivePerceptual(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);
#else
							printf("Not implemented in this version\n");
#endif
					}
				}
				else 
				{
					if(format==ETC2PACKAGE_R_NO_MIPMAPS||format==ETC2PACKAGE_RG_NO_MIPMAPS) 
					{
						//don't compress color
					}
					else if(format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
					{
						//this is only available for fast/nonperceptual
						if(speed == SPEED_SLOW && first_time_message)
						{
							printf("Slow codec not implemented for RGBA1 --- using fast codec instead.\n");
							first_time_message = false;
						}
						compressBlockETC2Fast(img, alphaimg,imgdec, expandedwidth, expandedheight, 4*x, 4*y, block1, block2,
							format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS);
					}
					else if(metric==METRIC_NONPERCEPTUAL) 
					{
						if(speed==SPEED_FAST)
							compressBlockETC2Fast(img, alphaimg,imgdec, expandedwidth, expandedheight, 4*x, 4*y, block1, block2,
								format == ETC2PACKAGE_RGBA1_NO_MIPMAPS || format == ETC2PACKAGE_sRGBA1_NO_MIPMAPS);
						else
#if EXHAUSTIVE_CODE_ACTIVE
							compressBlockETC2Exhaustive(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);		
#else
							printf("Not implemented in this version\n");
#endif
					}
					else 
					{
						if(speed==SPEED_FAST)
							compressBlockETC2FastPerceptual(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);
						else
#if EXHAUSTIVE_CODE_ACTIVE
							compressBlockETC2ExhaustivePerceptual(img, imgdec, expandedwidth, 4*x, 4*y, block1, block2);
#else
							printf("Not implemented in this version\n");
#endif
					}
				}
				
				//compression of alpha channel in case of 4-bit alpha. Uses 8-bit alpha channel as input, and has 8-bit precision.
				if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS) 
				{
					uint8_t alphadata[8];
					if(speed==SPEED_SLOW)
						compressBlockAlphaSlow(alphaimg,4*x,4*y,expandedwidth,alphadata);
					else
						compressBlockAlphaFast(alphaimg,4*x,4*y,expandedwidth,alphadata);
					//write the 8 bytes of alphadata into f.
					fwrite(alphadata,1,8,f);
				}

				//store compressed color channels
				if(format!=ETC2PACKAGE_R_NO_MIPMAPS&&format!=ETC2PACKAGE_RG_NO_MIPMAPS) 
				{
					write_big_endian_4byte_word(&block1, f);
					write_big_endian_4byte_word(&block2, f);
				}

				//1-channel or 2-channel alpha compression: uses 16-bit data as input, and has 11-bit precision
				if(format==ETC2PACKAGE_R_NO_MIPMAPS||format==ETC2PACKAGE_RG_NO_MIPMAPS) 
				{ 
					uint8_t alphadata[8];
					compressBlockAlpha16(alphaimg,4*x,4*y,expandedwidth,alphadata,formatSigned);
					fwrite(alphadata,1,8,f);
				}
				//compression of second alpha channel in RG-compression
				if(format==ETC2PACKAGE_RG_NO_MIPMAPS) 
				{
					uint8_t alphadata[8];
					compressBlockAlpha16(alphaimg2,4*x,4*y,expandedwidth,alphadata,formatSigned);
					fwrite(alphadata,1,8,f);
				}
#if 1
				if(verbose)
				{
					if(speed==SPEED_FAST) 
					{
						if( ((int)(percentageblocks) != (int)(oldpercentageblocks) ) || percentageblocks == 100.0)
							printf("Compressed %d of %d blocks, %.0f%% finished.\r", countblocks, totblocks, 100.0*countblocks / (1.0*totblocks));
					}
					else
						printf("Compressed %d of %d blocks, %.0f%% finished.\r", countblocks, totblocks, 100.0*countblocks / (1.0*totblocks));
				}
#endif
			}
		}
		printf("\n");
		fclose(f);
		printf("Saved file <%s>.\n",dstfile);
	}
}

// Compress an file.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
void compressFile(char *srcfile,char *dstfile)
{
	uint8_t *srcimg = NULL;
	int width,height;
	int extendedwidth, extendedheight;
	struct _timeb tstruct;
	long long tstart;
	long long tstop;
	// 0: compress from .any to .pkm with SPEED_FAST, METRIC_NONPERCEPTUAL, ETC 
	// 1: compress from .any to .pkm with SPEED_MEDIUM, METRIC_NONPERCEPTUAL, ETC
	// 2: compress from .any to .pkm with SPEED_SLOW, METRIC_NONPERCEPTUAL, ETC
	// 3: compress from .any to .pkm with SPEED_FAST, METRIC_PERCEPTUAL, ETC
	// 4: compress from .any to .pkm with SPEED_MEDIUM, METRIC_PERCEPTUAL, ETC
	// 5: compress from .any to .pkm with SPEED_SLOW, METRIC_PERCEPTUAL, ETC
	// 6: decompress from .pkm to .any
	// 7: calculate PSNR between .any and .any
	// 8: compress from .any to .pkm with SPEED_FAST, METRIC_NONPERCEPTUAL, ETC2 
	// 9: compress from .any to .pkm with SPEED_MEDIUM, METRIC_NONPERCEPTUAL, ETC2
	//10: compress from .any to .pkm with SPEED_SLOW, METRIC_NONPERCEPTUAL, ETC2
	//11: compress from .any to .pkm with SPEED_FAST, METRIC_PERCEPTUAL, ETC2
	//12: compress from .any to .pkm with SPEED_MEDIUM, METRIC_PERCEPTUAL, ETC2
	//13: compress from .any to .pkm with SPEED_SLOW, METRIC_PERCEPTUAL, ETC2

	printf("\n");
	if(codec==CODEC_ETC)
		printf("ETC codec, ");
	else
		printf("ETC2 codec, ");
	if(speed==SPEED_FAST)
		printf("using FAST compression mode and ");
	else if(speed==SPEED_MEDIUM)
		printf("using MEDIUM compression mode and ");
	else
		printf("using SLOW compression mode and ");
	if(metric==METRIC_PERCEPTUAL)
		printf("PERCEPTUAL error metric, ");
	else
		printf("NONPERCEPTUAL error metric, ");
	if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS)
		printf("in RGBA format");
	else if(format==ETC2PACKAGE_sRGBA_NO_MIPMAPS)
		printf("in sRGBA format");
	else if(format==ETC2PACKAGE_RGBA1_NO_MIPMAPS)
		printf("in RGB + punch-through alpha format");
	else if(format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS)
		printf("in sRGB + punch-through alpha format");
	else if(format==ETC2PACKAGE_R_NO_MIPMAPS)
		printf("in R format");
	else if(format==ETC2PACKAGE_RGB_NO_MIPMAPS||format==ETC1_RGB_NO_MIPMAPS)
		printf("in RGB format");
	else if(format==ETC2PACKAGE_RG_NO_MIPMAPS)
		printf("in RG format");
	else
		printf("in OTHER format");
	printf("\n");
	if(format==ETC2PACKAGE_R_NO_MIPMAPS||readSrcFile(srcfile,srcimg,width,height,extendedwidth, extendedheight))
	{
		//make sure that alphasrcimg contains the alpha channel or is null here, and pass it to compressimagefile
		uint8_t* alphaimg=NULL;
		if(format==ETC2PACKAGE_RGBA_NO_MIPMAPS||format==ETC2PACKAGE_RGBA1_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA_NO_MIPMAPS||format==ETC2PACKAGE_sRGBA1_NO_MIPMAPS) 
		{
			char str[300];
			//printf("reading alpha channel....");
			sprintf(str,"convert %s -alpha extract alpha.pgm\n",srcfile);
			system(str);
			readAlpha(alphaimg,width,height,extendedwidth,extendedheight);
			printf("ok!\n");
		}
		else if(format==ETC2PACKAGE_R_NO_MIPMAPS) 
		{
			char str[300];
			sprintf(str,"convert %s alpha.pgm\n",srcfile);
			system(str);
			readAlpha(alphaimg,width,height,extendedwidth,extendedheight);
			printf("read alpha ok, size is %d,%d (%d,%d)",width,height,extendedwidth,extendedheight);
		}
		printf("Compressing...\n");

		tstart=time(NULL);
		_ftime( &tstruct );
		tstart=tstart*1000+tstruct.millitm;
		compressImageFile(srcimg,alphaimg,width,height,dstfile,extendedwidth, extendedheight);			
		tstop = time(NULL);
		_ftime( &tstruct );
		tstop = tstop*1000+tstruct.millitm;
		printf( "It took %u milliseconds to compress:\n", (unsigned)(tstop - tstart));
		calculatePSNRfile(dstfile,srcimg,alphaimg);
	}
}

// Calculates the PSNR between two files.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
double calculatePSNRTwoFiles(char *srcfile1,char *srcfile2)
{
	uint8_t *srcimg1;
	uint8_t *srcimg2;
	int width1, height1;
	int width2, height2;
	double PSNR = 0;
	double perceptually_weighted_PSNR;

	if(readSrcFileNoExpand(srcfile1,srcimg1,width1,height1))
	{
		if(readSrcFileNoExpand(srcfile2,srcimg2,width2,height2))
		{
			if((width1 == width2) && (height1 == height2))
			{
				PSNR = calculatePSNR(srcimg1, srcimg2, width1, height1);
				printf("%f\n",PSNR);
				perceptually_weighted_PSNR = calculateWeightedPSNR(srcimg1, srcimg2, width1, height1, 0.299, 0.587, 0.114);
			}
			else
			{
				printf("\n Width and height do no not match for image: width, height = (%d, %d) and (%d, %d)\n",width1,height1, width2, height2);
			}
		}
		else
		{
			printf("Couldn't open file %s.\n",srcfile2);
		}
	}
	else
	{
		printf("Couldn't open file %s.\n",srcfile1);
	}

	return PSNR;
}

// Main function
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
int main(int argc,char *argv[])
{
	if(argc==3 || argc==4 || argc == 5 || argc == 7 || argc == 9 || argc == 11 || argc == 13)
	{
		// The source file is always the second last one. 
		char srcfile[200];
		char dstfile[200];
		readArguments(argc,argv,srcfile,dstfile);
		
		if(!fileExist(srcfile))
		{
			printf("Error: file <%s> does not exist.\n",srcfile);
			exit(0);
		}
		
		if(mode==MODE_UNCOMPRESS)
		{
			printf("Decompressing .pkm/.ktx file ...\n");
			uint8_t* alphaimg=NULL, *img;
			int w, h;
			uncompressFile(srcfile,img,alphaimg,w,h);
			writeOutputFile(dstfile,img,alphaimg,w,h);
		}
		else if(mode==MODE_PSNR)
		{
			calculatePSNRTwoFiles(srcfile,dstfile);
		}
		else
		{
			compressFile(srcfile, dstfile);
		}
	}
	else
	{
		printf("ETCPACK v2.74 For ETC and ETC2\n");
		printf("Compresses and decompresses images using the Ericsson Texture Compression (ETC) version 1.0 and 2.0.\n\nUsage: etcpack srcfile dstfile\n\n");
		printf("      -s {fast|slow}                     Compression speed. Slow = exhaustive \n");
		printf("                                         search for optimal quality\n");
		printf("                                         (default: fast)\n");
		printf("      -e {perceptual|nonperceptual}      Error metric: Perceptual (nicest) or \n");
		printf("                                         nonperceptual (highest PSNR)\n");
		printf("                                         (default: perceptual)\n");
		printf("      -c {etc1|etc2}                     Codec: etc1 (most compatible) or \n");
		printf("                                         etc2 (highest quality)\n");
		printf("                                         (default: etc2)\n");
		printf("      -f {R|R_signed|RG|RG_signed|       Format: one, two, three or four \n");
		printf("          RGB|RGBA1|RGBA8|               channels, and 1 or 8 bits for alpha\n");
        printf("          sRGB|sRGBA1|sRGBA8|}           RGB or sRGB.\n");
		printf("                                         (1 equals punchthrough)\n");
		printf("                                         (default: RGB)\n");
		printf("      -v {on|off}                        Detailed progress info. (default on)\n");
		printf("                                                            \n");
		printf("Examples: \n");
		printf("  etcpack img.ppm img.pkm                Compresses img.ppm to img.pkm in\n"); 
		printf("                                         ETC2 RGB format\n");
		printf("  etcpack img.ppm img.ktx                Compresses img.ppm to img.ktx in\n"); 
		printf("                                         ETC2 RGB format\n");
		printf("  etcpack img.pkm img_copy.ppm           Decompresses img.pkm to img_copy.ppm\n");
		printf("  etcpack -s slow img.ppm img.pkm        Compress using the slow mode.\n");
		printf("  etcpack -p orig.ppm copy.ppm           Calculate PSNR between orig and copy\n");
		printf("  etcpack -f RGBA8 img.tga img.pkm       Compresses img.tga to img.pkm, using \n");
		printf("                                         etc2 + alpha.\n");
		printf("  etcpack -f RG img.ppm img.pkm          Compresses red and green channels of\n");
		printf("                                         img.ppm\n");
	}
 	return 0;
}
