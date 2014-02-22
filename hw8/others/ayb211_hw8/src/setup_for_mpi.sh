export LAMRSH="ssh -q -x"
source /opt/lam-mpi/etc/lam-mpi.env

recon hostfile.txt

lamboot -v hostfile.txt
