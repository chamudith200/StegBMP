# bmp.h
A simple C header file for reading and writing BMP image files.

## Usage
Following code block is a example of reading a existing BMP file (original.bmp) and creating a copy of it (cpy.bmp).
``` c
// Craete a BMP structs to hold the data fromo the original.bmp file   
BMP bmp_in;
BMP_ReadFile(&bmp_in, "original.bmp");

// Create a BMP struct to hold the data of the BMP file to be created
BMP bmp_out;
BMP_InitHeader(&bmp_out, bmp_in.infoHeader.Width, bmp_in.infoHeader.Height);
bmp_out.pixelData = bmp_in.pixelData;
BMP_CreateFile(&bmp_out, "cpy.bmp");
 ```
## Struct definitions
__BMP_FileHeader__
This struct holds all the data in the fileHeader part of the BMP file.
``` c
typedef struct BMP_Struct{
    // Header
    uint16_t Signature;     
    uint32_t FileSize;
    uint32_t Reserved;
    uint32_t DataOffset;
}BMP_FileHeader;
```
__BMP_InfoHeader__
This struct hold all the data in the infoHeader part of the BMP file.
``` c
typedef struct {
    uint32_t Size;
    uint32_t Width;
    uint32_t Height;
    uint16_t Planes;
    uint16_t BitsPerPixel;
    uint32_t Compression;
    uint32_t ImageSize;
    uint32_t XpixelsPerM; 
    uint32_t YpixelsPerM;
    uint32_t ColorsUsed;
    uint32_t ImportantColors;
}BMP_InfoHeader;
```
__BMP__
This structure holds all the data of the BMP file and some others require by some other functions as well.
``` c
typedef struct {
    BMP_FileHeader fileHeader;
    BMP_InfoHeader infoHeader;
    uint8_t *pixelData;     
    int pxSize;
    int padding;
}BMP;
```
**pixelData :** unisgned int BGR 8-bit color array containg all the color data of pixels  
**pxSize :** size of the pixel data array with padding (bytes)
**padding :** BMP specific padding at the end of the line (bytes)
## Function definitions
__void BMP_InitHeader(BMP *bmp, int width, int height)__

This function initialize a BMP structure to recieve data.

parameters:

&emsp; bmp : pointer to the BMP structure (BMP *)

&emsp; width, height: width and height of the BMP image. (int) (just set it to 0 if don't know it yet)

returns:

&emsp;does not returns anything

exapmle:

``` c
BMP bmp_in;
BMP_ReadFile(&bmp_in, "original.bmp");
```
__int BMP_CreateFile(BMP *bmp, char *fileName)__
This function create a BMP file on the disk according to the given BMP structure
parameters:
&emsp; bmp : pointer to the bmp stucture (BMP *)
&emsp; filename : filename fo the BMP file to be created (char *)
returns:
&emsp; STEG_SUCCESS (0) on success
&emsp; STEG_FAILURE (1) on failure
example:
``` c
BMP bmp_out;
// Initialize the strcut and add pixel data
BMP_CreateFile(&bmp_out, "cpy.bmp");
```
