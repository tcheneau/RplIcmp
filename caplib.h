/* Tony Cheneau <tony.cheneau@nist.gov> */
/* this code is heavily borrowed from the iputils project */


#include <sys/prctl.h>
#include <sys/capability.h>

extern int limit_capabilities(void);
extern int drop_capabilities(void);

static int enable_capability_raw(void);
static int disable_capability_raw(void);
static int enable_capability_admin(void);
static int disable_capability_admin(void);

extern int modify_capability(cap_value_t, cap_flag_value_t);
static inline int enable_capability_raw(void)		{ return modify_capability(CAP_NET_RAW,   CAP_SET);   }
static inline int disable_capability_raw(void)		{ return modify_capability(CAP_NET_RAW,   CAP_CLEAR); }
static inline int enable_capability_admin(void)		{ return modify_capability(CAP_NET_ADMIN, CAP_SET);   }
static inline int disable_capability_admin(void)	{ return modify_capability(CAP_NET_ADMIN, CAP_CLEAR); }

