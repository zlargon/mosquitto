/*
Copyright (c) 2009, Roger Light <roger@atchoo.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of mosquitto nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>

#include <mqtt3.h>

/* Options for logging should be:
 *
 * A combination of:
 * Via syslog
 * To a file
 * To stdout/stderr
 * To topics
 */

/* Give option of logging timestamp.
 * Logging pid.
 */
static int log_destinations = 0;
static int max_level = 0;

int mqtt3_log_init(int level, int destinations)
{
	int rc = 0;

	max_level = level;
	log_destinations = destinations;

	if(log_destinations & LOG_SYSLOG){
		openlog("mosquitto", LOG_PID, LOG_DAEMON);
	}
	/* FIXME - do something for all destinations! */

	return rc;
}

int mqtt3_log_close(void)
{
	if(log_destinations & LOG_SYSLOG){
		closelog();
	}
	/* FIXME - do something for all destinations! */

	return 0;
}

int mqtt3_log_printf(int level, const char *fmt, ...)
{
	va_list va;
	char s[500];

	if(level <= max_level){
		va_start(va, fmt);
		vsnprintf(s, 500, fmt, va);
		va_end(va);

		/* FIXME - do something! */
	}

	return 0;
}

