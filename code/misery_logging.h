/* date = September 12th 2023 9:43 pm */

#ifndef MISERY_LOGGING_H
#define MISERY_LOGGING_H

FILE *LogFile;
b32 IsLogFileOpen;

void
Log(string String)
{
    if(!IsLogFileOpen)
    {
        return;
    }
    
    for(int i = 0; i < String.Length; i++)
    {
        // NOTE(aidan): does stdout work on windows?
        putc(String.Data[i], LogFile);
    }
    putc('\n', LogFile);
}

void
Log(const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    string Str = _String(Format, Args);
    Log(Str);
    FreeString(Str);
}

#endif //MISERY_LOGGING_H
