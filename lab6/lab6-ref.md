<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML><HEAD>
<META http-equiv=Content-Type content="text/html; charset=gb2312">
<LINK href="labs.css" type="text/css" rel="stylesheet">
<link rel="stylesheet" href="./github.css" type="text/css">
<META content="MSHTML 6.00.6000.16705" name=GENERATOR></HEAD>
<BODY>
<H2>Paxos Reference</H2>

<h3>Understanding how Paxos is used for view changes</h3>

<p>There are two classes that together implement the Paxos protocol:
  <tt>acceptor</tt> and <tt>proposer</tt>.  Each replica runs both
  classes.  The <tt>proposer</tt> class leads the Paxos protocol by
  proposing new values and sending requests to all replicas.
  The <tt>acceptor</tt> class processes the requests from the
  <tt>proposer</tt> and sends responses back.  The method
  <tt><nobr>proposer::run(nodes, v)</nobr></tt> is used to get the
  members of the current view (<tt>nodes</tt>) to agree to a value
  <tt>v</tt>. When an agreement instance completes, <tt>acceptor</tt>
  will call <tt>config</tt>'s <tt>paxos_commit(instance, v)</tt>
  method with the value that was chosen. As explained below, other
  nodes may also attempt to start Paxos and propose a value, so there
  is no guarantee that the value that a server proposed is the
  same as the one that is actually chosen. (In fact, Paxos might abort
  if it can't get a majority to accept its <tt> prepare </tt> or 
<tt>accept</tt> messages!)

<p>The <tt>config</tt> module performs view changes among the set of
participating nodes.  The first view of the system is specified
manually.  Subsequent view changes rely on Paxos to agree on a unique
next view to displace the current view.

<p>When the system starts from scratch, the first node creates view 1
containing itself only, i.e. view_1={1}.  When node 2 joins after the
first node, node two's RSM module joins node 1 and transfers view 1 from the
node one. Then, node 2 asks its <tt>config</tt> module to add
itself to view 1. The config module will use Paxos to propose to
nodes in view_1={1} a new view_2 containing node 1 and 2.  When Paxos
succeeds, view_2 is formed, <i>i.e.</i>, view_2={1,2}.  When node 3
joins, its
RSM module will download the last view from the first node (view 2)
and it will then attempt to propose to nodes in view 2 a new
view_3={1,2,3}. And so on.

<p>The config module will also initiate view changes when it
discovers that some nodes in the current view are not
responding.  In particular, the node with the smallest id periodically
sends heartbeat RPCs to all others (and all other servers periodically
send heartbeats to the node with the smallest id).  If a heartbeat RPC
times out, the config module calls the <tt>proposer</tt>'s
<tt><nobr>run(nodes, v)</nobr></tt> method to start a new round of the Paxos
protocol.  Because each node
independently decides if it should run Paxos, there may be several
instances of Paxos running simultaneously; Paxos sorts that out
correctly.

<p>The <tt>proposer</tt> keeps track of whether the current view is
stable or not (using the <tt>proposer::stable</tt> variable).  If the
current view is stable, there are no on-going Paxos view change
attempts by this node.  When the current view is not stable,
the node is initiating the Paxos protocol.

<p>The <tt>acceptor</tt> logs important Paxos events as well as a complete
history of all values agreed to on disk. At any time a node can reboot
and when it re-joins, it may be many views behind. Unless the node
brings itself up-to-date on the current view, it won't be able to
participate in Paxos.  By remembering all views, the other nodes can
bring this re-joined node up to date.

<h3>The Paxos Protocol</h3>

<p>The <a href="https://ipads.se.sjtu.edu.cn/courses/cse/2017/labs/lab6/paxos-simple.pdf">Paxos Made Simple
  paper</a> describes a protocol that agrees on a value and then
  terminates. Since we want to run another instance of Paxos every
  time there is a view change, we need to ensure that messages from
  different instances are not confused. We do this by adding instance
  numbers (which are not the same as proposal numbers) to all messages.
  Since we are using Paxos to agree on view changes, the instance
  numbers in our use of Paxos are the same as the view numbers in the
  config module.

<p>Paxos can't guarantee that every node learns the chosen value right
  away; some of them may be partitioned or crashed. Therefore, some
  nodes may be behind, stuck in an old instance of Paxos while the
  rest of the system has moved on to a new instance. If a node's
  <tt>acceptor</tt> gets an RPC request for an old instance, it should
  reply to the <tt>proposer</tt> with a
  special RPC response (set <tt>oldinstance</tt> to true). This response
  informs the calling <tt>proposer</tt> that it is
  behind and tells it what value was chosen for that instance.

<p>Below is the pseudocode for Paxos. The <tt>acceptor</tt> and <tt>proposer</tt> skeleton
classes contain member variables, RPCs, and RPC handlers
corresponding to this code. Except for the additions to handle
instances as described above, it mirrors the protocol described in the paper.

<pre>
<font color = "green">
proposer run(instance, v):
 choose n, unique and higher than any n seen so far
 send prepare(instance, n) to all servers including self
 if oldinstance(instance, instance_value) from any node:
   commit to the instance_value locally
 else if prepare_ok(n_a, v_a) from majority:
   v' = v_a with highest n_a; choose own v otherwise
   send accept(instance, n, v') to all
   if accept_ok(n) from majority:
     send decided(instance, v') to all

acceptor state:
 must persist across reboots
 n_h (highest prepare seen)
 instance_h, (highest instance accepted)
 n_a, v_a (highest accept seen)

acceptor prepare(instance, n) handler:
 if instance <= instance_h
   reply oldinstance(instance, instance_value)
 else if n > n_h
   n_h = n
   reply prepare_ok(n_a, v_a)
 else
   reply prepare_reject

acceptor accept(instance, n, v) handler:
 if n >= n_h
   n_a = n
   v_a = v
   reply accept_ok(n)
 else
   reply accept_reject

acceptor decide(instance, v) handler:
 paxos_commit(instance, v)
</font>
</pre>

<p>
For a given instance of Paxos, potentially many
nodes can make proposals, and each of
these proposals has a unique proposal number.  When comparing
different proposals, the highest proposal number wins.  To ensure that
each proposal number is unique, each proposal consists of a number and
the node's identifier.  We provide you with a struct <tt>prop_t</tt>
in <tt>paxos_protocol.h</tt> that you should use for proposal numbers;
we also provide the <tt>&gt;</tt> and <tt>&gt;=</tt> operators for the
class.

<p>Each replica must log certain change to its Paxos state (in particular
the <tt>n_a</tt>, <tt>v_a</tt>, and <tt>n_h</tt> fields), as well as
log every agreed value.  The provided <tt>log</tt> class does this for
you; please use it without modification, as the test program depends
on its output being in a particular format.

<p>
Add the extra
parameter <tt>rpcc::to(1000)</tt> to your RPC calls to prevent the
RPC library from spending a long time attempting to contact crashed nodes.

</BODY>
</HTML>
