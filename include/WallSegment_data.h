//
// Exported by obj2c.py (C) 2011-2012 Matti Dahlbom 
//

#ifndef __WALLSEGMENT_DATA_H__
#define __WALLSEGMENT_DATA_H__

// Numeric constants
static const unsigned int WallSegmentIndicesDatatype = GL_UNSIGNED_SHORT;
static const unsigned int WallSegmentNumVertices = 42;
static const unsigned int WallSegmentNumPolygons = 14;

// Object dimensions:
//   X: 28.000000 (half: 14.000000), xmin: -14.000000, xmax: 14.000000
//   Y: 3.648842 (half: 1.824421), ymin: -1.824421, ymax: 1.824421
//   Z: 2.612161 (half: 1.306081), zmin: -1.306081, zmax: 1.306081

static const float WallSegmentHalfWidth = 14.000000;
static const float WallSegmentHalfHeight = 1.824421;
static const float WallSegmentHalfDepth = 1.306081;

// { x, y, z,  u, v,  nx, ny, nz }
const VertexAttribs WallSegment_vertices[] = {
    {14.0, -1.82442109167, 0.64172975889,	3.034999, 0.0,	-0.0, -0.0, 1.0},
    {13.9999946053, 0.872908431684, 0.641731107555,	3.034999, 0.205786,	-0.0, -0.0, 1.0},
    {-14.0, 0.872908431684, 0.64172975889,	0.0, 0.205786,	-0.0, -0.0, 1.0},
    {14.0, -1.82442109167, 0.64172975889,	3.034999, 0.0,	-0.0, -0.0, 1.0},
    {-14.0, 0.872908431684, 0.64172975889,	0.0, 0.205786,	-0.0, -0.0, 1.0},
    {-13.9999986513, -1.82442109167, 0.64172975889,	0.0, 0.0,	-0.0, -0.0, 1.0},
    {14.0, 0.872908431684, -0.641731107555,	3.035, 0.205786,	0.0, 0.0, -1.0},
    {14.0, -1.82442109167, -0.641731107555,	3.034999, 0.0,	0.0, 0.0, -1.0},
    {-13.999995954, -1.82442109167, -0.641731107555,	0.0, 0.0,	0.0, 0.0, -1.0},
    {14.0, 0.872908431684, -0.641731107555,	3.035, 0.205786,	0.0, 0.0, -1.0},
    {-13.999995954, -1.82442109167, -0.641731107555,	0.0, 0.0,	0.0, 0.0, -1.0},
    {-13.9999973027, 0.872908431684, -0.641731107555,	0.0, 0.205786,	0.0, 0.0, -1.0},
    {14.0, 0.872908431684, -0.641731107555,	3.035, 0.2839,	0.0, -1.0, 0.0},
    {-13.9999973027, 0.872908431684, -0.641731107555,	0.0, 0.2839,	0.0, -1.0, 0.0},
    {-13.9999973027, 0.872908431684, -1.30608067183,	0.0, 0.380731,	0.0, -1.0, 0.0},
    {14.0, 0.872908431684, -0.641731107555,	3.035, 0.2839,	0.0, -1.0, 0.0},
    {-13.9999973027, 0.872908431684, -1.30608067183,	0.0, 0.380731,	0.0, -1.0, 0.0},
    {14.0, 0.872908431684, -1.30607932316,	3.035, 0.380731,	0.0, -1.0, 0.0},
    {-14.0, 0.872908431684, 0.64172975889,	-0.0, 0.096831,	0.0, -1.0, 0.0},
    {13.9999946053, 0.872908431684, 0.641731107555,	3.034999, 0.096831,	0.0, -1.0, 0.0},
    {13.9999946053, 0.872908431684, 1.30608067183,	3.034999, 0.0,	0.0, -1.0, 0.0},
    {-14.0, 0.872908431684, 0.64172975889,	-0.0, 0.096831,	0.0, -1.0, 0.0},
    {13.9999946053, 0.872908431684, 1.30608067183,	3.034999, 0.0,	0.0, -1.0, 0.0},
    {-14.0, 0.872908431684, 1.30607932316,	0.0, 0.0,	0.0, -1.0, 0.0},
    {-14.0, 0.872908431684, 1.30607932316,	0.0, 0.0,	-0.0, -0.0, 1.0},
    {13.9999946053, 0.872908431684, 1.30608067183,	3.034999, 0.0,	-0.0, -0.0, 1.0},
    {13.9999946053, 1.82442109167, 1.30608067183,	3.034999, 0.074266,	-0.0, -0.0, 1.0},
    {-14.0, 0.872908431684, 1.30607932316,	0.0, 0.0,	-0.0, -0.0, 1.0},
    {13.9999946053, 1.82442109167, 1.30608067183,	3.034999, 0.074266,	-0.0, -0.0, 1.0},
    {-14.0, 1.82442109167, 1.30607932316,	0.0, 0.074266,	-0.0, -0.0, 1.0},
    {14.0, 0.872908431684, -1.30607932316,	3.035, 0.0,	0.0, 0.0, -1.0},
    {-13.9999973027, 0.872908431684, -1.30608067183,	0.0, 0.0,	0.0, 0.0, -1.0},
    {-13.9999973027, 1.82442109167, -1.30608067183,	0.0, 0.074266,	0.0, 0.0, -1.0},
    {14.0, 0.872908431684, -1.30607932316,	3.035, 0.0,	0.0, 0.0, -1.0},
    {-13.9999973027, 1.82442109167, -1.30608067183,	0.0, 0.074266,	0.0, 0.0, -1.0},
    {14.0, 1.82442109167, -1.30607932316,	3.035, 0.074266,	0.0, 0.0, -1.0},
    {14.0, 1.82442109167, -1.30607932316,	3.035, 0.380731,	0.0, 1.0, 0.0},
    {-13.9999973027, 1.82442109167, -1.30608067183,	0.0, 0.380731,	0.0, 1.0, 0.0},
    {-14.0, 1.82442109167, 1.30607932316,	0.0, 0.0,	0.0, 1.0, 0.0},
    {14.0, 1.82442109167, -1.30607932316,	3.035, 0.380731,	0.0, 1.0, 0.0},
    {-14.0, 1.82442109167, 1.30607932316,	0.0, 0.0,	0.0, 1.0, 0.0},
    {13.9999946053, 1.82442109167, 1.30608067183,	3.034999, 0.0,	0.0, 1.0, 0.0}
};

#endif

