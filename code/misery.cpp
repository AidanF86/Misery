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
            DrawTexture(Layer->Texture, Layer->Position.x, Layer->Position.y, Layer->ModColor);
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
GetLayerRect(layer *Layer)
{
    switch(Layer->Type)
    {
        case LayerType_Bitmap:
        {
            return Rect(Layer->Position.x, Layer->Position.y, Layer->Texture.width, Layer->Texture.height);
        }
        break;
    }
    return Rect(0,0,0,0);
}

rect
GetLayerScreenRect(layer *Layer, document *Document, rect *View)
{
    rect DocRect = GetDocumentRect(Document, View);
    rect LayerRect = GetLayerRect(Layer);
    f32 ViewDocRatio = View->height / Document->h;
    
    f32 x = DocRect.x + (LayerRect.x * Document->Scale * ViewDocRatio);
    f32 y = DocRect.y + (LayerRect.y * Document->Scale * ViewDocRatio);
    f32 w = LayerRect.width * (Document->Scale * ViewDocRatio);
    f32 h = LayerRect.height * (Document->Scale * ViewDocRatio);
    
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

extern "C"
{
    PROGRAM_UPDATE_AND_RENDER(ProgramUpdateAndRender)
    {
        program_state *ProgramState = (program_state *)Memory->Data;
        rect *View = &ProgramState->View;
        tool_state *ToolState = &ProgramState->ToolState;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            
            ProgramState->Tool = Tool_Transform;
            ProgramState->ShowLayerOutline = true;
            
            ProgramState->OpenDocuments = DocumentList(20);
            ListAdd(&ProgramState->OpenDocuments, NewDocument(1500, 2000));
            //ProgramState->OpenDocuments[0].Scale = 0.2f;
            
            layer Layer;
            Layer.Type = LayerType_Bitmap;
            Layer.Texture = LoadTexture("../data/test/hopeless.png");
            Layer.ModColor = WHITE;
            Layer.Position = V2(0, 0);
            ListAdd(&ProgramState->OpenDocuments[0].Layers, Layer);
            ProgramState->CurrentDocumentIndex = 0;
            
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
        
        if(IsWindowResized())
        {
            SetupView(ProgramState);
        }
        
        rect LayerRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
        v2 LayerCenter = V2(LayerRect.x + LayerRect.width / 2.0f,
                            LayerRect.y + LayerRect.height / 2.0f);
        f32 TransformToolArrowLength = 100;
        f32 TransformToolArrowWidth = 30;
        f32 TransformToolBoxSize = 30;
        rect TransformToolXArrowRect = Rect(LayerCenter.x, 
                                            LayerCenter.y - TransformToolArrowWidth / 2.0f,
                                            TransformToolArrowLength,
                                            TransformToolArrowWidth);
        rect TransformToolYArrowRect = Rect(LayerCenter.x - TransformToolArrowWidth / 2.0f, 
                                            LayerCenter.y,
                                            TransformToolArrowWidth,
                                            TransformToolArrowLength);
        rect TransformToolBoxRect = Rect(LayerCenter.x, LayerCenter.y,
                                         TransformToolBoxSize, TransformToolBoxSize);
        
        // tools
        if(ProgramState->Tool == Tool_Transform)
        {
            if(!ToolState->DraggingX && !ToolState->DraggingY && !ToolState->DraggingBoth)
            {
            }
        }
        
        
        
        if(IsKeyDown(KEY_ESCAPE))
        {
            Memory->IsRunning = false;
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
            if(IsMouseButtonDown(0))
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
            
            if(ProgramState->Tool == Tool_Transform)
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
                DrawRectangleLinesEx(TransformToolYArrowRect, 5, XColor);
                DrawRectangleLinesEx(TransformToolXArrowRect, 5, YColor);
                DrawRectangleLinesEx(TransformToolBoxRect, 5, BoxColor);
            }
            
            rlImGuiBegin();
            {
                //ImGui::SetNextWindowPos({0, 0});
                
                ImGui::Text("Scale: %f", CurrentDocument->Scale);
                
                if(ImGui::Button("New Bitmap Layer"))
                {
                    layer Layer = *CurrentLayer;
                    ListAdd(&CurrentDocument->Layers, Layer);
                }
                
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
                
                if(ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    f32 LayerAlpha = ((f32)CurrentLayer->ModColor.a)/255.0f;
                    ImGui::SliderFloat(" ", &LayerAlpha, 0, 1, "Alpha: %.3f");
                    CurrentLayer->ModColor.a = (u8)(LayerAlpha*255.0f);
                    CurrentLayer->ModColor.r = 255;
                    CurrentLayer->ModColor.g = 255;
                    CurrentLayer->ModColor.b = 255;
                    
                    
                    ImGui::DragFloat2("Position", (f32 *)(&CurrentLayer->Position));
                    
                    if(ImGui::BeginListBox(""))
                    {
                        char Buffer[20];
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
                
            }
            rlImGuiEnd();
        }
        EndDrawing();
    }
}