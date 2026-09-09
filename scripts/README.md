=============================================================================

Here are scripts that might be useful for logging into the campus cluster.

   ccc       -- connect to campus cluster
   getcc     -- pull file "t.tgz" from your home directory on the campus cluster
   pushcc    -- push file "t.tgz" to your home directory on the campus cluster

   driver    -- launch a batch job from your working directory
   mpi.batch -- sets the controls for your mpi job on the compute node (of which there are 4)
             -- please keep max runtime at < 10 minutes (!)
   qstat     -- ./qstat will check your job status, but you must change NETID to your netid.


=============================================================================

In the scripts, change "NETID" to your netid.

   ccc:   ssh -o TCPKeepAlive=no -o ServerAliveInterval=15 -XYA NETID@cc-login.campuscluster.illinois.edu

   getcc: rm -f t.tgz
          scp NETID@cc-login.campuscluster.illinois.edu:t.tgz .
          tar -zxvf t.tgz
          rm t.tgz

   pushcc: scp t.tgz NETID@cc-login.campuscluster.illinois.edu:

   qstat:  squeue -u NETID

=============================================================================

My favorite commands, in a working directory:

      tar -zcvf t.tgz *
      mv t.tgz ~/

which moves t.tgz to my top directory from which I can pull or push as needed.)

=============================================================================
