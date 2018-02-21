<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Lab 2: Basic File Server</H1>
<P><B>Hand out: Sep 25th
<BR>Deadline: Oct 8th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<H2>Introduction</H2>
<P>In this lab, you will start your file system implementation by getting the following FUSE operations to work:
  <PRE>
    CREATE/MKNOD, LOOKUP, and READDIR
    SETATTR, WRITE and READ
    MKDIR and UNLINK
    SIMBOLIC LINK
  </PRE>
  (For your own convenience, you may want to implement rmdir to facilitate your debugging/testing.)</P>
<P>At first, let's review the YFS architecture:</P>
<CENTER><IMG SRC="img/yfs_client_and_fuse.png"></IMG></CENTER>

<P>In lab2, what you should concern about are the parts framed by the red box above: <B>FUSE</B> and <B>YFS client</B>.</P>

<P>The <B>FUSE</B> interface, in <B>fuse.cc</B>. It translates FUSE operations from the FUSE kernel modules into YFS client calls. We provide you with much of the code needed to register with FUSE and receive FUSE operations. We have implemented all of those methods for you except for Symbol Link. So don't modify <B>fuse.cc</B> unless you want to implement <B>Symbol Link</B>.</P>
<P>The <B>YFS client</B>, in <B>yfs_client.{cc,h}</B>. The YFS client implements the file system logic. For example, when creating a new file, your yfs_client will add directory entries to a directory block.</P>

<P>If you have questions about this lab, either in programming environment or requirement, please ask TA: Dong Du (ddnirvana1@Gmail.com).</P>

<H2>Getting started</H2>
<P>
  <PRE>
    % mkdir lab-cse
    % cd lab-cse
    % git clone http://ipads.se.sjtu.edu.cn:1312/lab/cse-2017.git lab2 -b lab2
    % cd lab2
  </PRE>
  Note: here when you execute “git branch” you should see that you are in <B>lab2 branch</B>.
  <PRE>
    % cp ../lab1/inode_manager.* ./
    % docker pull ddnirvana/cselab_env:latest
    % sudo docker run -it --privileged --cap-add=ALL -v /home/xx/lab-cse:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
    % cd /home/stu/devlop/lab2 
    % make
  </PRE>
  if there's no error in make, an executable file <B>yfs_client</B> will be generated.
</P>
<P>If you added/edited files other than inode_manager.* during your first lab, please do a git merge like this:
  <PRE>
    % cd lab1
    % git commit -am "my solution to lab1"
    % git pull
    % git checkout lab2
    % git merge lab1
  </PRE>
  Note: We have updated the cselab_env image for Lab2, please pull the most <B> latest </B> container image before starting your Lab2. For Lab2 and beyond, you'll need the use environment that has the <B> FUSE </B> library, and headers installed. These modules are already installed in the container image, so you can ignore the environment configuration. You can learn more things about <B> FUSE <B> in  <a href="http://fuse.sourceforge.net/">fuse.sourceforge.net</a>.
</P>

<P>We provide you with the script <B>start.sh</B> to automatically start <B>yfs_client</B>, and </B>stop.sh</B> to kill previously started processes. Actually, <B>start.sh</B> starts one <B>yfs_client</B> with <B>./yfs1</B> mountpoint. Thus you can type:
  <PRE>
    % cd lab2
    % make
    % sudo ./start.sh
    % sudo ./test-lab-2-a.pl ./yfs1
    % sudo ./test-lab-2-b.pl ./yfs1
    % sudo ./stop.sh
  </PRE>
</P>
<P>Note 1: Since you need to mount fuse file system, so you should add <B>sudo</B> to above commands;</P>
<P>
  Note 2: If <B>stop.sh</B> reports "Device or resource busy", please <B>keep executing</B> <B>stop.sh</B> until it says "not found in /etc/mtab", such as:
  <PRE>
    fusermount: entry for /home/your_name/yfs-class/yfs1 not found in /etc/mtab
    fusermount: entry for /home/your_name/yfs-class/yfs2 not found in /etc/mtab
    …
  </PRE>
</P>
<P>This lab will be divided into <B>4 parts</B>:</P>

<P>At the beginning, it will be helpful to scan the <B>interfaces and structs</B> in <B>yfs_client.h</B> and some other files. The functions you have implemented in lab1 are the fundament of this lab.</P>

<H3>Part 1: CREATE/MKNOD, LOOKUP, and READDIR</H3>

<H4>Your job</H4>

<P>In Part 1 your job is to implement the CREATE/MKNOD, LOOKUP and READDIR of <B>yfs_client.cc</B> in YFS. You may modify or add any files you like, except that you should not modify the test scripts. Your code should pass the <B>test-lab-2-a.pl</B> script, which tests creating empty files, looking up names in a directory, and listing directory contents.</P>

<P>On some systems, FUSE uses the MKNOD operation to create files, and on others, it uses CREATE. The two interfaces have slight differences, but in order to spare you the details, we have given you wrappers for both that call the common routine <B>createhelper()</B>. You can see it in <B>fuse.cc</B>.</P>

<P>As before, if your YFS passes our tester, you are done. If you have questions about whether you have to implement specific pieces of file system functionality, then you should be guided by the tester: if you can pass the tests without implementing something, then you do not have to implement it. For example, you don't need to implement the exclusive create semantics of the CREATE/MKNOD operation.</P>

<H4>Detailed Guidance</H4>

<P>1. When creating a new file (fuseserver_createhelper) or directory (fuseserver_mkdir), you must assign a unique inum (which you’ve done in lab1).</P>

<P>Note: Though you are free to choose any inum identifier you like for newly created files, FUSE assumes that the inum for the root directory is 0x00000001. Thus, you'll need to ensure that when yfs_client starts, it is ready to export an empty directory stored under that inum.</P>

<P>2. Directory format: Next, you must choose the <B>format for directories</B>. A directory's content contains a set of name to inode number mappings. You should store a directory's entire content in a directory (recall what you learned). A simple design will make your code simple. You may refer to the <B>FAT32</B> specification (<a href="http://staff.washington.edu/dittrich/misc/fatgen103.pdf">http://staff.washington.edu/dittrich/misc/fatgen103.pdf</a>) or the EXT inode design (<a href="http://en.wikipedia.org/wiki/Inode_pointer_structure">http://en.wikipedia.org/wiki/Inode_pointer_structure</a>) for an example to follow.</P>

<P><B>Note: As is mentioned in Wikipedia (<a href="http://en.wikipedia.org/wiki/Ext3">http://en.wikipedia.org/wiki/Ext3</a>), the EXT3 filesystem which we go after supports any characters but '\0' and '/' in the filename. Make sure your code passes when there's '$', '_', ' ', etc, in the filename.</B></P>

<P>3. FUSE:When a program (such as ls or a test script) manipulates a file or directory (such as yfs1) served by your <B>yfs_client</B>, the FUSE code in the kernel sends corresponding operations to yfs_client via FUSE. The code we provide you in <B>fuse.cc</B> responds to each such operation by calling one of a number of procedures, for create, read, write, etc. operations. You should modify the relevant routines in fuse.cc to call methods in <B>yfs_client.cc</B>. <B>fuse.cc</B> should just contain glue code, and the core of your file system logic should be in <B>yfs_client.cc</B>. For example, to handle file creation, fuseserver_createhelper to call yfs->create(...), and you should complete the create(...) method to <B>yfs_client.cc</B>. Look at getattr() in <B>fuse.cc</B> for an example of how a fuse operation handler works, how it calls methods in <B>yfs_client</B>, and how it sends results and errors back to the kernel. YFS uses FUSE's "lowlevel" API.</P>

<P>4. YFS code:The bulk of your file system logic should be in <B>yfs_client</B>, for the most part in routines that correspond to fuse operations (create, read, write, mkdir, etc.). Your <B>fuse.cc</B> code should pass inums, file names, etc. to your <B>yfs_client</B> methods. Your <B>yfs_client</B> code should retrieve file and directory contents from the extent client with get(), using the inum as the extent ID. In the case of directories, your <B>yfs_client</B> code should parse the directory content into a sequence of name/inum pairs (i.e. yfs_client::dirents), for lookups, and be able to add new name/inum pairs.</P>

<P>5. A reasonable way to get going on <B>fuss.cc</B> is to run <B>test-lab-2-a.pl</B>, find the function in <B>fuse.cc</B> whose missing implementation is causing the tester to fail, and start fixing that function. Look at the end of <B>yfs_client1.log</B> and/or add your own print statements to <B>fuse.cc</B>. If a file already exists, the CREATE operator (fuseserver_create and fuseserver_mknod) should return EEXIST to FUSE.</P>

<P>6. <B>start.sh</B> <B>redirects</B> the STDOUT and STDERR of the servers to files in the current directory. For example, any output you make from <B>fuse.cc</B> will be written to yfs_client1.log. Thus, you should look at these files for any debug information you print out in your code.</P>

<P>7. If you wish to test your code with only some of the FUSE hooks implemented, be advised that FUSE may implicitly try to call other hooks. For example, FUSE calls LOOKUP when mounting the file system, so you may want to implement that first. FUSE prints out to the <B>yfs_client1.log</B> file the requests and results of operations it passes to your file system. You can study this file to see exactly what hooks are called at every step.</P>

<H4>About Test</H4>

<P>
  The Lab 1 tester for Part 1 is <B>test-lab-2-a.pl</B>. Run it with your YFS mountpoint as the argument. Here's what a successful run of <B>test-lab-2-a.pl</B> looks like:
  <PRE>
    % make
    % sudo ./start.sh
    starting ./yfs_client /home/lab/Courses/CSE-g/lab1-sol/yfs1  > yfs_client1.log 2>&1 &
    % sudo ./test-lab-2-a.pl ./yfs1
    create file-yyuvjztagkprvmxjnzrbczmvmfhtyxhwloulhggy-18674-0
    create file-hcmaxnljdgbpirprwtuxobeforippbndpjtcxywf-18674-1
    …
    Passed all tests!
  </PRE>
  The tester creates lots of files with names like file-XXX-YYY-Z and checks that they appear in directory listings.
</P>

<P>If <B>test-lab-2-a.pl</B> exits without printing "Passed all tests!", then it thinks something is wrong with your file server. For example, if you run <B>test-lab-2-a.pl</B> on the skeleton code we give you, you'll probably see some error message like this:
  <PRE>
    test-lab-2-a: cannot create /tmp/b/file-ddscdywqxzozdoabhztxexkvpaazvtmrmmvcoayp-21501-0 : No such file or directory
  </PRE>
This error message appears because you have not yet provided code to handle the CREATE/MKNOD operation with FUSE. That code belongs in fuseserver_createhelper in <B>fuse.cc</B>.</P>

<P>Note: testing Part 1 on the command line using commands like touch will not work until you implement the SETATTR operation in Part 2. For now, you should do your testing via the creat/open, lookup, and readdir system calls in a language like Perl, or simply use the provided test script.</P>
<P>Note: if you are sure that there is not any mistake in your implementation for part1 and still cannot pass this test, maybe there are some <B>bugs in your lab1</B>, especially read_file and write_file. Remeber that passing the test do not guarantee completely correct.</P>

<H3>Part 2: SETATTR, READ, WRITE</H3>

<H4>Your job</H4>

<P>In Part 2 your job is to implement SETATTR, WRITE, and READ FUSE operations in <B>fuse.cc</B> and <B>yfs_client.cc</B>. Once your server passes <B>test-lab-2-b.pl</B>, you are done. Please don't modify the test program or the RPC library. We will use our own versions of these files during grading.</P>

<H4>Detailed Guidance</H4>

<P>1. Implementing SETATTR. The operating system can tell your file system to set one or more attributes via the FUSE SETATTR operation. See <a href="https://sourceforge.net/u/noon/fuse/ci/ecd073bd7054c9e13516041e3ef930e39270c8df/tree/include/fuse_lowlevel.h">fuse_lowlevel.h</a> for the relevant definitions. The <B>to_set</B> argument to your SETATTR handler is a mask that indicates which attributes should be set. There is really only one attribute (the file size attribute) you need to pay attention to (but feel free to implement the others if you like), indicated by bit <B>FUSE_SET_ATTR_SIZE</B>. Just AND (i.e., &) the to_set mask value with an attribute's bitmask to see if the attribute is to be set. The new value for the attribute to be set is in the attar parameter passed to your SETATTR handler. The operating system may implement overwriting an existing file with a call to SETATTR (to truncate the file) rather than CREATE. Setting the size attribute of a file can correspond to truncating it completely to zero bytes, truncating it to a subset of its current length, or even padding bytes on to the file to make it bigger. Your system should handle all these cases correctly.</P>

<P>2. Implementing READ/WRITE:A read (fuseserver_read) wants up to size bytes from a file, starting from a certain offset. When less than size bytes are available, you should return to fuse only the available number of bytes. See the manpage for read(2) for details. For writes (fuseserver_write), a non-obvious situation may arise if the client tries to write at a file offset that's past the current end of the file. Linux expects the file system to return '\0's for any reads of unwritten bytes in these "holes" (see the manpage for <a href="http://man7.org/linux/man-pages/man2/lseek.2.html">lseek(2)</a> for details). Your write should handle this case correctly.</B>

<H4>About Test</H4>

<P><B>test-lab-2-b.pl</B> tests reading, writing, and appending to files. To run the tester, first start one yfs_client using the <B>start.sh</B> script.
  <PRE>
    % sudo ./start.sh
  </PRE>
  Now run test-lab-2-b.pl by passing the yfs1 mountpoint.
  <PRE>
    % sudo ./test-lab-2-b.pl ./yfs1
    Write and read one file: OK
    Write and read a second file: OK
    Overwrite an existing file: OK
    Append to an existing file: OK
    Write into the middle of an existing file: OK
    Check that one cannot open non-existant file: OK
    Check directory listing: OK
    Passed all tests
    
    % sudo ./stop.sh
  </PRE>
</P>
<P>If <B>test-lab-2-b.pl</B> exits without printing "Passed all tests!" or hangs indefinitely, then something is wrong with your file server. After you are done with Part 2, you should go back and test with <B>test-lab-2-a.pl</B> again to make sure you did not break anything.</P>

<H3>Part 3: MKDIR and UNLINK</H3>

<H4>Your job</H4>

<P>In Part 3 your job is to handle the MKDIR and UNLINK FUSE operations. For MKDIR, you do not have to create "." or ".." entries in the new directory since the Linux kernel handles them transparently to YFS. UNLINK should always free the file's extent; you do not need to implement UNIX-style link counts.</P>

<H4>About Test</H4>

<P>If your implementation passes the <B>test-lab-2-c.pl</B> script, you are done with part 3. The test script creates a directory, creates and deletes lots of files in the directory, and checks file and directory mtimes and ctimes. Note that this is the first test that explicitly checks the correctness of these time attributes. A create or delete should change both the parent directory's mtime and ctime (here you should decide which level you can <B>modify the 3 time attributes</B>, and think about why?). Here is a successful run of the tester:
  <PRE>
    % sudo ./start.sh
    % sudo ./test-lab-2-c .pl ./yfs1

    mkdir ./yfs1/d3319
    create x-0
    delete x-0
    create x-1
    checkmtime x-1
    ...
    delete x-33
    dircheck
    Passed all tests!

    % sudo ./stop.sh
  </PRE>
  Note: Now run the command sudo ./grade and you should pass A, B, C and E.</P>

<H3>Part 4: SYMLINK, READLINK</H3>

<P>Please implement symbolic link. To implement this feature, you should refer to the FUSE documentation available online and figure out the methods you need to implement. It's all on yourself. Also, look out for comments and hints in the hand-out code.

Note: remember to <B>add related method</B> to fuse.cc.

You may want to refer to <a href="http://stackoverflow.com/questions/6096193/how-to-make-symbolic-links-in-fuse">http://stackoverflow.com/questions/6096193/how-to-make-symbolic-links-in-fuse</a> and <a href="https://fossies.org/dox/fuse-2.9.7/structfuse__operations.html">https://fossies.org/dox/fuse-2.9.7/structfuse__operations.html</a>.

<H4>GRADING</H4>
<P>
  Finally, after you've implemented all these features, run the grading script:
  <PRE>
    % ./grade.sh
    Passed A
    Passed B
    Passed C
    Passed D
    Passed E
    Passed all tests!

    Total score: 100
  </PRE>

  Note that if you encounter a <B>"yfs_client DIED"</B>, your filesystem is not working. In such cases the requests are served by the system's file system (usually EXT3 or btrfs or tmpfs). <B>You would not be awarded credits if your yfs_client crashes, but could get partial credit if it produces incorrect result for some test cases.</B> So do look out for such mistakes. <B>We've seen dozens of students every year thinking that they've passed lots of tests before realizing this.</B></P>

<H2>Handin procedure</H2>

<P>After all above done:
  <PRE>
    % cd ~/lab2

    % make handin
  </PRE>

  That should produce a file called lab2.tgz in your lab2/ directory. Change the file name to your student id:
  <PRE>
    % mv lab.tgz lab2_[your student id].tgz
  </PRE>

  Then upload <B>lab2_[your student id].tgz</B> file to <B>ftp://Dd_nirvana:public@public.sjtu.edu.cn/upload/cse/lab2/</B> before the deadline. You are only given the permission to list and create new file, but no overwrite and read. So make sure your implementation has passed all the tests before final submit.
  </P>

<P>You will receive full credits if your software passes the same tests we gave you when we run your software on our machines.</P>

</BODY></HTML>

