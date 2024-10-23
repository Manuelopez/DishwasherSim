#pragma once
#include "raylib.h"
#include "raymath.h"

typedef struct Range1 {
  float min;
  float max;
} Range1;
typedef struct Range2 {
  Vector2 min;
  Vector2 max;
} Range2;

inline Range2 range2_make(Vector2 min, Vector2 max) {
  return (Range2){min, max};
}

Range2 range2_shift(Range2 r, Vector2 shift) {
  r.min = Vector2Add(r.min, shift);
  r.max = Vector2Add(r.max, shift);
  return r;
}

Range2 range2_make_bottom_happen(Vector2 size) {
  Range2 range = {0};
  range.max = size;
  range = range2_shift(range, (Vector2){size.x * -0.5, 0.0});
  return range;
}

Vector2 range2_size(Range2 range) {
  Vector2 size = {0};
  size = Vector2Subtract(range.min, range.max);
  size.x = fabsf(size.x);
  size.y = fabsf(size.y);
  return size;
}

bool range2_contains(Range2 range, Vector2 v) {
  return v.x >= range.min.x && v.x <= range.max.x && v.y >= range.min.y &&
         v.y <= range.max.y;
}
