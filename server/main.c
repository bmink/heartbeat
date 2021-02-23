#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <time.h>
#include "bstr.h"
#include "blog.h"
#include "hiredis_helper.h"

int do_list(void);
int do_update(void);

#define REDIS_KEY	"rpihb:entries"

int
main(int argc, char **argv)
{
	int	err;
	char	*val;
	int	ret;
	char	*execn;

	err = 0;

	printf("Content-Type: text/plain;charset=UTF-8\n\n");

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
		if(ret != 0) {
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

	return err;
}


#define MAX_INPUT	4096

int
do_update(void)
{
	char	*val;
	int	contlen;
	char	buf[MAX_INPUT];
	int	nread;
	bstr_t	*entry;
	int	err;
	int	ret;

	err = 0;
	entry = NULL;

	entry = binit();
	if(entry == NULL) {
		blogf("Could not allocate entry");
		return ENOMEM;
	}

	val = getenv("REMOTE_ADDR");
	if(xstrempty(val)) {
		blogf("Could not get REMOTE_ADDR");
		err = ENOENT;
		goto end_label;
	}

	bstrcat(entry, val);
	bprintf(entry, ": ");

	val = getenv("CONTENT_LENGTH");
	if(xstrempty(val)) {
		blogf("Could not get CONTENT_LENGTH");
		err = ENOENT;
		goto end_label;
	}

	contlen = atoi(val);
	if(contlen <= 0) {
		blogf("Invalid CONTENT_LENGTH");
		err = EINVAL;
		goto end_label;
	}

	/* Read content-length bytes from stdin */
	nread = fread(buf, 1, contlen, stdin);
	if(nread < contlen) {
		blogf("Could not read CONTENT_LENGTH (%d) bytes.", contlen);
		err = EINVAL;
		goto end_label;
	}

	bmemcat(entry, buf, nread);
	bprintf(entry, "\n");

	ret = hiredis_zadd(REDIS_KEY, time(NULL), entry, NULL);
	if(ret != 0) {
		blogf("Could not add entry to redis");
		err = ret;
		goto end_label;
	}

	printf("Update successful.\n");

end_label:

	buninit(&entry);	

	return err;
}


int
do_list(void)
{
	return 0;
}

