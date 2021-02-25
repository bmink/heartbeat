#include "entry.h"
#include <errno.h>
#include "bstr.h"
#include "blog.h"

static int getval(const char *, const char *, bstr_t *);
static entry_t *_entry_init(void);

entry_t *
entry_init_frompostdata(const char *ipaddr, const char *postdata)
{
	entry_t	*entry;
	int	err = 0;

	err = 0;
	entry = NULL;

	entry = _entry_init();
	if(entry == NULL) {
		err = ENOMEM;
		goto end_label;
	}


end_label:

	if(err != 0)
		entry_uninit(&entry);

	return entry;
}


entry_t *
entry_init_fromredis(const char *hostn)
{
	return NULL;
}


int
entry_savetoredis(entry_t *entry)
{
	return 0;
}




entry_t *
_entry_init(void)
{
	entry_t	*entry;
	int err;

	err = 0;

	entry = malloc(sizeof(entry_t));
	if(entry == NULL) {
		blogf("Couldn't allocate entry");
		err = ENOMEM;
		goto end_label;
	}

	memset(entry, 0, sizeof(entry_t));

	entry->en_name = binit();
	if(entry->en_name == NULL) {
		blogf("Could not allocate en_name");
		err = ENOMEM;
		goto end_label;
	}

	entry->en_ipaddr = binit();
	if(entry->en_name == NULL) {
		blogf("Could not allocate en_ipaddr");
		err = ENOMEM;
		goto end_label;
	}

	entry->en_free_outp = binit();
	if(entry->en_free_outp == NULL) {
		blogf("Could not allocate en_free_outp");
		err = ENOMEM;
		goto end_label;
	}

	entry->en_vmstat_outp = binit();
	if(entry->en_vmstat_outp == NULL) {
		blogf("Could not allocate en_vmstat_outp");
		err = ENOMEM;
		goto end_label;
	}

	entry->en_uptime_outp = binit();
	if(entry->en_uptime_outp == NULL) {
		blogf("Could not allocate en_uptime_outp");
		err = ENOMEM;
		goto end_label;
	}


end_label:
	if(err != 0)
		entry_uninit(&entry);

	return entry;
}


void
entry_uninit(entry_t **entryp)
{
	if(entryp == NULL || *entryp == NULL)
		return;

	buninit(&((*entryp)->en_name));
	buninit(&((*entryp)->en_ipaddr));
	buninit(&((*entryp)->en_free_outp));
	buninit(&((*entryp)->en_vmstat_outp));
	buninit(&((*entryp)->en_uptime_outp));

	free(*entryp);

	*entryp = NULL;

	return;
}


int
getval(const char *nam, const char *buf, bstr_t *dst)
{
	return 0;
}


