#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

#include "bmp.h"

#define true 1

#define X 600
#define Y 800

#define THREADS 20

#define RADIX 2
#define WIDTH 3

/* int ipow(int base, int exp) { */
/*   int result = 1; */
/*   while(true) { */
/*     if(exp & 1) { */
/*       result *= base; */
/*     } */
/*     exp >>= 1; */
/*     if(!exp) { */
/*       break; */
/*     } */
/*     base *= base; */
/*   } */
/* } */

int ilog10(int i) {
  int ret = -1;
  for(; i != 0; i /= 10) {
    ret++;
  }
  return ret;
}

int ipow(int base, int exp) {
  int result = 1;
  for(; exp != 0; exp--) {
    result *= base;
  }
  return result;
}

typedef struct {
  int width;
  int min;
  int max;
  int radix;
  int* cases;
} rules;

rules make_rules(int width, int radix, int* cases) {
  rules ret;
  ret.width = width;
  ret.min = -width/2;
  ret.max = width/2 + (width % 2 == 0 ? 0 : 1); 
  ret.radix = radix;
  ret.cases = cases;
  return ret;
}

rules make_random_rules(int width, int radix) {
  size_t size = ipow(radix, width);
  int* cases = malloc(sizeof(int)*size);
  for(size_t i = 0; i < size; i++) {
    cases[i] = rand() % radix;
  }
  return make_rules(width, radix, cases);
}

rules make_specific_rules(int width, int radix, int def, int** scases) {
  size_t size = ipow(radix, width);
  int* cases = malloc(sizeof(int)*size);
  for(int i = 0; i < size; i++) {
    cases[i] = def;
  }

  int code;
  for(size_t i = 0; scases[i] != NULL; i++) {
    code = 0;
    for(int j = 1; j <= width; j++) {
      code *= radix;
      code += scases[i][j];
    }
    cases[code] = scases[i][0];
  }
  return make_rules(width, radix, cases);
}

rules make_empty_rules(int width, int radix) {
  size_t size = ipow(radix, width);
  int* cases = malloc(sizeof(int)*size);

  for(; size != 0; size--) {
    cases[size - 1] = 0;
  }
  
  return make_rules(width, radix, malloc(sizeof(int)*ipow(radix, width)));
}

rules copy_rules(rules ruls) {
  rules ret;
  ret.width = ruls.width;
  ret.min = ruls.min;
  ret.max = ruls.max;
  ret.radix = ruls.radix;

  size_t size = ipow(ruls.radix, ruls.width);
  ret.cases = malloc(sizeof(int)*size);
  memcpy(ret.cases, ruls.cases, size);
  
  return ret;
}

void rules_next(rules rules) {
  for(int i = 0; true; i++) {
    rules.cases[i] += 1;
    if(rules.cases[i] != rules.radix) {
      break;
    }
    rules.cases[i] = 0;
  }
}

void rules_next_by(rules rules, int by) {
  for(int i = 0; true; i++) {
    rules.cases[i] += by;
    if(rules.cases[i] < rules.radix) {
      break;
    }
    by = rules.cases[i]/rules.radix;
    rules.cases[i] %= rules.radix;
  }
}

typedef struct {
  int width;
  int* data;
} state;

state make_state(int width, int* data) {
  state ret;
  ret.width = width;
  ret.data = data;
  return ret;
}

state make_empty_state(int width) {
  return make_state(width, malloc(sizeof(int)*width));
}

state make_random_state(int width, int radix) {
  int* data = malloc(sizeof(int)*width);
  for(int i = 0; i < width; i++) {
    data[i] = rand() % radix;
  }
  return make_state(width, data);
}

state make_center_state(int width, int val, int def) {
  int* data = malloc(sizeof(int)*width);
  for(int i = 0; i < width; i++) {
    data[i] = def;
  }
  data[width/2] = val;
  return make_state(width, data);
}

void set_center_state(state state, int val, int def) {
  for(int i = 0; i < state.width; i++) {
    state.data[i] = def;
  }
  state.data[state.width/2] = val;
}

int* automata(state state, int* new_state, rules rules) {
  uint code;

  for(int i = 0; i < -rules.min; i++) {
    code = 0;
    for(int j = rules.min; j < rules.max; j++) {
      code *= rules.radix;
      code += state.data[(state.width + i + j) % state.width];
    }
    new_state[i] = rules.cases[code];
  }
  
  for(int i = -rules.min; i < state.width - rules.max; i++) {
    code = 0;
    for(int j = rules.min; j < rules.max; j++) {
      code *= rules.radix;
      code += state.data[i + j];
    }
    new_state[i] = rules.cases[code];
  }

 for(int i = state.width - rules.max; i < state.width; i++) {
    code = 0;
    for(int j = rules.min; j < rules.max; j++) {
      code *= rules.radix;
      code += state.data[(state.width + i + j) % state.width];
    }
    new_state[i] = rules.cases[code];
  } 
  
  return new_state;
}

image* make_automata_image(state state, rules rules, int depth, colortable* color_table) {
  image* img = make_empty_image(state.width, depth, color_table);

  int* swap_state = malloc(sizeof(int)*state.width);
  int* swap_tmp;

  for(int i = 0; i < state.width; i++) {
    image_set(img, i, depth - 1, state.data[i]);
  }

  for(depth -= 1; depth != 0; depth--) {
    automata(state, swap_state, rules);

    swap_tmp = state.data;
    state.data = swap_state;
    swap_state = swap_tmp;

    for(int j = 0; j < state.width; j++) {
      image_set(img, j, depth - 1, state.data[j]);
    }
  }

  return img;
}

void set_automata_image_sstate(state state, int* swap_state, rules rules, int depth, image* img) {
  int* swap_tmp;

  for(int i = 0; i < state.width; i++) {
    image_set(img, i, depth - 1, state.data[i]);
  }

  for(depth -= 1; depth != 0; depth--) {
    automata(state, swap_state, rules);

    swap_tmp = state.data;
    state.data = swap_state;
    swap_state = swap_tmp;

    for(int j = 0; j < state.width; j++) {
      image_set(img, j, depth - 1, state.data[j]);
    }
  }
}

image* sierpinski_image(int level, colortable* color_table) {
  state state = make_center_state(ipow(2, level + 1) - 1, 1, 0);

  int* spec[3];
  spec[0] = (int[]){1, 0, 0, 1};
  spec[1] = (int[]){1, 1, 0, 0};
  spec[2] = NULL;
   
  rules rules = make_specific_rules(3, 2, 0, spec);
  return make_automata_image(state, rules, ipow(2, level), color_table);
}

typedef struct {
  int space;
  int32_t row_size;
  int32_t data_size;
  int32_t size;
  colortable* color_table;
  rules rules;
  int from;
} gen_images_data;

void* gen_images(void* pass) {
  gen_images_data* data = (gen_images_data*)pass;
    
  image* img = make_empty_image(X, Y, data->color_table);
  char* bytes = malloc(sizeof(char)*data->size);

  state state = make_empty_state(X);
  int* swap_state = malloc(sizeof(int)*state.width);

  char str[ilog10(data->size) + 13];
  for(int i = data->from; i < data->space; i += THREADS) {
    set_center_state(state, 1, 0);
    set_automata_image_sstate(state, swap_state, data->rules, Y, img);
    img_to_bmp_reuse(img, bytes, data->row_size, data->data_size, &data->size);

    sprintf(str, "images/%d.bmp\0", i);

    FILE* fptr = fopen(str, "w");
    fwrite(bytes, data->size, 1, fptr);
    fclose(fptr);

    rules_next_by(data->rules, THREADS);
    //rules_next(data->rules);
  }
}

int main() {
  // srand(time(NULL));

  colortable* color_table = make_empty_colortable(8); 
  color_table->colors[0] = (color){0,   0,   0  };
  color_table->colors[1] = (color){255, 255, 255};
  color_table->colors[2] = (color){255, 0,   0  };
  color_table->colors[3] = (color){0,   255, 0  };
  color_table->colors[4] = (color){0,   0,   255};
  color_table->colors[5] = (color){255, 255, 0  };
  color_table->colors[6] = (color){0,   255, 255};
  color_table->colors[7] = (color){255, 0,   255};

  struct stat st;
  if (stat("images", &st) != 0 || ~S_ISDIR(st.st_mode)) {
    mkdir("images", 0700);
  }
  
  int space = ipow(RADIX, ipow(RADIX, WIDTH));

  int32_t row_size = ((X*3 + 3)/4)*4;
  int32_t data_size = row_size*Y;
  int32_t size = 54 + data_size;

  rules rules = make_empty_rules(WIDTH, RADIX);

  pthread_t threads[THREADS];
  int thread = 0;
  while(true) {
    gen_images_data* pass = malloc(sizeof(gen_images_data));
    pass->space = space;
    pass->row_size = row_size;
    pass->data_size = data_size;
    pass->size = size;
    pass->color_table = color_table;
    pass->rules = copy_rules(rules);
    pass->from = thread;
    
    pthread_create(&threads[thread], NULL, gen_images, (void*)pass);

    thread++;
    if(thread == THREADS) {
      break;
    }
    rules_next(rules);
  }

  for(int i = 0; i < THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
}
