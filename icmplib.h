/* Tony Cheneau <tony.cheneau@nist.gov> */

int init_capability(void);

int dropCapabilities(void);

int createSocket(char * iface);

int sendMsg(int icmp_sock, char * target, void * msg, size_t msg_len);

int recvMsg(int icmp_sock, char * buffer, size_t buflen, char * from, socklen_t from_len, char * to, socklen_t to_len, unsigned int * if_idx, int blocking);

void initICMPFilter(struct icmp6_filter * filter);

void allowTypeICMPfilter(struct icmp6_filter * filter, int type);

int enableICMPfilter(int icmp_sock, struct icmp6_filter * filter);