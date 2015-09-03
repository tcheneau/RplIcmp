A python module for handling ICMP messaging for the RPL protocol
================================================================

[Cython]:http://cython.org/

This module allows easy creation of ICMPv6 socket in Python tailored to send RPL messages. Non-RPL
message (message type != 155) are filtered out and are thus not received by the socket.
The RplSocket object contains a send() and receive() methods to send and
receives raw ICMP messages (i.e. you have to build the header by yourself).

How to install
--------------

The source directory contains a make file with a *rpm* target. This target
created a RPM package that will be located in the dist directory that makes
deployment of the library easier.

I'm not very sure what is the proper way to install Cython packages because I
use exclusively the RPM built for my own needs. My best guess is that the
following line should suffice:

	apt-get install libcap-dev # install dependancy
	make                       # to build the library files
    python setup.py install    # install the package

Dependencies
------------

* [Cython]
* [libcap](https://sites.google.com/site/fullycapable/)

Examples of code
----------------

### Building a simple RPL ICMP client

```python
import RplIcmp
import random
from time import sleep
r = RplIcmp.RplSocket("eth0")
message_header = "\x9b\x01\x00\x00"
while True:
	message = "".join([ chr(random.randrange(255)) for x in range(100) ])
	r.send("fe80::1", message_header + message)
	print "sent one message"
	sleep(0.1)
    ```


### Building a simple RPL ICMP server

```python
import RplIcmp
r = RplIcmp.RplSocket("eth0")
while True:
    (msg, from_node, to, iface) = r.receive()
    print "received a message from %s" % from_node
```

Caveats
-------

ICMPv6 filtering does not seem to work if the packet is actually smaller than
the ICMPv6 header size. Such kind of message will then be received and must be
handled by any application that uses this library.

Authors
-------

* Tony Cheneau (tony.cheneau@nist.gov or tony.cheneau@amnesiak.org)

Acknowledgment
--------------

This work was supported by the Secure Smart Grid project at the Advanced
Networking Technologies Division, NIST.

Conditions Of Use
-----------------

<em>This software was developed by employees of the National Institute of
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
this software.</em>
