#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <time.h>
#include "bstr.h"
#include "blog.h"
#include "hiredis_helper.h"
#include "entry.h"

int do_list(void);
int do_update(void);


int
main(int argc, char **argv)
{
	int	err;
	char	*val;
	int	ret;
	char	*execn;

	err = 0;

	printf("Content-Type: text/html;charset=UTF-8\n\n");
	printf("<html>\n");

        execn = basename(argv[0]);
        if(xstrempty(execn)) {
                printf("Could not get executable name\n");
		err = -1;
                goto end_label;
        }

	ret = blog_init(execn);
	if(ret != 0) {
                printf("Could not initialize logging\n");
		err = -1;
                goto end_label;
	}

	printf("<head><title>%s</title></head>\n", execn);
	printf("<body>\n");
	printf("<pre>\n");

	ret = hiredis_init();
	if(ret != 0) {
                printf("Could not connect to redis\n");
		err = -1;
                goto end_label;
	}

	val = getenv("REQUEST_METHOD");
	if(xstrempty(val)) {
		blogf("Could not get method\n");
		err = -1;
		goto end_label;
	}

	if(!xstrcmp(val, "GET")) {
		ret = do_list();
		if(ret != 0) {
			blogf("Could not display list: %s", strerror(ret));
		}
	} else 
	if(!xstrcmp(val, "POST")) {
		ret = do_update();
		if(ret == 0) {
			printf("Update successful.\n");
		} else {
			blogf("Could not update: %s", strerror(ret));
		} 
	} else {
		blogf("Unsupported HTTP method: %s", val);
		err = -1;
	}


end_label:

	if(err != 0) {
		printf("Error (check logs)\n");
	}

	hiredis_uninit();

	(void) blog_uninit();

	printf("</pre>\n</body>\n</html>\n");

	return err;
}


#define MAX_INPUT	10240

int
do_update(void)
{
	char	*ipaddr;
	char	*contlenstr;
	int	contlen;
	char	buf[MAX_INPUT];
	int	nread;
	entry_t	*entry;
	int	err;
	int	ret;

	err = 0;
	entry = NULL;

	ipaddr = getenv("REMOTE_ADDR");
	if(xstrempty(ipaddr)) {
		blogf("Could not get REMOTE_ADDR");
		err = ENOENT;
		goto end_label;
	}

	contlenstr = getenv("CONTENT_LENGTH");
	if(xstrempty(contlenstr)) {
		blogf("Could not get CONTENT_LENGTH");
		err = ENOENT;
		goto end_label;
	}

	contlen = atoi(contlenstr);
	if(contlen <= 0) {
		blogf("Invalid CONTENT_LENGTH");
		err = EINVAL;
		goto end_label;
	}

	if(contlen >= MAX_INPUT) {	/* >= so we leave space for zero
					 * termination */
		blogf("Post data too large.");
		err = EINVAL;
		goto end_label;
	}

	memset(buf, 0, MAX_INPUT);

	/* Read content-length bytes from stdin */
	nread = fread(buf, 1, contlen, stdin);
	if(nread < contlen) {
		blogf("Could not read CONTENT_LENGTH (%d) bytes.", contlen);
		err = EINVAL;
		goto end_label;
	}

	entry = entry_init_frompostdata(ipaddr, buf);
	if(entry == NULL) {
		blogf("Couldn't initialize entry from post data");
		err = ENOEXEC;
		goto end_label;
	}

	ret = entry_savetoredis(entry);
	if(ret != 0) {
		blogf("Could not save entry");
		err = ret;
		goto end_label;
	}

end_label:

	entry_uninit(&entry);	

	return err;
}


int
do_list(void)
{
	blist_t		*entries;
	int		err;
	entry_t		*entry;

	err = 0;
	entries = NULL;

	entries = entry_getall_fromredis();
	if(entries == NULL) {
		blogf("Could not get entries from redis");
		err = ENOEXEC;
		goto end_label;
	}


end_label:

	if(entries) {
		while(entries->bl_cnt > 0) {
			entry = (entry_t *) blist_rpop(entries);
			entry_uninit(&entry);
		}
		blist_uninit(&entries);
	}
	
	return err;
}

