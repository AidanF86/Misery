/* date = September 24th 2023 11:25 pm */

#ifndef MISERY_TOOL_H
#define MISERY_TOOL_H

void
FlipImageIfNecessary(program_state *ProgramState, layer *Layer)
{
    // TODO(cheryl): REIMPLEMENT
}


b32
PointInGrabPoint(v2 Pos, grab_point *GrabPoint, f32 Rotation)
{
    // TODO(cheryl): this might be inefficient
    f32 RadRotation = Rotation*DEG2RAD;
    v2 Vertices[4];
    Vertices[0] = GrabPoint->Pos + Vector2Rotate(V2(-GrabPoint->w/2, -GrabPoint->h/2), RadRotation);
    Vertices[1] = GrabPoint->Pos + Vector2Rotate(V2(GrabPoint->w/2, -GrabPoint->h/2), RadRotation);
    Vertices[2] = GrabPoint->Pos + Vector2Rotate(V2(GrabPoint->w/2, GrabPoint->h/2), RadRotation);
    Vertices[3] = GrabPoint->Pos + Vector2Rotate(V2(-GrabPoint->w/2, GrabPoint->h/2), RadRotation);
    
    return CheckCollisionPointPoly(Pos, Vertices, 4);
}

i32
GetGrabbedPointIndex(grab_point *Points, int Count)
{
    for(int i = 0; i < Count; i++)
    {
        if(Points[i].IsGrabbed)
            return i;
    }
    return -1;
}

b32
AnyGrabPointIsAlreadyGrabbed(grab_point *Points, int Count)
{
    return GetGrabbedPointIndex(Points, Count) != -1;
}

void
ProcessTool(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[CurrentDocument->LayerIndex];
    v2 MousePosition = GetMousePosition();
    trans Trans = GetLayerScreenTrans(Layer, CurrentDocument, View);
    f32 Rotation = Trans.Rotation;
    transform_tool_data *Data = &ProgramState->TransformToolData;
    
    if(ProgramState->Tool == Tool_Translate)
    {
        if(!AnyGrabPointIsAlreadyGrabbed(Data->Translate_GrabPoints, 3))
        {// Check for grabbing
            for(int i = 0; i < 3; i++)
            {
                if(PointInGrabPoint(MousePosition, &Data->Translate_GrabPoints[i], Rotation))
                {
                    Data->Translate_GrabPoints[i].IsGrabbed = true;
                    Print("BRUH");
                }
            }
        }
        else
        {// Compute dragging
            // Get grabbed grab point
            grab_point *Point = &Data->Translate_GrabPoints[GetGrabbedPointIndex(Data->Translate_GrabPoints, 3)];
            Print("AAA");
        }
    }
    
    
#if 0
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
                
                if(CheckCollisionPointRec(MousePosition, Tool->Translate_BoxRect))
                    Tool->Translate_DraggingBoth = true;
                else if(CheckCollisionPointRec(MousePosition, Tool->Translate_XArrowRect))
                    Tool->Translate_DraggingX = true;
                else if(CheckCollisionPointRec(MousePosition, Tool->Translate_YArrowRect))
                    Tool->Translate_DraggingY = true;
                else
                    Tool->BeingUsed = false;
                
                if(Tool->BeingUsed)
                {
                    Tool->Translate_InitialPosition = V2(Layer->Rect.x, Layer->Rect.y);
                }
            }
            
            f32 ViewDocRatio = CurrentDocument->h / View->height / CurrentDocument->Scale;
            f32 Scalar = ViewDocRatio;
            v2 dPosition = GetMouseDelta();
            
            if(Tool->Translate_DraggingBoth)
            {
                Layer->Rect += dPosition * Scalar;
            }
            else if(Tool->Translate_DraggingX)
            {
                Layer->Rect.x += dPosition.x * Scalar;
            }
            else if(Tool->Translate_DraggingY)
            {
                Layer->Rect.y += dPosition.y * Scalar;
            }
        }
        else
        {
            if(Tool->Translate_DraggingX || Tool->Translate_DraggingY || Tool->Translate_DraggingBoth)
            {
                action Action;
                Action.Type = Action_Translate;
                Action.Translate_InitialPosition = Tool->Translate_InitialPosition;
                Action.FinalPosition = V2(Layer->Rect.x, Layer->Rect.y);
                Action.LayerIndex = CurrentDocument->LayerIndex;
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
                    if(CheckCollisionPointRec(MousePosition, Tool->Transform_Rects[i]))
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
            
            rect NewRect = Layer->Rect;
            
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
            
            Layer->Rect = NewRect;
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
                f32 OuterRadius = Tool->Rotate_Radius + Tool->Rotate_Thickness / 2.0f;
                f32 InnerRadius = Tool->Rotate_Radius - Tool->Rotate_Thickness / 2.0f;
                
                b32 InLargeCircle = CheckCollisionPointCircle(MousePosition, LayerCenter, OuterRadius);
                b32 InSmallCircle = CheckCollisionPointCircle(MousePosition, LayerCenter, InnerRadius);
                
                Tool->Rotation_Dragging = false;
                if(InLargeCircle)
                {
                    // start dragging
                    Tool->Rotation_Dragging = true;
                    Tool->Rotation_InitialAngle = Layer->Rotation;
                    
                    v2 UpVector = V2(0, 1);
                    rect LayerScreenRect = GetLayerScreenRect(Layer, CurrentDocument, View);
                    v2 LayerScreenCenter = V2(LayerScreenRect.x + LayerScreenRect.width / 2.0f,
                                              LayerScreenRect.y + LayerScreenRect.height / 2.0f);
                    v2 MouseVector = MousePosition - LayerScreenCenter;
                    
                    Tool->Rotation_InitialMouseAngle = Vector2Angle(UpVector, MouseVector);
                }
            }
            else
            {
                v2 UpVector = V2(0, 1);
                rect LayerScreenRect = GetLayerScreenRect(Layer, CurrentDocument, View);
                v2 LayerScreenCenter = V2(LayerScreenRect.x + LayerScreenRect.width / 2.0f,
                                          LayerScreenRect.y + LayerScreenRect.height / 2.0f);
                v2 MouseVector = MousePosition - LayerScreenCenter;
                
                f32 MouseAngle = Vector2Angle(UpVector, MouseVector);
                f32 dAngle = MouseAngle - Tool->Rotation_InitialMouseAngle;
                f32 NewAngle = Tool->Rotation_InitialAngle + dAngle;
                
                printf("%f, ", dAngle);
                printf("%f\n", NewAngle);
                
                Layer->Rotation = NewAngle;
            }
        }
        else if(Tool->Rotation_Dragging)
        {
            if(Tool->Rotation_Dragging)
            {
                action Action;
                Action.Type = Action_Rotate;
                Action.InitialAngle = Tool->Rotation_InitialAngle;
                Action.FinalAngle = Layer->Rotation;
                Action.LayerIndex = CurrentDocument->LayerIndex;
                AddAction(ProgramState, Action);
            }
            Tool->Rotation_Dragging = false;
        }
    }
    
    FlipImageIfNecessary(Layer, Tool);
#endif
}


void
InitializeTransformToolData(program_state *ProgramState)
{
    transform_tool_data *Data = &ProgramState->TransformToolData;
    for(int i = 0; i < sizeof(*Data) / sizeof(grab_point); i++)
    {
        // TODO(cheryl): check this number
        Data->GrabPoints[i] = {0};
    }
    Data->XArrow.MoveX = true;
    Data->YArrow.MoveY = true;
    Data->BothBox.MoveX = true;
    Data->BothBox.MoveY = true;
    
    Data->GrabCircle.Rotate = true;
    
    Data->LeftScalar.ScaleLeft = true;
    Data->RightScalar.ScaleRight = true;
    Data->TopScalar.ScaleTop = true;
    Data->BottomScalar.ScaleBottom = true;
    
    Data->TopLeft.ScaleTop = true;
    Data->TopLeft.ScaleLeft = true;
    Data->Top.ScaleTop = true;
    Data->TopRight.ScaleTop = true;
    Data->TopRight.ScaleRight = true;
    Data->Left.ScaleLeft = true;
    Data->Right.ScaleRight = true;
    Data->BottomLeft.ScaleBottom = true;
    Data->BottomLeft.ScaleLeft = true;
    Data->Bottom.ScaleBottom = true;
    Data->BottomRight.ScaleBottom = true;
    Data->BottomRight.ScaleRight = true;
    Data->WholeLayer.MoveX = true;
    Data->WholeLayer.MoveY = true;
    Data->TopLeftRotate.Rotate = true;
    Data->TopRightRotate.Rotate = true;
    Data->BottomLeftRotate.Rotate = true;
    Data->BottomRightRotate.Rotate = true;
}

void
UpdateTransformToolGeometry(program_state *ProgramState)
{
    // TODO(cheryl): Account for rotation (this is a big task).
    rect *View = &ProgramState->View;
    document *Document = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &Document->Layers[Document->LayerIndex];
    trans Trans = GetLayerScreenTrans(Layer, Document, View);
    
    transform_tool_data *Data = &ProgramState->TransformToolData;
    
    // Copy values
    f32 TranslateTool_ArrowLength = ProgramState->TranslateTool_ArrowLength;
    f32 TranslateTool_ArrowWidth = ProgramState->TranslateTool_ArrowWidth;
    f32 TranslateTool_BoxSize = ProgramState->TranslateTool_BoxSize;
    
    f32 TransformTool_BoxSize = ProgramState->TransformTool_BoxSize;
    
    f32 RotateTool_Radius = ProgramState->RotateTool_Radius;
    f32 RotateTool_Thickness = ProgramState->RotateTool_Thickness;
    
    v2 Pos = Trans.Pos;
    f32 x = Pos.x;
    f32 y = Pos.y;
    v2 Dim = Trans.Dim;
    f32 w = Dim.x;
    f32 h = Dim.y;
    f32 Rotation = Trans.Rotation;
    f32 RadRotation = Rotation*DEG2RAD;
    
    // Useful Positions
    v2 TopLeft = Pos + Vector2Rotate(V2(-w/2, -h/2), RadRotation);
    v2 TopRight = Pos + Vector2Rotate(V2(w/2, -h/2), RadRotation);
    v2 BottomRight = Pos + Vector2Rotate(V2(w/2, h/2), RadRotation);
    v2 BottomLeft = Pos + Vector2Rotate(V2(-w/2, h/2), RadRotation);
    
    v2 Top = Pos + Vector2Rotate(V2(0, -h/2), RadRotation);
    v2 Bottom = Pos + Vector2Rotate(V2(0, h/2), RadRotation);
    v2 Right = Pos + Vector2Rotate(V2(w/2, 0), RadRotation);
    v2 Left = Pos + Vector2Rotate(V2(-w/2, 0), RadRotation);
    
    // TRANSLATE
    // TODO(cheryl): add option to have local and global transform
    Data->XArrow.Pos = Pos + Vector2Rotate(V2(TranslateTool_ArrowLength/2, 0), RadRotation);
    Data->XArrow.Dim = V2(TranslateTool_ArrowLength,
                          TranslateTool_ArrowWidth);
    Data->YArrow.Pos = Pos + Vector2Rotate(V2(0, TranslateTool_ArrowLength/2), RadRotation);
    Data->YArrow.Dim = V2(TranslateTool_ArrowWidth,
                          TranslateTool_ArrowLength);
    Data->BothBox.Pos = Pos + Vector2Rotate(V2(TransformTool_BoxSize/2, TransformTool_BoxSize/2), RadRotation);
    Data->BothBox.Dim = V2(TransformTool_BoxSize, TransformTool_BoxSize);
    
    
    // ROTATE
    Data->GrabCircle.Pos = Pos;
    Data->GrabCircle.MinRadius = RotateTool_Radius - RotateTool_Thickness;
    Data->GrabCircle.MaxRadius = RotateTool_Radius + RotateTool_Thickness;
    
    //Data->TopLeft.Pos = Pos  
}

void
DrawGrabPointRect(grab_point *Point, f32 Rotation)
{
    DrawRectangleLinesPro(Point->Pos, Point->Dim, Rotation, 5, BLUE);
}

void
DrawTool(program_state *ProgramState)
{
    rect *View = &ProgramState->View;
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[CurrentDocument->LayerIndex];
    trans Trans = GetLayerScreenTrans(Layer, CurrentDocument, View);
    transform_tool_data *Data = &ProgramState->TransformToolData;
    
    f32 Rotation = Trans.Rotation;
    
    switch(ProgramState->Tool)
    {
        case Tool_Translate: {
            DrawGrabPointRect(&Data->Translate_GrabPoints[0], Rotation);
            DrawGrabPointRect(&Data->Translate_GrabPoints[1], Rotation);
            DrawGrabPointRect(&Data->Translate_GrabPoints[2], Rotation);
            break;
        };
        
    }
}

#endif
//MISERY_TOOL_H
