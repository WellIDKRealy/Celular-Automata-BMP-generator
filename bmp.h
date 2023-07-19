#include <stdint.h>

#ifndef BMP_H
#define BMP_H

typedef struct {
  char red;
  char green;
  char blue;
} color;


typedef struct {
  int32_t size;
  color colors[];
} colortable;

colortable* make_empty_colortable(int32_t size);

typedef struct {
  int32_t x;
  int32_t y;
  colortable* color_table;
  char data[];
} image;

image* make_empty_image(int32_t x, int32_t y, colortable* color_table);

void image_set(image* img, int32_t x, int32_t y, char color);
void image_fill(image* img, char color);
char* img_to_bmp(image* img, int32_t* size);

#endif
