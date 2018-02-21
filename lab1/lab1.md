<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Lab 1: Inode Manager</H1>
<P><B>Hand out: Sep 17th
<BR>Deadline: Sep 24th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<H2>Get Ready</H2>
<P>
  You should be able to do Lab 1 on any Unix-style machine, including your own Linux desktops, MacOS laptops. However, for Labs 2 and beyond, you'll need to use a computer that has the FUSE module, library, and headers installed. You should be able to install these on your own machine by following the instructions at <a href="http://fuse.sourceforge.net">fuse.sourceforge.net</a>. 
</P>
<P>
  In the past, we will provide you a VM image running on VMware so you can directly use this VM image without any environment configuration. However, in this year, we decided to use Container(Docker) for all of your CSE Labs. Also, we will provide a Container Image including all the environments your need for these labs. If you are not familiar with Container(Docker), read the <a href="./docker_tutorial.md">docker tutorial</a> first.
</p>

<H2>Introduction</H2>
<P>
  In this lab, you will firstly implement an  inode manager to support your file system, where following APIs should work properly:
  <PRE>
    CREATE, GETATTR
    PUT, GET
    REMOVE
  </PRE>
  Before implementing your inode manger, let's have a glance at the  YFS architecture:
  <CENTER><IMG SRC="img/yfs_architecture.png"></IMG></CENTER>
</P>
<P>
  In lab1, you can completely ignore the fuse and yfs_client, but just concern about the parts framed by the red box: <B>extent_client</B>, <B>extent_server</B> and <B>inode_manager</B>.
</P>
<P>
  Extent_client acts as a block provider just like a disk. It will communicate with extent_server using rpc (which you will implement in the future, just now it only uses direct and local function call).
</P>

<P>
  The inode manager mimics the inode layer of alloc_inode, free_inode, read_file, write_file, remove_file, getattr,  which support the five APIs (<B>CREATE/GETATTR/PUT/GET/REMOVE</B>) provided by extent_server.
</P>
<P>
  If you have questions about this lab, either in programming environment or requirement, please ask TA: Dong Du (ddnirvana1 AT Gmail DOT com ).
</P>

<H2>Getting Started</H2>
<P>
  <PRE>
    % mkdir lab-cse 
    % cd lab-cse
    % git clone http://ipads.se.sjtu.edu.cn:1312/lab/cse-2017.git lab1 -b lab1
    % cd lab1
    % git checkout lab1
    % docker pull ddnirvana/cselab_env:latest
       # suppose the absoulte path of lab-cse is /home/xx/lab-cse 
    % sudo docker run -it --privileged --cap-add=ALL -v /home/xx/lab-cse:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
       # now you will enter in a container environment, the codes you downloaded in lab-cse will apper in /home/stu/devlop in the container
    %  cd /home/stu/devlop
    %  make
  </PRE>
</P>
<P>
   <B>Notes</B>:Only files in the volume will be persistent, <B><span style="color: red">put all the files useful to the volume directory</span></B>, in the above example: <i>/home/stu/devlop</i> directory.
</P>
<P>
    if there's no error in make, an executable file lab1_tester will be generated, and after you type
  <PRE>
    % ./lab1_tester
  </PRE>
  you will get following output:
  <PRE>
    ========== begin test create and getattr ==========
    ...
    [TEST_ERROR]: error ...
    --------------------------------------------------
    Final score is : 0
  </PRE>
  If you see additional warnings/errors, it's most likely because you don't have some specific libraries installed. Use the apt-file utility to look up the correct package that contains the file you need, if you are on debian-based system.
</P>
<P>
  This lab will be divided into 3 parts.
</P>
<P>
  Before you write any code, we <B>suggest</B> that you should read <B>inode_manager.h</B> first and be familiar with all the classes. We have already provide you some useful functions such as get_inode and put_inode.
</P>
<P>
  In <B>part 1</B>, you should implement disk::read_block and disk::write_block inode_manager::alloc_inode and inode_manager::getattr, to support <B>CREATE</B> and <B>GETATTR</B> APIs. Your code should pass the <B>test_create_and_getattr()</B> in lab1_tester, which tests creating empty files, getting their attributes like type.
</P>
<P>
  In <B>part 2</B>, you should implement inode_manager::write_file, inode_manager::read_file, block_manager::alloc_block, block_manager::free_block, to support <B>PUT</B> and <B>GET</B> APIs. Your code should pass the <B>test_put_and_get()</B> in lab1_tester, which, write and read files.
</P>
<P>
  In <B>part 3</B>, you should implement inode_manager::remove_file and inode_manager::free_inode, to support <B>REMOVE</B> API. Your code should pass the <B>test_remove()</B> in lab1_tester.
</P>
<P>
  In this lab, you should only need to make changes to inode_manager.cc. (Although you are allowed to change many other files, except those directly used to implement tests.) Although maybe we won't check all the corner case, you should try your best to make your code <B>robust</B>. It will be good for the coming labs.
</P>

<h3>Part 1: CREATE/GETATTR</h3>
    <P>
      Your job in Part 1 is to implement the read_block and write_block of disk and the alloc_inode and getattr of inode_manager, to support the <B>CREATE</B> and <B>GETATTR</B> APIs of extent_server. You may modify or add any files you like, except that you should not modify the lab1_tester.cc. (Although our sample solution, for lab1, contains changes to inode_manager.cc only.)
    </P>

   <P>
      The tips can be found on the codes of inode_manager.[h|cc]. Be aware that you should firstly scan through the code in inode_manager.h, where defines most of the variables, structures and macros you can use, as well as the functions get_inode and put_inode of inode_manager I leave to you to refer to.
    </P>

   <P>
      Meanwhile, pay attention to one of the comments in inode_manager.c:
      <PRE>
	// The layout of disk should be like this:
	// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
      </PRE>
      It may be helpful for you to understand most of the process of the data access.
      After you finish these 4 functions implementation, run:
      <PRE>		
	% make
	% ./lab1_tester
      </PRE>
	You should get following output:
      <PRE>	
	========== begin test create and getattr ==========
	…
	…
	========== pass test create and getattr ==========
	========== begin test put and get ==========
	…
	…
	[TEST_ERROR] : error …
	--------------------------------------------------
	Final score is : 40
      </PRE>
    </P>
<h3>Part 2: PUT/GET</h3>
<P>Your job in Part 2 is to implement the write_file and read_file of inode_manager, and alloc_block and free_block of block_manager, to support the <B>PUT</B> and <B>GET</B> APIs of extent_server.</P>
<P>You should pay attention to the indirect block test. In our inode manager, each file has only one additional level of indirect block, which means one file has 32 direct block and 1 indirect block which point to a block filled with other blocks id.</P>
<P>After you finish these 4 functions implementation, run:
  <PRE>
    % make
    % ./lab1_tester
  </PRE>
  You should get following output:
  <PRE>
    ========== begin test create and getattr ==========
    …
    …
    ========== pass test create and getattr ==========
    ========== begin test put and get ==========
    …
    …
    ========== pass test put and get ==========
    ========== begin test remove ==========
    …
    ...
    [TEST_ERROR] : error …
    --------------------------------------------------
    Final score is : 80
  </PRE>
</P>
<h3>Part 3: REMOVE</h3>
<P>Our job in Part 3 is to implement the remove_file and free_inode of inode_manager, to support the <B>REMOVE</B> API of extent_server.</P>
<P>After you finish these 2 functions implementation, run:
  <PRE>
    % make
    % ./lab1_tester
  </PRE>
  You should get following output:
  <PRE>
    ========== begin test create and getattr ==========
    …
    …
    ========== pass test create and getattr ==========
    ========== begin test put and get ==========
    …
    …
    ========== pass test put and get ==========
    ========== begin test remove ==========
    …
    ...
    ========== pass test remove ==========
    --------------------------------------------------
    Final score is : 100
  </PRE>
</P>

<h2>Handin Procedure</h2>
<P>After all above done:
  <PRE>
    % cd /path_to_cselab/lab1
    % make handin
  </PRE>
  That should produce a file called lab1.tgz in your lab1/ directory. Change the file name to your student id:
  <PRE>
    % mv lab.tgz [your student id]-lab1.tgz
  </PRE>
  Then upload <B>[your student id]-lab1.tgz</B> file to <B>ftp://Dd_nirvana:public@public.sjtu.edu.cn/upload/cse/lab1/</B> before the deadline. You are only given the permission to list and create new file, but no overwrite and read. So make sure your implementation has passed all the tests before final submit. (If you must re-submit a new version, add explicit version number such as "V2" to indicate).
</P>
<P>You will receive full credit if your software passes the same tests we gave you when we run your software on our machines.</P>
<P>Please take your time examining this lab and the overall architecture of yfs. There are more interesting challenges ahead waiting for you.</P>
</BODY></HTML>

