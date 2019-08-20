// See license.txt for details about licensing.

#include <stdint.h>
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nanosvg.h"
#include <vector>

extern "C" {

struct svg_path {
  float* pts;
  uint32_t npts;
  char closed;
  float* bounds;
};

struct svg_image {
  svg_path** paths;
  uint32_t npaths;
};

svg_image* svg_read(char *filename, char * format, float res);
void svg_image_delete(svg_image *image);
void svg_path_delete(svg_path *path);
}

svg_image* svg_read(char *filename, char * format, float resolution) {
  NSVGimage* g_image = nsvgParseFromFile(filename, format, resolution);
  if (g_image == ((NSVGimage*)0)) {
    return ((svg_image*)0);
  } else {
    std::vector<svg_path*> paths;
    for (NSVGshape* shape = g_image->shapes; shape != NULL; shape = shape->next) {
      for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
        svg_path* svg_path = (struct svg_path*)malloc(sizeof(struct svg_path));
        float* bounds = new float[4];
        float* pts = new float[path->npts * 2];
        svg_path->bounds = bounds;
        svg_path->pts = pts;
        for (int i = 0; i < (path->npts * 2); i++)
          pts[i] = path->pts[i];
        svg_path->npts = path->npts;
        svg_path->closed = path->closed;
        paths.push_back(svg_path);
        // drawPath(path->pts, path->npts, path->closed, px * 1.5f);
      }
    }
    svg_image* image = (svg_image*)malloc(sizeof(struct svg_image));
    svg_path** svg_paths = (svg_path**)malloc(paths.size() * sizeof(svg_path*));
    for (int i = 0; i < paths.size(); i++)
      svg_paths[i] = paths[i];
    image->paths = svg_paths;
    image->npaths = paths.size();
    return image;
  }
}

void svg_image_delete(svg_image *image) {
  for (uint32_t j = 0; j < image->npaths; j++) {
    svg_path_delete(image->paths[j]);
  }
  delete image;
}

void svg_path_delete(svg_path *path) {
  delete path->pts;
  delete path->bounds;
  delete path;
}
