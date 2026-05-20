#ifndef BMP_H
#define BMP_H

/* Set to 1 to enable debug output, 0 to disable */
#define DBG 0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    STG_SUCCESS,
    STG_FAILURE
};

// BMP Structs
#pragma pack(push, 1)

typedef struct BMP_Struct{
    // Header
    uint16_t Signature;
    uint32_t FileSize;
    uint32_t Reserved;
    uint32_t DataOffset;
}BMP_FileHeader;

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

#pragma pack(pop)

typedef struct {
    BMP_FileHeader fileHeader;
    BMP_InfoHeader infoHeader;
    uint8_t *pixelData;
    int pxSize;
    int padding;
}BMP;


/*
* Initialize the BMP structure accoring to the width and height of the program
*
* Parameters:
* - bmp: Pointer to a valid BMP structure.
* - width: Width of the BMP file.
* - height: Height of the BMP file
*
* Does not return anything.
*/ 
void BMP_InitHeader(BMP *bmp, int width, int height) {
    BMP_FileHeader *fileHeader = &bmp->fileHeader;
    BMP_InfoHeader *infoHeader = &bmp->infoHeader;

    // real file size with padding

    int padding = ((width * 3) / 4 + (width * 3) % 4) * 4 - (width * 3);
    bmp->padding = padding;
    int fileSize = ((width * 3) + padding) * height;
    bmp->pxSize = fileSize;
    
    // fileHeader
    fileHeader->Signature = 0x4d42; // "BM"
    fileHeader->FileSize = fileSize + sizeof(BMP_FileHeader) + sizeof(BMP_InfoHeader);
    fileHeader->Reserved = 0x0;
    fileHeader->DataOffset = sizeof(BMP_InfoHeader) + sizeof(BMP_FileHeader);

    // infoHeader
    infoHeader->Size = sizeof(BMP_InfoHeader);
    infoHeader->Width = width;
    infoHeader->Height = height;
    infoHeader->Planes = 1;
    infoHeader->BitsPerPixel = 24;
    infoHeader->Compression = 0;
    infoHeader->ImageSize = 0;
    infoHeader->XpixelsPerM = 0;
    infoHeader->YpixelsPerM = 0;
    infoHeader->ColorsUsed = 0;
    infoHeader->ImportantColors = 0;

    if (DBG) printf("BMP init done.\n");
}


/*
* Create a bmp file on the disk using the provided BMP data
* Requirements:
* - BMP structure must be fully initialized and contain valid image data before passing it to this funtion.
*
* Parameters:
* - bmp: Pointer to the valid initialized BMP structure.
* - filename: the destination path/filename of the new BMP file.
*
* Returns:
* - STG_SUCCESS (0) on success.
* - STG_FAILURE (1) on failure 
*/ 
int BMP_CreateFile(BMP *bmp, char *fileName) {
    BMP_FileHeader *fileHeader = &bmp->fileHeader;
    BMP_InfoHeader *infoHeader = &bmp->infoHeader;
    FILE *fp_bmp = fopen(fileName, "wb");
    if (!fp_bmp) {
        printf("Failed to open %s\n", fileName);
        return STG_FAILURE;
    }
    
    // fileHeader
    fwrite(fileHeader, sizeof(*fileHeader), 1, fp_bmp);

    // infoHeader
    fwrite(infoHeader, sizeof(*infoHeader), 1, fp_bmp);

    // PixelData
    fwrite(bmp->pixelData, bmp->pxSize, 1, fp_bmp);

    fclose(fp_bmp);
    if (DBG) printf("Created : %s\n", fileName);
    return STG_SUCCESS;
}


/*
* Read a existing BMP file from the disk (uncompressed)
* Parameters:
* - bmp: Pointer to BMP structure to store the ne BMP file
* - filename: Filename of the bmp file need to read
*
* Returns:
* - STG_SUCCESS (0) on success.
* - STG_FAILURE (1) on failure.
*/ 
int BMP_ReadFile(BMP *bmp, char *fileName) {
    FILE *fp_bmp = fopen(fileName, "rb");
    if (!fp_bmp) {
        printf("Failed to open %s\n", fileName);
        goto FUNC_FAIL;
    }
    
    BMP_FileHeader *fileHeader = &bmp->fileHeader;
    BMP_InfoHeader *infoHeader = &bmp->infoHeader;

    if (!fread(fileHeader, sizeof(BMP_FileHeader), 1, fp_bmp)) {
        printf("Failed to read file header: %s\n", fileName);
        goto FUNC_FAIL;
    }

    if (!fread(infoHeader, sizeof(BMP_InfoHeader), 1, fp_bmp)) {
        printf("Failed to read info header: %s\n", fileName);
        goto FUNC_FAIL;
    }

    int padding = ((infoHeader->Width * 3) / 4 + (infoHeader->Width * 3) % 4) * 4 - (infoHeader->Width * 3);
    bmp->padding = padding;
    int fileSize = ((infoHeader->Width * 3) + padding) * infoHeader->Height;
    bmp->pxSize = fileSize;

    bmp->pixelData = (uint8_t *)malloc(bmp->pxSize);
    if (!bmp->pixelData) {
        printf("Failed to allocate memory: bmp->pixelData\n");
        goto FUNC_FAIL;
    }

    if (!fread(bmp->pixelData, bmp->pxSize, 1, fp_bmp)) {
        printf("Failed to read pixelData: %s\n", fileName);
        goto FUNC_FAIL;
    }

    fclose(fp_bmp);
    if (DBG) printf("BMP \"%s\" read successsfuly.\n", fileName);
    return STG_SUCCESS;

    FUNC_FAIL:
    printf("Failed to read: %s\n", fileName);
    fclose(fp_bmp);
    return STG_FAILURE;
}

#endif