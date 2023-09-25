/* date = September 24th 2023 11:25 pm */

#ifndef MISERY_TOOL_H
#define MISERY_TOOL_H



void
FlipImageIfNecessary(layer *Layer, tool_state *Tool)
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
        if(Tool->Transform_DraggingLeft)
        {
            Tool->Transform_DraggingLeft = false;
            Tool->Transform_DraggingRight = true;
        }
        else if(Tool->Transform_DraggingTopLeft)
        {
            Tool->Transform_DraggingTopLeft = false;
            Tool->Transform_DraggingTopRight = true;
        }
        else if(Tool->Transform_DraggingBottomLeft)
        {
            Tool->Transform_DraggingBottomLeft = false;
            Tool->Transform_DraggingBottomRight = true;
        }
        else if(Tool->Transform_DraggingRight)
        {
            Tool->Transform_DraggingRight = false;
            Tool->Transform_DraggingLeft = true;
        }
        else if(Tool->Transform_DraggingTopRight)
        {
            Tool->Transform_DraggingTopRight = false;
            Tool->Transform_DraggingTopLeft = true;
        }
        else if(Tool->Transform_DraggingBottomRight)
        {
            Tool->Transform_DraggingBottomRight = false;
            Tool->Transform_DraggingBottomLeft = true;
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
        if(Tool->Transform_DraggingTop)
        {
            Tool->Transform_DraggingTop = false;
            Tool->Transform_DraggingBottom = true;
        }
        else if(Tool->Transform_DraggingTopLeft)
        {
            Tool->Transform_DraggingTopLeft = false;
            Tool->Transform_DraggingBottomLeft = true;
        }
        else if(Tool->Transform_DraggingTopRight)
        {
            Tool->Transform_DraggingTopRight = false;
            Tool->Transform_DraggingBottomRight = true;
        }
        else if(Tool->Transform_DraggingBottom)
        {
            Tool->Transform_DraggingBottom = false;
            Tool->Transform_DraggingTop = true;
        }
        else if(Tool->Transform_DraggingBottomLeft)
        {
            Tool->Transform_DraggingBottomLeft = false;
            Tool->Transform_DraggingTopLeft = true;
        }
        else if(Tool->Transform_DraggingBottomRight)
        {
            Tool->Transform_DraggingBottomRight = false;
            Tool->Transform_DraggingTopRight = true;
        }
    }
}



void
ProcessTool(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    tool_state *Tool = &ProgramState->Tool;
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *CurrentLayer = &CurrentDocument->Layers[CurrentDocument->CurrentLayerIndex];
    v2 MousePosition = GetMousePosition();
    rect LayerRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
    v2 LayerCenter = V2(LayerRect.x + LayerRect.width / 2.0f,
                        LayerRect.y + LayerRect.height / 2.0f);
    
    if(Tool->Type == Tool_Translate)
    {
        if(IsMouseButtonDown(0))
        {
            if(!Tool->Translate_DraggingX && !Tool->Translate_DraggingY && !Tool->Translate_DraggingBoth)
            {
                Tool->BeingUsed = true;
                Tool->Translate_DraggingBoth = false;
                Tool->Translate_DraggingX = false;
                Tool->Translate_DraggingY = false;
                
                if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolBoxRect))
                    Tool->Translate_DraggingBoth = true;
                else if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolXArrowRect))
                    Tool->Translate_DraggingX = true;
                else if(CheckCollisionPointRec(MousePosition, ProgramState->TranslateToolYArrowRect))
                    Tool->Translate_DraggingY = true;
                else
                    Tool->BeingUsed = false;
                
                if(Tool->BeingUsed)
                {
                    Tool->Translate_InitialPosition = V2(CurrentLayer->Rect.x, CurrentLayer->Rect.y);
                }
            }
            
            f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
            f32 Scalar = ViewDocRatio;
            v2 dPosition = GetMouseDelta();
            
            if(Tool->Translate_DraggingBoth)
            {
                CurrentLayer->Rect += dPosition * Scalar;
            }
            else if(Tool->Translate_DraggingX)
            {
                CurrentLayer->Rect.x += dPosition.x * Scalar;
            }
            else if(Tool->Translate_DraggingY)
            {
                CurrentLayer->Rect.y += dPosition.y * Scalar;
            }
        }
        else
        {
            if(Tool->Translate_DraggingX || Tool->Translate_DraggingY || Tool->Translate_DraggingBoth)
            {
                action Action;
                Action.Type = Action_Translate;
                Action.Translate_InitialPosition = Tool->Translate_InitialPosition;
                Action.FinalPosition = V2(CurrentLayer->Rect.x, CurrentLayer->Rect.y);
                Action.LayerIndex = CurrentDocument->CurrentLayerIndex;
                AddAction(ProgramState, Action);
            }
            Tool->Translate_DraggingBoth = false;
            Tool->Translate_DraggingX = false;
            Tool->Translate_DraggingY = false;
        }
    }
    else if(Tool->Type == Tool_Transform)
    {
        b32 DraggingAnyTransformControl = Tool->Transform_Dragging[0] || Tool->Transform_Dragging[1] || Tool->Transform_Dragging[2] || Tool->Transform_Dragging[3] || Tool->Transform_Dragging[4] || Tool->Transform_Dragging[5] || Tool->Transform_Dragging[6] || Tool->Transform_Dragging[7] || Tool->Transform_Dragging[8];
        
        if(IsMouseButtonDown(0))
        {
            if(!DraggingAnyTransformControl)
            {
                for(int i = 0; i < 9; i++)
                {
                    Tool->Transform_Dragging[i] = false;
                }
                
                for(int i = 0; i < 9; i++)
                {
                    if(CheckCollisionPointRec(MousePosition, ProgramState->TransformToolRects[i]))
                    {
                        Tool->Transform_Dragging[i] = true;
                    }
                }
                // TODO(Cheryl): set initial state
            }
            
            DraggingAnyTransformControl = Tool->Transform_Dragging[0] || Tool->Transform_Dragging[1] || Tool->Transform_Dragging[2] || Tool->Transform_Dragging[3] || Tool->Transform_Dragging[4] || Tool->Transform_Dragging[5] || Tool->Transform_Dragging[6] || Tool->Transform_Dragging[7] || Tool->Transform_Dragging[9];
            
            f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
            f32 Scalar = ViewDocRatio;
            v2 dPosition = GetMouseDelta();
            v2 dPosScaled = dPosition * Scalar;
            
            rect NewRect = CurrentLayer->Rect;
            
            if(Tool->Transform_DraggingRight)
            {
                NewRect.width += dPosScaled.x;
            }
            else if(Tool->Transform_DraggingLeft)
            {
                NewRect.x += dPosScaled.x;
                NewRect.width -= dPosScaled.x;
            }
            else if(Tool->Transform_DraggingBottom)
            {
                NewRect.height += dPosScaled.y;
            }
            else if(Tool->Transform_DraggingTop)
            {
                NewRect.y += dPosScaled.y;
                NewRect.height -= dPosScaled.y;
            }
            else if(Tool->Transform_DraggingTopLeft)
            {
                NewRect.x += dPosScaled.x;
                NewRect.width -= dPosScaled.x;
                NewRect.y += dPosScaled.y;
                NewRect.height -= dPosScaled.y;
            }
            else if(Tool->Transform_DraggingTopRight)
            {
                NewRect.width += dPosScaled.x;
                NewRect.y += dPosScaled.y;
                NewRect.height -= dPosScaled.y;
            }
            else if(Tool->Transform_DraggingBottomLeft)
            {
                NewRect.height += dPosScaled.y;
                NewRect.x += dPosScaled.x;
                NewRect.width -= dPosScaled.x;
            }
            else if(Tool->Transform_DraggingBottomRight)
            {
                NewRect.height += dPosScaled.y;
                NewRect.width += dPosScaled.x;
            }
            else if(Tool->Transform_DraggingWhole)
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
                Tool->Transform_Dragging[i] = false;
            }
        }
    }
    else if(Tool->Type == Tool_Rotate)
    {
        if(IsMouseButtonDown(0))
        {
            if(!Tool->Rotation_Dragging)
            {
                // start dragging if the cursor is on the tool
                f32 OuterRadius = ProgramState->RotateToolRadius + ProgramState->RotateToolThickness / 2.0f;
                f32 InnerRadius = ProgramState->RotateToolRadius - ProgramState->RotateToolThickness / 2.0f;
                
                b32 InLargeCircle = CheckCollisionPointCircle(MousePosition, LayerCenter, OuterRadius);
                b32 InSmallCircle = CheckCollisionPointCircle(MousePosition, LayerCenter, InnerRadius);
                
                Tool->Rotation_Dragging = false;
                if(InLargeCircle)
                {
                    // start dragging
                    Tool->Rotation_Dragging = true;
                    Tool->Rotation_InitialAngle = CurrentLayer->Angle;
                    
                    v2 UpVector = V2(0, 1);
                    rect LayerScreenRect = GetLayerScreenRect(CurrentLayer, CurrentDocument, View);
                    v2 LayerScreenCenter = V2(LayerScreenRect.x + LayerScreenRect.width / 2.0f,
                                              LayerScreenRect.y + LayerScreenRect.height / 2.0f);
                    v2 MouseVector = MousePosition - LayerScreenCenter;
                    
                    Tool->Rotation_InitialMouseAngle = Vector2Angle(UpVector, MouseVector);
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
                f32 dAngle = MouseAngle - Tool->Rotation_InitialMouseAngle;
                f32 NewAngle = Tool->Rotation_InitialAngle + dAngle;
                
                printf("%f, ", dAngle);
                printf("%f\n", NewAngle);
                
                CurrentLayer->Angle = NewAngle;
            }
        }
        else if(Tool->Rotation_Dragging)
        {
            if(Tool->Rotation_Dragging)
            {
                // TODO(cheryl): append action
                action Action;
                Action.Type = Action_Rotate;
                Action.InitialAngle = Tool->Rotation_InitialAngle;
                Action.FinalAngle = CurrentLayer->Angle;
                Action.LayerIndex = CurrentDocument->CurrentLayerIndex;
                AddAction(ProgramState, Action);
            }
            Tool->Rotation_Dragging = false;
        }
    }
    
    // TODO(cheryl): flip image
    FlipImageIfNecessary(CurrentLayer, Tool);
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

#endif
//MISERY_TOOL_H
