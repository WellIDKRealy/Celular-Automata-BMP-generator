#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "bmp.h"

#define true 1

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
  int size = ipow(radix, width);
  int* cases = malloc(sizeof(int)*size);
  for(int i = 0; i < size; i++) {
    cases[i] = def;
  }

  int code;
  for(int i = 0; scases[i] != NULL; i++) {
    code = 0;
    for(int j = 1; j <= width; j++) {
      code *= radix;
      code += scases[i][j];
    }
    cases[code] = scases[i][0];
  }
  return make_rules(width, radix, cases);
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
    image_set(img, i, 0, state.data[i]);
  }

  for(int i = 1; i < depth; i++) {
    automata(state, swap_state, rules);

    swap_tmp = state.data;
    state.data = swap_state;
    swap_state = swap_tmp;

    for(int j = 0; j < state.width; j++) {
      image_set(img, j, i, state.data[j]);
    }
  }

  return img;
}

int main() {
  srand(time(NULL));

  colortable* color_table = make_empty_colortable(8); 
  color_table->colors[0] = (color){0,   0,   0  };
  color_table->colors[1] = (color){255, 255, 255};
  color_table->colors[2] = (color){255, 0,   0  };
  color_table->colors[3] = (color){0,   255, 0  };
  color_table->colors[4] = (color){0,   0,   255};
  color_table->colors[5] = (color){255, 255, 0  };
  color_table->colors[6] = (color){0,   255, 255};
  color_table->colors[7] = (color){255, 0,   255};

  state state = make_center_state(ipow(2, level + 1) - 1, 1, 0);
  // state state = make_random_state(40, 2);
  //rules rules = make_random_rules(3, 2);

  int* spec[3];
  spec[0] = (int[]){1, 0, 0, 1};
  spec[1] = (int[]){1, 1, 0, 0};
  spec[2] = NULL;
  
  rules rules = make_specific_rules(3, 2, 0, spec);

  image* img = make_automata_image(state, rules, ipow(2, level), color_table);
  
  /* char ref_table[2] = {' ', '#'}; */

  /* display_automata(state, rules, ipow(2, level), ref_table); */
  /* fflush(stdout); */

  int32_t size;
  char* bytes = img_to_bmp(img, &size);
  
  FILE* fptr = fopen("out.bmp", "w");
  fwrite(bytes, size, 1, fptr);
  fclose(fptr);
  
}
