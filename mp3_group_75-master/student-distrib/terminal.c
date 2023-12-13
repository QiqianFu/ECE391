#include "terminal.h"
#include "lib.h"
#include "syscall.h"
#include "sched.h"
terminal_data the_terminal[TERMINAL_PAGE];

#define VIDEO       0xB8000

/*ter_read*
 * input: return data by pressing Enter, or buffer
 * return: the number of bytes read
 * description: read the data from buffer when enter is pressed
 *  
 */
int32_t ter_read(int32_t fd, void* buf, int32_t nybtes){
    if (buf == NULL || nybtes <= 0 ){
        return -1;
    }
    the_terminal[SchedID].enter_press = 0;             // init, since we need to wait for enter
    the_terminal[SchedID].counting=0;
    memset(the_terminal[SchedID].buffer_data, 0, MAX_BUFFER);
    while(the_terminal[SchedID].enter_press==0){}

    int i =0;
    for(; i < nybtes ; i++ ){
        char current_char = the_terminal[SchedID].buffer_data[i];
        if(current_char != '\0'){
            (((char*)buf)[i]) = the_terminal[SchedID].buffer_data[i];
        }else{
            break;
        }
    }
    return i;

}

/*ter_write*
 * input: return data by pressing Enter, or buffer
 * return: the number of bytes read
 * description: read the data from buffer when enter is pressed
 *  
 */
int32_t ter_write(int32_t fd, const void* buf, int32_t nbytes){
    if (buf==NULL || nbytes<=0){
        return -1;
    }
    int i = 0;
    for( ; i < nbytes; i++){
        if(((char*)buf)[i] == '\0'){
            break;
        }
        putc(((char*)buf)[i]);
    }
    return i;
}

/*ter_init*
 * input: none
 * return: none
 * description: init the terminal 
 */
int32_t ter_init(){
    int i ;
    for(i=0;i<TERMINAL_PAGE;i++){
        memset(the_terminal[i].buffer_data ,0,MAX_BUFFER);
        the_terminal[i].counting = 0 ;
        the_terminal[i].enter_press = 0;
        the_terminal[i].x_cor = 0;
        the_terminal[i].y_cor = 0;
        the_terminal[i].active_process_ID = -1;
    }

    return 0;
}

/*ter_open*
 * input: filename
 * return: none
 * description: we don't need to use it so far
 */
int32_t ter_open(const uint8_t* filename){
    return 0;
}


/*ter_close*
 * input: file id 
 * return: none
 * description: we don't need it so far
 */
int32_t ter_close(int32_t fd){
    return 0;
}
