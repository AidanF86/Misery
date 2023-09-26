/* date = August 27th 2023 11:39 am */

#ifndef MISERY_H
#define MISERY_H

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 

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
    
    union
    {
        struct
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
        
        trans Trans;
    };
    
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
    
    int LayerIndex;
    
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

struct grab_point
{
    b32 IsGrabbed;
    
    union {
        struct {
            f32 x, y;
        };
        v2 Pos; // Center
    };
    
    union{
        struct {// Rect
            union {
                struct{
                    f32 w, h;
                };
                v2 Dim;
            };
        };
        struct {// Circle
            f32 MinRadius, MaxRadius;
        };
    };
    
    b32 ScaleLeft, ScaleRight, ScaleTop, ScaleBottom, MoveX, MoveY, Rotate;
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

struct transform_tool_data
{
    union {
        struct {
            union {
                struct {// Translate
                    grab_point XArrow, YArrow, BothBox;
                };
                grab_point Translate_GrabPoints[1];
            };
            union {
                struct {// Rotate
                    grab_point GrabCircle;
                };
                grab_point Rotate_GrabPoints[1];
            };
            union {
                struct {// Scale
                    grab_point LeftScalar, RightScalar, TopScalar, BottomScalar;
                };
                grab_point Scale_GrabPoints[1];
            };
            union {
                struct {// Transform
                    grab_point TopLeft, Top, TopRight, Left, Right, BottomLeft, Bottom, BottomRight;
                    grab_point WholeLayer;
                    grab_point TopLeftRotate, TopRightRotate, BottomLeftRotate, BottomRightRotate;
                };
                grab_point Transform_GrabPoints[1];
            };
        };
        grab_point GrabPoints[1];
    };
};

struct program_state
{
    f32 dTime;
    i32 ScreenWidth, ScreenHeight;
    
    b32 CursorInImGui;
    
    document_list OpenDocuments;
    int CurrentDocumentIndex;
    
    rect View;
    
    action_list ActionStack;
    int PrevActionIndex;
    
    struct { // Options
        // CHANGABLE
        b32 ShowLayerOutline = true;
        
        // UNCHANGABLE
        f32 TranslateTool_ArrowLength;
        f32 TranslateTool_ArrowWidth;
        f32 TranslateTool_BoxSize;
        
        f32 TransformTool_BoxSize;
        
        f32 RotateTool_Radius;
        f32 RotateTool_Thickness;
    };
    
    tool_type Tool;
    transform_tool_data TransformToolData;
};



void
DrawRectangleLinesPro(v2 Pos, v2 Dim, f32 Rotation, f32 Thickness, Color DrawColor)
{
    f32 RadRotation = Rotation*DEG2RAD;
    v2 TopLeft = Pos + Vector2Rotate(V2(-Dim.x/2, -Dim.y/2), RadRotation);
    v2 TopRight = Pos + Vector2Rotate(V2(Dim.x/2, -Dim.y/2), RadRotation);
    v2 BottomRight = Pos + Vector2Rotate(V2(Dim.x/2, Dim.y/2), RadRotation);
    v2 BottomLeft = Pos + Vector2Rotate(V2(-Dim.x/2, Dim.y/2), RadRotation);
    
    DrawLineEx(TopLeft + V2(-Thickness/2, 0), TopRight + V2(Thickness/2, 0), Thickness, DrawColor);
    DrawLineEx(TopRight, BottomRight, Thickness, DrawColor);
    DrawLineEx(BottomRight + V2(Thickness/2, 0), BottomLeft + V2(-Thickness/2, 0), Thickness, DrawColor);
    DrawLineEx(BottomLeft, TopLeft, Thickness, DrawColor);
}

#endif //MISERY_H