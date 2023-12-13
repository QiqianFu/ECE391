#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "fs.h"
#include "terminal.h"
#include "rtc.h"

#define PASS 1
#define FAIL 0

#define checkpoint1 0
#define checkpoint2 0
#define checkpoint3 1
#define checkpoint4 0
#define checkpoint5 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */


int div_by0(){
	TEST_HEADER;
	int a = 0;
	int b;
	b = 1 / a;
	return FAIL;
}


int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide by 0 test
 * 
 * Test if the div0 exception handler work
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition, IDT initialization, Exception.
 * Files: x86_desc.h/S, idt.c/h, exception.c/h
 */
int divby0_test(){
	TEST_HEADER;
	int a = 0;
	int b;
	b = 1 / a;
	printf("Reaches UNEXPECTED line!!\n");
	return FAIL;
}

/* Page upperbound test
 * 
 * Test if the upper address (greater than 8M, Video Mem + 4KB - 3B)
 * is accessible (which shouldn't be)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition, IDT initialization, Exception, paging.
 * Files: x86_desc.h/S, idt.c/h, exception.c/h, page.c/h
 */
int page_upperbound_test(){
	TEST_HEADER;
	uint32_t *a = (uint32_t *)((KERNEL_ADDR << 1) + 1);
	uint32_t b = *a;
	printf("Reaches UNEXPECTED line1!!");
	a = (uint32_t *)((VMEM_IDX << 12) + ALIGN_4KB - 3);
	b = *a;
	printf("Reaches UNEXPECTED line2!!\n");
	return FAIL;
}

/* Page lowerbound test
 * 
 * Test if the lower addresses (0, 4MB - 1B, Video mem - 1B)
 * is accessible (which shouldn't be)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition, IDT initialization, Exception, paging.
 * Files: x86_desc.h/S, idt.c/h, exception.c/h, page.c/h
 */
int page_lowerbound_test(){
	TEST_HEADER;
	uint32_t *a = (uint32_t *)(KERNEL_ADDR - 1);
	uint32_t b = *a;
	printf("Reaches UNEXPECTED line1!!");
	a = 0;
	b = *a;
	printf("Reaches UNEXPECTED line2!!");
	a = (uint32_t *)((VMEM_IDX << 12) - 1);
	b = *a;
	printf("Reaches UNEXPECTED line3!!\n");
	return FAIL;
}

/* Page important_value test
 * 
 * Test if the important addresses (4MB, 8MB - 4B, Video mem, Video mem + 4KB - 4B) can be accessed
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition, IDT initialization, Exception, paging.
 * Files: x86_desc.h/S, idt.c/h, exception.c/h, paging.c/h
 */
int page_importantval_test(){
	TEST_HEADER;
	uint32_t *a = (uint32_t *)(KERNEL_ADDR);
	uint32_t b = *a;
	printf("Reach4MB, ");
	a = (uint32_t *)((KERNEL_ADDR << 1) - 4);
	b = *a;
	printf("Reach8MB, ");
	a = (uint32_t *)(VMEM_IDX << 12);
	b = *a;
	printf("Reach start of Video mem, ");
	a = (uint32_t *)((VMEM_IDX << 12) + ALIGN_4KB - 4);
	b = *a;
	printf("Reach end of Video mem, Success!\n");
	return PASS;
}

// add more tests here

/* Checkpoint 2 tests */

int test_ls() {
	TEST_HEADER;
	extern BootBlock_t *BootBlock_ptr;
	extern Inode_t *Inode_ptr;
	int32_t i, j;
	for (i = 0; i < BootBlock_ptr->Dentry_cnt; i++){
		Dentry_t d = BootBlock_ptr->Dentries[i];
		printf("filename:");
		for (j = 0; j < 32; j++) {
			if (d.FileName[j] != '\0') printf("%c", (int8_t)d.FileName[j]);
			else printf(" ");
		}
		printf("  filetype: %u  filesize: %u\n", d.FileType, Inode_ptr[d.Inode_num].len);
	}
	return PASS;
}

/* Read regular file test (small size 512bytes)
 * 
 * Test if the regular files can be read
 * Inputs: *filename: name of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), read_data(), file_read_byname(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_read_file_ss(uint8_t *filename){
	TEST_HEADER;
	uint8_t buff[514];
	if (file_read_byname(filename, buff, 512) == -1) return FAIL;
	printf("%s\n", buff);
	printf("filename = %s\n", filename);
	return PASS;
}

/* Read regular file test (big size 5120bytes)
 * 
 * Test if the regular files can be read
 * Inputs: *filename: name of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), read_data(), file_read_byname(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_read_file_bs(uint8_t *filename){
	TEST_HEADER;
	uint8_t buff[5280];
	if (file_read_byname(filename, buff, 5277) == -1) return FAIL;
	printf("%s\n", buff);
	printf("filename = %s\n", filename);
	return PASS;
}

/* Read executable test
 * 
 * Test if the executable files can be read
 * Inputs: *filename: name of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), read_data(), file_read_byname(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_read_executable(uint8_t *filename){
	TEST_HEADER;
	int32_t i;
	uint8_t buff[6330];
	if (file_read_byname(filename, buff, 6327) == -1) return FAIL;
	for (i = 0; i < 6327; i++) {
		if (buff[i] == '\0') continue;
		printf("%c", buff[i]);
	}
	printf("\n");
	return PASS;
}

/* Read directory test
 * 
 * Test if the regular directory can be read
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, directory_read_byname(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_read_dir(){
	TEST_HEADER;
	uint8_t buff[FILENAME_LEN];
	uint32_t i, j;
	extern BootBlock_t *BootBlock_ptr;
	extern Inode_t *Inode_ptr;
	for (i = 0; i < DENTRY_CNT; i++) {
		for (j = 0; j < FILENAME_LEN; j++) buff[j] = ' ';
		if (dir_read_byidx(i, buff) == -1) break;
		Dentry_t d = BootBlock_ptr->Dentries[i];
		printf("filename:");
		for (j = 0; j < 32; j++) {
			if (buff[j] != '\0') printf("%c", (int8_t)buff[j]);
			else printf(" ");
		}
		printf("  filetype: %u  filesize: %u\n", d.FileType, Inode_ptr[d.Inode_num].len);
	}
	return PASS;
}

/* Write regular file test
 * 
 * Test if the regular files can be written
 * Inputs: *filename: name of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, file_write_byname(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_write_file(uint8_t *filename){
	TEST_HEADER;
	uint8_t buff[8];
	buff[0] = buff[1] = buff[2] = 'w';
	if (file_write_byname(filename, buff, 3) != -1) {
		printf("Error, Read only but write successfully!\n");
		return FAIL;
	}
	printf("Not able to write, success!\n");
	return PASS;
}

/* Write regular directory test
 * 
 * Test if the regular directories can be written
 * Inputs: *filename: name of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, dir_write_byname(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_write_dir(uint8_t *filename){
	TEST_HEADER;
	uint8_t buff[8];
	buff[0] = buff[1] = buff[2] = 'w';
	if (dir_write_byname(filename, buff, 3) != -1) {
		printf("Error, Read only but write successfully!\n");
		return FAIL;
	}
	printf("Not able to write, success!\n");
	return PASS;
}

/* Open regular file test
 * 
 * Test if the regular files can be opened
 * Inputs: *filename: name of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), read_data(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_open_file(uint8_t *filename){
	TEST_HEADER;
	if (file_open(filename) == -1) {
		printf("No such file!\n");
		return FAIL;
	}
	printf("Open file [%s] successfully!\n", filename);
	return PASS;
}

/* Open directory test
 * 
 * Test if the directories can be opened
 * Inputs: *filename: name of the directory
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), dir_open(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_open_dir(uint8_t *filename){
	TEST_HEADER;
	if (dir_open(filename) == -1) {
		printf("No such directory!\n");
		return FAIL;
	}
	printf("Open directory [%s] successfully!\n", filename);
	return PASS;
}

/* Close file test
 * 
 * Test if the files can be closed
 * Inputs: fd: file descriptor index of the file
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), file_close(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_close_file(int32_t fd){
	TEST_HEADER;
	if (file_close(fd) == -1) {
		printf("Invalid file to close!\n");
		return FAIL;
	}
	printf("Close file with fd index %d successfully!\n", fd);
	return PASS;
}

/* Close directory test
 * 
 * Test if the directories can be closed
 * Inputs: fd: file descriptor index of the directory
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: fs initialize, read_dentry_by_name/index(), dir_close(), paging;
 * Files: fs.c/h, paging.c/h, kernel.c
 */
int test_close_dir(int32_t fd){
	TEST_HEADER;
	if (dir_close(fd) == -1) {
		printf("Invalid directory to close!\n");
		return FAIL;
	}
	printf("Close directory with fd index %d successfully!\n", fd);
	return PASS;
}

/* terminal_test_read_and_write
 * Input: the number of read_nbytes and write_nbytes
 * Output: write the character to the screen when enter is pressed 
 */
int terminal_test_read_and_write(int32_t read_nbytes, int32_t write_nbytes) {
	TEST_HEADER;
	int32_t readed_bytes, wrotten_bytes;
	while(1){
		uint8_t buf[128] = {'\0'};
		readed_bytes = ter_read(0, buf, read_nbytes);
		wrotten_bytes = ter_write(0, buf, write_nbytes);
		printf("Read %d bytes, and wrote %d bytes\n", readed_bytes, wrotten_bytes);
	}

	return PASS;
}
/* different_sized_strings_to_the_terminal
 * Input: the number of read_nbytes and write_nbytes
 * Output: write the character to the screen when enter is pressed 
 */
int different_sized_strings_to_the_terminal(int32_t write_nbytes){
	TEST_HEADER;
	int32_t wrotten_bytes;
	uint8_t buf[10] = {'1','2','3','4','5','6','7','8','9','\n'};
	wrotten_bytes = ter_write(0,buf,write_nbytes);
	printf("the required written bytes is %d, the lenth of buffer is %d\n",write_nbytes,wrotten_bytes);
	return PASS;
}
/* test_rtc
 * Input: None
 * Output: PASS
 * Function: Initialize RTC, set frequency
 */
int test_rtc(){
	TEST_HEADER;

	init_rtc();
	setfreq_rtc(4);

	return PASS;
}

/* test_read_rtc
 * Input: None
 * Output: PASS
 * Function: Test reading from RTC (not stuck in loop)
 */
int test_read_rtc(){
	TEST_HEADER;

	init_rtc();
	open_rtc(NULL);
	printf("wait for tick");
	read_rtc(NULL, NULL, NULL);
	printf("\nTick is coming\n");
	close_rtc(NULL);


	return PASS;
}

/* test_rw_rtc
 * Input: None
 * Output: PASS
 * Function: Test read/write, showcase each frequency
 */
int test_rw_rtc(){
	TEST_HEADER;
	uint32_t i, j;
	
	init_rtc();
	open_rtc(NULL);

	for (i = 2; i <= 1024; i *= 2){
		write_rtc(NULL, &i, sizeof(uint32_t));
		printf("Test: %d Hz [", i);
		for (j = 0; j < i; j++){
			read_rtc(NULL, NULL, NULL);
			printf("1");
		}
		printf("]\n");
	}

	return PASS;
}
/* Checkpoint 3 tests */

int test_sysread_dir() {
	uint8_t buff[FILENAME_LEN * DENTRY_CNT];
	int32_t res = dir_read(0, buff, 0);
	if (res == -1) return FAIL;
	printf("bytes read = %d\n%s\n", res, buff);
	return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	#if (checkpoint1)
		//TEST_OUTPUT("page_importantval_test", page_importantval_test());
		// TEST_OUTPUT("page_lowerbound_test", page_lowerbound_test());
		// TEST_OUTPUT("page_upperbound_test", page_upperbound_test());
		// TEST_OUTPUT("divby0_test", divby0_test());
		// TEST_OUTPUT("idt_test", idt_test());
	#endif

	#if (checkpoint2)
		//TEST_OUTPUT("test_rtc", test_rtc());
		//TEST_OUTPUT("test_read_rtc", test_read_rtc());
		// TEST_OUTPUT("test_rw_rtc", test_rw_rtc());
		// TEST_OUTPUT("test_read_file_ss", test_read_file_ss((uint8_t *)"frame0.txt"));
		// TEST_OUTPUT("test_read_executable", test_read_executable((uint8_t *)"hello"));
		// TEST_OUTPUT("test_read_file_bs", test_read_file_bs((uint8_t *)"verylargetextwithverylongname.tx"));
		// TEST_OUTPUT("test_read_file_bs", test_read_file_bs((uint8_t *)"verylargetextwithverylongname.txt"));
		TEST_OUTPUT("test_read_directory", test_read_dir());
		// TEST_OUTPUT("test_write_file", test_write_file((uint8_t *)"frame1.txt"));
		// TEST_OUTPUT("test_write_dir", test_write_dir((uint8_t *)"."));
		// TEST_OUTPUT("test_open_file", test_open_file((uint8_t *)"created.txt"));
		// TEST_OUTPUT("test_open_file", test_open_file((uint8_t *)"verylargetextwithverylongname.txt"));
		// TEST_OUTPUT("test_open_dir", test_open_dir((uint8_t *)"."));
		// TEST_OUTPUT("test_close_file", test_close_file(2));
		// TEST_OUTPUT("test_close_dir", test_close_dir(2));
		// TEST_OUTPUT("terminal_test_read_and_write", terminal_test_read_and_write(128, 128));
		// TEST_OUTPUT("different_sized_strings_to_the_terminal", different_sized_strings_to_the_terminal(10));
	#endif

	#if (checkpoint3)
		// TEST_OUTPUT("test_sysread_dir", test_sysread_dir());
	#endif

	#if (checkpoint4)

	#endif

	#if (checkpoint5)

	#endif
	// launch your tests here
}
