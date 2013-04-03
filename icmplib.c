/* Tony Cheneau <tony.cheneau@nist.gov> */

/* This software was developed by employees of the National Institute of
Standards and Technology (NIST), and others.
This software has been contributed to the public domain.
Pursuant to title 15 Untied States Code Section 105, works of NIST
employees are not subject to copyright protection in the United States
and are considered to be in the public domain.
As a result, a formal license is not needed to use this software.

This software is provided "AS IS."
NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED
OR STATUTORY, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
AND DATA ACCURACY.  NIST does not warrant or make any representations
regarding the use of the software or the results thereof, including but
not limited to the correctness, accuracy, reliability or usefulness of
this software. */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>

#include <linux/filter.h>

#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <net/if.h>

#include <unistd.h>

#include <stdio.h>

#include <stdlib.h>

#include "caplib.h"

#include "icmplib.h"

#ifdef DEBUG
/**
 * Helper function that print hex values of a string
 * @param pointer data to be printed
 * @param size    size of the data
 */
static void hex_print(void * pointer, int size)
{
	int i=0;
	unsigned char * p = (unsigned char *) pointer;

	for(i=0; i< size; ++i)
	{
		printf("%02X ", p[i]);
	}
	putchar('\n');

}
#endif /* DEBUG */


/* copied from netinet/in.h */
struct in6_pktinfo
{
	struct in6_addr ipi6_addr;  /* src/dst IPv6 address */
	unsigned int ipi6_ifindex;  /* send/recv interface index */
};


/**
 * Initialize the default capabilities
 */
int init_capability(void) { return limit_capabilities(); }


/**
 * Drop all capabilities
 */
int dropCapabilities(void) { return drop_capabilities(); }

/**
 * Create an ICMPv6 socket
 * @return an ICMPv6 socket descriptor
 */
int createSocket(char * iface)
{
	int icmp_sock = 0;
	int hold = 65535;
	int sockopt;
	int err;
    struct ipv6_mreq mreq6;
    struct in6_addr mcast_group;

    memset(&mcast_group, 0, sizeof(mcast_group));
    memset(&mreq6, 0, sizeof(mreq6));

	if (enable_capability_raw() == -1)
		return -1;

	icmp_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

    if (strncmp(iface, "unspec", sizeof("unspec")) != 0)
    {
    	err = setsockopt(icmp_sock, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface));
    	if (err < 0) {
    		perror("setsockopt(SO_BINTDODEVICE)");
    		goto error;
    	}
    }

	if (disable_capability_raw() == -1)
		return -1;

	if (icmp_sock == -1)
	{
		perror("socket() fails");
		return icmp_sock;
	}

	err = setsockopt(icmp_sock, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));
	if (err < 0) {
        perror("setsockopt(SO_RCVBUF)");
		goto error;
	}

    sockopt = 1;
    /* on Linux, this is IPV6_RECVPKTINFO, where on all other unices, it is IPV6_PKTINFO
       go figure... */
    err = setsockopt(icmp_sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &sockopt, sizeof(sockopt));

	if (err < 0) {
        perror("setsockopt(IP_PKTINFO)");
        goto error;
    }

    err = inet_pton(AF_INET6, "ff02::1a", &mcast_group); /* this is the all-RPL-nodes multicast address */

    if (err == 0) {
        perror("inet_pton");
        goto error;
    }

    memcpy(&mreq6.ipv6mr_multiaddr, &mcast_group, sizeof(struct in6_addr));
    mreq6.ipv6mr_interface = if_nametoindex(iface);
    err = setsockopt(icmp_sock, SOL_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6));

    if (err < 0) {
        perror("setsockopt(IPV6_JOIN_GROUP)");
        goto error;
    }

    sockopt = 255; /* hop limit */
    err = setsockopt(icmp_sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &sockopt, sizeof(sockopt));
    if (err < 0)
    {
        perror("setsockopt(IPV6_UNICAST_HOPS)");
        goto error;
    }

    err = setsockopt(icmp_sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &sockopt, sizeof(sockopt));
    if (err < 0)
    {
        perror("setsockopt(IPV6_MULTICAST_HOPS");
        goto error;
    }

	return icmp_sock;

error:
	close(icmp_sock);
	return -1;
}


int sendMsg(int icmp_sock, char * target, void * msg, size_t msg_len)
{
	int cc, gai;
	struct addrinfo hints, *ai;
	struct sockaddr_in6 whereto;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;

	gai = getaddrinfo(target, NULL, &hints, &ai);
	if (gai) {
		fprintf(stderr, "unknown host\n");
		return(-2);
	}

	memcpy(&whereto, ai->ai_addr, sizeof(whereto));

	freeaddrinfo(ai);

	whereto.sin6_port = htons(IPPROTO_ICMPV6);

	cc = sendto(icmp_sock, (char *) msg, msg_len, MSG_CONFIRM,
			    (struct sockaddr *) &whereto,
			    sizeof(struct sockaddr_in6));

	return cc;
}

/**
 * Receives an ICMPv6 message or fail of the socket queue is empty and the blocking flag is set
 * @param  icmp_sock [description]
 * @return           [description]
 */
int recvMsg(int icmp_sock, char * buffer, size_t buflen, char * from, socklen_t from_len, char * to, socklen_t to_len, unsigned int * if_idx, int blocking)
{
	struct sockaddr_in6 from_sock;
	struct sockaddr_in6 to_sock;

	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr * cmsg;
	u_char cbuf[2048];


	struct in6_pktinfo *pi = NULL;

	int cc;

	iov.iov_base = buffer;
	iov.iov_len = buflen;

	msg.msg_name = &from_sock;
	msg.msg_namelen = sizeof(struct sockaddr_in6);
	msg.msg_iov = & iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);



	memset((void *) &from_sock, 0, sizeof(struct sockaddr_in6));
	memset((void *) &to_sock, 0, sizeof(struct sockaddr_in6));
	memset((void *) buffer, 0, buflen);


	cc = recvmsg(icmp_sock, &msg, blocking ? 0: MSG_DONTWAIT);

    inet_ntop(AF_INET6, (void *) &from_sock.sin6_addr, from, from_len);

	for (cmsg = CMSG_FIRSTHDR(&msg);cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg))
	{
		/* only process the IPV6_PKTINFO */
	    if (cmsg->cmsg_level != IPPROTO_IPV6 ||
    	    cmsg->cmsg_type != IPV6_PKTINFO)
	    	    continue;

		pi = (struct in6_pktinfo *) CMSG_DATA(cmsg);
		inet_ntop(AF_INET6,  &pi->ipi6_addr, to, to_len);
		* if_idx = pi->ipi6_ifindex;
	}

#ifdef DEBUG
	/* receive packet and print its source */
	int i;
    printf("receiving an ICMP message from %s\n", from);
	for(i=0; i<cc; ++i){
			printf("%02X ", buffer[i]);
	}
	putchar('\n');
#endif /* DEBUG */

	return cc;
}

/**
 * Initialize an ICMP filter. By default, the filter blocks all type of messages.
 * @param filter is the filter object that is being initialized
 */
void initICMPFilter(struct icmp6_filter * filter)
{
	/* block all ICMP packets */
	ICMP6_FILTER_SETBLOCKALL(filter);
}

/**
 * Allow a type of ICMP messages to go through this filter
 * @param filter is the filter object that is being modified
 * @param type is the type of ICMP message that is now enabled to go through
 */
void allowTypeICMPfilter(struct icmp6_filter * filter, int type)
{
	ICMP6_FILTER_SETPASS(type, filter);
}

/**
 * Activate an ICMPv6 filter on a socket
 * @param icmp_sock is the ICMP socket on which the filter is applied
 * @param filter is the filter being applied
 * @return 0 on success, any other value on failure
 */
int enableICMPfilter(int icmp_sock, struct icmp6_filter * filter)
{
	int err;
	struct icmp6_filter applied_filter;
	socklen_t applied_filter_len = sizeof(struct icmp6_filter);

	err = setsockopt(icmp_sock, IPPROTO_ICMPV6, ICMP6_FILTER, filter,
			 sizeof(struct icmp6_filter));

	if (err < 0) {
		perror("setsockopt(ICMP6_FILTER)");
		return err;
	}

	err = getsockopt(icmp_sock, IPPROTO_ICMPV6, ICMP6_FILTER, &applied_filter,
			&applied_filter_len);

	if (err <0)
	{
		perror("getsockopt(ICMP6_FILTER)");
		return err;
	}

#ifdef DEBUG
	hex_print(filter, sizeof(struct icmp6_filter));
	hex_print(&applied_filter, sizeof(struct icmp6_filter));
#endif

	return memcmp(&applied_filter, filter, sizeof(struct icmp6_filter));
}
