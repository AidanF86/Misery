/* date = August 27th 2023 11:46 am */

#ifndef MISERY_MATH_H
#define MISERY_MATH_H

#include "math.h"

v2 V2(f32 x, f32 y)
{
    v2 Result = {x,y};
    return Result;
}

inline v2
operator+(v2 A, v2 B)
{
    return {A.x + B.x, A.y + B.y};
}
inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A+B;
    return A;
}
inline v2
operator-(v2 A, v2 B)
{
    return {A.x - B.x, A.y - B.y};
}
inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A-B;
    return A;
}
inline v2
operator*(v2 V, f32 Scalar)
{
    return {V.x * Scalar, V.y * Scalar};
}
inline v2 &
operator*=(v2 &A, f32 B)
{
    A = A*B;
    return A;
}
inline v2
operator/(v2 V, f32 Scalar)
{
    return {V.x / Scalar, V.y / Scalar};
}
inline v2 &
operator/=(v2 &A, f32 B)
{
    A = A/B;
    return A;
}
inline bool
operator<(v2 A, v2 B)
{
    return A.x < B.x && A.y < B.y;
}
inline bool
operator>(v2 A, v2 B)
{
    return A.x > B.x && A.y > B.y;
}

inline rect
operator+(rect R, v2 V)
{
    rect Result = {R.x + V.x, R.y + V.y, R.width, R.height};
    return Result;
}
inline rect
operator-(rect R, v2 V)
{
    rect Result = {R.x - V.x, R.y - V.y, R.width, R.height};
    return Result;
}
inline rect &
operator+=(rect &R, v2 V)
{
    R = R + V;
    return R;
}

inline rect
operator*(rect R, f32 Scalar)
{
    rect Result = R;
    Result.x *= Scalar;
    Result.y *= Scalar;
    Result.width *= Scalar;
    Result.height *= Scalar;
    return Result;
}

inline v3
V3(f32 x, f32 y, f32 z)
{
    v3 Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    return Result;
}

inline v3
V2ToV3(v2 V)
{
    v3 Result;
    Result.x = V.x;
    Result.y = V.y;
    Result.z = 0;
    return Result;
}

inline v2
V3ToV2(v3 V)
{
    v2 Result;
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}

inline rect
Rect(f32 x, f32 y, f32 w, f32 h)
{
    rect Result;
    Result.x = x;
    Result.y = y;
    Result.width = w;
    Result.height = h;
    return Result;
}

inline trans
operator+(trans T, v2 V)
{
    trans Result = T;
    Result.Pos += V;
    return Result;
}
inline trans
operator-(trans T, v2 V)
{
    trans Result = T;
    Result.Pos -= V;
    return Result;
}
inline trans &
operator+=(trans &T, v2 V)
{
    T = T + V;
    return T;
}
inline trans &
operator-=(trans &T, v2 V)
{
    T = T - V;
    return T;
}

inline trans
Trans(v2 Pos, v2 Dim, f32 Rotation)
{
    trans Result;
    Result.Pos = Pos;
    Result.Dim = Dim;
    Result.Rotation = Rotation;
    return Result;
}
inline trans
Trans(f32 x, f32 y, f32 w, f32 h, f32 Rotation)
{
    trans Result;
    Result.x = x;
    Result.y = y;
    Result.w = w;
    Result.h = h;
    Result.Rotation = Rotation;
    return Result;
}

bool CheckCollisionCircleRecFloat(Vector2 center, float radius, Rectangle rec)
{
    bool collision = false;
    
    f32 recCenterX = (rec.x + rec.width/2.0f);
    f32 recCenterY = (rec.y + rec.height/2.0f);
    
    float dx = fabsf(center.x - recCenterX);
    float dy = fabsf(center.y - recCenterY);
    
    if (dx > (rec.width/2.0f + radius)) { return false; }
    if (dy > (rec.height/2.0f + radius)) { return false; }
    
    if (dx <= (rec.width/2.0f)) { return true; }
    if (dy <= (rec.height/2.0f)) { return true; }
    
    float cornerDistanceSq = (dx - rec.width/2.0f)*(dx - rec.width/2.0f) +
    (dy - rec.height/2.0f)*(dy - rec.height/2.0f);
    
    collision = (cornerDistanceSq <= (radius*radius));
    
    return collision;
}

#endif //MISERY_MATH_H
