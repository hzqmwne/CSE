<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Lab 7: Erasure Coding</H1>
<P><B>Hand out: Dec 7th
<BR>Deadline: Dec 24th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<H2>Introduction</H2>
<P>In this lab, you should provide the yfs with the ability of fault tolerance.</P>
<P>There are many important data stored in the file system implemented by you.
So it is necessary to detect and correct any potential errors, such as bit
flip, in your system.</P>
<P>This lab is based on Lab5 !!!</P>
<P>If you have questions about this lab, either in programming environment or
requirement, please ask TA: Zhichao Hua(zhichaohua@foxmail.com).</P>

<H2>Getting started</H2>
<P>
  <PRE>
	% git clone http://ipads.se.sjtu.edu.cn:1312/lab/cse-2017.git lab7 -b lab5
	% cd lab7
  </PRE>
	<B>Now, copy all your modified files in lab5 to this lab7 directory.</B>
  <PRE>
	% git commit -am "sol for lab5"
	% git fetch origin lab7:lab7
	% git checkout lab7
	% git merge lab5
  </PRE>
	Note: there must be some conflicts in inode_manager.cc, you need to merge it
	manually. Specifically, merge the implementation related to "disk::disk()".
 	Note: here when you execute "git branch", you should see that you are in lab7
	branch.
</P>

<H2>Fault tolerance techniques for yfs</H2>

<p>The file data stored in the disk may be corrupted. In previous labs, we do
not consider this situation. That's to say, we maybe read the corrupted content
from your yfs.</p>
<P><B>In this lab, you must guarantee that yfs always returns the original file
	data to clients.</B> So, your yfs should be able to <B>detect</B> the data corruptions and
		<B>correct</B> them.</p>
<p>There exists some famous techniques for detection or correction of data
corruptions, such as <a href="https://en.wikipedia.org/wiki/Parity_bit">parity check</a> and <a href="https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction">Reed-Solomon
	codes</a>. <a href="https://ipads.se.sjtu.edu.cn/courses/cse/2017/labs/ercode.pdf">Here</a> is a previous work based on Reed-Solomon code. You can refer to it.<p>
<p><B>Hint:</B> The direct solution of this lab is to implement Reed-Solomon code
during file operations. Besides, you can also design your own coding algorithm
or use other algorithms to pass the tests.</p>
<p><B>Hint:</B> Replication is another solution to tolerant data corrections. If
taking this way, you need to think about how to replicate data in the same disk.</p>
<p>In this lab, we only consider about bit flips. We recommand you to read the
<B>Detailed Guidance</B> before designing the algorithm. Besides, you must ensure 
your project can pass all the tests before round 1. The test script is a bit
more strict than before, which may help you find the hidden bugs.

<H2>Detailed Guidance</H2>

<P>In principle, you can implement whatever design you like as long as it
satisfies the requirements and passes the testers. In practice, you should
follow the detailed guidance below. The disk layout of yfs is given as follows:</P>
<P>|super block|-----bitmap----|-----inode table-----|---------------data
blocks--------------|</P>
<P><B>Do not modify <font color="red">disk.cc, test-lab-5.c, GNUmakefile and the implementation of "disk::disk()".</font> During grading, we will replace them with the
	original ones.</B></p>
<P>The test is divided into five rounds:</P>
<P>
  <UL>
		<LI>In the first round, there will be one bit flip at the same location in each block that
			contains file data (data blocks in the above layout). Note that the
			metadata will be unchanged.</LI>
		<LI>In the second round, there will be one bit flip at a random location in
			each block that contains file data. Note that the
			metadata will be unchanged.</LI>
		<LI>In the third round, there will be two bit flips at two random locations in
			each block that contains file data. Note that the
			metadata will be unchanged.</LI>
		<LI>In the fourth round, there will be one bit flip at a random location in
			each block except for super blocks. Note that the
			metadata will be <B>changed</B>.</LI>
		<LI>In the last round, which is a bonus, there will be one bit flip at a random location in
			each <B>byte</B> that contains file data. Note that the
			metadata will be unchanged.</LI>
  </UL>
</P>

<H2>Testers & Grading</H2>

<P>The test for this lab is test-lab-7. The test take two directories as
arguments, issue operations in the two directories, and check that the results with the operations. Here's a successful execution of the tester:
  <PRE>
    % ./start.sh
    starting ./lock_server 10453 > lock_server.log 2>&1 &
    starting ./extent_server 10447 > extent_server.log 2>&1 &
    starting ./yfs_client /home/hzc/cse/2017/lab5-2017/yfs1 10447 10453 ./cert/root.pem > yfs_client1.log 2>&1 &
    starting ./yfs_client /home/hzc/cse/2017/lab5-2017/yfs2 10447 10453 ./cert/root.pem > yfs_client2.log 2>&1 &
    % ./test-lab-7 ./yfs1 ./yfs2
    Create then read: OK
    Unlink: OK
    Append: OK
		...
		round 1:
		OK: You got 20 points now!
		round 2:
		OK: You got 50 points now!
		round 3:
		OK: You got 80 points now!
		round 4:
		OK: You got 95 points now!
		cleanup:
		OK: You got 100 points now!
		bonus:
		OK: You got 105 points now!
		test-lab-5: Passed all tests.
    % ./stop.sh
  </PRE>
</P>

<H2>Handin procedure</H2>

<P>
  After all above done:
  <PRE>
    % make handin
  </PRE>
  That should produce a file called lab7.tgz in the directory. Change the file name to your student id:
  <PRE>
    % mv lab7.tgz lab7_[your student id].tgz
  </PRE>
  Then upload <B>lab7_[your student id].tgz</B> file to <B>ftp://Dd_nirvana:public@public.sjtu.edu.cn/upload/cse/lab7/</B> before the deadline. You are only given the permission to list and create new file, but no overwrite and read. So make sure your implementation has passed all the tests before final submit.
</P>
<P>You will receive full credits if your software passes the same tests we gave you when we run your software on our machines.</P>

</BODY></HTML>
