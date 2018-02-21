<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Lab 5: Log-based Version Control</H1>
<P><B>Hand out: Nov 8th
<BR>Deadline: Nov 15th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<H2>Introduction</H2>
<P>Usually, we depend on log systems to survive from crashes. However, human error are even more harmful than crashes. Thus, version control is needed to help us. In this lab, we will add support for version control operations based on log.</P>

<P>If you have questions about this lab, either in programming environment or
requirement, please ask TA: XiaoLi Zhou(zxlmillie@sjtu.edu.cn).</P>



<H2>Getting started</H2>
<P>Please back up your solution to lab4 before going on.</P>
<P>
  <PRE>
    	% cd lab-cse/lab4
	% git commit -am "sol for lab4"
	% git fetch origin lab5:lab5
	% git checkout lab5
	% git merge lab4
  </PRE>

</P>
<P>
<B>Note:</B> There may be some conflicts. If so, you need to merge it manually. Please make sure that you can run it properly in the docker(under the guidance of lab4).<br>
<B>Note:</B> Here when you execute "git branch", you should see that you are in lab5 branch.<br>
</P>



<H2>Part I - Try</H2>
<P>
New API

Three operations have been added. By typing
  <PRE>
    %./yfs-version -h
  </PRE>

it will show you the usage.
  <PRE>
    %./start.sh
    %./yfs-version -c
    %./yfs-version -p
    %./yfs-version -n
  </PRE>
</P>
<P>
Each of the above will send a signal to your yfs_client. (Note: You must start a yfs_client by start.sh, before performing those operations and there can be only one yfs_client running in the background.)
</P>
<P>
To receive the signals, you should register a signal handler in fuse.cc. Here's a example,
  <PRE>
    void sig_handler(int no) {
      switch (no) {
      case SIGINT:
          printf("commit a new version\n");
          break;
      case SIGUSR1:
          printf("to previous version\n");
          break;
      case SIGUSR2:
          printf("to next version\n");
          break;
      }
    }
  </PRE>
  Try those operations and test your system before going on.
</P>



<H2>Part II - Undo and Redo</H2>
<H3>Commit</H3>
<P>Contents of yfs filesystem should be remembered and marked as a new version. For example, we have committed v0 and v1. After committing, we will get a new version, v2, and be in version 3.</P>

<H3>Roll back</H3>
<P>Recover contents of yfs filesystem by undoing logs. For example, we are in version 2  currently(have committed v0 and v1). After rolling back, we will be in version 1 and contents of yfs filesystem should be the same as that of version 1.</P>

<H3>Step forward</H3>
<P>Recover contents of yfs filesystem by redoing logs. For example, we are in version 2  currently(have committed v0 and v1). After stepping forward, we will be in version 3 and contents of yfs filesystem should be the same as that of version 3.</P>

<B>Hint:</B>

<LI>Be careful to delete log entries.</LI>
<LI>Although some log can not be delete, it's still good practice to keep checkpoints. Also, you can roll back and step forward from a checkpoint directly.</LI>

<H3>Pass Test</H3>
<P>Type the following commands to test your lab5.
  <PRE>
    %./start.sh
    %./test-lab-5 ./yfs1
    %./stop.sh
  </PRE>
</P>

<P>If you pass the lab, it will look like this.
  <PRE>
  ===Undo Test:
  ...
  Pass Undo Test
  ===Redo Test:
  ...
  Pass Redo Test
  </PRE>
</P>



<H2>Handin procedure</H2>

<P>
  After all above done:
  <PRE>
    % make handin
  </PRE>
  That should produce a file called lab5.tgz in the directory. Change the file name to your student id:
  <PRE>
    % mv lab5.tgz lab5_[your student id].tgz
  </PRE>
  Then upload <B>lab5_[your student id].tgz</B> file to <B>ftp://zxlmillie:public@public.sjtu.edu.cn/upload/cse/lab5/</B> before the deadline. You are only given the permission to list and create new file, but no overwrite and read. So make sure your implementation has passed all the tests before final submit.
</P>
<P>You will receive full credits if your software passes the same tests we gave you when we run your software on our machines.</P>

</BODY></HTML>
