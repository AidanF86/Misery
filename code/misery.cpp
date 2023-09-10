#include <stdio.h>

#include "raylib.h"
#include "rlImGui.cpp"

#include "misery_base.h"
#include "misery_math.h"
#include "misery.h"
#include "misery_lists.h"

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
            DrawTexturePro(Layer->Texture, SourceRect, Layer->Rect, V2(0, 0), 0, Layer->ModColor);
        }
        break;
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
ProcessTool(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    tool_state *ToolState = &ProgramState->ToolState;
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *CurrentLayer = &CurrentDocument->Layers[CurrentDocument->CurrentLayerIndex];
    v2 MousePosition = GetMousePosition();
    
    if(ProgramState->Tool == Tool_Translate)
    {
        if(IsMouseButtonDown(0))
        {
            if(!ToolState->DraggingX && !ToolState->DraggingY && !ToolState->DraggingBoth)
            {
                ToolState->BeingUsed = true;
                ToolState->DraggingBoth = false;
                ToolState->DraggingX = false;
                ToolState->DraggingY = false;
                
                if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolBoxRect))
                    ToolState->DraggingBoth = true;
                else if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolXArrowRect))
                    ToolState->DraggingX = true;
                else if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolYArrowRect))
                    ToolState->DraggingY = true;
                else
                    ToolState->BeingUsed = false;
                
                if(ToolState->BeingUsed)
                {
                    ToolState->InitialPosition = V2(CurrentLayer->Rect.x, CurrentLayer->Rect.y);
                }
            }
        }
        else
        {
            if(ToolState->DraggingX || ToolState->DraggingY || ToolState->DraggingBoth)
            {
                action Action;
                Action.Type = Action_Translate;
                Action.InitialPosition = ToolState->InitialPosition;
                Action.FinalPosition = V2(CurrentLayer->Rect.x, CurrentLayer->Rect.y);
                Action.LayerIndex = CurrentDocument->CurrentLayerIndex;
                AddAction(ProgramState, Action);
            }
            ToolState->DraggingBoth = false;
            ToolState->DraggingX = false;
            ToolState->DraggingY = false;
        }
        
        f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
        f32 Scalar = ViewDocRatio;
        v2 dPosition = GetMouseDelta();
        
        if(ToolState->DraggingBoth)
        {
            CurrentLayer->Rect += dPosition * Scalar;
        }
        else if(ToolState->DraggingX)
        {
            CurrentLayer->Rect.x += dPosition.x * Scalar;
        }
        else if(ToolState->DraggingY)
        {
            CurrentLayer->Rect.y += dPosition.y * Scalar;
        }
    }
    else if(ProgramState->Tool == Tool_Transform)
    {
        b32 DraggingAnyTransformControl = ToolState->Dragging[0] || ToolState->Dragging[1] || ToolState->Dragging[2] || ToolState->Dragging[3] || ToolState->Dragging[4] || ToolState->Dragging[5] || ToolState->Dragging[6] || ToolState->Dragging[7] || ToolState->Dragging[8];
        
        if(IsMouseButtonDown(0))
        {
            if(!DraggingAnyTransformControl)
            {
                for(int i = 0; i < 9; i++)
                {
                    ToolState->Dragging[i] = false;
                }
                
                for(int i = 0; i < 9; i++)
                {
                    if(CheckCollisionPointRec(MousePosition, ProgramState->TransformToolRects[i]))
                    {
                        ToolState->Dragging[i] = true;
                    }
                }
                // TODO(Cheryl): set initial state
            }
        }
        else
        {
            if(DraggingAnyTransformControl)
            {
                // TODO(cheryl): append action
            }
            for(int i = 0; i < 9; i++)
            {
                ToolState->Dragging[i] = false;
            }
        }
        
        DraggingAnyTransformControl = ToolState->Dragging[0] || ToolState->Dragging[1] || ToolState->Dragging[2] || ToolState->Dragging[3] || ToolState->Dragging[4] || ToolState->Dragging[5] || ToolState->Dragging[6] || ToolState->Dragging[7] || ToolState->Dragging[9];
        
        f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
        f32 Scalar = ViewDocRatio;
        v2 dPosition = GetMouseDelta();
        v2 dPosScaled = dPosition * Scalar;
        
        rect NewRect = CurrentLayer->Rect;
        
        if(ToolState->DraggingRight)
        {
            NewRect.width += dPosScaled.x;
        }
        else if(ToolState->DraggingLeft)
        {
            NewRect.x += dPosScaled.x;
            NewRect.width -= dPosScaled.x;
        }
        else if(ToolState->DraggingBottom)
        {
            NewRect.height += dPosScaled.y;
        }
        else if(ToolState->DraggingTop)
        {
            NewRect.y += dPosScaled.y;
            NewRect.height -= dPosScaled.y;
        }
        else if(ToolState->DraggingTopLeft)
        {
            NewRect.x += dPosScaled.x;
            NewRect.width -= dPosScaled.x;
            NewRect.y += dPosScaled.y;
            NewRect.height -= dPosScaled.y;
        }
        else if(ToolState->DraggingTopRight)
        {
            NewRect.width += dPosScaled.x;
            NewRect.y += dPosScaled.y;
            NewRect.height -= dPosScaled.y;
        }
        else if(ToolState->DraggingBottomLeft)
        {
            NewRect.height += dPosScaled.y;
            NewRect.x += dPosScaled.x;
            NewRect.width -= dPosScaled.x;
        }
        else if(ToolState->DraggingBottomRight)
        {
            NewRect.height += dPosScaled.y;
            NewRect.width += dPosScaled.x;
        }
        else if(ToolState->DraggingWhole)
        {
            NewRect.x += dPosScaled.x;
            NewRect.y += dPosScaled.y;
        }
        
        CurrentLayer->Rect = NewRect;
    }
    
    rect *Rect = &CurrentLayer->Rect;
    // TODO(cheryl): flip image
    if(Rect->width < 0)
    {
        //TODO(cheryl): check for tool == transform
        
        // adjust rect
        rect NewRect = *Rect;
        NewRect.x = Rect->x + Rect->width;
        NewRect.width = -Rect->width;
        *Rect = NewRect;
        
        // Flip if bitmap
        if(CurrentLayer->Type == LayerType_Bitmap)
        {
            CurrentLayer->FlippedX = !CurrentLayer->FlippedX;
        }
        
        // Flip tool point horizontally
        if(ToolState->DraggingLeft)
        {
            ToolState->DraggingLeft = false;
            ToolState->DraggingRight = true;
        }
        else if(ToolState->DraggingTopLeft)
        {
            ToolState->DraggingTopLeft = false;
            ToolState->DraggingTopRight = true;
        }
        else if(ToolState->DraggingBottomLeft)
        {
            ToolState->DraggingBottomLeft = false;
            ToolState->DraggingBottomRight = true;
        }
        else if(ToolState->DraggingRight)
        {
            ToolState->DraggingRight = false;
            ToolState->DraggingLeft = true;
        }
        else if(ToolState->DraggingTopRight)
        {
            ToolState->DraggingTopRight = false;
            ToolState->DraggingTopLeft = true;
        }
        else if(ToolState->DraggingBottomRight)
        {
            ToolState->DraggingBottomRight = false;
            ToolState->DraggingBottomLeft = true;
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
        if(CurrentLayer->Type == LayerType_Bitmap)
        {
            CurrentLayer->FlippedY = !CurrentLayer->FlippedY;
        }
        
        // Flip tool point horizontally
        if(ToolState->DraggingTop)
        {
            ToolState->DraggingTop = false;
            ToolState->DraggingBottom = true;
        }
        else if(ToolState->DraggingTopLeft)
        {
            ToolState->DraggingTopLeft = false;
            ToolState->DraggingBottomLeft = true;
        }
        else if(ToolState->DraggingTopRight)
        {
            ToolState->DraggingTopRight = false;
            ToolState->DraggingBottomRight = true;
        }
        else if(ToolState->DraggingBottom)
        {
            ToolState->DraggingBottom = false;
            ToolState->DraggingTop = true;
        }
        else if(ToolState->DraggingBottomLeft)
        {
            ToolState->DraggingBottomLeft = false;
            ToolState->DraggingTopLeft = true;
        }
        else if(ToolState->DraggingBottomRight)
        {
            ToolState->DraggingBottomRight = false;
            ToolState->DraggingTopRight = true;
        }
    }
    
}

void
UndoAction(program_state *ProgramState, action *Action)
{
    printf("Undoing");
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[Action->LayerIndex];
    if(Action->Type == Action_Translate)
    {
        printf("UNDOING: Translate Action: %d to %d\n", Action->InitialPosition, Action->FinalPosition);
        Layer->Rect.x = Action->InitialPosition.x;
        Layer->Rect.y = Action->InitialPosition.y;
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
        printf("REDOING: Translate Action: %d to %d\n", Action->InitialPosition, Action->FinalPosition);
        Layer->Rect.x = Action->FinalPosition.x;
        Layer->Rect.y = Action->FinalPosition.y;
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
            
            ProgramState->Tool = Tool_Transform;
            ProgramState->ShowLayerOutline = true;
            
            ProgramState->TranslateToolArrowLength = 100;
            ProgramState->TranslateToolArrowWidth = 30;
            ProgramState->TranslateToolBoxSize = 30;
            
            ProgramState->TransformToolBoxSize = 20;
            
            ProgramState->OpenDocuments = DocumentList(20);
            ListAdd(&ProgramState->OpenDocuments, NewDocument(1500, 2000));
            
            MakeNewLayerFromPath(ProgramState, "../data/test/hopeless.png");
            
            View->x = 0;
            View->y = 0;
            View->width = 200;
            View->height = 800;
            
            SetupView(ProgramState);
        }
        
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
        
        f32 TranslateToolArrowLength = ProgramState->TranslateToolArrowLength;
        f32 TranslateToolArrowWidth = ProgramState->TranslateToolArrowWidth;
        f32 TranslateToolBoxSize = ProgramState->TranslateToolBoxSize;
        
        if(IsWindowResized())
        {
            SetupView(ProgramState);
        }
        
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
        
        
        if(IsFileDropped())
        {
            HandleFileDrop(ProgramState);
        }
        
        
        // TODO(cheryl): move view movement processing to before tool rect determination to
        // prevent it from lagging behind
        
        
        
        if(IsKeyDown(KEY_ESCAPE))
        {
            Memory->IsRunning = false;
        }
        
        if(!ProgramState->CursorInImGui)
        {
            
            ProcessTool(ProgramState);
            
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
                if(ToolState->DraggingX)
                    XColor = YELLOW;
                if(ToolState->DraggingY)
                    YColor = YELLOW;
                if(ToolState->DraggingBoth)
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
                    if(ToolState->Dragging[i])
                        RectColor = YELLOW;
                    DrawRectangleLinesEx(ProgramState->TransformToolRects[i], 5, RectColor);
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
                            if(Selected)
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
                                        (int)Action->InitialPosition.x, (int)Action->InitialPosition.y,
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