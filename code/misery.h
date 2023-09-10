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
    v2 Position;
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

enum tool
{
    Tool_Translate,
    Tool_Transform,
};

#define ToolCount 2
const char *ToolStrings[] = {
    "Translate",
    "Transform",
};

struct tool_state
{
    b32 BeingUsed;
    union
    {
        struct
        {// translate
            b32 DraggingX;
            b32 DraggingY;
            b32 DraggingBoth;
            v2 InitialPosition;
        };
        struct
        {// transform
            union
            {
                struct
                {
                    b32 DraggingLeft;
                    b32 DraggingRight;
                    b32 DraggingTop;
                    b32 DraggingBotom;
                    b32 DraggingTopLeft;
                    b32 DraggingTopRight;
                    b32 DraggingBottomLeft;
                    b32 DraggingBottomRight;
                };
                b32 Dragging[8];
            };
        };
    };
};

enum action_type
{
    Action_Translate,
    Action_ScaleAndTranslate,
};
struct action
{
    action_type Type;
    
    int LayerIndex;
    union{
        struct{
            // Translate
            v2 InitialPosition;
            v2 FinalPosition;
        };
        struct{
            // ScaleAndTranslate
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
    tool_state ToolState;
    
    action_list ActionStack;
    int PrevActionIndex;
    
    struct { // Options
        // Changable
        b32 ShowLayerOutline;
        
        // Unchangeable
        
    };
    
    // Translate Tool
    f32 TranslateToolArrowLength;
    f32 TranslateToolArrowWidth;
    f32 TranslateToolBoxSize;
    
    rect TranslateToolXArrowRect;
    rect TranslateToolYArrowRect;
    rect TranslateToolBoxRect;
    
    // Transform Tool
    f32 TransformToolBoxSize;
    
    union
    {
        struct
        {
            rect TransformToolTopLeftRect;
            rect TransformToolTopRect;
            rect TransformToolTopRightRect;
            rect TransformToolLeftRect;
            rect TransformToolRightRect;
            rect TransformToolBottomLeftRect;
            rect TransformToolBottomRect;
            rect TransformToolBottomRightRect;
        };
        rect TransformToolRects[8];
    };
};


#endif //MISERY_H