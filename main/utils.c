#include <stdbool.h>
#include <string.h>

#define CONTROLL_CHRS "[0123456789;"
#define ENDING_CHRS "m"

int remove_vt100(int in_size, char *in, int out_size, char *out) {
  int o = 0;
  bool esc = 0;
	for (int j = 0; j < in_size; j++) {
    if (in[j] == '\x1b') {
      esc = 1;
      continue;
    }
    if (esc && strchr(CONTROLL_CHRS, in[j])) continue;
    if (esc && strchr(ENDING_CHRS, in[j])) {
      esc = 0;
      continue;
    }
    if (o < out_size) {
      out[o] = in[j];
      o++;
    } else {
      break;
    }
  }
  return o;
}

int replace_tabs(int in_size, char *in, int out_size, char *out) {
  int o = 0;
  for (int j = 0; j < in_size; j++) {
    if (in[j] == '\t') {
      if (o + 4 < out_size) {
        for (int r = 0; r < 4; r++) {
          out[o] = ' ';
          o++;
        }
      }
      continue;
    } else if (o < out_size) {
      out[o] = in[j];
      o++;
    } else {
      break;
    }
  }
  return o;
}
