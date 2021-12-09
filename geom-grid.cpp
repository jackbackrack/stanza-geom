// See license.txt for details about licensing.

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>

static inline int idx (int w, int h, int d, int i, int j, int ak) {
  // TODO: FIX THIS FOR 3D
  const int k = 0;
  return (k * w * h) + j * w + i;
}

static inline uint8_t get (uint8_t* g, int w, int h, int d, int i, int j, int k) {
  // if (i < 0 || i >= w || j < 0 || j >= h || k < 0 || k >= d) {
  //   printf("GET BAD INDEX %d %d %d (%d %d %d)\n", i, j, k, w, h, d);
  //   return 0;
  // } else 
  return g[idx(w, h, d, i, j, k)];
}

static inline void set (uint8_t v, uint8_t* g, int w, int h, int d, int i, int j, int k) {
  // if (i < 0 || i >= w || j < 0 || j >= h || k < 0 || k >= d)
  //   printf("SET BAD INDEX %d %d %d (%d %d %d)\n", i, j, k, w, h, d);
  // else 
  g[idx(w, h, d, i, j, k)] = v;
}

static inline uint8_t safe_get (uint8_t* g, int w, int h, int d, int i, int j, int k) {
  if (i < 0 || i >= w || j < 0 || j >= h || k < 0 || k >= d)
    return 0;
  else {
    return g[idx(w, h, d, i, j, k)];
  }
}

static inline void safe_set (uint8_t v, uint8_t* g, int w, int h, int d, int i, int j, int k) {
  if (!(i < 0 || i >= w || j < 0 || j >= h || k < 0 || k >= d))
    g[idx(w, h, d, i, j, k)] = v;
}

struct v3i_t {
  int x;
  int y;
  int z;
};

static inline const v3i_t V3i_t (int i, int j, int k) {
  const v3i_t res = {i, j, k};
  return res;
}

static inline const v3i_t V3i_t (int i, int j, int k, const v3i_t off) {
  v3i_t res = {i + off.x, j + off.y, k + off.z};
  return res;
}

const struct v3i_t deltas[] = {{-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0},
                               {-1, -1, 0}, {-1, 1, 0}, {1, -1, 0}, {1, 1, 0}};

static const int n_deltas = 8;

static void print_grid (int w, int h, int d, uint8_t* grid) {
  for (int k = 0; k < d; k++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        printf("%d ", get(grid, w, h, d, i, j, k));
      }
      printf("\n");
    }
  }
}

extern "C" void find_limits (int w, int h, int d, float* src, int* limits);

static inline int min (int a, int b) { return a < b ? a : b; }
static inline int max (int a, int b) { return a > b ? a : b; }

void find_limits (int w, int h, int d, uint8_t* src, int* limits) {
  int mx = (1 << 30);
  int mn = -mx;
  limits[0] = mx;
  limits[1] = mx;
  limits[2] = mx;
  limits[3] = mn;
  limits[4] = mn;
  limits[5] = mn;
  for (int k = 0; k < d; k++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        uint8_t v = get(src, w, h, d, i, j, k);
        if (v > 0) {
          limits[0] = min(limits[0], i);
          limits[1] = min(limits[1], j);
          limits[2] = min(limits[2], k);
          limits[3] = max(limits[3], i);
          limits[4] = max(limits[4], j);
          limits[5] = max(limits[5], k);
          // printf("  [%d,%d,%d] LIMITS [%d,%d,%d] -> [%d,%d,%d]\n", i, j, k, limits[0], limits[1], limits[2], limits[3], limits[4], limits[5]);
        }
      }
    }
  }
  // printf("LIMITS [%d,%d,%d] -> [%d,%d,%d]\n", limits[0], limits[1], limits[2], limits[3], limits[4], limits[5]);
}

extern "C" void grid_fill (int w, int h, int d, uint8_t* dst, uint8_t val) {
  // printf("GRID-FILL %d [%d %d %d]\n", val, w, h, d);
  size_t num = w * h * d;
  for (size_t k = 0; k < num; k++)
    dst[k] = val;
}

extern "C" void grid_fill_range (int w, int h, int d, int x0, int y0, int z0, int x1, int y1, int z1, uint8_t* dst, uint8_t val) {
  // printf("GRID-FILL %d [%d %d %d] FROM [%d %d %d] TO [%d %d %d]\n", val, w, h, d, x0, y0, z0, x1, y1, z1);
  for (int k = z0; k <= z1; k++) 
    for (int j = max(0, y0); j <= min(h - 1, y1); j++) 
      for (int i = max(0, x0); i <= min(w - 1, x1); i++) 
        set(val, dst, w, h, d, i, j, k);
}

extern "C" int grid_equals (int w, int h, int d, uint8_t* a, uint8_t* b) {
  size_t num = w * h * d;
  for (size_t k = 0; k < num; k++)
    if (a[k] != b[k]) return 0;
  return 1;
}

// extern "C" void grid_invert_off (int dw, int dh, int dd, int dx, int dy, int dz, uint8_t* dst,
//                                  int sw, int sh, int sd, uint8_t* src) {
//   // printf("GRID-INVERT [%d %d %d] OFF [%d %d %d] [%d %d %d]\n", dw, dh, dd, dx, dy, dz, sw, sh, sd);
//   for (int k = 0; k < sd; k++) 
//     for (int j = 0; j < sh; j++) 
//       for (int i = 0; i < sw; i++) 
//         set(1 - get(src, sw, sh, sd, i, j, k), dst, dw, dh, dd, i + dx, j + dy, k + dz);
// }

extern "C" void grid_invert (int w, int h, int d, uint8_t* dst, uint8_t* src) {
  // printf("GRID-INVERT [%d %d %d]\n", w, h, d);
  size_t num = w * h * d;
  for (int n = 0; n < num; n++) 
    dst[n] = 1 - src[n];
}

extern "C" void grid_copy_off (int dw, int dh, int dd, int dx, int dy, int dz, uint8_t* dst,
                               int sw, int sh, int sd, uint8_t* src) {
  // printf("GRID-COPY [%d %d %d] OFF [%d %d %d]  [%d %d %d]\n", dw, dh, dd, dx, dy, dz, sw, sh, sd);
  for (int k = 0; k < sd; k++) 
    for (int j = 0; j < sh; j++) 
      for (int i = 0; i < sw; i++) 
        set(get(src, sw, sh, sd, i, j, k), dst, dw, dh, dd, i + dx, j + dy, k + dz);
}

extern "C" void grid_copy (int w, int h, int d, uint8_t* dst, uint8_t* src) {
  // printf("GRID-COPY [%d %d %d]\n", w, h, d);
  size_t num = w * h * d;
  for (int n = 0; n < num; n++) 
    dst[n] = src[n];
}

extern "C" void grid_union_off (int dw, int dh, int dd, int dx, int dy, int dz, uint8_t* dst, uint8_t* s0,
                                int sw, int sh, int sd, uint8_t* s1) {
  // printf("GRID-UNION [%d %d %d] OFF [%d %d %d] [%d %d %d]\n", dw, dh, dd, dx, dy, dz, sw, sh, sd);
  for (int k = 0; k < sd; k++) 
    for (int j = 0; j < sh; j++) 
      for (int i = 0; i < sw; i++) 
        safe_set(safe_get(s0, dw, dh, dd, i + dx, j + dy, k + dz) | get(s1, sw, sh, sd, i, j, k), dst, dw, dh, dd, i + dx, j + dy, k + dz);
}

extern "C" void grid_union (int w, int h, int d, uint8_t* dst, uint8_t* s0, uint8_t* s1) {
  // printf("GRID-UNION [%d %d %d]\n", w, h, d);
  size_t num = w * h * d;
  for (int n = 0; n < num; n++) 
    dst[n] = s0[n] | s1[n];
}

// extern "C" void grid_intersect_off (int dw, int dh, int dd, int dx, int dy, int dz, uint8_t* dst, uint8_t* s0,
//                                     int sw, int sh, int sd, uint8_t* s1) {
//   // printf("GRID-INTERSECT [%d %d %d] OFF [%d %d %d] [%d %d %d]\n", dw, dh, dd, dx, dy, dz, sw, sh, sd);
//   for (int k = 0; k < sd; k++) 
//     for (int j = 0; j < sh; j++) 
//       for (int i = 0; i < sw; i++) 
//         set(get(s0, dw, dh, dd, i + dx, j + dy, k + dz) & get(s1, sw, sh, sd, i, j, k),
//             dst, dw, dh, dd, i + dx, j + dy, k + dz);
// }

extern "C" void grid_intersect (int w, int h, int d, uint8_t* dst, uint8_t* s0, uint8_t* s1) {
  // printf("GRID-INTERSECT [%d %d %d]\n", w, h, d);
  size_t num = w * h * d;
  for (int n = 0; n < num; n++) 
    dst[n] = s0[n] & s1[n];
}

extern "C" void grid_convolve_around
    (int dw, int dh, int dd, uint8_t* dst, int sw, int sh, int sd, uint8_t* src, int mw, int mh, int md, uint8_t* msk) {
  // printf("GRID-CONVOLVE [%d %d %d] MASK [%d %d %d]\n", dw, dh, dd, mw, mh, md);
  for (int dk = 0; dk < dd; dk++) {
    int edk = dk;
    for (int dj = 0; dj < (dh - mh); dj++) { 
      int edj = dj - mh;   // J OFFSET FOR PLACING MASK
      for (int di = 0; di < (dw - mw); di++) {
        int edi = di - mw; // I OFFSET FOR PLACING MASK
        uint8_t pix = 0;
        // TODO: FIX THIS FOR 3D
        for (int mk = 0; mk < md; mk++) {
          int edmk = edk + mk;
          for (int edmj = max(0, edj); edmj < min(sh, mh + edj); edmj++) { 
            for (int edmi = max(0, edi); edmi < min(sw, mw + edi); edmi++) {
              uint8_t a = get(src, sw, sh, sd, edmi, edmj, edmk); 
              uint8_t b = get(msk, mw, mh, md, edmi - edi, edmj - edj, mk);
              if ((a & b) != 0) { pix = 1; goto done; }
              // pix = pix | (a & b);
            }
          }
        }
      done:
        set(pix, dst, dw, dh, dd, di, dj, dk);
      }
    }
  }
}

extern "C" void grid_convolve (int dw, int dh, int dd, uint8_t* dst, int sw, int sh, int sd, uint8_t* src, int mw, int mh, int md, uint8_t* msk) {
  // printf("GRID-CONVOLVE [%d %d %d] MASK [%d %d %d]\n", dw, dh, dd, mw, mh, md);
  for (int dk = 0; dk < dd; dk++) {
    int edk = dk;
    for (int dj = 0; dj < dh; dj++) { 
      int edj = dj;
      for (int di = 0; di < dw; di++) {
        int edi = di;
        uint8_t pix = 0;
        // TODO: FIX THIS FOR 3D
        for (int mk = 0; mk < md; mk++) {
          int edmk = edk + mk;
          if (((mh + edj) > sh) || ((mw + edi) > sw)) {
            // printf("OUTSIDE 1\n");
            pix = 1; goto done;
          } else {
            for (int edmj = max(0, edj); edmj < (mh + edj); edmj++) { 
              for (int edmi = max(0, edi); edmi < (mw + edi); edmi++) {
                uint8_t a = get(src, sw, sh, sd, edmi, edmj, edmk); 
                uint8_t b = get(msk, mw, mh, md, edmi - edi, edmj - edj, mk);
                // printf("S[%d,%d] A %d M[%d,%d] B %d\n", edmi, edmj, a, edmi - edj, edmj - edj, b);
                if ((a & b) != 0) { pix = 1; goto done; }
              }
            }
          }
        }
      done:
        // printf(">>> SET [%d,%d] = %d\n", di, dj, pix);
        set(pix, dst, dw, dh, dd, di, dj, dk);
      }
    }
  }
}

extern "C" void grid_dilate (int dw, int dh, int dd, uint8_t* dst, uint8_t* src) {
  // printf("GRID-DILATE [%d %d %d]\n", dw, dh, dd);
  for (int dk = 0; dk < dd; dk++) {
    for (int dj = 0; dj < dh; dj++) { 
      for (int di = 0; di < dw; di++) {
        uint8_t pix = 0;
        for (int mk = -1; mk <= 1; mk++) {
          int edk = dk + mk;
          for (int mj = -1; mj <= 1; mj++) {
            int edj = dj + mj;
            for (int mi = -1; mi <= 1; mi++) {
              int edi = di + mi;
              if (edk >= 0 && edk < dd && edj >= 0 && edj < dh && edi >= 0 && edi < dw) {
                if (get(src, dw, dh, dd, edi, edj, edk)) { pix = 1; goto done; }
              }
            }
          }
        }
      done:
        // printf(">>> SET [%d,%d] = %d\n", di, dj, pix);
        set(pix, dst, dw, dh, dd, di, dj, dk);
      }
    }
  }
}

/*
extern "C" void grid_convolve (int dw, int dh, int dd, uint8_t* dst, int sw, int sh, int sd, uint8_t* src, int mw, int mh, int md, uint8_t* msk) {
  printf("GRID-CONVOLVE [%d %d %d] MASK [%d %d %d]\n", dw, dh, dd, mw, mh, md);
  for (int dk = 0; dk < dd; dk++) {
    int edk = dk;
    for (int dj = 0; dj < dh; dj++) { 
      int edj = dj - mh;
      for (int di = 0; di < dw; di++) {
        int edi = di - mw;
        uint8_t pix = 0;
        // TODO: FIX THIS FOR 3D
        for (int mk = 0; mk < md; mk++) {
          int edmk = edk + mk;
          for (int edmj = max(0, edj); edmj < min(sh, mh + edj); edmj++) { 
            for (int edmi = max(0, edi); edmi < min(sw, mw + edi); edmi++) {
              uint8_t a = get(src, sw, sh, sd, edmi, edmj, edmk); 
              uint8_t b = get(msk, mw, mh, md, edmi - edi, edmj - edj, mk);
              printf("S[%d,%d] A %d M[%d,%d] B %d\n", edmi, edmj, a, edmi - edi, edmj - edj, b);
              if ((a & b) != 0) { pix = 1; goto done; }
            }
          }
        }
      done:
        printf("SET [%d,%d] = %d\n", di, dj, pix);
        set(pix, dst, dw, dh, dd, di, dj, dk);
      }
    }
  }
}
*/

extern "C" void grid_ones_box (int w, int h, int d, uint8_t* src, v3i_t *lo, v3i_t *hi) {
  int lo_x = 10000000;
  int lo_y = 10000000;
  int lo_z = 10000000;
  int hi_x = -1;
  int hi_y = -1;
  int hi_z = -1;
  for (int k = 0; k < d; k++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (get(src, w, h, d, i, j, k) == 1) {
          lo_x = min(lo_x, i);
          lo_y = min(lo_y, j);
          lo_z = min(lo_z, k);
          hi_x = max(hi_x, i);
          hi_y = max(hi_y, j);
          hi_z = max(hi_z, k);
        }
      }
    }
  }
  lo->x = lo_x;
  lo->y = lo_y;
  lo->z = lo_z;
  hi->x = hi_x;
  hi->y = hi_y;
  hi->z = hi_z;
}

extern "C" int grid_count_ones (int w, int h, int d, uint8_t* src, v3i_t *lo, v3i_t *hi) {
  int tot = 0;
  for (int k = 0; k < d; k++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (get(src, w, h, d, i, j, k) == 1) {
          tot += 1;
        }
      }
    }
  }
  return tot;
}

extern "C" std::vector<v3i_t> *todos_fab () {
  std::vector<v3i_t> *res = new std::vector<v3i_t>();
  return res;
}

extern "C" int *todos_del (std::vector<v3i_t> *v) {
  delete v;
  return 0;
}

extern "C" int *todos_clear (std::vector<v3i_t> *v) {
  v->clear();
  return 0;
}

extern "C" int *todos_elt (std::vector<v3i_t> *v, int i, int *pos) {
  v3i_t res = v->at(i);
  pos[0] = res.x;
  pos[1] = res.y;
  pos[2] = res.z;
  return 0;
}

extern "C" int todos_len (std::vector<v3i_t> *v) {
  return v->size();
}

extern "C" void grid_find_ones (int w, int h, int d, uint8_t* src, std::vector<v3i_t> *todos) {
  for (int k = 0; k < d; k++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (get(src, w, h, d, i, j, k) == 1) {
          todos->push_back(V3i_t(i, j, k));
        }
      }
    }
  }
}

extern "C" int grid_hash (int w, int h, int d, uint8_t* src) {
  int res = w * h * d;
  for (int k = 0; k < d; k++) 
    for (int j = 0; j < h; j++) 
      for (int i = 0; i < w; i++) 
        res += (7 * get(src, w, h, d, i, j, k));
  return res;
}

