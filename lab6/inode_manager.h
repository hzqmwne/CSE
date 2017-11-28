// inode layer interface.

#ifndef inode_h
#define inode_h

#include <stdint.h>
#include "extent_protocol.h"

#define DISK_SIZE  1024*1024*16
#define BLOCK_SIZE 512
#define BLOCK_NUM  (DISK_SIZE/BLOCK_SIZE)

typedef uint32_t blockid_t;

// disk layer -----------------------------------------

class disk {
 private:
  unsigned char blocks[BLOCK_NUM][BLOCK_SIZE];

 public:
  disk();
  void read_block(uint32_t id, char *buf);
  void write_block(uint32_t id, const char *buf);
};

// block layer -----------------------------------------

typedef struct superblock {
  uint32_t size;
  uint32_t nblocks;
  uint32_t ninodes;
} superblock_t;

class block_manager {
 private:
  disk *d;
  std::map <uint32_t, int> using_blocks;
 public:
  block_manager();
  struct superblock sb;

  uint32_t alloc_block();
  void free_block(uint32_t id);
  void read_block(uint32_t id, char *buf);
  void write_block(uint32_t id, const char *buf);
};

// inode layer -----------------------------------------

#define INODE_NUM  1024

// Inodes per block.
#define IPB           (BLOCK_SIZE / sizeof(struct inode))

// Block containing inode i
#define IBLOCK(i, nblocks)     ((nblocks)/BPB + (i)/IPB + 3)

// Bitmap bits per block
#define BPB           (BLOCK_SIZE*8)

// Block containing bit for block b
#define BBLOCK(b) ((b)/BPB + 2)

#define NDIRECT 32
#define NINDIRECT (BLOCK_SIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)



/* ========================================================================== */
/* ===================   Log-based Version Control   ======================== */
/* ========================================================================== */

enum InodeOperation {
    CREATE_INODE = 0, WRITE_INODE = 1, REMOVE_INODE = 2, ERROR = -1, EMPTY = -2,
};

static InodeOperation InodeOperationMerge[3][3] = {{ERROR, CREATE_INODE, EMPTY}, {ERROR, WRITE_INODE, REMOVE_INODE}, {WRITE_INODE, ERROR, ERROR}};

struct LogHeader {
    int current_version;
    int max_version;
};

struct CommitHeader {
    int commit_id;
    int entry_count;
};

struct CommittedLogEntry {
    int ino;
    InodeOperation operation;     // create/write/remove
    int old_content_ino;    // for create, it is unused; for write and remove, it is the old ino
    int current_content_backup_ino;    // for step forward
};

struct UncommittedLogEntry {
    int ino;
    InodeOperation operation;     // new/update/delete
};

/* ========================================================================== */
/* === end ===========   Log-based Version Control   ================ end === */
/* ========================================================================== */


typedef struct inode {
  short type;
  unsigned int size;
  unsigned int atime;
  unsigned int mtime;
  unsigned int ctime;
  blockid_t blocks[NDIRECT+1];   // Data block addresses
} inode_t;

class inode_manager {
 private:
  block_manager *bm;
  struct inode* get_inode(uint32_t inum);
  void put_inode(uint32_t inum, struct inode *ino);

    // Log-based Version Control
    int Enable_Log;
    int Log_Start_From_Ino;
    int Committed_Log_Ino;
    int Uncommitted_Log_Ino;
    int Version_Control_Log_Ino;
    uint32_t allocInodeForLog();
    void writeUncommittedLog(int ino, InodeOperation operation);
    void writeVersionControlLog(int ino, InodeOperation operation);
    void undoUncommittedOperations(std::map<int, InodeOperation> &ops);

 public:
  inode_manager();
  uint32_t alloc_inode(uint32_t type);
  void free_inode(uint32_t inum);
  void read_file(uint32_t inum, char **buf, int *size);
  void write_file(uint32_t inum, const char *buf, int size);
  void remove_file(uint32_t inum);
  void getattr(uint32_t inum, extent_protocol::attr &a);

    // Log-based Version Control
    void commit();
    void rollBack();
    void stepForward();
};

#endif

