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
            DrawTexture(Layer->Texture, 0, 0, WHITE);
        }
        break;
    }
}

void
RenderDocument(document *Document)
{
    BeginTextureMode(Document->Texture);
    ClearBackground(WHITE);
    for(int i = 0; i < Document->Layers.Count; i++)
    {
        DrawLayer(&(Document->Layers[i]));
    }
    EndTextureMode();
}

void
DrawDocument(document *Document, rect *View)
{
    rect SourceRect = { 0, 0, (f32)Document->w, -(f32)Document->h};
    
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
    f32 x = CenterX - (Height*DocumentWHRatio/2);
    
    f32 CenterY = View->y + View->height/2;
    f32 y = CenterY - Height/2;
    
    DestRect = {x, y, Width, Height};
    
    DrawTexturePro(Document->Texture.texture, SourceRect, DestRect, Document->Offset*Document->Scale, 0, WHITE);
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
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            
            ProgramState->OpenDocuments = DocumentList(20);
            ListAdd(&ProgramState->OpenDocuments, NewDocument(1500, 2000));
            //ProgramState->OpenDocuments[0].Scale = 0.2f;
            
            layer Layer;
            Layer.Type = LayerType_Bitmap;
            Layer.Texture = LoadTexture("../data/test/hopeless.png");
            ListAdd(&ProgramState->OpenDocuments[0].Layers, Layer);
            ProgramState->CurrentDocumentIndex = 0;
            
            View->x = 0;
            View->y = 0;
            View->width = 200;
            View->height = 800;
            
            SetupView(ProgramState);
        }
        
        document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
        ProgramState->dTime = GetFrameTime();
        
        if(IsWindowResized())
        {
            SetupView(ProgramState);
        }
        
        
        
        if(IsKeyDown(KEY_ESCAPE))
        {
            Memory->IsRunning = false;
        }
        
        f32 MouseScroll = GetMouseWheelMoveV().y;
        f32 dScale;
        dScale += MouseScroll;
        
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
        
        
        
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);
            
            RenderDocument(&ProgramState->OpenDocuments[0]);
            DrawDocument(&ProgramState->OpenDocuments[0], View);
            
#if 0
            char Buffer[20];
            sprintf(Buffer, "%f", CurrentDocument->Scale);
            DrawText(Buffer, 0, 0, 20, RED);
#endif
            
            DrawRectangleLinesEx(*View, 10, RED);
            
#if 1
            rlImGuiBegin();
            
            ImGui::SetNextWindowPos({0, 0});
            
            
            ImGui::Text("HaaaaI");
            
            
            //ImGui::ShowMetricsWindow();
            rlImGuiEnd();
#endif
        }
        EndDrawing();
    }
}