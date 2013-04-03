# Tony Cheneau <tony.cheneau@nist.gov>

# This software was developed by employees of the National Institute of
# Standards and Technology (NIST), and others.
# This software has been contributed to the public domain.
# Pursuant to title 15 Untied States Code Section 105, works of NIST
# employees are not subject to copyright protection in the United States
# and are considered to be in the public domain.
# As a result, a formal license is not needed to use this software.
# 
# This software is provided "AS IS."
# NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED
# OR STATUTORY, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
# AND DATA ACCURACY.  NIST does not warrant or make any representations
# regarding the use of the software or the results thereof, including but
# not limited to the correctness, accuracy, reliability or usefulness of
# this software.

cdef extern from "netinet/icmp6.h":
	cdef struct icmp6_filter:
		pass

cdef extern from "icmplib.h":
	cdef int init_capability()
	cdef int dropCapabilities()

	cdef int createSocket(char *)

	cdef int sendMsg(int icmp_sock, char * target, void * msg, int msg_len)

	cdef int recvMsg(int, char *, int, char *, int, char *, int, unsigned int *, int)

	cdef void initICMPFilter(icmp6_filter * filter)

	cdef void allowTypeICMPfilter(icmp6_filter * filter, int type)

	cdef int enableICMPfilter(int icmp_sock, icmp6_filter * filter)

cdef extern from "net/if.h":
	cdef char * if_indextoname(unsigned ifindex, char * ifname)
