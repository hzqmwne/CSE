<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Lab 3: RPC</H1>
<P><B>Hand out: Oct 10th
<BR>Deadline: Oct 17th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<H2>Introduction</H2>
<P>In this lab, you will use RPC to implement a single client file server.</P>
<P>In the previous lab, you have finished a filesystem on a single machine. And in lab 3 (and 4), your aim is to extend it to a distributed file server. And from the architectural point of view, it now moves on to the RPC part.</P>
<CENTER><IMG SRC="img/yfs_rpc.png"></IMG></CENTER>

<P>If you have questions about this lab, either in programming environment or requirement, please ask TA: Dong Du (ddnirvana1@gmail.com).</P>

<H2>Getting started</H2>
<P>
  <PRE>
    % cd lab-cse
    % git clone http://ipads.se.sjtu.edu.cn:1312/lab/cse-2017.git lab3 -b lab3
    % cd lab3
  </PRE>
  Note: here when you execute “git branch” you should see that you are in lab3 branch
</P>
<P><B>Now, copy all your modified files(except fuse.cc) in lab2 to this lab3 directory. </B></P>
<P><B>As for fuse.cc, you can either copy your modified code segments in lab2 to lab3 or copy the whole file to lab3 and then:</B></P>
<P>
  <B>modify</B>
  <PRE>
    if(argc != 2){
        fprintf(stderr, "Usage: yfs_client &lt;mountpoint&gt;\n");
  </PRE>
  <B>to</B>
  <PRE>
    if(argc != 3){
        fprintf(stderr, "Usage: yfs_client &lt;mountpoint&gt; &lt;port-extent-server&gt;\n");
  </PRE>
  <B>and modify</B>
  <PRE>
    yfs = new yfs_client();
  </PRE>
  <B>to</B>
  <PRE>
    yfs = new yfs_client(argv[2]);
  </PRE>
</P>
<P><B>You'll also need to modify constructor for yfs_client class like the one we give in lab3's yfs_client.{hh,cc} after copying these two*, i.e.:</B></P>
<P>
  <B>modify</B>
  <PRE>
    yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
  </PRE>
  <B>to</B>
  <PRE>
    yfs_client::yfs_client(std::string extent_dst)
  </PRE>
  <PRE>
    % sudo docker run -it --privileged --cap-add=ALL -v /home/xx/lab-cse:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
    % make
  </PRE>
</P>

<P>If there's no error in make, 3 executable files: <B>yfs_client</B>, <B>extent_server</B>, <B>test-lab-3-g</B> will be generated.</P>

<P><B>Note 1</B>: For Lab2 and beyond, you'll need to use a computer that has the FUSE module, library, and headers installed. You should be able to install these on your own machine by following the instructions at <a href="https://github.com/libfuse/libfuse">FUSE: Filesystem in Userspace</a>.</P>

<P><B>Note 2</B>: Both 32-bit and 64-bit librpc are provided, so the lab should be architecture independent.</P>
<P><B>Note 3</B>: For this lab, you will not have to worry about server failures or client failures. You also need not be concerned about malicious or buggy applications.</P>

<H2>Distributed FileSystem (Strawman's Approach)</H2>

<P>In lab2, we have implemented a file system on a single machine. In this lab, we just extend the single machine fils system to a distributed file system.</P>
<P>Separating extent service from yfs logic brings us a lot of advantages, such as no fate sharing with yfs client, high availability.</P>
<P>Luckily, most of your job has been done in the previous lab. You now can use extent service provided by extent_server through RPC in extent_client. Then a strawman distributed file system has been finished.</P>
<P><B>You had better test your code with the previous test suit before any progress.</B></P>

<H2>Detailed Guidance</H2>

<P>In principle, you can implement whatever design you like as long as it satisfies the requirements in the "Your Job" section and passes the testers. In practice, you should follow the detailed guidance below.</P>
<P>Using the RPC system:</P>
<P>
  <UL>
    <LI>The RPC library. In this lab, you don't need to care about the implementation of RPC mechanisms, rather you'll use the RPC system to make your local filesystem become a distributed filesystem.</LI>
    <LI>A server uses the RPC library by creating an RPC server object (rpcs) listening on a port and registering various RPC handlers (see main() function in demo_server.cc).</LI>
    <LI>A client creates a RPC client object (rpcc), asks for it to be connected to the demo_server's address and port, and invokes RPC calls (see demo_client.cc).</LI>
    <LI><B>You can learn how to use the RPC system by studying the stat call implementation.</B> please note it's for illustration purpose only, you won't need to follow the implementation:</LI>
    <UL><LI>use <I>make rpcdemo</I> to build the RPC demo</LI></UL>
    <LI>RPC handlers have a standard interface with one to six request arguments and a reply value implemented as a last reference argument. The handler also returns an integer status code; the convention is to return zero for success and to return positive numbers for various errors. If the RPC fails in the RPC library (e.g.timeouts), the RPC client gets a negative return value instead. The various reasons for RPC failures in the RPC library are defined in rpc.h under rpc_const.</LI>
    <LI>The RPC system marshalls objects into a stream of bytes to transmit over the network and unmarshalls them at the other end. Beware: the RPC library does not check that the data in an arriving message have the expected type(s). If a client sends one type and the server is expecting a different type, something bad will happen. You should check that the client's RPC call function sends types that are the same as those expected by the corresponding server handler function.</LI>
    <LI>The RPC library provides marshall/unmarshall methods for standard C++ objects such asstd::string, int, and char. You should be able to complete this lab with existing marshall/unmarshall methods.</LI>
  </UL>
</P>
<P> To simulate the distributed environment, you need to run your file system in <B>two containers</B>. One is for extent_server and another for yfs_client. </P>

<H3>Configure the Network</H3>
<P> Go through <a href="https://github.com/docker/labs/blob/master/networking/A1-network-basics.md">Network Basics in Docker</a> first.</P>
<P> You should be able to get the ip address of your container by <i>docker network</i> command now. Compare it with the result you get by running the command <i>ifconfig</i> directly in your container. </P>

<P> Start two containers in two different terminals
  In the Termianl_1
  <PRE>
    ## modify the path in the following command to your lab's directory
    % sudo docker run -it --privileged --cap-add=ALL -v /home/xx/lab-cse:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
  </PRE>
  In the Termianl_2
  <PRE>
    ## modify the path in the following command to your lab's directory
    % sudo docker run -it --privileged --cap-add=ALL -v /home/xx/lab-cse:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
  </PRE>
  In the Termianl_2's container
  <PRE>
    ## get the ip of container 2 
    %  ifconfig
  </PRE>
  the results will be look like:
  <PRE>
eth0      Link encap:Ethernet  HWaddr 02:42:ac:11:00:04  
          inet addr:172.17.0.4  Bcast:0.0.0.0  Mask:255.255.0.0
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:13 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:1747 (1.7 KB)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

  </PRE>
  Here, the ip address of the container2 is <B>172.17.0.4</B>. (You may get different result, it's normal)
  
  In the Termianl_1's container
  <PRE>
     # Try to connect container2 in container1 by 
     %  ping 172.17.0.4 # 172.17.0.4 is the ip of container2, change this to your values
  </PRE>
  If the result are some thins like
  <PRE>
stu@f3a3f2bfa1c6:~$ ping 172.17.0.4
PING 172.17.0.4 (172.17.0.4) 56(84) bytes of data.
64 bytes from 172.17.0.4: icmp_seq=1 ttl=64 time=0.172 ms
64 bytes from 172.17.0.4: icmp_seq=2 ttl=64 time=0.079 ms
64 bytes from 172.17.0.4: icmp_seq=3 ttl=64 time=0.086 ms
64 bytes from 172.17.0.4: icmp_seq=4 ttl=64 time=0.090 ms
64 bytes from 172.17.0.4: icmp_seq=5 ttl=64 time=0.092 ms
64 bytes from 172.17.0.4: icmp_seq=6 ttl=64 time=0.091 ms
64 bytes from 172.17.0.4: icmp_seq=7 ttl=64 time=0.084 ms
^C
--- 172.17.0.4 ping statistics ---
7 packets transmitted, 7 received, 0% packet loss, time 5998ms
rtt min/avg/max/mdev = 0.079/0.099/0.172/0.030 ms

  </PRE>
  It means your two containers are successfully connected.
</P>


	

<H2>Testers & Grading</H2>

<P> Your will need two containers for your testing and grading. Open two terminals, and start two containers following the above guidence. We assume the container 1 is used for yfs_client and the container 2 is used for extent server. </P> 
<P>The test for this lab is test-lab-3-g. The test take two directories as arguments, issue operations in the two directories, and check that the results are consistent with the operations. Here's a successful execution of the tester.
  In the container 2(extent server container).
  <PRE>
   % make
   % ./start_server.sh
  </PRE>
  In the container 1(yfs client container).
  Modify the server IP address in the <i> start_client.sh</i> file.
  change the 
  <PRE> 
      EXTENT_SERVER_HOST=127.0.0.1
  </PRE>
  to , here the 172.17.0.4 is the ip address of the container2(extent server container)
  <PRE> 
      EXTENT_SERVER_HOST=172.17.0.4
  </PRE>
  <PRE>
    % ./start_client.sh
    starting ./extent_server 29409 > extent_server.log 2>&1 &
    starting ./yfs_client /home/cse/cse-2014/yfs1 29409 > yfs_client1.log 2>&1 &
    starting ./yfs_client /home/cse/cse-2014/yfs2 29409 > yfs_client2.log 2>&1 &
    % ./test-lab-3-g ./yfs1 ./yfs2
    Create then read: OK
    Unlink: OK
    Append: OK
    Readdir: OK
    Many sequential creates: OK
    Write 20000 bytes: OK
    test-lab-2-g: Passed all tests.
    % ./stop_client.sh
  </PRE>
</P>

<P>
  To grade this part of lab, a test script grade.sh is provided. It contains are all tests from lab2 (tested on both clients), and test-lab-3-g. Here's a successful grading.
  Start the server in the container 2 first
  <PRE>
   % make
   % ./start_server.sh
  </PRE>
  In the container 1(yfs client container).
  <PRE>
    % ./grade.sh
    Passed A
    Passed B
    Passed C
    Passed D
    Passed E
    Passed test-lab-3-g (consistency)
    Passed all tests!
    >> Lab 3 OK
  </PRE>
</P>


<H2>Tips</H2>

<P>This is also the first lab that writes null ('\0') characters to files. The std::string(char*)constructor treats '\0' as the end of the string, so if you use that constructor to hold file content or the written data, you will have trouble with this lab. Use the std::string(buf, size) constructor instead. Also, if you use C-style char[] carelessly you may run into trouble!</P>
<P>Do notice that a non RPC version may pass the tests, but RPC is checked against in actual grading. So please refrain yourself from doing so ;)</P>
<P> You can use stop_server.sh in extent_server container side to stop the server, use stop_client in yfs_client container side to stop the client. </P>
<H2>Handin procedure</H2>

<P>
  After all above done:
  <PRE>
    % make handin
  </PRE>
  That should produce a file called lab3.tgz in the directory. Change the file name to your student id:
  <PRE>
    % mv lab3.tgz lab3_[your student id].tgz
  </PRE>
  Then upload <B>lab3_[your student id].tgz</B> file to <B>ftp://Dd_nirvana:public@public.sjtu.edu.cn/upload/cse/lab3/</B> before the deadline. You are only given the permission to list and create new file, but no overwrite and read. So make sure your implementation has passed all the tests before final submit.
</P>
<P>You will receive full credits if your software passes the same tests we gave you when we run your software on our machines.</P>

</BODY></HTML>
