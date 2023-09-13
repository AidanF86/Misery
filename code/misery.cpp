#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlImGui.cpp"

#include "misery_base.h"
#include "misery_math.h"
#include "misery_string.h"
#include "misery.h"
#include "misery_lists.h"
#include "misery_logging.h"

#define IsAnyShiftKeyDown (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) 


document NewDocument(u32 w, u32 h)
{
    document Result;
    Result.w = w;
    Result.h = h;
    Result.Layers = LayerList();
    Result.Scale = 1;
    Result.Offset = V2(0, 0);
    Result.CurrentLayerIndex = 0;
    
    Result.Texture = LoadRenderTexture(w, h);
    
    return Result;
}



void
DrawLayer(layer *Layer)
{
    switch(Layer->Type)
    {
        case LayerType_Bitmap:
        {
            int XFlip = (!Layer->FlippedX) * 2 - 1;
            int YFlip = (!Layer->FlippedY) * 2 - 1;
            rect SourceRect = Rect(0, 0, XFlip * Layer->Texture.width, YFlip * Layer->Texture.height);
            
            // Draw Rect adjusted for rotation offset (yeah ik it sucks)
            v2 Origin = V2(Layer->Rect.width / 2.0f,
                           Layer->Rect.height / 2.0f);
            rect DrawRect = Layer->Rect + V2(Layer->Rect.width / 2.0f, Layer->Rect.height / 2.0f);
            
            DrawTexturePro(Layer->Texture, SourceRect, DrawRect, Origin, Layer->Angle * RAD2DEG, Layer->ModColor);
        } break;
        case LayerType_Rectangle:
        {
            DrawRectanglePro(Layer->Rect, V2(0, 0), 0, Layer->ModColor);
        } break;
    }
}

void
RenderDocument(document *Document)
{
    BeginBlendMode(BLEND_ALPHA);
    
    BeginTextureMode(Document->Texture);
    ClearBackground(WHITE);
    for(int i = 0; i < Document->Layers.Count; i++)
    {
        DrawLayer(&(Document->Layers[i]));
    }
    EndTextureMode();
    
    EndBlendMode();
}


rect
GetDocumentRect(document *Document, rect *View)
{
    f32 DocumentWHRatio = (f32)Document->w / (f32)Document->h;
    f32 ViewWHRatio = (f32)View->width / (f32)View->height;
    
    rect DestRect;
    f32 Height, Width;
    if(ViewWHRatio > DocumentWHRatio)
    {
        // fit to height
        Height = View->height*Document->Scale;
        Width = Height*DocumentWHRatio;
    }
    else
    {
        // fit to width
        Width = View->width*Document->Scale;
        Height = Width*1/DocumentWHRatio;
    }
    
    f32 CenterX = View->x + View->width/2;
    f32 x = (Document->Offset.x * Document->Scale * -1) + CenterX - (Height*DocumentWHRatio/2);
    
    f32 CenterY = View->y + View->height/2;
    f32 y = (Document->Offset.y * Document->Scale * -1) + CenterY - Height/2;
    
    DestRect = {x, y, Width, Height};
    return DestRect;
}

rect
GetLayerScreenRect(layer *Layer, document *Document, rect *View)
{
    rect DocRect = GetDocumentRect(Document, View);
    //rect LayerRect = GetLayerRect(Layer);
    f32 ViewDocRatio = View->height / Document->h;
    
    f32 x = DocRect.x + (Layer->Rect.x * Document->Scale * ViewDocRatio);
    f32 y = DocRect.y + (Layer->Rect.y * Document->Scale * ViewDocRatio);
    f32 w = Layer->Rect.width * (Document->Scale * ViewDocRatio);
    f32 h = Layer->Rect.height * (Document->Scale * ViewDocRatio);
    
    return Rect(x, y, w, h);
}

void
DrawDocument(document *Document, rect *View)
{
    rect SourceRect = { 0, 0, (f32)Document->w, -(f32)Document->h};
    rect DestRect = GetDocumentRect(Document, View);
    
    DrawTexturePro(Document->Texture.texture, SourceRect, DestRect, V2(0,0), 0, WHITE);
}

void
SetupView(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    
    i32 Width = GetScreenWidth();
    i32 Height = GetScreenHeight();
    
    // 100 pixels on each side
    i32 LeftPadding = 100;
    i32 RightPadding = 100;
    i32 TopPadding = 20;
    i32 BottomPadding = 20;
    
    f32 ViewWidth = Width - LeftPadding - RightPadding;
    f32 ViewHeight = Height - TopPadding - BottomPadding;
    *View = Rect(LeftPadding, TopPadding, ViewWidth, ViewHeight);
}

void
AddAction(program_state *ProgramState, action Action)
{
    // TODO(cheryl): test this
    action_list *ActionStack = &ProgramState->ActionStack;
    if(ProgramState->PrevActionIndex == ActionStack->Count - 1 || ActionStack->Count == 0)
    {
        printf("Adding normally\n");
        // add normally
        if(ActionStack->Count > 0)
            ProgramState->PrevActionIndex++;
        
        ListAdd(ActionStack, Action);
    }
    else
    {
        printf("Adding weirdly\n");
        // add weirdly
        ActionStack->Count = ProgramState->PrevActionIndex + 1;
        ListAdd(ActionStack, Action);
        
        ProgramState->PrevActionIndex++;
    }
}

void
FlipImageIfNecessary(layer *Layer, tool_state *ToolState)
{
    rect *Rect = &Layer->Rect;
    
    if(Rect->width < 0)
    {
        //TODO(cheryl): check for tool == transform
        
        // adjust rect
        rect NewRect = *Rect;
        NewRect.x = Rect->x + Rect->width;
        NewRect.width = -Rect->width;
        *Rect = NewRect;
        
        // Flip if bitmap
        if(Layer->Type == LayerType_Bitmap)
        {
            Layer->FlippedX = !Layer->FlippedX;
        }
        
        // Flip tool point horizontally
        if(ToolState->Transform_DraggingLeft)
        {
            ToolState->Transform_DraggingLeft = false;
            ToolState->Transform_DraggingRight = true;
        }
        else if(ToolState->Transform_DraggingTopLeft)
        {
            ToolState->Transform_DraggingTopLeft = false;
            ToolState->Transform_DraggingTopRight = true;
        }
        else if(ToolState->Transform_DraggingBottomLeft)
        {
            ToolState->Transform_DraggingBottomLeft = false;
            ToolState->Transform_DraggingBottomRight = true;
        }
        else if(ToolState->Transform_DraggingRight)
        {
            ToolState->Transform_DraggingRight = false;
            ToolState->Transform_DraggingLeft = true;
        }
        else if(ToolState->Transform_DraggingTopRight)
        {
            ToolState->Transform_DraggingTopRight = false;
            ToolState->Transform_DraggingTopLeft = true;
        }
        else if(ToolState->Transform_DraggingBottomRight)
        {
            ToolState->Transform_DraggingBottomRight = false;
            ToolState->Transform_DraggingBottomLeft = true;
        }
    }
    if(Rect->height < 0)
    {
        //TODO(cheryl): check for tool == transform
        
        // adjust rect
        rect NewRect = *Rect;
        NewRect.y = Rect->y + Rect->height;
        NewRect.height = -Rect->height;
        *Rect = NewRect;
        
        // Flip if bitmap
        if(Layer->Type == LayerType_Bitmap)
        {
            Layer->FlippedY = !Layer->FlippedY;
        }
        
        // Flip tool point horizontally
        if(ToolState->Transform_DraggingTop)
        {
            ToolState->Transform_DraggingTop = false;
            ToolState->Transform_DraggingBottom = true;
        }
        else if(ToolState->Transform_DraggingTopLeft)
        {
            ToolState->Transform_DraggingTopLeft = false;
            ToolState->Transform_DraggingBottomLeft = true;
        }
        else if(ToolState->Transform_DraggingTopRight)
        {
            ToolState->Transform_DraggingTopRight = false;
            ToolState->Transform_DraggingBottomRight = true;
        }
        else if(ToolState->Transform_DraggingBottom)
        {
            ToolState->Transform_DraggingBottom = false;
            ToolState->Transform_DraggingTop = true;
        }
        else if(ToolState->Transform_DraggingBottomLeft)
        {
            ToolState->Transform_DraggingBottomLeft = false;
            ToolState->Transform_DraggingTopLeft = true;
        }
        else if(ToolState->Transform_DraggingBottomRight)
        {
            ToolState->Transform_DraggingBottomRight = false;
            ToolState->Transform_DraggingTopRight = true;
        }
    }
}

void
ProcessTool(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    tool_state *ToolState = &ProgramState->ToolState;
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *CurrentLayer = &CurrentDocument->Layers[CurrentDocument->CurrentLayerIndex];
    v2 MousePosition = GetMousePosition();
    rect LayerRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
    v2 LayerCenter = V2(LayerRect.x + LayerRect.width / 2.0f,
                        LayerRect.y + LayerRect.height / 2.0f);
    
    if(ProgramState->Tool == Tool_Translate)
    {
        if(IsMouseButtonDown(0))
        {
            if(!ToolState->Translate_DraggingX && !ToolState->Translate_DraggingY && !ToolState->Translate_DraggingBoth)
            {
                ToolState->BeingUsed = true;
                ToolState->Translate_DraggingBoth = false;
                ToolState->Translate_DraggingX = false;
                ToolState->Translate_DraggingY = false;
                
                if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolBoxRect))
                    ToolState->Translate_DraggingBoth = true;
                else if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolXArrowRect))
                    ToolState->Translate_DraggingX = true;
                else if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolYArrowRect))
                    ToolState->Translate_DraggingY = true;
                else
                    ToolState->BeingUsed = false;
                
                if(ToolState->BeingUsed)
                {
                    ToolState->Translate_InitialPosition = V2(CurrentLayer->Rect.x, CurrentLayer->Rect.y);
                }
            }
            
            f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
            f32 Scalar = ViewDocRatio;
            v2 dPosition = GetMouseDelta();
            
            if(ToolState->Translate_DraggingBoth)
            {
                CurrentLayer->Rect += dPosition * Scalar;
            }
            else if(ToolState->Translate_DraggingX)
            {
                CurrentLayer->Rect.x += dPosition.x * Scalar;
            }
            else if(ToolState->Translate_DraggingY)
            {
                CurrentLayer->Rect.y += dPosition.y * Scalar;
            }
        }
        else
        {
            if(ToolState->Translate_DraggingX || ToolState->Translate_DraggingY || ToolState->Translate_DraggingBoth)
            {
                action Action;
                Action.Type = Action_Translate;
                Action.Translate_InitialPosition = ToolState->Translate_InitialPosition;
                Action.FinalPosition = V2(CurrentLayer->Rect.x, CurrentLayer->Rect.y);
                Action.LayerIndex = CurrentDocument->CurrentLayerIndex;
                AddAction(ProgramState, Action);
            }
            ToolState->Translate_DraggingBoth = false;
            ToolState->Translate_DraggingX = false;
            ToolState->Translate_DraggingY = false;
        }
    }
    else if(ProgramState->Tool == Tool_Transform)
    {
        b32 DraggingAnyTransformControl = ToolState->Transform_Dragging[0] || ToolState->Transform_Dragging[1] || ToolState->Transform_Dragging[2] || ToolState->Transform_Dragging[3] || ToolState->Transform_Dragging[4] || ToolState->Transform_Dragging[5] || ToolState->Transform_Dragging[6] || ToolState->Transform_Dragging[7] || ToolState->Transform_Dragging[8];
        
        if(IsMouseButtonDown(0))
        {
            if(!DraggingAnyTransformControl)
            {
                for(int i = 0; i < 9; i++)
                {
                    ToolState->Transform_Dragging[i] = false;
                }
                
                for(int i = 0; i < 9; i++)
                {
                    if(CheckCollisionPointRec(MousePosition, ProgramState->TransformToolRects[i]))
                    {
                        ToolState->Transform_Dragging[i] = true;
                    }
                }
                // TODO(Cheryl): set initial state
            }
            
            DraggingAnyTransformControl = ToolState->Transform_Dragging[0] || ToolState->Transform_Dragging[1] || ToolState->Transform_Dragging[2] || ToolState->Transform_Dragging[3] || ToolState->Transform_Dragging[4] || ToolState->Transform_Dragging[5] || ToolState->Transform_Dragging[6] || ToolState->Transform_Dragging[7] || ToolState->Transform_Dragging[9];
            
            f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
            f32 Scalar = ViewDocRatio;
            v2 dPosition = GetMouseDelta();
            v2 dPosScaled = dPosition * Scalar;
            
            rect NewRect = CurrentLayer->Rect;
            
            if(ToolState->Transform_DraggingRight)
            {
                NewRect.width += dPosScaled.x;
            }
            else if(ToolState->Transform_DraggingLeft)
            {
                NewRect.x += dPosScaled.x;
                NewRect.width -= dPosScaled.x;
            }
            else if(ToolState->Transform_DraggingBottom)
            {
                NewRect.height += dPosScaled.y;
            }
            else if(ToolState->Transform_DraggingTop)
            {
                NewRect.y += dPosScaled.y;
                NewRect.height -= dPosScaled.y;
            }
            else if(ToolState->Transform_DraggingTopLeft)
            {
                NewRect.x += dPosScaled.x;
                NewRect.width -= dPosScaled.x;
                NewRect.y += dPosScaled.y;
                NewRect.height -= dPosScaled.y;
            }
            else if(ToolState->Transform_DraggingTopRight)
            {
                NewRect.width += dPosScaled.x;
                NewRect.y += dPosScaled.y;
                NewRect.height -= dPosScaled.y;
            }
            else if(ToolState->Transform_DraggingBottomLeft)
            {
                NewRect.height += dPosScaled.y;
                NewRect.x += dPosScaled.x;
                NewRect.width -= dPosScaled.x;
            }
            else if(ToolState->Transform_DraggingBottomRight)
            {
                NewRect.height += dPosScaled.y;
                NewRect.width += dPosScaled.x;
            }
            else if(ToolState->Transform_DraggingWhole)
            {
                NewRect.x += dPosScaled.x;
                NewRect.y += dPosScaled.y;
            }
            
            CurrentLayer->Rect = NewRect;
        }
        else
        {
            if(DraggingAnyTransformControl)
            {
                // TODO(cheryl): append action
            }
            for(int i = 0; i < 9; i++)
            {
                ToolState->Transform_Dragging[i] = false;
            }
        }
    }
    else if(ProgramState->Tool == Tool_Rotate)
    {
        if(IsMouseButtonDown(0))
        {
            if(!ToolState->Rotation_Dragging)
            {
                // start dragging if the cursor is on the tool
                f32 OuterRadius = ProgramState->RotateToolRadius + ProgramState->RotateToolThickness / 2.0f;
                f32 InnerRadius = ProgramState->RotateToolRadius - ProgramState->RotateToolThickness / 2.0f;
                
                b32 InLargeCircle = CheckCollisionPointCircle(MousePosition, LayerCenter, OuterRadius);
                b32 InSmallCircle = CheckCollisionPointCircle(MousePosition, LayerCenter, InnerRadius);
                
                ToolState->Rotation_Dragging = false;
                if(InLargeCircle)
                {
                    // start dragging
                    ToolState->Rotation_Dragging = true;
                    ToolState->Rotation_InitialAngle = CurrentLayer->Angle;
                    
                    v2 UpVector = V2(0, 1);
                    rect LayerScreenRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
                    v2 LayerScreenCenter = V2(LayerScreenRect.x + LayerScreenRect.width / 2.0f,
                                              LayerScreenRect.y + LayerScreenRect.height / 2.0f);
                    v2 MouseVector = MousePosition - LayerScreenCenter;
                    
                    ToolState->Rotation_InitialMouseAngle = Vector2Angle(UpVector, MouseVector);
                }
            }
            else
            {
                v2 UpVector = V2(0, 1);
                rect LayerScreenRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
                v2 LayerScreenCenter = V2(LayerScreenRect.x + LayerScreenRect.width / 2.0f,
                                          LayerScreenRect.y + LayerScreenRect.height / 2.0f);
                v2 MouseVector = MousePosition - LayerScreenCenter;
                
                f32 MouseAngle = Vector2Angle(UpVector, MouseVector);
                f32 dAngle = MouseAngle - ToolState->Rotation_InitialMouseAngle;
                f32 NewAngle = ToolState->Rotation_InitialAngle + dAngle;
                
                printf("%f, ", dAngle);
                printf("%f\n", NewAngle);
                
                CurrentLayer->Angle = NewAngle;
            }
        }
        else if(ToolState->Rotation_Dragging)
        {
            if(ToolState->Rotation_Dragging)
            {
                // TODO(cheryl): append action
                action Action;
                Action.Type = Action_Rotate;
                Action.InitialAngle = ToolState->Rotation_InitialAngle;
                Action.FinalAngle = CurrentLayer->Angle;
                Action.LayerIndex = CurrentDocument->CurrentLayerIndex;
                AddAction(ProgramState, Action);
            }
            ToolState->Rotation_Dragging = false;
        }
    }
    
    // TODO(cheryl): flip image
    FlipImageIfNecessary(CurrentLayer, ToolState);
    
}

void
UndoAction(program_state *ProgramState, action *Action)
{
    printf("Undoing");
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[Action->LayerIndex];
    if(Action->Type == Action_Translate)
    {
        // TODO(cheryl): wtf?? %d is useless here
        // TODO(cheryl): IMPLEMENT A GODDAMN LOGGING SYSTEM
        // AND A GOOD ONE AT THAT
        printf("UNDOING: Translate Action: %d to %d\n", Action->Translate_InitialPosition, Action->FinalPosition);
        Layer->Rect.x = Action->Translate_InitialPosition.x;
        Layer->Rect.y = Action->Translate_InitialPosition.y;
    }
    else if(Action->Type == Action_Rotate)
    {
        printf("UNDOING ROTATE\n");
        Layer->Angle = Action->InitialAngle;
    }
    
    ProgramState->PrevActionIndex--;
}

void
RedoAction(program_state *ProgramState, action *Action)
{
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[Action->LayerIndex];
    if(Action->Type == Action_Translate)
    {
        printf("REDOING: Translate Action: %d to %d\n", Action->Translate_InitialPosition, Action->FinalPosition);
        Layer->Rect.x = Action->FinalPosition.x;
        Layer->Rect.y = Action->FinalPosition.y;
    }
    else if(Action->Type == Action_Rotate)
    {
        printf("REDOING ROTATE\n");
        Layer->Angle = Action->FinalAngle;
    }
    
    ProgramState->PrevActionIndex++;
}

void
Undo(program_state *ProgramState)
{
    if(ProgramState->ActionStack.Count > 0 &&  ProgramState->PrevActionIndex > -1)
    {
        UndoAction(ProgramState, &ProgramState->ActionStack[ProgramState->PrevActionIndex]);
    }
}

void
Redo(program_state *ProgramState)
{
    if(ProgramState->ActionStack.Count > ProgramState->PrevActionIndex > -1)
    {
        RedoAction(ProgramState, &ProgramState->ActionStack[ProgramState->PrevActionIndex + 1]);
    }
}


void
UndoOrRedoTo(program_state *ProgramState, int Index)
{
    b32 MovingForward = false;
    while(Index > ProgramState->PrevActionIndex + 1)
    {
        Redo(ProgramState);
        MovingForward = true;
    }
    if(MovingForward)
        return;
    while(Index <= ProgramState->PrevActionIndex)
    {
        Undo(ProgramState);
    }
}

b32
SaveDocument(document *Document)
{
    // TODO(cheryl)
    return false;
}

b32
ExportDocument(document *Document)
{
    // write to png file
    // TODO(cheryl): add file dialog, add multiple export filetypes
    
    Image DocumentImage = LoadImageFromTexture(Document->Texture.texture);
    
    // Flip image
    ImageFlipVertical(&DocumentImage);
    
    b32 Result = ExportImage(DocumentImage, "TestDocument.png");
    
    return Result;
}

void
MakeNewLayerFromImage(program_state *ProgramState, Image ImageToUse)
{
    layer Layer;
    Layer.Type = LayerType_Bitmap;
    Layer.Texture = LoadTextureFromImage(ImageToUse);
    Layer.FlippedX = false;
    Layer.FlippedY = false;
    Layer.ModColor = WHITE;
    Layer.Rect = Rect(0, 0, Layer.Texture.width, Layer.Texture.height);
    ListAdd(&ProgramState->OpenDocuments[0].Layers, Layer);
}

void
MakeNewRectLayer(program_state *ProgramState)
{
    layer Layer;
    Layer.Type = LayerType_Rectangle;
    // TODO(cheryl): put this in the middle and scale based on canvas size
    Layer.Rect = Rect(0, 0, 100, 100);
    // TODO(cheryl): determines on how we handle the ui for layer color
    Layer.ModColor = RED;
    ListAdd(&ProgramState->OpenDocuments[0].Layers, Layer);
}

void
MakeNewLayerFromPath(program_state *ProgramState, const char *Path)
{
    // TODO(cheryl): probably handle urls here
    Image LayerImage = LoadImage(Path);
    MakeNewLayerFromImage(ProgramState, LayerImage);
}

void
HandleFileDrop(program_state *ProgramState)
{
    FilePathList Files = LoadDroppedFiles();
    
    // TODO(cheryl): once again, handle different image filetypes
    // TODO(cheryl): handle loading from urls
    
    // Only load one image
    if(Files.count == 0)
    {
        printf("No files were dropped!\n");
    }
    else
    {
        for(int i = 0; i < Files.count; i++)
        {
            char *ImagePath = Files.paths[i];
            Image DroppedImage = LoadImage(ImagePath);
            
            if(IsImageReady(DroppedImage))
            {
                MakeNewLayerFromImage(ProgramState, DroppedImage);
            }
            
            UnloadImage(DroppedImage);
        }
    }
    
    UnloadDroppedFiles(Files);
}

void
UpdateToolRects(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *CurrentLayer = &CurrentDocument->Layers[CurrentDocument->CurrentLayerIndex];
    
    f32 TranslateToolArrowLength = ProgramState->TranslateToolArrowLength;
    f32 TranslateToolArrowWidth = ProgramState->TranslateToolArrowWidth;
    f32 TranslateToolBoxSize = ProgramState->TranslateToolBoxSize;
    
    rect LayerRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
    v2 LayerCenter = V2(LayerRect.x + LayerRect.width / 2.0f,
                        LayerRect.y + LayerRect.height / 2.0f);
    
    ProgramState->TranslateToolXArrowRect = Rect(LayerCenter.x, 
                                                 LayerCenter.y - TranslateToolArrowWidth / 2.0f,
                                                 TranslateToolArrowLength,
                                                 TranslateToolArrowWidth);
    ProgramState->TranslateToolYArrowRect = Rect(LayerCenter.x - TranslateToolArrowWidth / 2.0f, 
                                                 LayerCenter.y,
                                                 TranslateToolArrowWidth,
                                                 TranslateToolArrowLength);
    ProgramState->TranslateToolBoxRect = Rect(LayerCenter.x, LayerCenter.y,
                                              TranslateToolBoxSize, TranslateToolBoxSize);
    
    int i = 0;
    for(f32 y = LayerRect.y; y <= LayerRect.y + LayerRect.height + 1; y += LayerRect.height / 2.0f)
    {
        for(f32 x = LayerRect.x; x <= LayerRect.x + LayerRect.width + 1; x += LayerRect.width / 2.0f)
        {
            if(i == 4)
            {
                i++;
                continue;
            }
            int Index = i;
            if(i > 4)
            {
                Index = i-1;
            }
            ProgramState->TransformToolRects[Index] = Rect(x - ProgramState->TransformToolBoxSize / 2.0f,
                                                           y - ProgramState->TransformToolBoxSize / 2.0f,
                                                           ProgramState->TransformToolBoxSize,
                                                           ProgramState->TransformToolBoxSize);
            i++;
        }
    }
    ProgramState->TransformToolWholeRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
    
}

extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        rect *View = &ProgramState->View;
        tool_state *ToolState = &ProgramState->ToolState;
        action_list *ActionStack = &ProgramState->ActionStack;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            
            ProgramState->ActionStack = ActionList(20);
            ProgramState->PrevActionIndex = 0;
            
            ProgramState->Tool = Tool_Rotate;
            ProgramState->ShowLayerOutline = true;
            
            ProgramState->TranslateToolArrowLength = 100;
            ProgramState->TranslateToolArrowWidth = 30;
            ProgramState->TranslateToolBoxSize = 30;
            
            ProgramState->TransformToolBoxSize = 20;
            
            ProgramState->RotateToolRadius = 70;
            ProgramState->RotateToolThickness = 5;
            
            ProgramState->OpenDocuments = DocumentList(20);
            ListAdd(&ProgramState->OpenDocuments, NewDocument(1500, 2000));
            
            MakeNewLayerFromPath(ProgramState, "../data/test/hopeless.png");
            
            View->x = 0;
            View->y = 0;
            View->width = 200;
            View->height = 800;
            
            SetupView(ProgramState);
        }
        
        Print(String("Hello! %d", 5));
        
        ProgramState->dTime = GetFrameTime();
        document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
        layer *CurrentLayer = &CurrentDocument->Layers[CurrentDocument->CurrentLayerIndex];
        ImGuiIO& IO = ImGui::GetIO();
        ProgramState->CursorInImGui = IO.WantCaptureMouse;
        ProgramState->ScreenWidth = GetScreenWidth();
        ProgramState->ScreenHeight = GetScreenHeight();
        i32 ScreenWidth = ProgramState->ScreenWidth;
        i32 ScreenHeight = ProgramState->ScreenHeight;
        v2 MousePosition = GetMousePosition();
        
        rect LayerRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
        v2 LayerCenter = V2(LayerRect.x + LayerRect.width / 2.0f,
                            LayerRect.y + LayerRect.height / 2.0f);
        
        f32 TranslateToolArrowLength = ProgramState->TranslateToolArrowLength;
        f32 TranslateToolArrowWidth = ProgramState->TranslateToolArrowWidth;
        f32 TranslateToolBoxSize = ProgramState->TranslateToolBoxSize;
        
        if(IsWindowResized())
        {
            SetupView(ProgramState);
        }
        
        if(!ProgramState->CursorInImGui)
        {
            f32 dScale = 0;
            f32 MouseScroll = GetMouseWheelMoveV().y;
            dScale += MouseScroll;
            // TODO(cheryl): move towards (or away from) cursor upon mouse scroll
            
            if(IsKeyDown(KEY_EQUAL))
            {
                if(IsAnyShiftKeyDown)
                {
                    dScale += 0.1;
                }
                else
                {
                    dScale = 0;
                    CurrentDocument->Scale = 1;
                    CurrentDocument->Offset = V2(0, 0);
                }
            }
            else if(IsKeyDown(KEY_MINUS))
            {
                dScale-= 0.1;
            }
            
            CurrentDocument->Scale += dScale / 2 * CurrentDocument->Scale;
            
            if(CurrentDocument->Scale <= 0.1)
            {
                CurrentDocument->Scale = 0.1;
            }
            
            v2 dOffset = V2(0, 0);
            dOffset.y += IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
            dOffset.x += IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
            f32 MoveSpeed = 10;
            dOffset *= MoveSpeed;
            if(IsMouseButtonDown(1))
            {
                dOffset += GetMouseDelta() * -1 ;
            }
            dOffset /= CurrentDocument->Scale;
            CurrentDocument->Offset += dOffset;
        }
        
        UpdateToolRects(ProgramState);
        ProcessTool(ProgramState);
        UpdateToolRects(ProgramState);
        
        
        if(IsFileDropped())
        {
            HandleFileDrop(ProgramState);
        }
        
        if(IsKeyDown(KEY_ESCAPE))
        {
            Memory->IsRunning = false;
        }
        
        
        BeginDrawing();
        {
            
            RenderDocument(CurrentDocument);
            {
                // background rects
                rect DocRect = GetDocumentRect(CurrentDocument, View);
                Color BGColor = DARKGRAY;
                rect Rect1 = Rect(0,
                                  0,
                                  ScreenWidth,
                                  DocRect.y);
                rect Rect2 = Rect(0,
                                  0,
                                  DocRect.x,
                                  ScreenHeight);
                rect Rect3 = Rect(0,
                                  DocRect.y+ DocRect.height,
                                  ScreenWidth,
                                  ScreenHeight - DocRect.y - DocRect.height);
                
                rect Rect4 = Rect(DocRect.x + DocRect.width,
                                  0,
                                  ScreenWidth - DocRect.x - DocRect.width,
                                  ScreenHeight);
                
                DrawRectangleRec(Rect1,BGColor);
                DrawRectangleRec(Rect2,BGColor);
                DrawRectangleRec(Rect3,BGColor);
                DrawRectangleRec(Rect4,BGColor);
                /*
                DrawRectangleLinesEx(Rect1, 5, YELLOW);
                DrawRectangleLinesEx(Rect2, 5, BLUE);
                DrawRectangleLinesEx(Rect3, 5, PURPLE);
                DrawRectangleLinesEx(Rect4, 5, GREEN);
            */
            }
            DrawDocument(CurrentDocument, View);
            
            DrawRectangleLinesEx(*View, 5, RED);
            
            if(ProgramState->ShowLayerOutline)
            {
                rect LayerRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, &ProgramState->View);
                DrawRectangleLinesEx(LayerRect, 5, PURPLE);
            }
            
            if(ProgramState->Tool == Tool_Translate)
            {
                Color XColor = BLUE;
                Color YColor = BLUE;
                Color BoxColor = BLUE;
                if(ToolState->Translate_DraggingX)
                    XColor = YELLOW;
                if(ToolState->Translate_DraggingY)
                    YColor = YELLOW;
                if(ToolState->Translate_DraggingBoth)
                    BoxColor = YELLOW;
                DrawRectangleLinesEx(ProgramState->TranslateToolXArrowRect, 5, XColor);
                DrawRectangleLinesEx(ProgramState->TranslateToolYArrowRect, 5, YColor);
                DrawRectangleLinesEx(ProgramState->TranslateToolBoxRect, 5, BoxColor);
            }
            else if(ProgramState->Tool == Tool_Transform)
            {
                for(int i = 0; i < 8; i++)
                {
                    Color RectColor = BLUE;
                    if(ToolState->Transform_Dragging[i])
                        RectColor = YELLOW;
                    DrawRectangleLinesEx(ProgramState->TransformToolRects[i], 5, RectColor);
                }
            }
            else if(ProgramState->Tool == Tool_Rotate)
            {
                f32 InnerRadius = ProgramState->RotateToolRadius - ProgramState->RotateToolThickness / 2.0f;
                f32 OuterRadius = ProgramState->RotateToolRadius + ProgramState->RotateToolThickness / 2.0f;
                Color ToolColor = BLUE;
                DrawRing(LayerCenter, InnerRadius, OuterRadius, 0, 360, 1, ToolColor);
                // TODO(cheryl): cool lines
                
                if(ToolState->BeingUsed)
                {
                    DrawRingLines(LayerCenter, 0, OuterRadius, ToolState->Rotation_InitialAngle, ToolState->Rotation_Angle, 1, ToolColor);
                }
            }
            
            rlImGuiBegin();
            {
                char Buffer[100];
                //ImGui::SetNextWindowPos({0, 0});
                
                ImGui::Text("Scale: %f", CurrentDocument->Scale);
                ImGui::Text("PrevActionIndex: %d", ProgramState->PrevActionIndex);
                ImGui::Text("ActionStack Count: %d", ProgramState->ActionStack.Count);
                if(ImGui::Button("Export"))
                {
                    ExportDocument(CurrentDocument);
                }
                
                
                if(ImGui::Button("New Bitmap Layer"))
                {
                    layer Layer = *CurrentLayer;
                    ListAdd(&CurrentDocument->Layers, Layer);
                }
                if(ImGui::Button("New Rect Layer"))
                {
                    MakeNewRectLayer(ProgramState);
                }
                
                // TOOLS
                if(ImGui::TreeNodeEx("Tools", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    for(int i = 0; i < ToolCount; i++)
                    {
                        b32 Selected = ImGui::RadioButton(ToolStrings[i], ProgramState->Tool == i);
                        if(Selected)
                        {
                            ProgramState->Tool = (tool)i;
                        }
                    }
                    ImGui::TreePop();
                }
                
                // LAYERS
                if(ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    f32 LayerAlpha = ((f32)CurrentLayer->ModColor.a)/255.0f;
                    ImGui::SliderFloat(" ", &LayerAlpha, 0, 1, "Alpha: %.3f");
                    CurrentLayer->ModColor.a = (u8)(LayerAlpha*255.0f);
                    CurrentLayer->ModColor.r = 255;
                    CurrentLayer->ModColor.g = 255;
                    CurrentLayer->ModColor.b = 255;
                    
                    
                    ImGui::DragFloat2("Position", (f32 *)(&CurrentLayer->Rect));
                    
                    if(ImGui::BeginListBox(""))
                    {
                        for(int i = 0; i < CurrentDocument->Layers.Count; i++)
                        {
                            sprintf(Buffer, "layer %d", i);
                            b32 Selected = ImGui::Selectable(Buffer, CurrentDocument->CurrentLayerIndex == i);
                            ImGui::SameLine();
                            b32 ShouldDelete = ImGui::Button("Delete");
                            if(ShouldDelete)
                            {
                                ListRemoveAt(&CurrentDocument->Layers, i);
                            }
                            else if(Selected)
                            {
                                CurrentDocument->CurrentLayerIndex = i;
                            }
                        }
                        ImGui::EndListBox();
                    }
                    
                    ImGui::TreePop();
                }
                
                // ACTIONS
                sprintf(Buffer, "Actions (%d)", ActionStack->Count);
                
                if(ImGui::TreeNodeEx(Buffer, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    b32 ShouldUndo = false;
                    b32 ShouldRedo = false;
                    if(ActionStack->Count > 0 && ProgramState->PrevActionIndex > -1)
                        ShouldUndo = ImGui::Button("Undo");
                    else
                        ImGui::Text("Undo");
                    ImGui::SameLine();
                    if(ActionStack->Count > ProgramState->PrevActionIndex + 1)
                        ShouldRedo = ImGui::Button("Redo");
                    else
                        ImGui::Text("Redo");
                    
                    if(ShouldUndo)
                        Undo(ProgramState);
                    if(ShouldRedo)
                        Redo(ProgramState);
                    
                    if(ImGui::BeginListBox(""))
                    {
                        for(int i = ActionStack->Count; i > 0; i--)
                        {
                            // TODO(cheryl): proper action naming
                            action *Action = &ProgramState->ActionStack[i-1];
                            if(Action->Type == Action_Translate)
                            {
                                sprintf(Buffer, "#%d T L%d (%d,%d)->(%d,%d)",
                                        i, Action->LayerIndex,
                                        (int)Action->Translate_InitialPosition.x, (int)Action->Translate_InitialPosition.y,
                                        (int)Action->FinalPosition.x, (int)Action->FinalPosition.y);
                            }
                            else if(Action->Type == Action_ScaleAndTranslate)
                            {
                                sprintf(Buffer, "#%d S", i);
                            }
                            else
                            {
                                sprintf(Buffer, "??? (This is a bug, please report)");
                            }
                            ImVec4 TextColor = {1, 1, 1, 1};
                            if(i > ProgramState->PrevActionIndex + 1)
                            {
                                TextColor = {1, 1, 1, 0.5};
                            }
                            
                            ImGui::PushStyleColor(ImGuiCol_Text, TextColor); 
                            b32 ShouldUndoOrRedoTo = ImGui::Selectable(Buffer, i == ProgramState->PrevActionIndex + 1);
                            ImGui::PopStyleColor();
                            
                            if(ShouldUndoOrRedoTo)
                            {
                                UndoOrRedoTo(ProgramState, i);
                            }
                        }
                        
                        b32 ShouldUndoOrRedoToBeginning = ImGui::Selectable("Beginning", 0 == ProgramState->PrevActionIndex + 1);
                        if(ShouldUndoOrRedoToBeginning)
                        {
                            UndoOrRedoTo(ProgramState, 0);
                        }
                        
                        ImGui::EndListBox();
                    }
                    
                    ImGui::TreePop();
                }
            }
            rlImGuiEnd();
        }
        EndDrawing();
    }
}