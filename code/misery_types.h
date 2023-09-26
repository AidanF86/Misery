/* date = September 25th 2023 0:55 am */

#ifndef MISERY_TYPES_H
#define MISERY_TYPES_H

typedef Vector2 v2;
typedef Vector3 v3;
typedef Rectangle rect;

struct transform
{
    union {
        struct {
            f32 x, y;
        };
        v2 Pos;
    };
    union {
        struct {
            f32 w, h;
        };
        v2 Dim;
    };
    f32 Rotation;
};
typedef transform trans;

#endif //MISERY_TYPES_H
