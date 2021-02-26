#include "entry.h"
#include <errno.h>
#include "bstr.h"
#include "blog.h"

static int getval(const char *, const char *, bstr_t *);
static entry_t *_entry_init(void);

entry_t *
entry_init_frompostdata(const char *ipaddr, const char *postdata)
{
	entry_t		*entry;
	int		err;
	int		ret;

	err = 0;
	entry = NULL;

	if(xstrempty(ipaddr) || xstrempty(postdata)) {
		err = EINVAL;
		goto end_label;
	}

	entry = _entry_init();
	if(entry == NULL) {
		err = ENOMEM;
		goto end_label;
	}

	ret = getval("hostn", postdata, entry->en_name);
	if(ret != 0) {
		blogf("Could not get hostn");
		err = ret;
		goto end_label;
	}

	bstrcat(entry->en_ipaddr, ipaddr);

	ret = getval("free_output", postdata, entry->en_free_outp);
	if(ret != 0) {
		blogf("Could not get free_output");
		err = ret;
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
	const char	*cur;
	bstr_t		*srchstr;
	int		err;
	int		pos;
	bstr_t		*encoded;
	int		ret;

	err = 0;
	srchstr = NULL;

	if(xstrempty(nam) || xstrempty(buf) || dst == NULL)
		return EINVAL;

	srchstr = binit();
	if(srchstr == NULL) {
		blogf("Could not allocate srchstr");
		err = ENOMEM;
		goto end_label;
	}

	bprintf(srchstr, "%s=", nam);

	pos = xstrstr(buf, bget(srchstr));
	if(pos < 0) {
		blogf("Field not found: %s", nam);
		err = ENOMEM;
		goto end_label;
	}

	cur = buf + pos + bstrlen(srchstr);

	encoded = binit();
	if(encoded == NULL) {
		blogf("Could not allocate encoded");
		err = ENOMEM;
		goto end_label;
	}

	for(; *cur && *cur != '&'; ++cur) {
		bputc(encoded, *cur);
	}

	printf("encoded: %s\n", bget(encoded));

	ret = burldecode(bget(encoded), dst);
	if(ret != 0) {
		blogf("Could not urldecode value: %s", strerror(ret));
		err = ret;
		goto end_label;
	}
	
	printf("decoded: %s\n", bget(dst));

end_label:

	buninit(&srchstr);
	buninit(&encoded);

	return err;
}


