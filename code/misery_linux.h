/* date = August 28th 2023 4:10 pm */

#ifndef MISERY_LINUX_H
#define MISERY_LINUX_H

struct linux_program_code
{
    void *Code;
    program_update_and_render *UpdateAndRender;
    
    time_t LastWriteTime;
    b32 IsValid;
};

#endif //MISERY_LINUX_H
