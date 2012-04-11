/*
 * Copyright (C) 2012 Kian Mohageri (kian.mohageri@gmail.com).
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include <netinet/ip_carp.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern const char *__progname;
static const char *revision = "0.1";

/* return values used by Nagios plugins */
enum {
	STATE_OK,
	STATE_WARNING,
	STATE_CRITICAL,
	STATE_UNKNOWN
};

/* valid carp(4) states */
static const char *carp_states[] = { CARP_STATES };

static void help(void);
static void usage(void);
static void version(void);

/* 
 * Query a carp device for current state.
 * Print a single line for use by Nagios and return
 * a value Nagios expects from plugins.
 */
int
main(int argc, char *argv[])
{
	struct ifreq    ifr;
	struct carpreq  carpr;
	const char      *expect;
	const char      *msg;
	const char      *ifname;
	const char      *state;
	int             ch, ret, s;

	expect = NULL;
	
	while ((ch = getopt(argc, argv, "hVe:")) != -1) {
		switch (ch) {
		case 'h':
			help();
			/* NOTREACHED */
		case 'V':
			version();
			exit(EXIT_FAILURE);
		case 'e':
			expect = optarg;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	
	if (argc != 1)
		usage();
	
	ifname = *argv;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		(void)printf("CARP UNKNOWN - socket creation failed\n");
		return (STATE_UNKNOWN);
	}
	
	(void)memset(&ifr, 0, sizeof(ifr));
	(void)memset(&carpr, 0, sizeof(carpr));

	(void)strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_data = (caddr_t)&carpr;

	if ((ioctl(s, SIOCGVH, (caddr_t)&ifr)) == -1) {
		(void)printf("CARP UNKNOWN - ioctl failed (SIOCGVH)\n");
		return (STATE_UNKNOWN);
	}

	if ((carpr.carpr_vhid < 1) || (carpr.carpr_vhid > 255)) {
		(void)printf("CARP UNKNOWN - bad vhid %d\n", carpr.carpr_vhid);
		return (STATE_UNKNOWN);
	}

	if (carpr.carpr_state > CARP_MAXSTATE) {
		(void)printf("CARP UNKNOWN - state: ???\n");
		return (STATE_UNKNOWN);
	}

	state = carp_states[carpr.carpr_state];

	if (expect && (strcasecmp(expect, state) != 0))
	{
		msg = "CRITICAL";
		ret = STATE_CRITICAL;
	}
	else
	{
		msg = "OK";
		ret = STATE_OK;
	}

	(void)printf("CARP %s - state %s (vhid %d advbase %d advskew %d)\n",
	    msg, state, carpr.carpr_vhid, carpr.carpr_advbase, carpr.carpr_advskew);

	return (ret);
}

static void
help(void)
{
	version();
	(void)fprintf(stderr, 
	    "\n"
	    "This plugin checks what state a given CARP interface is in.\n"
	    "\n"
	    "If an expected state is given and the current state doesn't\n"
	    "match it, the plugin returns CRITICAL.  If no expected state\n"
	    "is given, the plugin returns OK.  If there is a problem\n"
	    "retrieving the information, the plugin returns UNKNOWN.\n"
	    "\n");
	usage();
}

static void
usage(void)
{
	(void)fprintf(stderr, 
	    "Usage: %s [-hV] [-e state] interface\n"
	    "\t-h        - Print the full plugin help\n"
	    "\t-V        - Print the plugin version\n"
	    "\t-e state  - Critical when state is not this\n", __progname);
	exit(EXIT_FAILURE);
}

static void
version(void)
{
	(void)fprintf(stderr, "%s %s\n", __progname, revision);
}
