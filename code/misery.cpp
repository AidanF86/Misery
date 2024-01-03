#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rlImGui.cpp"

#include "misery_base.h"
#include "misery_types.h"
#include "misery_math.h"
#include "misery_string.h"
#include "misery_logging.h"
#include "misery.h"
#include "misery_lists.h"

#include "misery_rects.h"
#include "misery_layer.h"
#include "misery_action.h"
#include "misery_tool.h"
#include "misery_file.h"

document NewDocument(u32 w, u32 h)
{
    document Result;
    Result.w = w;
    Result.h = h;
    Result.Layers = LayerList();
    Result.Scale = 1;
    Result.Offset = V2(0, 0);
    Result.LayerIndex = 0;
    
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
            
            // TODO(aidan): this doesn't feel right, maybe check later
            v2 Origin = V2(Layer->w/2, Layer->h/2);
            rect DrawRect = Rect(Layer->x, Layer->y, Layer->w, Layer->h);
            
            DrawTexturePro(Layer->Texture, SourceRect, DrawRect, Origin, Layer->Rotation, Layer->ModColor);
        } break;
        case LayerType_Rectangle:
        {
            //DrawRectanglePro(Layer->Rect, V2(0, 0), 0, Layer->ModColor);
        } break;
    }
}


void
DrawLayerOutline(layer *Layer, document *Document, rect *View)
{
    trans Trans = GetLayerScreenTrans(Layer, Document, View);
    
    DrawRectangleLinesPro(Trans.Pos, Trans.Dim, Trans.Rotation, 5, PURPLE);
    
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
        //tool **Tool = &ProgramState->Tool;
        action_list *ActionStack = &ProgramState->ActionStack;
        
        if(!Memory->Initialized)
        {
            Memory->Initialized = true;
            
            
            
            ProgramState->TranslateTool_ArrowLength = 100;
            ProgramState->TranslateTool_ArrowWidth = 30;
            ProgramState->TranslateTool_BoxSize = 30;
            
            ProgramState->TransformTool_BoxSize = 20;
            
            ProgramState->RotateTool_Radius = 70;
            ProgramState->RotateTool_Thickness = 5;
            
            
            
            ProgramState->ActionStack = ActionList(20);
            ProgramState->PrevActionIndex = 0;
            
            ProgramState->OpenDocuments = DocumentList(20);
            ListAdd(&ProgramState->OpenDocuments, NewDocument(1500, 2000));
            ProgramState->CurrentDocumentIndex = 0;
            ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex].LayerIndex = 0;
            
            MakeNewLayerFromPath(ProgramState, "../data/test/hopeless.png");
            
            View->x = 0;
            View->y = 0;
            View->width = 200;
            View->height = 800;
            
            SetupView(ProgramState);
        }
        
        //Log("Hello!");
        
        ProgramState->dTime = GetFrameTime();
        document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
        layer *Layer = &CurrentDocument->Layers[CurrentDocument->LayerIndex];
        ImGuiIO& IO = ImGui::GetIO();
        ProgramState->CursorInImGui = IO.WantCaptureMouse;
        ProgramState->ScreenWidth = GetScreenWidth();
        ProgramState->ScreenHeight = GetScreenHeight();
        i32 ScreenWidth = ProgramState->ScreenWidth;
        i32 ScreenHeight = ProgramState->ScreenHeight;
        v2 MousePosition = GetMousePosition();
        
        if(IsWindowResized())
        {
            SetupView(ProgramState);
        }
        
        if(!ProgramState->CursorInImGui)
        {
            f32 dScale = 0;
            f32 MouseScroll = GetMouseWheelMoveV().y;
            dScale += MouseScroll;
            // TODO(aidan): move towards (or away from) cursor upon mouse scroll
            
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
        
        UpdateTransformToolGeometry(ProgramState);
        ProcessTool(ProgramState);
        UpdateTransformToolGeometry(ProgramState);
        
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
                DrawLayerOutline(Layer, CurrentDocument, View);
            }
            
            DrawTool(ProgramState);
            
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
                    layer NewLayer = *Layer;
                    ListAdd(&CurrentDocument->Layers, NewLayer);
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
                            ProgramState->Tool = (tool_type)i;
                        }
                    }
                    ImGui::TreePop();
                }
                
                // LAYERS
                if(ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    f32 LayerAlpha = ((f32)Layer->ModColor.a)/255.0f;
                    ImGui::SliderFloat(" ", &LayerAlpha, 0, 1, "Alpha: %.3f");
                    Layer->ModColor.a = (u8)(LayerAlpha*255.0f);
                    Layer->ModColor.r = 255;
                    Layer->ModColor.g = 255;
                    Layer->ModColor.b = 255;
                    
                    
                    ImGui::DragFloat2("Position", (f32 *)(&Layer->Pos));
                    ImGui::DragFloat2("Dimensions", (f32 *)(&Layer->Dim));
                    ImGui::DragFloat("Rotation", &Layer->Rotation);
                    
                    if(ImGui::BeginListBox(""))
                    {
                        for(int i = 0; i < CurrentDocument->Layers.Count; i++)
                        {
                            sprintf(Buffer, "layer %d", i);
                            b32 Selected = ImGui::Selectable(Buffer, CurrentDocument->LayerIndex == i);
                            ImGui::SameLine();
                            b32 ShouldDelete = ImGui::Button("Delete");
                            if(ShouldDelete)
                            {
                                ListRemoveAt(&CurrentDocument->Layers, i);
                            }
                            else if(Selected)
                            {
                                CurrentDocument->LayerIndex = i;
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
                            // TODO(aidan): proper action naming
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