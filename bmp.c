
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"

colortable* make_empty_colortable(int32_t size) {
  colortable* ret = malloc(sizeof(colortable) + sizeof(color)*size);
  ret->size = size;
  return ret;
}

image* make_empty_image(int32_t x, int32_t y, colortable* color_table) {
  image* ret = malloc(sizeof(image) + x*y*sizeof(char));
  ret->x = x;
  ret->y = y;
  ret->color_table = color_table;
  return ret; 
}

void image_set(image* img, int32_t x, int32_t y, char color) {
  img->data[y*img->x + x] = color;
}

void image_fill(image* img, char color) {
  for(int32_t i = 0; i < img->x*img->y; i++) {
    img->data[i] = color;
  }
}

void img_to_bmp_reuse(image* img, char* bytes, int32_t row_size, int32_t data_size, int32_t* size) {
  bytes[0] = 'B';            // Signature
  bytes[1] = 'M';            // Signature
  *(int32_t*)(bytes + 2) = *size;      // File size
  *(int32_t*)(bytes + 6) = 0;          // Reserved
  *(int32_t*)(bytes + 10) = 54;        // Data offset
  *(int32_t*)(bytes + 14) = 40;        // DIB header size
  *(int32_t*)(bytes + 18) = img->x;    // Width
  *(int32_t*)(bytes + 22) = img->y;    // Height
  *(int32_t*)(bytes + 26) = 1;         // planes
  *(int16_t*)(bytes + 28) = 24;        // Bits per pixel
  *(int16_t*)(bytes + 30) = 0;         // Compression
  *(int32_t*)(bytes + 34) = data_size; // Data size
  *(int32_t*)(bytes + 38) = 3780;      // 96 DPI X
  *(int32_t*)(bytes + 42) = 3780;      // 96 DPI Y
  *(int32_t*)(bytes + 46) = 0;         // Colors in palette
  *(int32_t*)(bytes + 50) = 0;         // Important colors

  char* pixel_array = bytes + 54;
  for(int32_t y = 0; y < img->y; y++) {
    for(int32_t x = 0; x < img->x; x++) {
      color c = img->color_table->colors[img->data[y*img->x + x]];
      pixel_array[y*row_size + x*3] = c.red;
      pixel_array[y*row_size + x*3 + 1] = c.green;
      pixel_array[y*row_size + x*3 + 2] = c.blue;
    }
  }
}

char* img_to_bmp(image* img, int32_t* size) {
  int32_t row_size = ((img->x*3 + 3)/4)*4;
  int32_t data_size = row_size*img->y;
  *size = 54 + data_size;

  char* ret = malloc(*size);
  img_to_bmp_reuse(img, ret, row_size, data_size, size);
  
  return ret;
}
