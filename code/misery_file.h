/* date = September 24th 2023 11:30 pm */

#ifndef MISERY_FILE_H
#define MISERY_FILE_H

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

#endif //MISERY_FILE_H
