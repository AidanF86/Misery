#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.cpp"

#include "misery_base.h"
#include "misery_linux.h"

#define SO_FILE_NAME "misery.so"

time_t
LinuxGetFileLastWriteTime(const char *FileName)
{
    struct stat Buffer;
    if(stat(FileName, &Buffer))
    {
        // Failure
        printf("\nfstat failed with file \"%s\"!\n\n", FileName);
        return 0;
    }
    return Buffer.st_mtime;
}

void
LinuxUnloadProgramCode(linux_program_code *ProgramCode)
{
    if(ProgramCode->Code)
    {
        printf("CLOSING CODE\n");
        dlclose(ProgramCode->Code);
        ProgramCode->Code = 0;
    }
    else
    {
        printf("CODE WAS NULL\n");
    }
    ProgramCode->UpdateAndRender = 0;
    ProgramCode->IsValid = false;
}

linux_program_code
LinuxLoadProgramCode(const char *FileName)
{
    linux_program_code Result;
    Result.IsValid = true;
    Result.Code = dlopen(FileName, RTLD_NOW);
    
    char *Error = dlerror();
    if(Error)
    {
        Result.IsValid = false;
        printf("dlopen ERROR!\n");
        printf("    %s\n\n", Error);
    }
    else 
    {
        Result.LastWriteTime = LinuxGetFileLastWriteTime(FileName);
        
        Result.UpdateAndRender = (program_update_and_render *)
            dlsym(Result.Code, "ProgramUpdateAndRender");
        
        char *Error = dlerror();
        if(Error)
        {
            Result.UpdateAndRender = 0;
            Result.IsValid = false;
            printf("dlsym ERROR!\n");
            printf("    %s\n\n", Error);
        }
        
        //dlclose(Result.Code);
    }
    
    return Result;
}

#define Log(Format, ...) if(LogFile) { fprintf(LogFile, "[%ld]   ", time(NULL) - StartTime); fprintf(LogFile, Format "\n", ##__VA_ARGS__); }

int main()
{
    // TODO(cheryl): change this to a better location
    mkdir("./logs", 0755);
    char LogFileNameBuffer[50];
    time_t StartTime = time(NULL);
    tm *DateTime = localtime(&StartTime);
    sprintf(LogFileNameBuffer, "./logs/misery_log_%d_%d_%d_%d_%d_%d.txt", 
            DateTime->tm_year, DateTime->tm_mon, DateTime->tm_mday,
            DateTime->tm_hour, DateTime->tm_min, DateTime->tm_sec);
    FILE *LogFile = fopen(LogFileNameBuffer, "w");
    if(!LogFile)
    {
        printf("ERROR: Can't create log file!\n");
    }
    
    fprintf(LogFile, "MISERY LOG FILE\n");
    fprintf(LogFile, "%s\n", asctime(DateTime));
    fprintf(LogFile, "=============================\n");
    
    linux_program_code ProgramCode = LinuxLoadProgramCode(SO_FILE_NAME);
    if(!ProgramCode.IsValid)
    {
        Log("Invalid program code - Exiting.");
        return 1;
    }
    
    program_memory Memory;
    Memory.Initialized = false;
    Memory.Size = Kilobytes(1);
    Memory.Data = malloc(Memory.Size);
    Memory.WindowHeight = 800;
    Memory.WindowWidth = 1400;
    Memory.IsRunning = true;
    
    Log("Initialized memory");
    
    InitWindow(Memory.WindowWidth, Memory.WindowHeight, "Misery");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    rlImGuiSetup(true);
    
    while(Memory.IsRunning)
    {
#if 1
        time_t SOLastWriteTime = LinuxGetFileLastWriteTime(SO_FILE_NAME);
        if(SOLastWriteTime != ProgramCode.LastWriteTime)
        {
            Log("Unloading program code...");
            LinuxUnloadProgramCode(&ProgramCode);
            sleep(1);
            Log("Reloading program code from \"%s\"", SO_FILE_NAME);
            ProgramCode = LinuxLoadProgramCode(SO_FILE_NAME);
            if(ProgramCode.IsValid)
            {
                Log("Program code reloaded.\n");
            }
            else
            {
                Log("[ERROR] Failed to reload program code");
            }
        }
#endif
#if 0
        if(IsKeyPressed(KEY_U))
        {
            if(ProgramCode.Code)
            {
                LinuxUnloadProgramCode(&ProgramCode);
                printf("Unloading code\n");
            }
            else
            {
                printf("Code is already unloaded!\n");
            }
        }
        if(IsKeyPressed(KEY_L))
        {
            ProgramCode = LinuxLoadProgramCode(SO_FILE_NAME);
            printf("Loading code\n");
        }
#endif
        
        if(ProgramCode.UpdateAndRender)
        {
            ProgramCode.UpdateAndRender(&Memory);
        }
        else
        {
            Log("UpdateAndRender is NULL!");
            //BeginDrawing();
            //ClearBackground(WHITE);
            //EndDrawing();
        }
    }
    
    rlImGuiShutdown();
    
    return 0;
}