#pragma once

inline int clampi(int v, int minv, int maxv) {
    return v < maxv ? (v > minv ? v : minv) : maxv;
}
