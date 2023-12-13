#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "sig.h"
#include "process.h"
#include "syscall.h"
#include "sched.h"
#define ascii_num           0x3E 

#define VIDEO               0xB8
#define Left_Shift_Press    0x2A
#define Left_Shift_Release  0xAA
#define Right_Shift_Press   0x36
#define Right_Shift_Release 0xB6
#define Ctrl_Press          0x1D
#define Ctrl_Release        0x9D
#define Caps_Press          0x3A
#define Caps_Release        0xBA
#define Keyboard_Dataport   0x60
#define Alt_Press           0x38
#define Alt_Release         0xB8
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

uint8_t Shift_Pressed = 0;
uint8_t Ctrl_Pressed = 0;
uint8_t Caps_Pressed = 0;
uint8_t Cap_status = 0;
uint8_t Alt_Pressed = 0;

//transfer scan code to ascii
char Ascii_table[ascii_num] = {
    '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b','\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[' ,']' ,'\n','\0',
    'a' , 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';' ,'\'', '`','\0','\\',
    'z' , 'x', 'c', 'v', 'b', 'n', 'm',
	',' , '.', '/','\0','\0','\0', ' ',
};

//transfer scan code to ascii in shift case
char Ascii_table_shift[ascii_num] = {
    '\0','\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'\b','\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	'{' , '}','\n','\0',
	'A' , 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' ,
	':' ,'\"', '~','\0', '|',
	'Z' , 'X', 'C', 'V', 'B', 'N', 'M',
	'<' , '>', '?','\0','\0','\0', ' ',
};

//transfer scan code to ascii cap case 
char Ascii_table_cap[ascii_num] = {
    '\0','\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'\b','\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	'{' , '}','\n','\0',
	'A' , 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' ,
	':' ,'\"', '~','\0', '|',
	'Z' , 'X', 'C', 'V', 'B', 'N', 'M',
	'<' , '>', '?','\0','\0','\0', ' ',
};

//transfer scan code to ascii cap and shift case 
char Ascii_table_cap_shift[ascii_num] = {
    '\0','\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b','\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[' ,']' ,'\n','\0',
    'a' , 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';' ,'\'', '`','\0','\\',
    'z' , 'x', 'c', 'v', 'b', 'n', 'm',
	',' , '.', '/','\0','\0','\0', ' ',
};
//initializa keyboard
//input: none
//output: none
void keyboard_init(){
    enable_irq(1);  
}

// check press buttons' situation
// input: scan code
// output: none
// return value: 1 if special buttons are pressed, 0 otherwise
uint8_t check_press(uint8_t scan_code){
    switch(scan_code){
        case Left_Shift_Press:
        case Right_Shift_Press:
            Shift_Pressed = 1;
            return 1;
        case Left_Shift_Release:
        case Right_Shift_Release:
            Shift_Pressed = 0;
            return 1;
        case Ctrl_Press:
            Ctrl_Pressed = 1;
            return 1;
        case Ctrl_Release:
            Ctrl_Pressed = 0;
            return 1;
        case Caps_Press:
            Caps_Pressed = 1;
            if (Cap_status == 1){
                Cap_status= 0;
            }
            else {
                Cap_status = 1;
            }
            return 1;
        case Caps_Release:
            Caps_Pressed = 0;
            return 1;
        case Alt_Press:
            Alt_Pressed = 1;
            return 1;
        case Alt_Release:
            Alt_Pressed = 0;
            return 1;
        default:
            return 0;
    }
}


/*  Handles Keyboard Interrupts */
//input: none
// -initiates interrupt
// -takes keyboard input
// -prints pressed char to console
// -terminates interrupt
//output: none
extern void keyboard_handler(void){
    cli();
    int i ;
    uint8_t scan_code = inb(Keyboard_Dataport);
    if (check_press(scan_code) == 1){               // no opeartion is needed
        send_eoi(1);
        sti();
        return;
    }
    if( scan_code >= ascii_num){                    // out of ascii table's range
        send_eoi(1);
        sti();
        return;
    }
    char ascii ;
    ascii = Ascii_table[scan_code];
    if(Ctrl_Pressed==1){
        if(ascii=='l'){
            clear();
        }
        if (ascii == 'c'){
            SendSig(INTERRUPT);
        }
        send_eoi(1);
        sti();
        return;
    }
    if(Alt_Pressed==1){
        if(scan_code==F1){
            switching_ter(0);
        }else if(scan_code==F2){
            switching_ter(1);
        }else if(scan_code==F3){
            switching_ter(2);
        }
        send_eoi(1);
        sti();
        return;
    }
    if(ascii=='\0'){
        send_eoi(1);
        sti();
        return;
    }

    /* Keyboard input should directly display to screen */
    
    if(ascii=='\n' || ascii =='\r'){
        the_terminal[CurTID].enter_press = 1;	
        the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting++] = ascii;          // for buffer data, the max index is 126, so 127 is vacant for /n
        screen_putc(Ascii_table_shift[scan_code]);

    }else if(ascii=='\b'){                                                  //delete
        if(the_terminal[CurTID].counting>0){
            the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting--] = '\0';
            screen_putc(Ascii_table_shift[scan_code]);
        }
    }else if(ascii=='\t'){
        for(i = 0 ; i < 4; i++){
            if(the_terminal[CurTID].counting < MAX_BUFFER -1 ){
                the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting++] = ' ';
                screen_putc(' ');
            }
        }


    }else if(ascii !='\0' && the_terminal[CurTID].counting < MAX_BUFFER-1){
        if((Shift_Pressed==1)&&(Cap_status==0)){
            screen_putc(Ascii_table_shift[scan_code]);
            the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting++] = Ascii_table_shift[scan_code];
        }else if((Shift_Pressed==1)&&(Cap_status==1)){
            screen_putc(Ascii_table_cap_shift[scan_code]);
            the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting++] = Ascii_table_cap_shift[scan_code];
        }else if((Shift_Pressed==0)&&(Cap_status)==1){
            screen_putc(Ascii_table_cap[scan_code]);
            the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting++] = Ascii_table_cap[scan_code];
        }else if((Cap_status==0)&&(Shift_Pressed==0)){
            screen_putc(Ascii_table[scan_code]);
            the_terminal[CurTID].buffer_data[the_terminal[CurTID].counting++] = Ascii_table[scan_code];
        }
    }
    
    send_eoi(1);
    sti();

}

