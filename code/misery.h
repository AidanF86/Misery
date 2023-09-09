/* date = August 27th 2023 11:39 am */

#ifndef MISERY_H
#define MISERY_H

struct layer_position
{
    i32 x, y;
};
layer_position LayerPosition(i32 x, i32 y)
{
    layer_position Result = {x, y};
    return Result;
}

enum layer_type
{
    LayerType_Text,
    LayerType_Bitmap,
    LayerType_Rectangle,
    LayerType_Circle,
};

struct layer
{
    Color ModColor;
    layer_position Position;
    f32 Scale;
    
    layer_type Type;
    union{
        struct{
            Texture2D Texture;
        };
        struct{
        };
        struct{
        };
        struct{
        };
    };
};
typedef struct layer_list
{
    int Count;
    int ArraySize;
    layer *Data;
    inline layer& operator[](size_t Index) { return Data[Index]; }
    inline const layer& operator[](size_t Index) const { return Data[Index]; }
} layer_list;

struct document
{
    i32 w, h;
    layer_list Layers;
    
    RenderTexture2D Texture;
    
    f32 Scale;
    v2 Offset;
};
typedef struct document_list
{
    int Count;
    int ArraySize;
    document *Data;
    inline document& operator[](size_t Index) { return Data[Index]; }
    inline const document& operator[](size_t Index) const { return Data[Index]; }
} document_list;

struct program_state
{
    f32 dTime;
    
    document_list OpenDocuments;
    int CurrentDocumentIndex;
    
    rect View;
};


#endif //MISERY_H