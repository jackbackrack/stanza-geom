#include "clipper.hpp"
using namespace ClipperLib;

extern "C" {

struct clipper_vec2 { float x, y; };

struct clipper_path {
  clipper_vec2* pts;
  uint32_t count;
};

struct clipper_paths {
  clipper_path* ps;
  uint32_t count;
};

clipper_paths* clipper_fab_paths(int num_paths);
void clipper_fab_path(clipper_paths* paths, int path_idx, int num_pts);
void clipper_paths_set_point(clipper_paths* paths, int path_idx, int point_idx, float x, float y);
clipper_paths* clipper_offset(clipper_paths *paths, float amount, float res);
void clipper_paths_delete(clipper_paths* ps);

};

inline cInt float_to_cint (float d, float res) {
  // return static_cast<cInt>(((double)d) * ((double)(1 << log2n)) + 0.5);
  return static_cast<cInt>(((double)d) * ((double)res) + 0.5);
}

inline float cint_to_float (cInt d, float res) {
  // float i = (float)(d >> log2n);
  // int mask = ((1 << log2n) - 1);
  // float f = ((float)(d & mask)) / ((float)mask);
  // printf("D %lld LOG2N %d I %f F %f MASK %d RES %f\n", d, log2n, i, f, mask, i + f);
  // return i + f;
  return static_cast<float>(((double)d) / res);
}

clipper_paths* clipper_offset(clipper_paths *paths, float amount, float res) {
  Paths inputs;
  ClipperOffset co;
  int log2n = 10;
  // printf("PATHS %d\n", paths->count);
  for (uint32_t j = 0; j < paths->count; j++) {
    Path input;
    clipper_path path = paths->ps[j];
    // printf("  PATH %d\n", path.count);
    for (uint32_t i = 0; i < path.count; i++) {
      cInt x = float_to_cint(path.pts[i].x, res);
      cInt y = float_to_cint(path.pts[i].y, res);
      // printf("    [%lld %lld]\n", x, y);
      input << IntPoint(x, y);
    }
    co.AddPath(input, jtMiter, etClosedPolygon);
  }
  Paths solution;
  // printf("EXECUTING ...\n");
  co.Execute(solution, amount * res);
  clipper_paths* ret = (clipper_paths*)malloc(sizeof(clipper_paths));
  ret->ps = (clipper_path*)malloc(sizeof(clipper_path) * solution.size());
  ret->count = solution.size();
  // printf("RES PATHS %d\n", ret->count);
  for (uint32_t j = 0; j < solution.size(); j++) {
    Path elt = solution[j];
    ret->ps[j].pts = (clipper_vec2*)malloc(sizeof(clipper_vec2) * elt.size());
    ret->ps[j].count = elt.size();
    // printf("  RET PATH %lu\n", elt.size());
    for (uint32_t i = 0; i < elt.size(); i++) {
      // printf("    [%lld %lld]\n", elt[i].X, elt[i].Y);
      ret->ps[j].pts[i].x = cint_to_float(elt[i].X, res);
      ret->ps[j].pts[i].y = cint_to_float(elt[i].Y, res);
    }
  }
  return ret;
}

clipper_paths* clipper_fab_paths(int num_paths) {
  clipper_paths* res = (clipper_paths*)malloc(sizeof(clipper_paths));
  res->ps = (clipper_path*)malloc(sizeof(clipper_path) * num_paths);
  res->count = num_paths;
  return res;
}

void clipper_fab_path(clipper_paths* paths, int path_idx, int num_pts) {
  paths->ps[path_idx].pts = (clipper_vec2*)malloc(sizeof(clipper_vec2) * num_pts);
  paths->ps[path_idx].count = num_pts;
}

void clipper_paths_set_point(clipper_paths* paths, int path_idx, int point_idx, float x, float y) {
  paths->ps[path_idx].pts[point_idx].x = x;
  paths->ps[path_idx].pts[point_idx].y = y;
}

void clipper_paths_delete(clipper_paths *paths) {
  for (uint32_t j = 0; j < paths->count; j++) {
    clipper_path path = paths->ps[j];
    free(path.pts);
  }
  free(paths);
}
