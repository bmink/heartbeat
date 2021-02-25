#ifndef ENTRY_H
#define ENTRY_H

#include <time.h>
#include "bstr.h"

typedef struct entry {
	bstr_t	*en_name;
	bstr_t	*en_ipaddr;
	time_t	en_lasthb;

	bstr_t	*en_free_outp;
	bstr_t	*en_vmstat_outp;
	bstr_t	*en_uptime_outp;
} entry_t;

entry_t *entry_init_frompostdata(const char *, const char *);
entry_t *entry_init_fromredis(const char *);
int entry_savetoredis(entry_t *);
void entry_uninit(entry_t **);

#endif
