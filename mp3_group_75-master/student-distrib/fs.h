#ifndef _FS_H
#define _FS_H

#include "lib.h"
#include "paging.h"
#include "types.h"

#define BLOCKSIZE 4096 /* 4KB per block */
#define FILENAME_LEN 32
#define DENTRY_CNT 63
#define FILETYPE_RTC 0
#define FILETYPE_DIR 1
#define FILETYPE_REG 2

typedef struct Dentry {
    uint8_t FileName[FILENAME_LEN]; /* File name up to 32 characters */
    uint32_t FileType; /* 0: User-level access to RTC, 1: Directory, 2: Regular file */
    uint32_t Inode_num;
    uint8_t Reserved[24]; /* 24 reserved bytes */
} Dentry_t;

typedef struct Inode {
    uint32_t len;
    uint32_t DataBlock_num[(BLOCKSIZE >> 2) - 1];
} Inode_t;

typedef struct DataBlock {
    uint8_t Field[BLOCKSIZE];
} DataBlock_t;

typedef struct BootBlock {
    uint32_t Dentry_cnt;
    uint32_t Inode_cnt;
    uint32_t DataBlock_cnt;
    uint8_t Reserved[52]; /* 52 reserved bytes */
    Dentry_t Dentries[DENTRY_CNT]; /* 63 Dentries, each occupies 64B */
    /* This struct occupies 4KB in total */
} BootBlock_t;

typedef int32_t (*ReadFunc_t) (int32_t fd, void *buf, int32_t nbytes);
typedef int32_t (*WriteFunc_t) (int32_t fd, const void *buf, int32_t nbytes);
typedef int32_t (*OpenFunc_t) (const uint8_t *filename);
typedef int32_t (*CloseFunc_t) (int32_t fd);

typedef struct FileOperation {
    ReadFunc_t Read_ptr;
    WriteFunc_t Write_ptr;
    OpenFunc_t Open_ptr;
    CloseFunc_t Close_ptr;
} FileOperation_t;

typedef struct FileDescriptor {
    FileOperation_t File_op;
    uint32_t File_inode;
    int32_t File_pos;
    int32_t File_flags;
} FileDescriptor_t;

void fs_init(uint32_t boot_start);
int32_t read_dentry_by_name (const uint8_t* fname, Dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, Dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t file_read_byname (const uint8_t *filename, void *buf, int32_t nbytes);
int32_t dir_read_byidx (const uint32_t idx, void *buf);
int32_t file_write_byname (const uint8_t *filename, const void *buf, int32_t nbytes);
int32_t dir_write_byname (const uint8_t *filename, const void *buf, int32_t nbytes);

int32_t file_read (int32_t fd, void *buf, int32_t nbytes);
int32_t dir_read (int32_t fd, void *buf, int32_t nbytes);
int32_t file_write (int32_t fd, const void *buf, int32_t nbytes);
int32_t dir_write (int32_t fd, const void *buf, int32_t nbytes);
int32_t file_open (const uint8_t *filename);
int32_t dir_open (const uint8_t *filename);
int32_t file_close (int32_t fd);
int32_t dir_close (int32_t fd);

#endif
