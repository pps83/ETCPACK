#ifndef ETCPACK_H_
#define ETCPACK_H_

// Typedefs
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;

void decompressBlockETC2(unsigned int block_part1, unsigned int block_part2, uint8_t *img, int width, int startx, int starty); // Decompress ETC1 or ETC2 RGB8
void decompressBlockAlpha(uint8_t* data, uint8_t* img, int width, int ix, int iy);
void decompressBlockETC21BitAlpha(unsigned int block_part1, unsigned int block_part2, uint8_t *img, uint8_t* alphaimg, int width, int startx, int starty);
void decompressBlockAlpha16bit(uint8_t* data, uint8_t* img, int width, int ix, int iy, int formatSigned);

// Compress ETC1
double compressBlockDiffFlipFast(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);
void compressBlockDiffFlipFastPerceptual(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockETC2Fast(uint8_t *img, uint8_t* alphaimg, uint8_t *imgdec, int width, int height, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2, bool useRgba1);
void compressBlockETC2FastPerceptual(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockETC1Exhaustive(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);
void compressBlockETC1ExhaustivePerceptual(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockETC2Exhaustive(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);
void compressBlockETC2ExhaustivePerceptual(uint8_t *img, uint8_t *imgdec, int width, int startx, int starty, unsigned int &compressed1, unsigned int &compressed2);

void compressBlockAlphaFast(uint8_t * data, int ix, int iy, int width, uint8_t* returnData);
void compressBlockAlphaSlow(uint8_t* data, int ix, int iy, int width, uint8_t* returnData);
void compressBlockAlpha16(uint8_t* data, int ix, int iy, int width, uint8_t* returnData, int formatSigned);


#endif /* ETCPACK_H_ */
