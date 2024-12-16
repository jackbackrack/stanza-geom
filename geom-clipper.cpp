// See license.txt for details about licensing.

#include <stdint.h>
#include <unordered_map>
#include "clipper2/clipper.h"
// #include "clipper.hpp"
using namespace Clipper2Lib;

extern "C" {

struct clipper_vec2 {
  double x;
  double y;
};

struct clipper_path {
  clipper_vec2* pts;
  uint32_t count;
};

struct clipper_paths {
  clipper_path* ps;
  uint32_t count;
};

const double SIMPLE_RES = 2.5;

clipper_path* clipper_fab_path(int num_pts);
clipper_vec2* clipper_fab_vec2(double x, double y);
void clipper_path_set_point(clipper_path* path, int point_idx, double x, double y);
clipper_paths* clipper_fab_paths(int num_paths);
void clipper_fab_paths_path(clipper_paths* paths, int path_idx, int num_pts);
void clipper_paths_set_point(clipper_paths* paths, int path_idx, int point_idx, double x, double y);
clipper_paths* clipper_offset(clipper_paths *paths, double amount, double res);
clipper_paths* clipper_inflate_paths(clipper_paths *paths, double amount, double res);
clipper_paths* clipper_op(clipper_paths *a, clipper_paths *b, int op, double res);
clipper_paths* clipper_polyline_op(clipper_paths *a, clipper_paths *b, int op, double res);
clipper_paths* clipper_minkowski_diff(clipper_path *fpattern, clipper_path *fpath, double res);
int clipper_point_in_polygon(clipper_vec2 *pt, clipper_path* poly, double res);
void clipper_paths_delete(clipper_paths* ps);
void clipper_path_delete(clipper_path* p);
void clipper_vec2_delete(clipper_vec2* p);

Clipper64* clipper_fab_clipper64(void);
void clipper_clipper64_delete(Clipper64 *c64);

int clipper_add_subject(Clipper64* co, clipper_paths *a, double res);
int clipper_add_open_subject(Clipper64* co, clipper_paths *a, double res);
int clipper_add_clip(Clipper64* co, clipper_paths *a, double res);
clipper_paths* clipper_open_execute(Clipper64* co, int op, double res);
clipper_paths* clipper_execute(Clipper64* co, int op, double res);

};

size_t tot_allocd = 0; 

std::unordered_map<size_t,size_t> allocs;

void* jmalloc (size_t size) {
  // tot_allocd += size;
  void* res = malloc(size);
  // allocs.insert_or_assign((size_t)res, size);
  // std::cout << "  ALLOC " << size << " = " << tot_allocd << std::endl;
  return res;
}

void jfree (void* ptr) {
  // size_t size = allocs[(size_t)ptr];
  // allocs.erase((size_t)ptr);
  // tot_allocd -= size;
  // std::cout << "  FREE " << size << " = " << tot_allocd << std::endl;
  free(ptr);
}

inline int64_t double_to_cint (double d, double res) {
  // return static_cast<cInt>(((double)d) * ((double)(1 << log2n)) + 0.5);
  return static_cast<int64_t>(((double)d) * ((double)res) + 0.5);
}

inline double cint_to_double (int64_t d, double res) {
  // double i = (double)(d >> log2n);
  // int mask = ((1 << log2n) - 1);
  // double f = ((double)(d & mask)) / ((double)mask);
  // printf("D %lld LOG2N %d I %f F %f MASK %d RES %f\n", d, log2n, i, f, mask, i + f);
  // return i + f;
  return static_cast<double>(((double)d) / res);
}

Path64 clipper_path_to_path64 (clipper_path *cp, double res) {
  Path64 pattern;
  for (uint32_t i = 0; i < cp->count; i++) {
    int64_t x = double_to_cint(cp->pts[i].x, res);
    int64_t y = double_to_cint(cp->pts[i].y, res);
    // std::cout << x << "," << y << std::endl;
    pattern.push_back(Point64(x, y));
  }
  return pattern;
}

clipper_paths* paths64_to_clipper_paths (Paths64 p, double res) {
  clipper_paths* ret = clipper_fab_paths(p.size());
  for (uint32_t j = 0; j < p.size(); j++) {
    Path64 elt = p[j];
    clipper_fab_paths_path(ret, j, elt.size());
    for (uint32_t i = 0; i < elt.size(); i++) {
      ret->ps[j].pts[i].x = cint_to_double(elt[i].x, res);
      ret->ps[j].pts[i].y = cint_to_double(elt[i].y, res);
    }
  }
  return ret;
}

Paths64 clipper_paths_to_paths64 (clipper_paths *cps, double res) {
  Paths64 inputs;
  for (uint32_t j = 0; j < cps->count; j++) {
    Path64 input;
    clipper_path path = cps->ps[j];
    for (uint32_t i = 0; i < path.count; i++) {
      int64_t x = double_to_cint(path.pts[i].x, res);
      int64_t y = double_to_cint(path.pts[i].y, res);
      input.push_back(Point64(x, y));
    }
    inputs.push_back(input);
  }
  return inputs;
}

clipper_paths* clipper_minkowski_diff(clipper_path *fpattern, clipper_path *fpath, double res) {
  Path64 pattern = clipper_path_to_path64(fpattern, res);
  Path64 path = clipper_path_to_path64(fpath, res);
  Paths64 solution = SimplifyPaths(MinkowskiDiff(pattern, path, true), SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}

int clipper_point_in_polygon(clipper_vec2 *pt, clipper_path *polygon, double res) {
  Point64 point;
  point.x = double_to_cint(pt->x, res);
  point.y = double_to_cint(pt->y, res);
  Path64 poly = clipper_path_to_path64(polygon, res);
  auto out = PointInPolygon(point, poly);
  // std::cout << "POINT-IN? " << ((int)out) << std::endl;
  return (int)out;
}

clipper_paths* clipper_op(clipper_paths *a, clipper_paths *b, int op, double res) {
  Clipper64 co;
  Paths64 ai = clipper_paths_to_paths64(a, res);
  co.AddSubject(ai);
  Paths64 bi = clipper_paths_to_paths64(b, res);
  co.AddClip(bi);
  Paths64 result;
  co.Execute((ClipType)op, FillRule::NonZero, result);
  Paths64 solution = SimplifyPaths(result, SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}

clipper_paths* clipper_polyline_op(clipper_paths *a, clipper_paths *b, int op, double res) {
  Clipper64 co;
  Paths64 ai = clipper_paths_to_paths64(a, res);
  co.AddOpenSubject(ai);
  Paths64 bi = clipper_paths_to_paths64(b, res);
  co.AddClip(bi);
  Paths64 closed_result, result;
  co.Execute((ClipType)op, FillRule::NonZero, closed_result, result);
  Paths64 solution = SimplifyPaths(result, SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}
  
clipper_paths* clipper_offset(clipper_paths *paths, double amount, double res) {
  ClipperOffset co;
  Paths64 inputs = clipper_paths_to_paths64(paths, res);
  co.AddPaths(inputs, JoinType::Miter, EndType::Polygon);
  Paths64 result;
  co.Execute(amount * res, result);
  Paths64 solution = SimplifyPaths(result, SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}

clipper_paths* clipper_inflate_paths(clipper_paths *paths, double amount, double res) {
  Paths64 inputs = clipper_paths_to_paths64(paths, res);
  Paths64 result = InflatePaths(inputs, amount * res, JoinType::Miter, EndType::Square);
  Paths64 solution = SimplifyPaths(result, SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}

clipper_vec2* clipper_fab_vec2(double x, double y) {
  clipper_vec2* res = (clipper_vec2*)jmalloc(sizeof(clipper_vec2));
  res->x = x;
  res->y = y;
  return res;
}

clipper_paths* clipper_fab_paths(int num_paths) {
  clipper_paths* res = (clipper_paths*)jmalloc(sizeof(clipper_paths));
  res->ps = (clipper_path*)jmalloc(sizeof(clipper_path) * num_paths);
  res->count = num_paths;
  return res;
}

void clipper_fab_paths_path(clipper_paths* paths, int path_idx, int num_pts) {
  paths->ps[path_idx].pts = (clipper_vec2*)jmalloc(sizeof(clipper_vec2) * num_pts);
  paths->ps[path_idx].count = num_pts;
}

clipper_path* clipper_fab_path(int num_pts) {
  clipper_path* res = (clipper_path*)jmalloc(sizeof(clipper_path));
  res->pts = (clipper_vec2*)jmalloc(sizeof(clipper_vec2) * num_pts);
  res->count = num_pts;
  return res;
}

void clipper_path_set_point(clipper_path* path, int point_idx, double x, double y) {
  path->pts[point_idx].x = x;
  path->pts[point_idx].y = y;
}

void clipper_paths_set_point(clipper_paths* paths, int path_idx, int point_idx, double x, double y) {
  paths->ps[path_idx].pts[point_idx].x = x;
  paths->ps[path_idx].pts[point_idx].y = y;
}

void clipper_paths_delete(clipper_paths *paths) {
  for (uint32_t j = 0; j < paths->count; j++) {
    clipper_path path = paths->ps[j];
    jfree(path.pts);
  }
  jfree(paths->ps);
  jfree(paths);
}

void clipper_vec2_delete(clipper_vec2 *p) {
  jfree(p);
}

void clipper_path_delete(clipper_path *path) {
  jfree(path->pts);
  jfree(path);
}

int clipper_add_subject(Clipper64* co, clipper_paths *a, double res) {
  Paths64 inputs = clipper_paths_to_paths64(a, res);
  co->AddSubject(inputs);
  return 1;
}

int clipper_add_open_subject(Clipper64* co, clipper_paths *a, double res) {
  Paths64 inputs = clipper_paths_to_paths64(a, res);
  co->AddOpenSubject(inputs);
  return 1;
}

int clipper_add_clip(Clipper64* co, clipper_paths *a, double res) {
  Paths64 inputs = clipper_paths_to_paths64(a, res);
  co->AddClip(inputs);
  return 1;
}

clipper_paths* clipper_open_execute(Clipper64* co, int op, double res) {
  Paths64 closed_result, result;
  co->Execute((ClipType)op, FillRule::NonZero, closed_result, result);
  Paths64 solution = SimplifyPaths(result, SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}
  
clipper_paths* clipper_execute(Clipper64* co, int op, double res) {
  Paths64 result;
  co->Execute((ClipType)op, FillRule::NonZero, result);
  Paths64 solution = SimplifyPaths(result, SIMPLE_RES);
  clipper_paths* ret = paths64_to_clipper_paths(solution, res);
  return ret;
}
  
Clipper64* clipper_fab_clipper64(void) {
  auto res = new Clipper64();
  return res;
}

void clipper_clipper64_delete(Clipper64 *c64) {
  delete c64;
}
