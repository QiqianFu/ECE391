<<<<<<< HEAD
## Notes: Used for recording lastest modification
=======
# Notes: Used for recording lastest modification

## Checkpoint 1
Initial GDT: Austin

create gdt_dest at x86_desc.S and use gdt_dest at boot.S. I'm gonna initialize this GDT such that an emulated Intel IA-32 processor can use it

Initialize IDT: Shinan

Devices: Sizhao

Paging: Zaihe
create paging.c, paging.h
may modify something in x86_desc.S (to be determined)

i8259.c: maybe work together at Monday moring

test.c: write a few test command of your own subtask so that we can write the test.c for the whole file quickly


Some notes for the use of github branch: 

first, remember to switch to the branch, git branch check current branch and git checkout name to switch

second, git fetch origin and git merge origin/master is used to update the lastest version

last, if you finish your work, git add, commit, push your branch, then checkout master, git merge your-branch-name to master, git pull origin master, then if no conflict, git push origin master(no need add and commit)

btw, if you want to save your progress, you can always git add, commit, push your branch to gitlab.

>>>>>>> origin/master

Created init_idt() function and idt.h/idt.c files
    - Initializes each entry (256) in the IDT
    - See function/comments for more detail

Created int_wrap.S file for rtc/keyboard interrupt wrapper functions 

Called init_idt in boot.S

Created exception.h/exception.c files for handling exceptions
    - Print message and infinite while loop (acts as a halt)


<<<<<<< HEAD
***Still need to test code and finish interrupt for keyboard and rtc
=======
***Still need to test code and finish interrupt 1. Create file keyboard.c and keyboard.h for keyboard initializing
2. Create function keyboard_init() and keyboard_handler() in keyboard.c
3. Complete i8529_init(), enable_irq(), disable_irq(), send_eoi() 
>>>>>>> origin/master
