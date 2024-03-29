/* date = September 24th 2023 11:27 pm */

#ifndef MISERY_RECTS_H
#define MISERY_RECTS_H

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


#endif //MISERY_RECTS_H
