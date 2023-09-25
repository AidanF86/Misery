/* date = September 24th 2023 11:29 pm */

#ifndef MISERY_LAYER_H
#define MISERY_LAYER_H

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

#endif //MISERY_LAYER_H
