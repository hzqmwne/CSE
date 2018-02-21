<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H1>Lab 6: Paxos Protocol</H1>
<P><B>Hand out: Nov 27th
<BR>Deadline: Dec 24th 23:59 (GMT+8)
<span style="color: red">No Extension</span></B></P>

<H2>Introduction</H2>

<p>In this lab, you will implement <a href= "./lab6-ref.md" >Paxos</a> and use it to agree to a
  sequence of membership changes (<i>i.e.</i>, view changes). We have modified
  <tt>lock_smain.cc</tt> in this lab to start the
  RSM <i>instead of</i> the lock server; Actually, in this lab, 
  the <tt>lock_server</tt> processes are serving as the <tt>configuration servers</tt>.

<p>When you complete this lab, you will have a
  replicated state machine that manages a group of lock servers.
  Nodes can be added into the replica group, which will contact the
  master and ask to join the group. Nodes can also be removed
  from the replica group when they fail. The set of nodes in the group
  at a particular time is a <i>view</i>, and each time the view
  changes, you will run Paxos to agree on the new view.

<p>The design we have given you consists of three layered modules. The
  RSM and config layers make downcalls to tell the layers below them
  what to do. The config and Paxos modules also make upcalls to the
  layers above them to inform them of significant events (<i>e.g.</i>,
  Paxos agreed to a value, or a node became unreachable).
<DL>
<DT><strong>RSM module</strong></DT>
<DD>
The RSM module is in charge of replication. When a node joins, the RSM
module directs the config module to add the node. The RSM module also
runs a <i>recovery</i> thread on every node to ensure that nodes in the
same view have consistent states. In this lab, the only state to
recover is the sequence of Paxos operations that have been
executed.
</DD>

<DT><strong>config module</strong></DT>
<DD>The config module is in charge of view management. When the RSM
  module asks it to add a node to the current view, the config module
  invokes Paxos to agree on a new view. The config module also sends
  periodic heartbeats to check if other nodes are alive, and removes a
  node from the current view if it can't contact some of the members
  of the current view.  It removes a node by invoking Paxos to agree
  on a new view without the node.</DD>

<DT><strong>Paxos module</strong></DT>
<DD>The Paxos module is in charge of running Paxos to agree on a
  value. In principle the value could be anything. In
  our system, the value is the list of nodes constituting the next view.
</DD>
</DL>
The focus of this lab is on the Paxos module.
<font color = "red">What you need to modify are all in paxos.cc.</font> If you have questions about this lab, 
please ask TA: XiaoLi Zhou(zxlmillie@gmail.com).</P>
<p>

<H2>Getting Started</H2>

<p>Begin by initializing your Lab 6 branch with your implementation
from Lab 5.

<pre>
% cd lab-cse/lab5
% git commit -am 'my solution to lab5'
% git fetch origin lab6:lab6
% git checkout lab6
% git merge lab5
</pre>

<p><B>Note:</B> there must be some conflicts, you need to merge it
	manually. Specifically, except GNUmakefile and lock_smain.cc, others are all your previous implementation.

<p>This will add new files: <tt>paxos_protocol.h</tt>, <tt>paxos.{cc,h}</tt>,
<tt>log.{cc,h}</tt>, <tt>rsm_tester.pl</tt>, <tt>config.{cc,h}</tt>, <tt>rsm.{cc,h}</tt>,
<tt>rsm_protocol.h</tt> and <tt>handle.{cc,h}</tt> to your <tt>lab5/</tt> directory and update
the GNUmakefile from your previous lab.  It will also incorporate minor
changes into your <tt>lock_smain.cc</tt> to initialize the RSM module
when the lock server starts. <font color=red>Note that in this lab, we disable lock server you 
have implemented in lab4 and run rsm instead.</font>
The lock server will now take two
command-line arguments: the port that the master and the port that
the lock server you are starting should bind to. see <tt>rsm_tester.pl</tt> for detail.

<p>
In <tt>rsm.{cc,h}</tt>, we have provided you with code to set up the
appropriate RPC handlers and manage recovery in this lab. 

<p>
In files <tt>paxos.{cc,h}</tt>, you will find a sketch implementation of
the <tt>acceptor</tt> and <tt>proposer</tt> classes that will use the Paxos
protocol to agree on view changes.
The file <tt>paxos_protocol.h</tt> defines the suggested RPC
protocol between instances of Paxos running on different replicas,
including structures for arguments and return types, and marshall code
for those structures. You'll be finishing this Paxos code in this lab.

<p>The files <tt>log.{cc,h}</tt> provide a <i>full</i> implementation
of a <tt>log</tt> class, which should be used by your
<tt>acceptor</tt> and <tt>proposer</tt> classes
to log important Paxos events to disk.  Then, if the node fails and
later re-joins, it has some memory about past views of the system.
<font color="red">Do not make any changes to this class, as we will use
our own original versions of these files during testing.</font>

<p><tt>config.cc</tt> maintains views using Paxos. You will need to
  understand how it interacts with the Paxos and RSM layers, but you
  should not need to make any changes to it for this lab.

<p><tt>handle.cc</tt> this class maintains a cache of RPC connections to other servers. 
If the first call binds with the rpc server successfully, all latter calls would return the same rpcc object. 
There is an example in <tt>handle.h</tt> about how to use it. 

<p>The lab tester <tt>rsm_tester.pl</tt> will automatically start several lock
servers, kill and restart some of them, and check that you have
implemented the Paxos protocol and used it correctly.
	
<H2>Your Job</H2> 
The measure of success for this lab is to pass tests <b>0-4</b> of
<tt>rsm_tester.pl</tt>. The tester starts 3 lock servers, kills and
restarts some of them, and checks that all servers indeed go through a
unique sequence of view changes by examining their on-disk logs.

<pre>
% ./rsm_tester.pl 0 1 2 3 4 
test0...
...
test1...
...
test2...
...
test3...
...
test4...
...
tests done OK
</pre>

<p><b>Important</b>: If rsm_tester.pl fails during the middle of a test, <b>the remaining <tt>lock_server</tt> processes 
are not killed</b> and the log files are not cleaned up (so you can 
debug the causes.). Make sure you do 'killall lock_server; rm -f *.log' 
to clean up the lingering processes before running rsm_tester.pl again.


<H2>Detailed Guidance</H2>

<p>We guide you through a series of steps to get this lab working incrementally.

<h3>Step One: Implement Paxos</h3>

<p> Fill in the Paxos implementation in <tt>paxos.cc</tt>,
following the pseudo-code <a href = "./lab6-ref.md">on the other page</a>. Do not worry about failures yet.

<p>
Use the RPC protocol we provide in
<tt>paxos_protocol.h</tt>.  <font color='red'>In order to pass the tests, when the 
<tt>proposer</tt> sends a RPC, you should set an RPC timeout of
1000 milliseconds.</font>
Note that though the pseudocode shows different
types of responses to each kind of RPC, our protocol combines these responses
into one type of return structure.  For example, the <tt>prepareres</tt> struct
can act as a <tt>prepare_ok</tt>, an <tt>oldinstance</tt>, or a <tt>reject</tt>
message, depending on the situation.

<p>
You may find it helpful for debugging to look in the <tt>paxos-[port].log</tt> files, which are written
to by <tt>log.cc</tt>. <tt>rsm_tester.pl</tt> does not remove these logs when a test fails so that you can
use the logs for debugging. <tt>rsm_tester.pl</tt> also re-directs the stdout and stderr of 
your <tt>configuration server</tt> to <tt>lock_server-[arg1]-[arg2].log</tt>.

<p>Upon completing this step, you should be able to pass '<tt>rsm_tester.pl 0</tt>'. This test starts 
three <tt>configuration servers</tt> one after another and checks that all servers go through the same three views.

<h3>Step Two: Simple failures</h3>

<p>
Test whether your Paxos handles simple failures by
running '<tt>rsm_tester.pl 0 1 2</tt>'. 
You will not have to write any new code for this step if your code is already
correct.

<h3>Step Three: Logging Paxos state</h3>

<p>Modify your Paxos implementation to use the <tt>log</tt> class to
log changes to <tt>n_h</tt>, and
<tt>n_a</tt> and <tt>v_a</tt> when they are updated.  Convince yourself why these 
three values must be logged to disk if we want to re-start a previously crashed
node correctly. We have provided the code to write and read logs in
<tt>log.cc</tt> (see <tt>log::logprop()</tt>, and <tt>log::logaccept()</tt>), so
you just have to make sure to call the appropriate methods at the right times.

<p>Now you can run tests that involve restarting a node after it
  fails.  In particular, you should be able to pass '<tt>rsm_tester.pl 3 4 </tt>'.
In test 4, rsm_tester.pl starts three servers, kills the third server (the remaining 
two nodes should be able to agree on a new view), kills the second server
(the remaining one node tries to run Paxos, but cannot succeed since 
no majority of nodes are present in the current view), restarts the 
third server (it will not help with the agreement since the third server is not 
in the current view), kills the third server, restarts second server (now 
agreement can be reached), and finally restarts third server.

<H2>Handin procedure</H2>
<P>
  After all above done:
  <PRE>
    % make handin
  </PRE>
  That should produce a file called lab6.tgz in the directory. Change the file name to your student id:
  <PRE>
    % mv lab6.tgz lab6_[your student id].tgz
  </PRE>
  Then upload <B>lab6_[your student id].tgz</B> file to <B>ftp://zxlmillie:public@public.sjtu.edu.cn/upload/cse/lab6/</B> before the deadline. You are only given the permission to list and create new file, but no overwrite and read. So make sure your implementation has passed all the tests before final submit.
</P>
<P>You will receive full credits if your software passes the same tests we gave you when we run your software on our machines.</P>

</BODY>
</HTML>
