#ifndef BMP_H
#define BMP_H

/* Set to 1 to enable debug output, 0 to disable */
#define DBG 0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t BMP_color[8];

enum {
    BMP_SUCCESS,
    BMP_FAILURE
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
* - BMP_SUCCESS (0) on success.
* - BMP_FAILURE (1) on failure 
*/ 
int BMP_CreateFile(BMP *bmp, char *fileName) {
    BMP_FileHeader *fileHeader = &bmp->fileHeader;
    BMP_InfoHeader *infoHeader = &bmp->infoHeader;
    FILE *fp_bmp = fopen(fileName, "wb");
    if (!fp_bmp) {
        printf("Failed to open %s\n", fileName);
        return BMP_FAILURE;
    }
    
    // fileHeader
    if (fwrite(fileHeader, sizeof(*fileHeader), 1, fp_bmp) != 1) {
        printf("Fwrite failure : %s\n", fileName);
        goto FUNC_FAIL;
    }

    // infoHeader
    if (fwrite(infoHeader, sizeof(*infoHeader), 1, fp_bmp) != 1) {
        printf("Fwrite failure : %s\n", fileName);
        goto FUNC_FAIL;
    }

    // PixelData
    if (fwrite(bmp->pixelData, bmp->pxSize, 1, fp_bmp) != 1) {
        printf("Fwrite failure : %s\n", fileName);
        goto FUNC_FAIL;
    }

    fclose(fp_bmp);
    if (DBG) printf("Created : %s\n", fileName);
    return BMP_SUCCESS;

    FUNC_FAIL:
    printf("Failed to write: %s\n", fileName);
    fclose(fp_bmp);
    return BMP_FAILURE;

}


/*
* Read a existing BMP file from the disk (uncompressed)
* Parameters:
* - bmp: Pointer to BMP structure to store the ne BMP file
* - filename: Filename of the bmp file need to read
*
* Returns:
* - BMP_SUCCESS (0) on success.
* - BMP_FAILURE (1) on failure.
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
    return BMP_SUCCESS;

    FUNC_FAIL:
    printf("Failed to read: %s\n", fileName);
    fclose(fp_bmp);
    return BMP_FAILURE;
}


// 
int BMP_FillSolid(BMP *bmp, uint8_t color[3]) {
    uint8_t *pixelData = malloc(bmp->pxSize);
    if (!pixelData) {
        printf("Failed to aloocate memory: BMP_PxSolid\n");
        return BMP_FAILURE;
    }

    int lineSize = 3 * bmp->infoHeader.Width;
    uint8_t *solidFill = malloc(lineSize);
    if (!solidFill) {
        printf("Failed to aloocate memory: BMP_PxSolid\n");
        return BMP_FAILURE;
    }
    for (int x = 0; x < bmp->infoHeader.Width; x++) {
        int index = x * 3;
            solidFill[index + 0] = color[0];
            solidFill[index + 1] = color[1];
            solidFill[index + 2] = color[2];
    }

    for (int y = 0; y < bmp->infoHeader.Height; y++) {
        int index = lineSize * y + bmp->padding;
        memcpy(&pixelData[index], solidFill, lineSize);
    }

    bmp->pixelData = pixelData;
    return BMP_SUCCESS;
}

// Needs some polishing
int BMP_FillGradient(BMP *bmp, uint8_t color1[3], uint8_t color2[3]) {
    uint8_t *pixelData = malloc(bmp->pxSize);
    if (!pixelData) {
        printf("Failed to aloocate memory: BMP_PxSolid\n");
        return BMP_FAILURE;
    }

    int lineSize = 3 * bmp->infoHeader.Width + bmp->padding;
    for (int y = 0; y < bmp->infoHeader.Height; y++) {
        for (int x = 0; x < bmp->infoHeader.Width; x++) {
            int index = y * lineSize + x * 3;
            float ratio = (float)x / (bmp->infoHeader.Width - 1);

            pixelData[index + 0] = color2[0] + (int)((color2[0] - color1[0]) * ratio);
            pixelData[index + 1] = color2[1] + (int)((color2[1] - color1[1]) * ratio);
            pixelData[index + 2] = color2[2] + (int)((color2[2] - color1[2]) * ratio);
        }
    }

    bmp->pixelData = pixelData;
    return BMP_SUCCESS;
}


// Some abstracted funcs
int BMP_CreateSolidFill(char *fileName, int height, int width, uint8_t color[3]) {
    if (height <= 0 || width <= 0) {
        fprintf(stderr, "Height or width can't be <= 0 : %s\n", fileName);
        return BMP_FAILURE;
    }
    BMP bmp;
    BMP_InitHeader(&bmp, width, height);
    if (BMP_FillSolid(&bmp, color) != BMP_SUCCESS) {
        return BMP_FAILURE;
    }
    if (BMP_CreateFile(&bmp, fileName) != BMP_SUCCESS) {
        return BMP_FAILURE;
    }
    free(bmp.pixelData);
}

int BMP_CreateGradientFill(char *fileName, int height, int width, uint8_t color1[3], uint8_t color2[3]) {
    if (height <= 0 || width <= 0) {
        fprintf(stderr, "Height or width can't be <= 0 : %s\n", fileName);
        return BMP_FAILURE;
    }
    BMP bmp;
    BMP_InitHeader(&bmp, width, height);
    if (BMP_FillGradient(&bmp, color1, color2) != BMP_SUCCESS) {
        return BMP_FAILURE;
    }
    if (BMP_CreateFile(&bmp, fileName) != BMP_SUCCESS) {
        return BMP_FAILURE;
    }
    free(bmp.pixelData);
}

#endif