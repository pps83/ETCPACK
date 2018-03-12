#ifndef ETCPACK_H_
#define ETCPACK_H_

// Typedefs
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;

void decompressBlockETC2(unsigned int block_part1, unsigned int block_part2, uint8 *img, int width, int startx, int starty); // Decompress ETC1 or ETC2 RGB8
void decompressBlockAlpha(uint8* data, uint8* img, int width, int ix, int iy);
void decompressBlockETC21BitAlpha(unsigned int block_part1, unsigned int block_part2, uint8 *img, uint8* alphaimg, int width, int startx, int starty);
void decompressBlockAlpha16bit(uint8* data, uint8* img, int width, int ix, int iy, int formatSigned);

// Compress ETC1
double compressBlockDiffFlipFast(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);
void compressBlockDiffFlipFastPerceptual(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockETC2Fast(uint8 *img, uint8* alphaimg, uint8 *imgdec, int width, int height, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2, bool useRgba1);
void compressBlockETC2FastPerceptual(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockETC1Exhaustive(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);
void compressBlockETC1ExhaustivePerceptual(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockETC2Exhaustive(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);
void compressBlockETC2ExhaustivePerceptual(uint8 *img, uint8 *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockAlphaFast(uint8 * data, int ix, int iy, int width, uint8* returnData);
void compressBlockAlphaSlow(uint8* data, int ix, int iy, int width, uint8* returnData);
void compressBlockAlpha16(uint8* data, int ix, int iy, int width, uint8* returnData, int formatSigned);


#endif /* ETCPACK_H_ */
