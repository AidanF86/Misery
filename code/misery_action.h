/* date = September 24th 2023 11:13 pm */

#ifndef MISERY_ACTION_H
#define MISERY_ACTION_H

// TODO(cheryl): BROKEN, FIX

void
UndoAction(program_state *ProgramState, action *Action)
{
    printf("Undoing");
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[Action->LayerIndex];
#if 0
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
        Layer->Rotation = Action->InitialAngle;
    }
#endif
    
    ProgramState->PrevActionIndex--;
}

void
RedoAction(program_state *ProgramState, action *Action)
{
    document *CurrentDocument = &ProgramState->OpenDocuments[ProgramState->CurrentDocumentIndex];
    layer *Layer = &CurrentDocument->Layers[Action->LayerIndex];
#if 0
    if(Action->Type == Action_Translate)
    {
        printf("REDOING: Translate Action: %d to %d\n", Action->Translate_InitialPosition, Action->FinalPosition);
        Layer->Rect.x = Action->FinalPosition.x;
        Layer->Rect.y = Action->FinalPosition.y;
    }
    else if(Action->Type == Action_Rotate)
    {
        printf("REDOING ROTATE\n");
        Layer->Rotation = Action->FinalAngle;
    }
#endif
    
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




#endif //MISERY_ACTION_H
