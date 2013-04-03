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

/* this code is heavily borrowed from the iputils project */

#include "caplib.h"

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

#include <asm-generic/errno-base.h>

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))


static int uid;
static uid_t euid;

int limit_capabilities(void)
{
	cap_t cap_p;
	const cap_value_t caps[] = {
		CAP_NET_ADMIN,
		CAP_NET_RAW,
	};
	int i;

	cap_p = cap_init();
	if (!cap_p) {
		perror("cap_get_proc");
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(caps); i++) {
		if (cap_clear(cap_p) < 0) {
			perror("cap_clear");
			return -1;
		}

		if (cap_set_flag(cap_p, CAP_PERMITTED, ARRAY_SIZE(caps) - i, caps + i, CAP_SET) < 0) {
			perror("cap_set_flag");
			return -1;
		}

		if (cap_set_proc(cap_p) < 0)
			continue;

		break;
	}

	if (i == ARRAY_SIZE(caps)) {
		perror("cap_set_proc");
		if (errno != EPERM)
			return -1;
	}

	if (prctl(PR_SET_KEEPCAPS, 1) < 0) {
		perror("prctl");
		return -1;
	}

	if (setuid(getuid()) < 0) {
		perror("setuid");
		return -1;
	}

	if (prctl(PR_SET_KEEPCAPS, 0) < 0) {
		perror("prctl");
		return -1;
	}

	cap_free(cap_p);

	uid = getuid();
	euid = geteuid();

	return 0;
}

int modify_capability(cap_value_t cap, cap_flag_value_t on)
{
	cap_t cap_p = cap_get_proc();

	if (!cap_p) {
		perror("cap_get_proc");
		return -1;
	}

	if (cap_set_flag(cap_p, CAP_EFFECTIVE, 1, &cap, on) < 0) {
		perror("cap_set_flag");
		return -1;
	}

	if (cap_set_proc(cap_p) < 0) {
		perror("cap_set_proc");
		return -1;
	}

	if (cap_free(cap_p) < 0) {
		perror("cap_free");
		return -1;
	}

	return 0;
}


int drop_capabilities(void)
{
	cap_t cap = cap_init();
	if (cap_set_proc(cap) < 0) {
		perror("cap_set_proc");
		return -1;
	}
	cap_free(cap);
	return 0;
}
