/* date = September 12th 2023 9:45 pm */

#ifndef MISERY_STRING_H
#define MISERY_STRING_H

#include <cstdarg>

int
NullTerminatedStringLength(const char *Contents)
{
    int Result = 0;
    while(*Contents++) { Result++; }
    return Result;
}

int
NullTerminatedStringLength(int *Contents)
{
    int Result = 0;
    while(*Contents++) { Result++; }
    return Result;
}

// UTF string
struct string
{
    int Length;
    i32 *Data;
};

string
AllocString(int Length)
{
    string Result;
    
    Result.Length = Length;
    Result.Data = (i32 *)malloc(sizeof(i32) * Length);
    
    return Result;
}

void
FreeString(string *String)
{
    free(String->Data);
}


string
_String(const char *Contents)
{
    string Result = AllocString(NullTerminatedStringLength(Contents));
    
    for(int i = 0; i < Result.Length; i++)
    {
        Result.Data[i] = (i32)(Contents[i]);
    }
    
    return Result;
}

string
String(int *Contents)
{
    string Result = AllocString(NullTerminatedStringLength(Contents));
    
    for(int i = 0; i < Result.Length; i++)
    {
        Result.Data[i] = (i32)(Contents[i]);
    }
    
    return Result;
}

char StringFormatBuffer[512];
char StringVarBuffer[128];
string
String(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    
    int Index = 0;
    
    b32 NextIsVariable = false;
    while(*Format != '\0')
    {
        if(*Format == '%')
        {
            NextIsVariable = true;
        }
        else
        {
            if(NextIsVariable)
            {
                // Do proper printing
                switch(*Format)
                {
                    case 'd':
                    {
                        int Var = va_arg(Args, int);
                        sprintf(StringVarBuffer, "%d", Var);
                    }break;
                    case 'f':
                    {
                        f64 Var = va_arg(Args, f64);
                        sprintf(StringVarBuffer, "%.3f", Var);
                    }break;
                    case 's':
                    {
                        char *Var = va_arg(Args, char *);
                        sprintf(StringVarBuffer, "%s", Var);
                    }break;
                    // TODO(cheryl): add our string (%S)
                    case 'v':
                    {
                        v2 Var = va_arg(Args, v2);
                        sprintf(StringVarBuffer, "(%.3f, %.3f)", Var.x, Var.y);
                    }break;
                }
                
                strcpy(&(StringFormatBuffer[Index]), StringVarBuffer);
                Index += NullTerminatedStringLength(StringVarBuffer);
            }
            else
            {
                StringFormatBuffer[Index] = *Format;
                Index++;
            }
            
            NextIsVariable = false;
        }
        
        Format++;
    }
    
    return _String(StringFormatBuffer);
}

void
Print(string String)
{
    for(int i = 0; i < String.Length; i++)
    {
        // NOTE(cheryl): does stdout work on windows?
        putc(String.Data[i], stdout);
    }
    putc('\n', stdout);
}

void
Print(const char *Data)
{
    string Str = _String(Data);
    Print(Str);
}


#endif //MISERY_STRING_H
