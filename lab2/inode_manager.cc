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
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1) {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
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
  for(i = 0; i < (int)(INODE_NUM / IPB); ++i) {
    int block_index = IBLOCK(0, BLOCK_NUM) + i;
	bm->read_block(block_index, buf);
	for(int j = 0; j < (int)IPB; ++j) {
		if(i * IPB + j == 0) {
			continue;    // inode number 0 never used
		}
		struct inode *node = (struct inode *)buf + j;
		if(node->type == 0) {    // empty inode
			node->type = type;
			node->ctime = time(0);
			bm->write_block(block_index, buf);
			free(buf);
			return i * IPB + j;
		}
	}
  }
  int block_index = IBLOCK(0, BLOCK_NUM) + i;
  for(int j = 0; j < (int)(INODE_NUM % IPB); ++j) {
    if(i * IPB + j == 0) {
		continue;    // inode number 0 never used
	}
  	struct inode *node = (struct inode *)buf + j;
	if(node->type == 0) {    // empty inode
		node->type = type;
		node->ctime = time(0);
		bm->write_block(block_index, buf);
		free(buf);
		return i * IPB + j;
	}
  }
  return 1;
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

  printf("========debug write_file ino: %d ",inum);
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
}
