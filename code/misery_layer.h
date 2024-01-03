/* date = September 24th 2023 11:29 pm */

#ifndef MISERY_LAYER_H
#define MISERY_LAYER_H

trans
GetLayerScreenTrans(layer *Layer, document *Document, rect *View)
{
    rect DocRect = GetDocumentRect(Document, View);
    //rect LayerRect = GetLayerRect(Layer);
    f32 ViewDocRatio = View->height / Document->h;
    
    v2 Pos = V2(DocRect.x, DocRect.y) + (Layer->Pos * Document->Scale * ViewDocRatio);
    v2 Dim = Layer->Dim * (Document->Scale * ViewDocRatio);
    
    return Trans(Pos, Dim, Layer->Rotation);
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
    Layer.Trans = Trans(Layer.Texture.width / 2, Layer.Texture.height / 2,
                        Layer.Texture.width, Layer.Texture.height, 0);
    ListAdd(&ProgramState->OpenDocuments[0].Layers, Layer);
}

void
MakeNewRectLayer(program_state *ProgramState)
{
    layer Layer;
    Layer.Type = LayerType_Rectangle;
    // TODO(aidan): put this in the middle and scale based on canvas size
    Layer.Trans = Trans(50, 50, 100, 100, 0);
    // TODO(aidan): determines on how we handle the ui for layer color
    Layer.ModColor = RED;
    ListAdd(&ProgramState->OpenDocuments[0].Layers, Layer);
}

#endif //MISERY_LAYER_H
