#ifndef GFX_UTIL_H
#define GFX_UTIL_H

void renderBlob(const vec3f &col, const vec3d &pos);
void boxPoints(const vec2i &a, const vec2i &b, int cornerRadius, bool line);
void renderBox(const vec3f &bgCol, const vec3f &borderCol, const recti &rect, int cornerRadius);

#endif
