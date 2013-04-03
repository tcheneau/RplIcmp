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

from tinyICMPlib cimport *
from libc.string cimport memset, memcpy
from cython cimport bool

cdef enum:
    # from net/if.h
    IF_NAMESIZE = 16
    #  from netinet/in.h
    INET6_ADDRSTRLEN = 46

def dropAllCapabilities():
    """drop all the capabilities for the current process"""
    if dropCapabilities() == -1:
        raise Exception("Unable to drop capabilities")


cdef class RplSocket:
    cdef int socket
    cdef icmp6_filter rpl_filter
    cdef char from_addr[INET6_ADDRSTRLEN]
    cdef char to_addr[INET6_ADDRSTRLEN]
    cdef char[IF_NAMESIZE] iface
    cdef unsigned int if_idx
    cdef char buffer[65535]

    dropCapabilities = staticmethod(dropAllCapabilities)

    def __cinit__(self, iface="unspec", capabilities=False):
        """Create a RPL ICMPv6 socket.
        This socket filters out the ICMP traffic that is not RPL (i.e. type is not 154).

        >>> r = RplSocket()
        >>> r.dropAllCapabilities()
        >>> r.send("fe80::2", "test")
        >>> msg = r.receive()
        """
        self.socket = 0
        self.if_idx = 0
        memcpy(self.iface, <void *> iface, len(iface) + 1)

        if capabilities and init_capability() != 0:
            raise Exception("Unable to initialize capability")

        self.socket = createSocket(iface)

        if self.socket == -1:
            raise Exception("Unable to allocate socket, check you have the proper permission set, or you have not dropped down the capabilities already")

        # prevent message other that RPL Control messages to be interpreted
        initICMPFilter(&self.rpl_filter)
        allowTypeICMPfilter(&self.rpl_filter, 155)
        if enableICMPfilter(self.socket, &self.rpl_filter) != 0:
            raise Exception("Unable to set the socket filter")

    cpdef send(self, char * destination, msg):
        """Send msg, a message, to the IPv6 destination address"""
        return sendMsg(self.socket, destination, <char *>msg, len(msg))

    cpdef receive(self, blocking=True):
        """Receive an ICMPv6 packet.
        The blocking flags indicate if the recv() call should return immediately
        if no packet are awaiting or if it should block while waiting for a packet"""
        memset(self.buffer, 0, sizeof(self.buffer))
        memset(self.from_addr, 0, sizeof(self.from_addr))
        memset(self.to_addr, 0, sizeof(self.to_addr))

        cdef int msg_len
        cdef char * res
        cdef char[IF_NAMESIZE] if_name

        msg_len = recvMsg(self.socket, self.buffer, sizeof(self.buffer), \
                          self.from_addr, <unsigned int>sizeof(self.from_addr), \
                          self.to_addr, <unsigned int>sizeof(self.to_addr), \
                          & self.if_idx,
                          int(blocking))

        res = if_indextoname(self.if_idx, if_name)

        if (res == NULL):
            raise Exception("if_indextoname() obtain the index name")

        if (msg_len == -1):
            raise Exception("no message received")

        return (self.buffer[:msg_len], self.from_addr, self.to_addr, if_name)

    def get_fd(self):
        """Return the file descriptor of the socket"""
        return self.socket

    def close(self):
        raise UserWarning("close() is not implemented for this type of socket")
