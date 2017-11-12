#include "inode_manager.h"
#include <time.h>

// disk layer -----------------------------------------

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf)
{
  /*
   *your lab1 code goes here.
   *if id is smaller than 0 or larger than BLOCK_NUM 
   *or buf is null, just return.
   *put the content of target block into buf.
   *hint: use memcpy
  */
  if(id < 0 || id >= BLOCK_NUM || buf == NULL) {
  	return;
  }
  memcpy(buf, this->blocks[id], BLOCK_SIZE);
}

void
disk::write_block(blockid_t id, const char *buf)
{
  /*
   *your lab1 code goes here.
   *hint: just like read_block
  */
  if(id < 0 || id >= BLOCK_NUM || buf == NULL) {
  	return;
  }
  memcpy(this->blocks[id], buf, BLOCK_SIZE);
}

// block layer -----------------------------------------

// Allocate a free disk block.
blockid_t
block_manager::alloc_block()
{
  /*
   * your lab1 code goes here.
   * note: you should mark the corresponding bit in block bitmap when alloc.
   * you need to think about which block you can start to be allocated.

   *hint: use macro IBLOCK and BBLOCK.
          use bit operation.
          remind yourself of the layout of disk.
   */
  char *buf = (char *)malloc(BLOCK_SIZE);
  int bits = BLOCK_NUM;
  for(int i = 0; i < bits / BPB; ++i) {
	d->read_block(2 + i, buf);
	for(int j = 0; j < BLOCK_SIZE; ++j) {
		char tmp = buf[j];
		if(tmp != -1) {
			int offset = 0;
			while(((tmp>>(7-offset))&1) != 0) {
				++offset;
			}
			tmp |= 1 << (7-offset);
			buf[j] = tmp;
			d->write_block(2 + i, buf);
			int block_num = BPB * i + 8 * j + offset;
			free(buf);
			return block_num;
		}
	}
  }
  int rest = bits % BPB;
  if(rest > 0) {	
	int i = bits / BPB;
	d->read_block(2 + i, buf);
	for(int j = 0; j < rest / 8; ++j) {
		char tmp = buf[j];
		if(tmp != -1) {
			int offset = 0;
			while(((tmp>>(7-offset))&1) != 0) {
				++offset;
			}
			tmp |= 1 << (7-offset);
			buf[j] = tmp;
			d->write_block(2 + i, buf);
			int block_num = BPB * i + 8 * j + offset;
			free(buf);
			return block_num;
		}
	}

	char tmp = buf[rest / 8];
	if(tmp != -1) {
		int offset = 0;
		while(((tmp>>(7-offset))&1) != 0) {
			++offset;
		}
		tmp |= 1 << (7-offset);
		buf[rest / 8] = tmp;
		d->write_block(2 + i, buf);
		int block_num = BPB * i + 8 * (rest / 8) + offset;
		free(buf);
		return block_num;
	}
  }
  free(buf);
  return 0;
}

void
block_manager::free_block(uint32_t id)
{
  /* 
   * your lab1 code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
  int bit_block = BBLOCK(id);
  char *buf = (char *)malloc(BLOCK_SIZE);
  d->read_block(bit_block, buf);
  int index = (id % BPB) / 8;
  int offset = (id % BPB) % 8;
  buf[index] &= ~(1 << (7-offset));
  d->write_block(bit_block, buf);
  free(buf);
}

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager()
{
  d = new disk();

  // format the disk
  sb.size = BLOCK_SIZE * BLOCK_NUM;
  sb.nblocks = BLOCK_NUM;
  sb.ninodes = INODE_NUM;

  // ??????????????????
  // set block bitmap to 1 for boot block, super block, bitmap block, inode block
  int bits = IBLOCK(INODE_NUM, BLOCK_NUM) + 1;
  for(int i = 0; i < bits / BPB; ++i) {
    char *buf = (char *)malloc(BLOCK_SIZE);
  	memset(buf, -1, BLOCK_SIZE);
	d->write_block(2 + i, buf);
	free(buf);
  }
  int rest = bits % BPB;
  if(bits % BPB > 0) {	
    char *buf = (char *)malloc(BLOCK_SIZE);
	memset(buf, -1, rest / 8);
	char tmp = 0;
    for(int j = 0; j < rest % 8; ++j) {
      tmp |= 1 << (7 - j);
    }
	buf[rest / 8] = tmp;
	d->write_block(2 + bits / BPB, buf);
	free(buf);
  }
  
}

void
block_manager::read_block(uint32_t id, char *buf)
{
  d->read_block(id, buf);
}

void
block_manager::write_block(uint32_t id, const char *buf)
{
  d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager()
{
  bm = new block_manager();
    this->Enable_Log = 1;
    if(this->Enable_Log) {
        this->Log_Start_From_Ino = 900;
        this->Committed_Log_Ino = this->allocInodeForLog();
        this->Uncommitted_Log_Ino = this->allocInodeForLog();
        this->Version_Control_Log_Ino = this->allocInodeForLog();
        char *tmp = (char *)calloc(1, sizeof(LogHeader));
        this->Enable_Log = 0;
        this->write_file(Committed_Log_Ino, tmp, sizeof(LogHeader));
        this->Enable_Log = 1;
    }
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1) {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
    //////
    char *tmp = (char *)malloc((8+64)*3);
    *((unsigned long long *)tmp) = this->Committed_Log_Ino;
    strcpy(tmp + 8, "Committed_Log_Ino");
    *((unsigned long long *)(tmp+8+64)) = this->Uncommitted_Log_Ino;
    strcpy(tmp + 8+64+8, "Uncommitted_Log_Ino");
    *((unsigned long long *)(tmp+8+64+8+64)) = this->Version_Control_Log_Ino;
    strcpy(tmp + 8+64+8+64+8, "Version_Control_Log_Ino");
    this->write_file(root_dir, tmp, (8+64)*3);
    free(tmp);
    //////
    printf("=====debug===== init commmit start\n");
    this->commit();
    printf("=====debug===== init commmit end\n");
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type)
{
  /* 
   * your lab1 code goes here.
   * note: the normal inode block should begin from the 2nd inode block.
   * the 1st is used for root_dir, see inode_manager::inode_manager().
    
   * if you get some heap memory, do not forget to free it.
   */
    char *buf = (char *)malloc(BLOCK_SIZE);
    int i = 0;
    for(i = 0; i < (int)((INODE_NUM + IPB - 1) / IPB); ++i) {
        int block_index = IBLOCK(0, BLOCK_NUM) + i;
        bm->read_block(block_index, buf);
        for(int j = 0; j < (int)IPB; ++j) {
            int current_ino = i * IPB + j;
            if(current_ino >= INODE_NUM) {
                return -1;    // can't find free inode
            }
            if(current_ino == 0) {
                continue;    // inode number 0 never used
            }
            struct inode *node = (struct inode *)buf + j;
            if(node->type == 0) {    // empty inode
                node->type = type;
                node->ctime = time(0);
                node->size = 0;
                bm->write_block(block_index, buf);
                free(buf);
                if(this->Enable_Log) {
                    if(current_ino < this->Log_Start_From_Ino) {
                        writeUncommittedLog(current_ino, CREATE_INODE);
                    }
                }
                return current_ino;
            }
        }
    }
    return -1;
}

void
inode_manager::free_inode(uint32_t inum)    // need to free 
{
  /* 
   * your lab1 code goes here.
   * note: you need to check if the inode is already a freed one;
   * if not, clear it, and remember to write back to disk.
   * do not forget to free memory if necessary.
   */
  int block_index = IBLOCK(inum, BLOCK_NUM);
  char *buf = (char *)malloc(BLOCK_SIZE);
  bm->read_block(block_index, buf);
  struct inode *node = (struct inode *)buf + inum % IPB;
  if(node->type != 0) {
  	memset((char *)node, 0, sizeof(struct inode));
	bm->write_block(block_index, buf);
  }
  free(buf);
}


/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode* 
inode_manager::get_inode(uint32_t inum)
{
  struct inode *ino, *ino_disk;
  char buf[BLOCK_SIZE];

  printf("\tim: get_inode %d\n", inum);

  if (inum < 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return NULL;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);

  ino_disk = (struct inode*)buf + inum%IPB;
  if (ino_disk->type == 0) {
    printf("\tim: inode not exist\n");
    return NULL;
  }

  ino = (struct inode*)malloc(sizeof(struct inode));
  *ino = *ino_disk;

  return ino;
}

void
inode_manager::put_inode(uint32_t inum, struct inode *ino)
{
  char buf[BLOCK_SIZE];
  struct inode *ino_disk;

  printf("\tim: put_inode %d\n", inum);
  if (ino == NULL)
    return;

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  ino_disk = (struct inode*)buf + inum%IPB;
  *ino_disk = *ino;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_manager::read_file(uint32_t inum, char **buf_out, int *size)
{
  /*
   * your lab1 code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_out
   */
  struct inode *node = this->get_inode(inum);
  *size = node->size;
  unsigned int block_count = (*size + BLOCK_SIZE - 1) / BLOCK_SIZE;
  char *buf = (char *)malloc(block_count * BLOCK_SIZE);
  if(block_count <= NDIRECT) {
    for(unsigned int i = 0; i < block_count; ++i) {
      int block_index = node->blocks[i];
      bm->read_block(block_index, buf + i*BLOCK_SIZE);
    }
  }
  else if(block_count <= NDIRECT + NINDIRECT) {
    for(int i = 0; i < NDIRECT; ++i) {
      int block_index = node->blocks[i];
      bm->read_block(block_index, buf + i*BLOCK_SIZE);
    }
	blockid_t *indriect_block = (blockid_t *)malloc(BLOCK_SIZE);
	bm->read_block(node->blocks[NDIRECT], (char *)indriect_block);
	for(unsigned int i = 0; i < block_count - NDIRECT; ++i) {
      bm->read_block(indriect_block[i], buf + (NDIRECT+i)*BLOCK_SIZE);
	}
	free(indriect_block);
  }
  else {
    // file is too large !
  }
  *buf_out = buf;
  free(node);
}

/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf, int size)
{
  /*
   * your lab1 code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode.
   * you should free some blocks if necessary.
   */
  struct inode *node = this->get_inode(inum);
  unsigned int old_block_count = (node->size + BLOCK_SIZE - 1) / BLOCK_SIZE;
  unsigned int new_block_count = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
  if(new_block_count > NDIRECT + NINDIRECT) {
    printf("================debug write_file : file is too large\n");
    // file is too large !
  }
  blockid_t *indriect_block = NULL;
  if(old_block_count < new_block_count) {
    if(new_block_count <= NDIRECT) {
  	  for(unsigned int i = old_block_count; i < new_block_count; ++i) {
        node->blocks[i] = bm->alloc_block();
	  }
	}
	else if(old_block_count > NDIRECT) {
	  indriect_block = (blockid_t *)malloc(BLOCK_SIZE);
      bm->read_block(node->blocks[NDIRECT], (char *)indriect_block);
	  for(unsigned int i = old_block_count - NDIRECT; i < new_block_count - NDIRECT; ++i) {
        indriect_block[i] = bm->alloc_block();
	  }
	  bm->write_block(node->blocks[NDIRECT], (char *)indriect_block);
	}
	else {
	  for(int i = old_block_count; i < NDIRECT; ++i) {
        node->blocks[i] = bm->alloc_block();
	  }
      indriect_block = (blockid_t *)malloc(BLOCK_SIZE);
	  for(unsigned int i = 0; i < new_block_count - NDIRECT; ++i) {
        indriect_block[i] = bm->alloc_block();
	  }
	  blockid_t indriect_id = bm->alloc_block();
	  node->blocks[NDIRECT] = indriect_id;
	  bm->write_block(node->blocks[NDIRECT], (char *)indriect_block);
	}
  }
  else {
    if(old_block_count <= NDIRECT) {
      for(unsigned int i = new_block_count; i < old_block_count; ++i) {
	    bm->free_block(node->blocks[i]);
      }
	}
    else if(new_block_count > NDIRECT) {
	  indriect_block = (blockid_t *)malloc(BLOCK_SIZE);
      bm->read_block(node->blocks[NDIRECT], (char *)indriect_block);
	  for(unsigned int i = new_block_count - NDIRECT; i < old_block_count - NDIRECT; ++i) {
        bm->free_block(indriect_block[i]);
	  }
    }
    else {
	  indriect_block = (blockid_t *)malloc(BLOCK_SIZE);
      bm->read_block(node->blocks[NDIRECT], (char *)indriect_block);
	  for(unsigned int i = 0; i < old_block_count - NDIRECT; ++i) {
        bm->free_block(indriect_block[i]);
	  }
	  for(int i = new_block_count; i < NDIRECT + 1; ++i) {
        bm->free_block(node->blocks[i]);
	  }
    }
  }

  printf("========debug write_file ino: %d size:%d",inum, size);
  if(new_block_count <= NDIRECT) {
    unsigned int i = 0;
    for(i = 0; i + 1 < new_block_count; ++i) {
      bm->write_block(node->blocks[i], buf + i*BLOCK_SIZE);
printf(" %d ", node->blocks[i]);
    }
    char *buf_block = (char *)malloc(BLOCK_SIZE);
    memcpy(buf_block, buf + i*BLOCK_SIZE, (size+BLOCK_SIZE-1) % BLOCK_SIZE + 1);
    bm->write_block(node->blocks[i], buf_block);
printf(" %d |i:%d| \n",node->blocks[i], i);
    free(buf_block);
  }
  else {
    unsigned int i = 0;
    for(i = 0; i < NDIRECT; ++i) {
      bm->write_block(node->blocks[i], buf + i*BLOCK_SIZE);
    }
  	for(i = 0; i + 1 < new_block_count - NDIRECT; ++i) {
      bm->write_block(indriect_block[i], buf + (i+NDIRECT)*BLOCK_SIZE);
    }
    char *buf_block = (char *)malloc(BLOCK_SIZE);
    memcpy(buf_block, buf + (i+NDIRECT)*BLOCK_SIZE, (size+BLOCK_SIZE-1) % BLOCK_SIZE + 1);
    bm->write_block(indriect_block[i], buf_block);
    free(buf_block);
  }
  
  node->size = size;
  node->ctime = node->mtime = time(0);
  put_inode(inum, node);
  free(node);
  if(indriect_block != NULL) {
    free(indriect_block);
  }
    if(this->Enable_Log) {
        if(inum < this->Log_Start_From_Ino) {
            writeUncommittedLog(inum, WRITE_INODE);
        }
    }
}

void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a)
{
  /*
   * your lab1 code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */
  struct inode *attr = get_inode(inum);
  if(attr != NULL) {
    a.type = attr->type;
    a.atime = attr->atime;
    a.mtime = attr->mtime;
    a.ctime = attr->ctime;
    a.size = attr->size;
  }
  else {
    a.type = 0;
    a.atime = 0;
    a.mtime = 0;
    a.ctime = 0;
    a.size = 0;
  }
  free(attr);
}

void
inode_manager::remove_file(uint32_t inum)
{
  /*
   * your lab1 code goes here
   * note: you need to consider about both the data block and inode of the file
   * do not forget to free memory if necessary.
   */
  struct inode *node = get_inode(inum);
  int old_block_count = (node->size + BLOCK_SIZE - 1) / BLOCK_SIZE;
  if(old_block_count <= NDIRECT) {
    for(int i = 0; i < old_block_count; ++i) {
  	  bm->free_block(node->blocks[i]);
    }
  }
  else {
    blockid_t *indriect_block = (blockid_t *)malloc(BLOCK_SIZE);
    bm->read_block(node->blocks[NDIRECT], (char *)indriect_block);
    for(int i = 0; i < old_block_count - NDIRECT; ++i) {
      bm->free_block(indriect_block[i]);
	}
	for(int i = 0; i < NDIRECT + 1; ++i) {
      bm->free_block(node->blocks[i]);
    }
  }
  memset(node, 0, sizeof(struct inode));
  put_inode(inum, node);
  free(node);
    if(this->Enable_Log) {
        if(inum < this->Log_Start_From_Ino) {
            writeUncommittedLog(inum, REMOVE_INODE);
        }
    }
}


/* ========================================================================== */
/* ===================   Log-based Version Control   ======================== */
/* ========================================================================== */

uint32_t inode_manager::allocInodeForLog() {
    char *buf = (char *)malloc(BLOCK_SIZE);
    int i;
    for(i = this->Log_Start_From_Ino / IPB; i < (int)((INODE_NUM + IPB - 1) / IPB); ++i) {
        int block_index = IBLOCK(0, BLOCK_NUM) + i;
        bm->read_block(block_index, buf);
        for(int j = 0; j < (int)IPB; ++j) {
            int current_ino = i * IPB + j;
            if(current_ino >= INODE_NUM) {
                return -1;    // can't find free inode
            }
            if(current_ino == 0) {
                continue;    // inode number 0 never used
            }
            struct inode *node = (struct inode *)buf + j;
            if(node->type == 0) {    // empty inode
                node->type = 2;    // T_FILE
                node->ctime = time(0);
                node->size = 0;
                bm->write_block(block_index, buf);
                free(buf);
                return current_ino;
            }
        }
    }
    return -1;
}

void inode_manager::writeUncommittedLog(int ino, InodeOperation operation) {
    int size;
    char *buf;
    this->read_file(this->Uncommitted_Log_Ino, &buf, &size);
    int new_size = size + sizeof(UncommittedLogEntry);
    buf = (char *)realloc(buf, new_size);
    UncommittedLogEntry *tmp = (UncommittedLogEntry *)(buf + size);
    tmp->ino = ino;
    tmp->operation = operation;
    this->write_file(this->Uncommitted_Log_Ino, buf, new_size);
    free(buf);
    printf("=====debug===== writeUncommittedLog ino:%d operation:%d size:%d\n", ino, operation, new_size);
}

void inode_manager::writeVersionControlLog(int ino, InodeOperation operation) {
    int size;
    char *buf;
    this->read_file(this->Version_Control_Log_Ino, &buf, &size);
    int new_size = size + sizeof(UncommittedLogEntry);
    buf = (char *)realloc(buf, new_size);
    UncommittedLogEntry *tmp = (UncommittedLogEntry *)(buf + size);
    tmp->ino = ino;
    tmp->operation = operation;
    this->write_file(this->Version_Control_Log_Ino, buf, new_size);
    free(buf);
}

static uint32_t getOldContentIno(int ino, char *committedLogFile, int *commitOffset, int count) {
    for(int j = count - 1; j >= 0; --j) {
        CommitHeader *ch = (CommitHeader *)(committedLogFile + commitOffset[j]);
        CommittedLogEntry *entry = (CommittedLogEntry *)(ch + 1);
        for(int i = 0; i < ch->entry_count; ++i) {
            if(entry[i].ino == ino) {
                switch(entry[i].operation) {
                    case CREATE_INODE: case WRITE_INODE: {
                        printf("=====debug===== getOldContentIno commit_id:%d return:%d\n", ch->commit_id,entry[i].current_content_backup_ino);
                        return entry[i].current_content_backup_ino;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

static void mergeOperations(UncommittedLogEntry *entry, int count, std::map<int, InodeOperation> &ops) {
    for(int i = 0; i < count; ++i) {
        std::map<int, InodeOperation>::iterator it = ops.find(entry[i].ino);
        if(it != ops.end()) {
            InodeOperation merged = InodeOperationMerge[it->second][entry[i].operation];
            switch(merged) {
                case CREATE_INODE: case WRITE_INODE: case REMOVE_INODE: {
                    it->second = merged;
                    break;
                }
                case EMPTY: {
                    ops.erase(it);
                    break;
                }
                case ERROR: {
                    printf("=====debug===== mergeOperations abort %d %d\n", it->first, entry[i].operation);
                    abort();    ////
                    break;
                }
                default: {
                    abort();    ////
                    break;
                }
            }
        }
        else {
            ops[entry[i].ino] = entry[i].operation;
        }
    }
}

void inode_manager::undoUncommittedOperations(std::map<int, InodeOperation> &ops) {
    int size;
    char *buf;
    this->read_file(this->Committed_Log_Ino, &buf, &size);
    LogHeader *lh = (LogHeader *)buf;

    int earlierCommitCount = lh->current_version;
    int *earlierCommitOffset = NULL;

    printf("=====debug===== undoUncommittedOperations lh->current_version:%d \n", lh->current_version);
    if(earlierCommitCount > 0) {
        earlierCommitOffset = (int *)malloc(earlierCommitCount * sizeof(int));
        CommitHeader *ch = (CommitHeader *)(buf + sizeof(LogHeader));
        for(int i = 0; i < earlierCommitCount; ++i) {
            printf("=====debug===== undoUncommittedOperations ch->commit_id:%d \n", ch->commit_id);
            earlierCommitOffset[i] = (char *)ch - buf;
            ch = (CommitHeader *)((char *)ch + sizeof(CommitHeader) + ch->entry_count * sizeof(CommittedLogEntry));
        }
        printf("=====debug===== undoUncommittedOperations ch->commit_id:%d \n", ch->commit_id);
    }

    this->Enable_Log = 0;
    std::map<int, InodeOperation>::iterator it;
    for(it = ops.begin(); it != ops.end(); it++) {
        printf("=====debug===== undoUncommittedOperations it->first:%d it->second:%d\n", it->first, it->second);
        switch(it->second) {
            int old_content_ino;
            case CREATE_INODE: {
                this->remove_file(it->first);
                break;
            }
            case REMOVE_INODE: case WRITE_INODE: {
                old_content_ino = getOldContentIno(it->first, buf, earlierCommitOffset, earlierCommitCount);
                printf("=====debug===== old_content_ino:%d\n", old_content_ino);
                struct inode *file_inode = this->get_inode(old_content_ino);
                file_inode->size = 0;    // delete block alloc infomation !!!
                this->put_inode(it->first, file_inode);    // equal to create
                free(file_inode);
                int size2;
                char *buf2;
                this->read_file(old_content_ino, &buf2, &size2);
                printf("=====debug===== undouncommit old_ino:%d size:%d content:%s\n", old_content_ino, size2, buf2);
                this->write_file(it->first, buf2, size2);
                if(it->first == 1) {
                    printf("=====debug===== content ino:%d size:%d old_ino:%d entry2:ino:%d name:%s\n", it->first, size2, old_content_ino, *(unsigned long long *)(buf2+8+64), buf2+8+64+8);
                }
                free(buf2);
                break;
            }
        }
    }
    this->Enable_Log = 1;
}

void inode_manager::commit() {
    printf("=====debug===== commit start\n");
    if(this->Enable_Log == 0) {
        return;
    }
    struct inode *node = this->get_inode(this->Uncommitted_Log_Ino);
    printf("=====debug===== commit node->size is %d\n", node->size);
    if(node->size == 0) {
        free(node);
        return;
    }
    free(node);

    this->Enable_Log = 0;
    std::map<int, InodeOperation> ops;
    int size;
    char *buf;
    this->read_file(this->Version_Control_Log_Ino, &buf, &size);
    UncommittedLogEntry *tmp = (UncommittedLogEntry *)buf;
    mergeOperations(tmp, size/sizeof(UncommittedLogEntry), ops);    // ops will be modified
    this->write_file(this->Version_Control_Log_Ino, "", 0);
    free(buf);
    printf("=====debug===== begin read_file uncommitted_log\n");
    this->read_file(this->Uncommitted_Log_Ino, &buf, &size);
    printf("=====debug===== end read_file uncommitted_log\n");
    tmp = (UncommittedLogEntry *)buf;
    mergeOperations(tmp, size/sizeof(UncommittedLogEntry), ops);    // ops will be modified
    this->write_file(this->Uncommitted_Log_Ino, "", 0);
    free(buf);

    this->read_file(this->Committed_Log_Ino, &buf, &size);
    int new_size = size + sizeof(CommitHeader) + ops.size() * sizeof(CommittedLogEntry);
    buf = (char *)realloc(buf, new_size);
    LogHeader *lh = (LogHeader *)buf;
    CommitHeader *newCommitHeader = (CommitHeader *)(buf + size);
    CommittedLogEntry *tmp2 = (CommittedLogEntry *)(buf + size + sizeof(CommitHeader));
    printf("=====debug===== new commitheader\n");

    int earlierCommitCount = lh->max_version;
    int *earlierCommitOffset = NULL;
    if(earlierCommitCount > 0) {
        earlierCommitOffset = (int *)malloc(earlierCommitCount * sizeof(int));
        CommitHeader *ch = (CommitHeader *)(buf + sizeof(LogHeader));
        for(int i = 0; i < earlierCommitCount; ++i) {
            earlierCommitOffset[i] = (char *)ch - buf;
            ch = (CommitHeader *)((char *)ch + sizeof(CommitHeader) + ch->entry_count * sizeof(CommittedLogEntry));
        }
    }
    printf("=====debug===== earlierCommitCount %d\n", earlierCommitCount);

    std::map<int, InodeOperation>::iterator it;
    for(it = ops.begin(); it != ops.end(); it++) {
        CommittedLogEntry entry;
        entry.ino = it->first;
        entry.operation = it->second;
        printf("=====debug===== ops ino:%d operation:%d\n", entry.ino, entry.operation);
        if(it->second == CREATE_INODE || it->second == WRITE_INODE) {
            entry.current_content_backup_ino = this->allocInodeForLog();
            struct inode *file_inode = this->get_inode(it->first);
            file_inode->size = 0;
            this->put_inode(entry.current_content_backup_ino, file_inode);
            free(file_inode);
            int size2;
            char *buf2;
            this->read_file(it->first, &buf2, &size2);
            this->write_file(entry.current_content_backup_ino, buf2, size2);
            if(it->first == 1) {
                printf("=====debug===== content ino:%d size:%d backup_ino:%d entry2:ino:%d name:%s\n", it->first, size2, entry.current_content_backup_ino, *(unsigned long long *)(buf2+8+64), buf2+8+64+8);
            }
            free(buf2);
            {
                int size2;
                char *buf2;
                this->read_file(entry.current_content_backup_ino, &buf2, &size2);
                printf("=====debug===== special content ino:%d size:%d backup_ino:%d entry2:ino:%d name:%s\n", entry.current_content_backup_ino, size2, entry.current_content_backup_ino, *(unsigned long long *)(buf2+8+64), buf2+8+64+8);
                printf("=====debug===== special content2 ino:%d size:%d content:%s\n", entry.current_content_backup_ino, size2, buf2);
            }
        }
        if(it->second == WRITE_INODE || it->second == REMOVE_INODE) {
            entry.old_content_ino = getOldContentIno(it->first, buf, earlierCommitOffset, earlierCommitCount);
        }
        *(tmp2++) = entry;
    }

    newCommitHeader->commit_id = ++(lh->max_version);
    newCommitHeader->entry_count = ops.size();
    lh->current_version = lh->max_version;

    this->write_file(this->Committed_Log_Ino, buf, new_size);
    printf("=====debug===== commit, new version %d\n", lh->current_version);
    free(earlierCommitOffset);
    free(buf);
    this->Enable_Log = 1;
}

void inode_manager::rollBack() {
    if(this->Enable_Log == 0) {
        return;
    }

    struct inode *node = this->get_inode(this->Uncommitted_Log_Ino);
    if(node->size != 0) {
        free(node);
        std::map<int, InodeOperation> ops;
        int size;
        char *buf;
        this->read_file(this->Uncommitted_Log_Ino, &buf, &size);
        UncommittedLogEntry *tmp = (UncommittedLogEntry *)buf;
        mergeOperations(tmp, size/sizeof(UncommittedLogEntry), ops);
        printf("=====debug===== rollback node->size!=0 size:%d ops.size():%d\n", size, ops.size());
        this->undoUncommittedOperations(ops);
        this->write_file(this->Uncommitted_Log_Ino, "", 0);
        printf("=====debug===== rollback node->size!=0 \n");
        return;
    }

    free(node);
    int size;
    char *buf;
    this->read_file(this->Committed_Log_Ino, &buf, &size);
    LogHeader *lh = (LogHeader *)buf;
    if(lh->current_version <= 1) {
        free(buf);
        return;
    }
    CommitHeader *ch = (CommitHeader *)(buf + sizeof(LogHeader));
    printf("=====debug===== rollback lh->current_version:%d \n", lh->current_version);
    while(ch->commit_id < lh->current_version) {
        printf("=====debug===== rollback ch->commit_id:%d \n", ch->commit_id);
        ch = (CommitHeader *)((char *)ch + sizeof(CommitHeader) + ch->entry_count * sizeof(CommittedLogEntry));
    }
    printf("=====debug===== rollback ch->commit_id:%d \n", ch->commit_id);
    assert(ch->commit_id == lh->current_version);
    this->Enable_Log = 0;
    CommittedLogEntry *entry = (CommittedLogEntry *)(ch + 1);
    for(int i = 0; i < ch->entry_count; ++i) {
        switch(entry[i].operation) {
            case CREATE_INODE: {
                this->remove_file(entry[i].ino);
                writeVersionControlLog(entry[i].ino, REMOVE_INODE);
                break;
            }
            case REMOVE_INODE: {
                struct inode *file_inode = this->get_inode(entry[i].old_content_ino);
                file_inode->size = 0;
                this->put_inode(entry[i].ino, file_inode);    // equal to create
                free(file_inode);
                int size2;
                char *buf2;
                this->read_file(entry[i].old_content_ino, &buf2, &size2);
                this->write_file(entry[i].ino, buf2, size2);
                free(buf2);
                writeVersionControlLog(entry[i].ino, CREATE_INODE);
                break;
            }
            case WRITE_INODE: {
                int size2;
                char *buf2;
                this->read_file(entry[i].old_content_ino, &buf2, &size2);
                this->write_file(entry[i].ino, buf2, size2);
                free(buf2);
                writeVersionControlLog(entry[i].ino, WRITE_INODE);
                break;
            }
        }
    }
    --(lh->current_version);
    this->write_file(this->Committed_Log_Ino, buf, size);
    this->Enable_Log = 1;
    free(buf);
    printf("=====debug===== rollback, current version %d\n", lh->current_version);
}

void inode_manager::stepForward() {
    if(this->Enable_Log == 0) {
        return;
    }
    struct inode *node = this->get_inode(this->Uncommitted_Log_Ino);
    if(node->size != 0) {
        free(node);
        return;
    }
    free(node);

    int size;
    char *buf;
    this->read_file(this->Committed_Log_Ino, &buf, &size);
    LogHeader *lh = (LogHeader *)buf;
    if(lh->current_version == lh->max_version) {
        free(buf);
        return;
    }
    CommitHeader *ch = (CommitHeader *)(buf + sizeof(LogHeader));
    while(ch->commit_id <= lh->current_version) {
        ch = (CommitHeader *)((char *)ch + sizeof(CommitHeader) + ch->entry_count * sizeof(CommittedLogEntry));
    }
    assert(ch->commit_id == lh->current_version + 1);

    this->Enable_Log = 0;
    CommittedLogEntry *entry = (CommittedLogEntry *)(ch + 1);
    for(int i = 0; i < ch->entry_count; ++i) {
        switch(entry[i].operation) {
            case REMOVE_INODE: {
                this->remove_file(entry[i].ino);
                writeVersionControlLog(entry[i].ino, REMOVE_INODE);
                break;
            }
            case CREATE_INODE: {
                struct inode *file_inode = this->get_inode(entry[i].current_content_backup_ino);
                file_inode->size = 0;
                this->put_inode(entry[i].ino, file_inode);    // equal to create
                free(file_inode);
                int size2;
                char *buf2;
                this->read_file(entry[i].current_content_backup_ino, &buf2, &size2);
                this->write_file(entry[i].ino, buf2, size2);
                free(buf2);
                writeVersionControlLog(entry[i].ino, CREATE_INODE);
                break;
            }
            case WRITE_INODE: {
                int size2;
                char *buf2;
                this->read_file(entry[i].current_content_backup_ino, &buf2, &size2);
                this->write_file(entry[i].ino, buf2, size2);
                free(buf2);
                writeVersionControlLog(entry[i].ino, WRITE_INODE);
                break;
            }
        }
    }
    ++(lh->current_version);
    this->write_file(this->Committed_Log_Ino, buf, size);
    this->Enable_Log = 1;
    free(buf);
}

