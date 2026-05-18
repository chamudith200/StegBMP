# StegBMP

A simple steganography program tht can encode and decode data into/from BMP files using LSB written in C language

### How to buld
``` bash
gcc main.c -o bmp
```

### How to run
``` bash
./bmp
```
## Logic Explanation
BMP files contain a 24bit BGR pixels. So we can use this to store out data without making any significant visual artifacts to the original picture.
For this we use the LSB of the red channel to store data. Each row of pixel contains exactly one byte of data. Maximum amount of data we can store depend on the verticle resolution of the image. So we can store a single ASCII character in a row (ASCII character can be stored with one byte)

We start the encoding by the first bit of the byte. 
If it's 1: We check the red channe and make it odd if it's even using the XOR (^) operator and vice versa for bits that are 0. Only the red chanel is used for the encoding
The decodin is the same but in reverse. We check the red channel and if it's odd, it a 1 bit and if it's odd it's a 0 bit. then w construct a byte by looking at continuous 8 pixels and then move to the next row.

The number of

## Code Explanation
### Header file intialization

with the following function we can initializa the header files needed for this project.
``` c
void init_BMP_Header(BMP_FileHeader *fileHeader, BMP_InfoHeader *infoHeader);
```

It get's pointers to _BMP_fileHeader_ and _BMP_infoHeader_ as parameters which the user need to create (They are defined in the main.c file)
Example usage:
``` c
BMP_FileHeader bmp_FileHEader;
BMP_InfoHeader bmp_InfoHeader;

init_BMP_Header(&bmp_FileHEader, &bmp_InfoHeader);
```

### Creating and Encoding a BMP file
We can use the following helper file tp generate a custom BMP file for maily testing purposes. It generate the pixelData array according to the infoHeader data. (The pattern is based on the code and need to change if in need of a different pattern)
``` c
uint8_t *random_pic(BMP_FileHeader *fileHeader);
```

The original messege is recieved as a funtion parameter alongside its size.
``` c
uint8_t *encode_pic(BMP_FileHeader *fileHeader, BMP_InfoHeader *infoHeader, uint8_t *originalPixelData, char *messege, int size);
```

It outputs an array of pixel data (24bit BGR) with padding icnluded. Then it can be used to create a BMP file with _create_bmp(...)_ function.
``` c
int create_bmp(BMP_FileHeader *fileHeader, BMP_InfoHeader *infoHeader, uint8_t *pixelData, char *fileName);
```

Exaple usage:
``` c
uint8_t *pixelData = random_pic(&bmp_FileHEader);
create_bmp(&bmp_FileHEader, &bmp_InfoHeader, pixelData, "original.bmp");
uint8_t *encodedPixelData = encode_pic(&bmp_FileHEader, &bmp_InfoHeader, pixelData, "ABCDEFGH", 8);
create_bmp(&bmp_FileHEader, &bmp_InfoHeader, encodedPixelData, "encoded.bmp");
```
This will make create files original.bmp and encoded.bmp at the same directory as bmp.exe

### Decoding BMP file
We can use the following function to decode the BMP file to retrive the original data encoded into it. 
``` c
char *decode_bmp(BMP_InfoHeader *infoHeader, uint8_t *encodedPixelData);
```
Example usage:
``` c
uint8_t *decodePixeldata = read_bmp("encoded.bmp");
char *messege = decode_bmp(&bmp_InfoHeader, decodePixeldata);
messege[8] = '\0';
printf("messege : %s\n", messege);
```
Output:
``` bash
PS StegBMP> ./bmp                                       
Created : original.bmp
Created : encoded.bmp
65
66
67
68
69
70
71
72
messege : ABCDEFGH
```

### Memory ownership
This program follows a **caller allocation ownership** patter for dynamic memory allocation. So the user must free those buffers after use to avoid memory leaks.
Example usage:
``` c
uint8_t *pixelData = random_pic(&bmp_FileHEader);
uint8_t *decodePixeldata = read_bmp("encoded.bmp");
char *messege = decode_bmp(&bmp_InfoHeader, decodePixeldata);

free(pixelData);
free(encodedPixelData);
free(messege);
```