/* date = September 7th 2023 2:58 pm */

#ifndef MISERY_LISTS_H
#define MISERY_LISTS_H


/*
    Lists present:
    - layer
    - document
*/

#include <stdlib.h>

#define ListDoubleSize(List) {\
(List)->ArraySize *= 2;\
(List)->Data = (typeof((List)->Data))realloc((List)->Data, sizeof((List)->Data[0]) * (List)->ArraySize);\
}

#define ListAdd(List, E) {\
if((List)->Count + 1 > (List)->ArraySize) { ListDoubleSize(List); }\
(List)->Data[(List)->Count] = (E);\
(List)->Count++;\
}
/*======= layer List =======*/


layer_list LayerList()
{
    layer_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.Data = (layer *)malloc(20 * sizeof(layer));
    return Result;
}

layer_list LayerList(int Size)
{
    layer_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (layer *)malloc(Size * sizeof(layer));
    return Result;
}

#if 0
int ListIndexOf(layer_list *List, layer E)
{
    int Result = -1;
    for(int i = 0; i < List->Count; i++)
    {
        if(List->Data[i] == E)
        {
            Result = i;
            break;
        }
    }
    return Result;
}
#endif

int ListRemoveAt(layer_list *List, int Index)
{
    if(Index >= 0 && Index < List->Count)
    {
        for(int i = Index; i < List->Count; i++)
        {
            List->Data[i] = List->Data[i+1];
        }
        List->Count--;
        return 1;
    }
    return 0;
}
#if 0
int ListRemove(layer_list *List, layer E)
{
    int Index = ListIndexOf(List, E);
    if(Index != -1)
    {
        ListRemoveAt(List, Index);
    }
    return Index;
}
#endif

/*======= document List =======*/
document_list DocumentList()
{
    document_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.Data = (document *)malloc(20 * sizeof(document));
    return Result;
}

document_list DocumentList(int Size)
{
    document_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.Data = (document *)malloc(Size * sizeof(document));
    return Result;
}

#if 0
int ListIndexOf(document_list *List, document E)
{
    int Result = -1;
    for(int i = 0; i < List->Count; i++)
    {
        if(List->Data[i] == E)
        {
            Result = i;
            break;
        }
    }
    return Result;
}
#endif

int ListRemoveAt(document_list *List, int Index)
{
    if(Index >= 0 && Index < List->Count)
    {
        for(int i = Index; i < List->Count; i++)
        {
            List->Data[i] = List->Data[i+1];
        }
        List->Count--;
        return 1;
    }
    return 0;
}
#if 0
int ListRemove(document_list *List, document E)
{
    int Index = ListIndexOf(List, E);
    if(Index != -1)
    {
        ListRemoveAt(List, Index);
    }
    return Index;
}
#endif



#endif //MISERY_LISTS_H
