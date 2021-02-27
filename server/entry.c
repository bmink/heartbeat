#include "entry.h"
#include <errno.h>
#include "bstr.h"
#include "blog.h"
#include "cJSON.h"
#include "cJSON_helper.h"
#include "hiredis_helper.h"

static int getval(const char *, const char *, bstr_t *);
static entry_t *_entry_init(void);

#define REDIS_KEYPREF	"rpihb:entries:"

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

	ret = getval("hostn", postdata, entry->en_hostn);
	if(ret != 0) {
		blogf("Could not get hostn");
		err = ret;
		goto end_label;
	}

	bstrcat(entry->en_ipaddr, ipaddr);
	entry->en_lasthb = time(NULL);

	ret = getval("free_output", postdata, entry->en_free_outp);
	if(ret != 0) {
		blogf("Could not get free_output");
		err = ret;
		goto end_label;
	}

	ret = getval("vmstat_output", postdata, entry->en_vmstat_outp);
	if(ret != 0) {
		blogf("Could not get vmstat_output");
		err = ret;
		goto end_label;
	}

	ret = getval("uptime_output", postdata, entry->en_uptime_outp);
	if(ret != 0) {
		blogf("Could not get uptime_output");
		err = ret;
		goto end_label;
	}

	ret = getval("df_output", postdata, entry->en_df_outp);
	if(ret != 0) {
		blogf("Could not get df_output");
		err = ret;
		goto end_label;
	}

	ret = getval("logs_output", postdata, entry->en_logs_outp);
	if(ret != 0) {
		blogf("Could not get logs_output");
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
	cJSON	*entryj;
	cJSON	*child;
	int	err;
	char	*rendered;
	bstr_t	*key;
	int	ret;

	if(entry == NULL)
		return EINVAL;

	err = 0;
	entryj = NULL;
	rendered = NULL;
	key = NULL;

	entryj = cJSON_CreateObject();
	if(entryj == NULL) {
		blogf("Couldn't create JSON object");
		err = ENOMEM;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "hostn", bget(entry->en_hostn));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "ipaddr",
	    bget(entry->en_ipaddr));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddNumberToObject(entryj, "lasthb", entry->en_lasthb);
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "free_outp",
	    bget(entry->en_free_outp));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "vmstat_outp",
	    bget(entry->en_vmstat_outp));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "uptime_outp",
	    bget(entry->en_uptime_outp));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "df_outp",
	    bget(entry->en_df_outp));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	child = cJSON_AddStringToObject(entryj, "logs_outp",
	    bget(entry->en_logs_outp));
	if(child == NULL) {
		err = ENOEXEC;
		goto end_label;
	}

	rendered = cJSON_Print(entryj);
	if(xstrempty(rendered)) {
		blogf("Could not render into string");
		err = ENOEXEC;
		goto end_label;
	}

#if 0
	printf("%s\n", rendered);
#endif

	key = binit();
	if(key == NULL) {
		blogf("Could not allocate key");
		err = ENOMEM;
		goto end_label;
	}

	bprintf(key, "%s%s", REDIS_KEYPREF, bget(entry->en_hostn));

	ret = hiredis_set(bget(key), rendered);
	if(ret != 0) {
		blogf("Could not store entry in redis");
		err = ret;
		goto end_label;
	}
	
	
end_label:

	if(entryj)
		cJSON_Delete(entryj);
	if(rendered)
		free(rendered);
	buninit(&key);

	return err;
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

	entry->en_hostn = binit();
	if(entry->en_hostn == NULL) {
		blogf("Could not allocate en_hostn");
		err = ENOMEM;
		goto end_label;
	}

	entry->en_ipaddr = binit();
	if(entry->en_ipaddr == NULL) {
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

	entry->en_df_outp = binit();
	if(entry->en_df_outp == NULL) {
		blogf("Could not allocate en_df_outp");
		err = ENOMEM;
		goto end_label;
	}

	entry->en_logs_outp = binit();
	if(entry->en_logs_outp == NULL) {
		blogf("Could not allocate en_logs_outp");
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

	buninit(&((*entryp)->en_hostn));
	buninit(&((*entryp)->en_ipaddr));
	buninit(&((*entryp)->en_free_outp));
	buninit(&((*entryp)->en_vmstat_outp));
	buninit(&((*entryp)->en_uptime_outp));
	buninit(&((*entryp)->en_df_outp));
	buninit(&((*entryp)->en_logs_outp));

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

#if 0
	printf("encoded: %s\n", bget(encoded));
#endif

	ret = burldecode(bget(encoded), dst);
	if(ret != 0) {
		blogf("Could not urldecode value: %s", strerror(ret));
		err = ret;
		goto end_label;
	}
	
#if 0
	printf("decoded: %s\n", bget(dst));
#endif

end_label:

	buninit(&srchstr);
	buninit(&encoded);

	return err;
}


