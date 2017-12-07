// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

yfs_client::yfs_client()
{
  ec = NULL;
  lc = NULL;
}

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst, const char* cert_file)
{
  ec = new extent_client(extent_dst);
  lc = new lock_client(lock_dst);
  if (ec->put(1, "") != extent_protocol::OK)
      printf("error init root dir\n"); // XYB: init root dir
}

int
yfs_client::verify(const char* name, unsigned short *uid)
{
  	int ret = OK;

	return ret;
}


yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
    extent_protocol::attr a;

    //assert(inum > 0 && inum < 10000);
    //lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
	    //lc->release(inum);
        printf("error getting attr\n");
        return false;
    }
    //lc->release(inum);

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        return true;
    } 
    //printf("isfile: %lld is a dir\n", inum);
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */
bool yfs_client::issymlink(inum ino) {
    extent_protocol::attr a;

    //assert(ino> 0 && ino < 10000);
    //lc->acquire(ino);
    if (ec->getattr(ino, a) != extent_protocol::OK) {
	    //lc->release(ino);
        printf("error getting attr\n");
        return false;
    }
    //lc->release(ino);

    if (a.type == extent_protocol::T_SYMLINK) {
        printf("issymlink: %lld is a symbol link\n", ino);
        return true;
    } 
    return false;
}

bool
yfs_client::isdir(inum inum)
{
    // Oops! is this still correct when you implement symlink?
    extent_protocol::attr a;

    //assert(inum > 0 && inum < 10000);
    //lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        //lc->release(inum);
        printf("error getting attr\n");
        return false;
    }
    //lc->release(inum);

    if (a.type == extent_protocol::T_DIR) {
        printf("isdir: %lld is a dir\n", inum);
        return true;
    } 
    return false;
    //return ! isfile(inum);
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;

    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    //assert(inum > 0 && inum < 10000);
    //lc->acquire(inum);
	lc->acquire(0);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        //lc->release(inum);
        r = IOERR;
        goto release;
    }
	lc->release(0);
    //lc->release(inum);

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    //assert(inum > 0 && inum < 10000);
    //lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        //lc->release(inum);
        r = IOERR;
        goto release;
    }
    //lc->release(inum);
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    return r;
}

int
yfs_client::getsymlink(inum inum, fileinfo &fin)
{
    int r = OK;

    printf("getsymlink %016llx\n", inum);
    extent_protocol::attr a;
    //assert(inum > 0 && inum < 10000);
    //lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        //lc->release(inum);
        r = IOERR;
        goto release;
    }
    //lc->release(inum);
    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;

release:
    return r;
}


#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)

// Only support set size of attr
int
yfs_client::setattr(inum ino, filestat st, unsigned long toset)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */

    // really unefficient !
    std::string buf = std::string();
    //assert(ino > 0 && ino < 10000);
	lc->acquire(ino);
    ec->get(ino, buf);
    buf.resize(st.size);
    ec->put(ino, buf);
	lc->release(ino);
    return r;
}

// my own design for directory file
struct DirectoryEntry {
    yfs_client::inum ino;    // inode num of the file
    char name[64];    // file name, max length is 255 (last byte is '\0')
};

// my own function
int yfs_client::createTypeFile(inum parent, const char *name, mode_t mode, inum &ino_out, extent_protocol::types type) {
    int r = OK;

    extent_protocol::attr a;
    ec->getattr(parent, a);

    std::string buf = std::string();
    ec->get(parent, buf);
    char *directory = const_cast<char*>(buf.c_str());    // dangerous !

    DirectoryEntry *entry = (DirectoryEntry *)directory;
	unsigned int count = a.size / sizeof(DirectoryEntry);
	int empty_pos = -1;
    for(int i = 0; i < (int)count; ++i) {
        if(entry[i].ino != 0 && !strcmp(name, entry[i].name)) {
			ino_out = entry[i].ino;
			return r;    // if find, immediately return
        }
		if(entry[i].ino == 0) {
            empty_pos = empty_pos < 0 ? i : empty_pos;
            // first empty position in directory entry list
        }
    }

    // now, must not find
	inum new_ino;
    ec->create(type, new_ino);
    // should check return value
    ino_out = new_ino;
    if(empty_pos >= 0) {
        entry[empty_pos].ino = new_ino;
        strcpy(entry[empty_pos].name, name);
    }
    else {
        DirectoryEntry new_entry;
        new_entry.ino = new_ino;
        strcpy(new_entry.name, name);

        std::string tmp;
		tmp.assign((char *)&new_entry, sizeof(DirectoryEntry));
        buf.append(tmp);
    }
    ec->put(parent, buf);
    printf("==========debug create new ino: %d,name %s, empty_pos %d, old count %d\n",(int)ino_out,name, empty_pos,count);    ///////////////////
    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    //assert(parent > 0 && parent < 10000);
    lc->acquire(0);
    createTypeFile(parent, name, mode, ino_out, extent_protocol::T_FILE);
    lc->release(0);
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    
    //assert(parent > 0 && parent < 10000);
    lc->acquire(parent);
    createTypeFile(parent, name, mode, ino_out, extent_protocol::T_DIR);
    lc->release(parent);
    return r;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */

    //assert(parent > 0 && parent < 10000);
    //lc->acquire(parent);

    extent_protocol::attr a;
    ec->getattr(parent, a);

    std::string buf;
    ec->get(parent, buf);

    //lc->release(parent);

    const char *directory = buf.c_str();

    DirectoryEntry *entry = (DirectoryEntry *)directory;
	unsigned int count = a.size / sizeof(DirectoryEntry);
    for(int i = 0; i < (int)count; ++i) {
        if(entry[i].ino != 0 && !strcmp(name, entry[i].name)) {
            found = true;
			ino_out = entry[i].ino;
			return r;
        }
    }
    found = false;
    return r;
}

int
yfs_client::readdir(inum dir, std::list<dirent> &list)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */

    //assert(dir > 0 && dir < 10000);
    //lc->acquire(dir);

    extent_protocol::attr a;
    ec->getattr(dir, a);

    std::string buf;
    ec->get(dir, buf);

    //lc->release(dir);

    const char *directory = buf.c_str();

    DirectoryEntry *entry = (DirectoryEntry *)directory;
    unsigned int count = a.size / sizeof(DirectoryEntry);
    printf("=====debug===== yfs_client::readdir count:%d a.size:%d\n", count, a.size);
    for(int i = 0; i < (int)count; ++i) {
        if(entry[i].ino != 0) {
            dirent ent;
            ent.name = entry[i].name;
            ent.inum = entry[i].ino;
            list.push_back(ent);
        }
    }
    return r;
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: read using ec->get().
     */

    std::string buf;
    //assert(ino > 0 && ino < 10000);
	//lc->acquire(ino);
    ec->get(ino, buf);
    //lc->release(ino);
    if(off < (off_t)buf.size()) {
        data = buf.substr(off, size<buf.size()-off ? size : buf.size()-off);
    }
	else {
        data = std::string("");
    }
	printf("========debug read ino: %d buf: %s",(int)ino,data.c_str());
    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */

    //assert(ino > 0 && ino < 10000);
    lc->acquire(ino);

    std::string new_data;
	new_data.assign(data, size);
    
    std::string buf;
    ec->get(ino, buf);
   
    std::string tail;
	tail = off+size < buf.size() ? buf.substr(off+size, buf.size()) : "";

	buf.resize(off, '\0');
    buf.append(new_data);
	buf.append(tail);

    ec->put(ino, buf);
	bytes_written = size;

    lc->release(ino);

	printf("========debug write ino: %d buf: %s",(int)ino,buf.c_str());
    return r;
}

int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */

    //assert(parent > 0 && parent < 10000);
    lc->acquire(parent);

    extent_protocol::attr a;
    ec->getattr(parent, a);

    std::string buf = std::string();
    ec->get(parent, buf);
    char *directory = const_cast<char*>(buf.c_str());    // dangerous !

    DirectoryEntry *entry = (DirectoryEntry *)directory;
	unsigned int count = a.size / sizeof(DirectoryEntry);
    for(int i = 0; i < (int)count; ++i) {
        if(entry[i].ino != 0 && !strcmp(name, entry[i].name)) {
            lc->acquire(entry[i].ino);
            ec->remove(entry[i].ino);
            lc->release(entry[i].ino);
            memset((char *)(entry + i), 0, sizeof(DirectoryEntry));
            ec->put(parent, buf);
            lc->release(parent);
            return r;    // if find, immediately return
        }
    }
    lc->release(parent);

    return NOENT;
}

// my own readlink
int yfs_client::readlink(inum ino, std::string &link) {
    int r = OK;
    //assert(ino > 0 && ino < 10000);
    //lc->acquire(ino);
    ec->get(ino, link);
    //lc->release(ino);
    return r;
}

//my own symlink
int yfs_client::symlink(inum parent, const char *link, const char *name) {
    int r = OK;
    inum ino_out;
    size_t bytes_written;
    //assert(parent > 0 && parent < 10000);
    //lc->acquire(parent);
    createTypeFile(parent, name, 0, ino_out, extent_protocol::T_SYMLINK);
    printf("========debug createsymlink ino: %d , isfile %d , issymlink %d\n", (int)ino_out,isfile(ino_out),issymlink(ino_out));
    assert(ino_out > 0 && ino_out < 10000);
    //lc->acquire(ino_out);
    this->write(ino_out, strlen(link), 0, link, bytes_written);
    //lc->release(ino_out);
    printf("========debug writesymlink ino: %d , isfile %d , issymlink %d\n", (int)ino_out,isfile(ino_out),issymlink(ino_out));
    //lc->release(parent);
    return r;
}

// my own rmdir
int yfs_client::rmdir(inum parent, const char *name) {
    return this->unlink(parent, name);    // this is not good !
}

/* ============================================================== */

int yfs_client::commit() {
    int r = OK;
    ec->commit();
    return r;
}

int yfs_client::rollBack() {
    int r = OK;
    ec->rollBack();
    return r;
}

int yfs_client::stepForward() {
    int r = OK;
    ec->stepForward();
    return r;
}


