#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#define MAX_BUFFER 128
#define TERMINAL_PAGE 3

typedef struct terminal_data
{  
    uint8_t x_cor ;
    uint8_t y_cor ;
    char buffer_data[MAX_BUFFER];
    uint8_t counting; 
    uint8_t enter_press;
    int32_t active_process_ID;

}terminal_data;
extern terminal_data the_terminal[TERMINAL_PAGE];
extern int32_t ter_read(int32_t fd, void* buf, int32_t nybtes);
extern int32_t ter_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t ter_init(void);
extern int32_t ter_open(const uint8_t* filename);
extern int32_t ter_close(int32_t fd);

#endif
