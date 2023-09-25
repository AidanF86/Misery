/* date = August 27th 2023 11:39 am */

#ifndef MISERY_H
#define MISERY_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 

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
    LayerType_Ellipse,
};

struct layer
{
    Color ModColor;
    rect Rect;
    f32 Rotation;
    
    layer_type Type;
    union{
        struct{
            // Bitmap
            Texture2D Texture;
            b32 FlippedX, FlippedY;
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
    
    int CurrentLayerIndex;
    
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

struct grab_point
{
    v2 Pos;
    
    b32 ScaleLeft, ScaleRight, ScaleTop, ScaleBottom, MovesX, MovesY;
    b32 Rotates;
};

enum tool_type
{
    Tool_Translate,
    Tool_Rotate,
    Tool_Scale,
    Tool_Transform,
};

#define ToolCount 4
const char *ToolStrings[] = {
    "Translate",
    "Rotate",
    "Scale",
    "Transform",
};

struct tool
{
    tool_type Type;
    b32 BeingUsed;
    
    // RECTS
    struct {
        // Translate Tool
        f32 Translate_ArrowLength;
        f32 Translate_ArrowWidth;
        f32 Translate_BoxSize;
        
        rect Translate_XArrowRect;
        rect Translate_YArrowRect;
        rect Translate_BoxRect;
        
        // Rotate Tool
        f32 Rotate_Radius;
        f32 Rotate_Thickness;
        
        // Transform Tool
        f32 Transform_BoxSize;
        
        union
        {
            struct
            {
                rect Transform_TopLeftRect;
                rect Transform_TopRect;
                rect Transform_TopRightRect;
                rect Transform_LeftRect;
                rect Transform_RightRect;
                rect Transform_BottomLeftRect;
                rect Transform_BottomRect;
                rect Transform_BottomRightRect;
                
                rect Transform_WholeRect;
                
                rect Transform_BottomLeftRotate;
                rect Transform_TopLeftRotate;
                rect Transform_TopRightRotate;
                rect Transform_BottomRightRotate;
            };
            rect Transform_Rects[9];
        };
    };
    
    // DRAGGING
    struct {
        union
        {
            struct
            {// translate
                b32 Translate_DraggingX;
                b32 Translate_DraggingY;
                b32 Translate_DraggingBoth;
                
                v2 Translate_InitialPosition;
            };
            struct
            {// rotate
                b32 Rotation_Dragging;
                
                f32 Rotation_InitialMouseAngle;
                f32 Rotation_Angle;
                f32 Rotation_InitialAngle;
            };
            struct
            {// transform
                union
                {
                    struct
                    {
                        b32 Transform_DraggingTopLeft;
                        b32 Transform_DraggingTop;
                        b32 Transform_DraggingTopRight;
                        b32 Transform_DraggingLeft;
                        b32 Transform_DraggingRight;
                        b32 Transform_DraggingBottomLeft;
                        b32 Transform_DraggingBottom;
                        b32 Transform_DraggingBottomRight;
                        
                        b32 Transform_DraggingWhole;
                    };
                    b32 Transform_Dragging[9];
                };
            };
        };
    };
    
};

enum action_type
{
    Action_Translate,
    Action_ScaleAndTranslate,
    Action_Rotate,
};
struct action
{
    action_type Type;
    
    int LayerIndex;
    union{
        struct{
            // Translate
            v2 Translate_InitialPosition;
            v2 FinalPosition;
        };
        struct{
            // ScaleAndTranslate
            rect InitialRect;
            rect FinalRect;
        };
        struct{
            // Rotate
            f32 InitialAngle;
            f32 FinalAngle;
        };
        struct{
        };
    };
};
typedef struct action_list
{
    int Count;
    int ArraySize;
    action *Data;
    inline action& operator[](size_t Index) { return Data[Index]; }
    inline const action& operator[](size_t Index) const { return Data[Index]; }
} action_list;



struct program_state
{
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    b32 CursorInImGui;
    
    document_list OpenDocuments;
    int CurrentDocumentIndex;
    
    rect View;
    
    tool Tool;
    
    action_list ActionStack;
    int PrevActionIndex;
    
    struct { // Options
        // Changable
        b32 ShowLayerOutline;
        
        // Unchangeable
        
    };
    
    
};


#endif //MISERY_H