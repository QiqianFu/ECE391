#include "fs.h"
#include "process.h"

BootBlock_t *BootBlock_ptr;
Inode_t *Inode_ptr;
DataBlock_t *DataBlock_ptr;

extern int32_t CurPID;

/*
    void fs_init (uint32_t boot_start)
    Input: boot_start: work as the pointer for BootBlock_ptr;
    Return value: None;
    Side effect: Initialize BootBlock_ptr, Inode_ptr, DataBlock_ptr;
*/
void fs_init(uint32_t boot_start) {
    BootBlock_ptr = (BootBlock_t *)boot_start;
    Inode_ptr = (Inode_t *)((uint32_t)BootBlock_ptr + BLOCKSIZE);
    DataBlock_ptr = (DataBlock_t *)((uint32_t)Inode_ptr + BootBlock_ptr->Inode_cnt * BLOCKSIZE);
}

/*
    int32_t read_dentry_by_name (const uint8_t *fname, Dentry_t *dentry)
    Input: *fname: name of the file; *dentry: store the result;
    Return value: 0 if success, -1 if fail;
    Side effect: Store the corresponding structure in *dentry;
*/
int32_t read_dentry_by_name (const uint8_t *fname, Dentry_t *dentry) {
    uint32_t i;
    uint8_t *fname2;
    if (BootBlock_ptr == NULL) return -1;
    if (fname == NULL) return -1;
    /* Sanity check */
    
    for (i = 0; i < BootBlock_ptr->Dentry_cnt; i++) {
        fname2 = BootBlock_ptr->Dentries[i].FileName;
        if (fname2 == NULL) continue;

        /* Read the dentry filename, compare if they are the same */
        if (strncmp((int8_t *)fname, (int8_t *)fname2, FILENAME_LEN) == 0) {
            return read_dentry_by_index(i, dentry);
        }
    }
    return -1;
}

/*
    int32_t read_dentry_by_index (uint32_t index, Dentry_t *dentry)
    Input: index: the index of Inode_ptr; *dentry: store the result;
    Return value: 0 if success, -1 if fail;
    Side effect: Store the corresponding structure in *dentry;
*/
int32_t read_dentry_by_index (uint32_t index, Dentry_t *dentry) {
    Dentry_t *Cur_Dentry;
    if (BootBlock_ptr == NULL) return -1;
    if (index >= BootBlock_ptr->Dentry_cnt) return -1;
    /* Sanity check */

    /* Deep copy the fields except for 'Reserved' */
    Cur_Dentry = &BootBlock_ptr->Dentries[index];
    strncpy((int8_t *)dentry->FileName, (int8_t *)Cur_Dentry->FileName, FILENAME_LEN);
    dentry->FileType = Cur_Dentry->FileType;
    dentry->Inode_num = Cur_Dentry->Inode_num;
    return 0;
}

/*
    int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
    Input: inode: the index of Inode_ptr; offset: offset from the start of the data;
           *buf: store the data content; length: bytes need to read;
    Return value: Bytes read if success, -1 if fail;
    Side effect: Fill the buffer with the data content;
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length) {
    int32_t Bytes_read = 0;
    uint32_t i, StartIdx, EndIdx, CurBlock;
    uint32_t l, r;
    if (BootBlock_ptr == NULL) return -1;
    if (inode >= BootBlock_ptr->Inode_cnt) return -1;
    /* Sanity check */

    if (offset >= Inode_ptr[inode].len) return 0;
    if (length + offset > Inode_ptr[inode].len) {
        length = Inode_ptr[inode].len - offset;
    }
    if (length == 0) return 0;
    /* EOF check, adjust length */
    
    /* Start DataBlock index: offset / 4096; End DataBlock index: (offset + length - 1) / 4096 */
    StartIdx = offset / BLOCKSIZE;
    EndIdx = (offset + length - 1) / BLOCKSIZE;
    for (i = StartIdx; i <= EndIdx; i++) {
        CurBlock = Inode_ptr[inode].DataBlock_num[i];
        if (CurBlock >= BootBlock_ptr->DataBlock_cnt) return -1;
        /* Sanity check */

        /* Calculate the specific bytes read in this data block */
        l = 0, r = BLOCKSIZE - 1;
        if (i == StartIdx) l = offset % BLOCKSIZE;
        if (i == EndIdx) r = (offset + length - 1) % BLOCKSIZE;
        if (l > r) { /* Use for debug, which shouldn't happen */
            printf("FATAL ERROR! PROCESS ABORT.\n");
            return -1;
        }
        // printf("%c\n", DataBlock_ptr->Field[CurBlock]);
        memcpy((uint8_t *)(buf + Bytes_read), (uint8_t *)(&DataBlock_ptr[CurBlock].Field[l]), (r - l + 1));
        Bytes_read += (r - l + 1);
    }
    return Bytes_read;
}

/*
    int32_t file_read_byname (const uint8_t *filename, const void *buf, int32_t nbytes)
    Input: *filename: name of the file; *buf: store the data content;
           nbytes: bytes need to read;
    Return value: Bytes read if success, -1 if fail;
    Side effect: Fill the buffer with the data content;
*/
int32_t file_read_byname (const uint8_t *filename, void *buf, int32_t nbytes) {
    Dentry_t Tmp_Dentry;
    if (read_dentry_by_name(filename, &Tmp_Dentry) == -1) return -1;
    if (Tmp_Dentry.FileType != FILETYPE_REG) return -1;
    return read_data(Tmp_Dentry.Inode_num, 0, (uint8_t *)buf, (uint32_t)nbytes);
}

/*
    int32_t dir_read_byidx (const uint32_t idx, const void *buf)
    Input: idx: index of the file; *buf: store the data content;
    Return value: Bytes read if success, -1 if fail;
    Side effect: Fill the buffer with the data content;
*/
int32_t dir_read_byidx (const uint32_t idx, void *buf) {
    uint32_t i, nbytes = 0;
    uint8_t c;
    if (BootBlock_ptr == NULL) return -1;
    if (idx >= BootBlock_ptr->DataBlock_cnt) return -1;
    for (i = 0; i < FILENAME_LEN; i++) {
        c = BootBlock_ptr->Dentries[idx].FileName[i];
        if (c == '\0') break;
        nbytes++;
    }
    if (nbytes > FILENAME_LEN || nbytes == 0) return -1;
    memcpy((uint8_t *)buf, (uint8_t *)(&BootBlock_ptr->Dentries[idx].FileName), nbytes);
    return (int32_t)nbytes;
}

/*
    int32_t file_write_byname (const uint8_t *filename, const void *buf, int32_t nbytes)
    Input: *filename: name of the file; *buf: the data need to be written;
           nbytes: bytes need to write;
    Return value: Bytes written if success, -1 if fail;
    Side effect: Fill the file with buf content;
*/
int32_t file_write_byname (const uint8_t *filename, const void *buf, int32_t nbytes) {
    return -1;
}

/*
    int32_t dir_write_byname (const uint8_t *filename, const void *buf, int32_t nbytes)
    Input: *filename: name of the directory; *buf: the data need to be written;
           nbytes: bytes need to write;
    Return value: Bytes written if success, -1 if fail;
    Side effect: Fill the directory with buf content;
*/
int32_t dir_write_byname (const uint8_t *filename, const void *buf, int32_t nbytes) {
    return -1;
}

/* The above parts are helper functions for checkpoint 2 */

/*
    int32_t file_read (int32_t fd, const void *buf, int32_t nbytes)
    Input: fd: index for the file descriptor; *buf: store the data content;
           nbytes: bytes need to read;
    Return value: Bytes read if success, -1 if fail;
    Side effect: Fill the buffer with the data content;
*/
int32_t file_read (int32_t fd, void *buf, int32_t nbytes) {
    if (fd < 0 || fd >= MAXFILE) return -1;
    FileDescriptor_t Cur_fd = getPCBAddrByPID(CurPID)->fdArray[fd];
    return read_data(Cur_fd.File_inode, Cur_fd.File_pos, (uint8_t *)buf, nbytes);
}

/*
    int32_t dir_read (int32_t fd, const void *buf, int32_t nbytes)
    Input: fd: index for the file descriptor; *buf: store the data content;
           nbytes: bytes need to read;
    Return value: Bytes read if success, -1 if fail;
    Side effect: Fill the buffer with all the file names in the directory;
*/
int32_t dir_read (int32_t fd, void *buf, int32_t nbytes) {
    if (fd < 0 || fd >= MAXFILE) return -1;
    static uint32_t i = 0;
    int32_t res;
    res = dir_read_byidx(i, buf);
    i++;
    if (res == -1){
        i = 0;
        return 0;
    }
    return res;
}

/*
    int32_t file_write (int32_t fd, const void *buf, int32_t nbytes)
    Input: fd: index for the file descriptor; *buf: store the data content;
           nbytes: bytes need to write;
    Return value: Bytes written if success, -1 if fail;
    Side effect: Fill the file with buf content;
*/
int32_t file_write (int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}

/*
    int32_t dir_write (int32_t fd, const void *buf, int32_t nbytes)
    Input: fd: index for the file descriptor; *buf: store the data content;
           nbytes: bytes need to write;
    Return value: Bytes written if success, -1 if fail;
    Side effect: Fill the directory with buf content;
*/
int32_t dir_write (int32_t fd, const void *buf, int32_t nbytes) {
    return -1;
}

/*
    int32_t file_open (const uint8_t *filename)
    Description: try to open a file
    Input: *filename: the name of the directory
    Return value: 0 if success, -1 if fail;
    Side effect: None;
*/
int32_t file_open (const uint8_t *filename) {
    Dentry_t Tmp_Dentry;
    if (read_dentry_by_name(filename, &Tmp_Dentry) == -1) return -1;
    if (Tmp_Dentry.FileType != FILETYPE_REG) return -1;
    return 0;
}

/*
    int32_t dir_open (const uint8_t *filename)
    Description: try to open a directory
    Input: *filename: the name of the directory
    Return value: 0 if success, -1 if fail;
    Side effect: None;
*/
int32_t dir_open (const uint8_t *filename) {
    return 0;
}

/*
    int32_t file_close (int32_t fd)
    Input: fd: index for the file descriptor;
    Return value: 0 if success, -1 if fail;
    Side effect: Close the file with index fd;
*/
int32_t file_close (int32_t fd) {
    if (fd <= 1 || fd >= MAXFILE) return -1;
    /* fd sanity check */
    PCB_t *p = getPCBAddrByPID(CurPID);
    if (p->fdArray[fd].File_flags == 0) return -1;
    /* check if the file was open (close can't success if not even opened) */
    p->fdArray[fd].File_flags = 0;
    return 0;
}

/*
    int32_t dir_close (int32_t fd)
    Input: fd: index for the file descriptor;
    Return value: 0 if success, -1 if fail;
    Side effect: Close the directory(None);
*/
int32_t dir_close (int32_t fd) {
    return 0;
}
