#!/bin/sh

IDFILE=/home/bmink/.idstr
URL=http://192.168.1.128/cgi-bin/rpihbsrv.cgi

if [ ! -f "$IDFILE" ]; then
        echo "No file found at $IDFILE"
        exit 1
fi

IDSTR=`cat $IDFILE`
FREE_OUT=`free -h`
UPTIME_OUT=`uptime`
VMSTAT_OUT=`vmstat -S M`


echo "$POSTDATA"

echo -n "$POSTDATA" | curl -X POST -H "Content-Type: text/plain" --data-urlencode "idstr=$IDSTR" --data-urlencode "free_output=$FREE" --data-urlencode "uptime_output=$UPTIME_OUT" --data-urlencode "vmstat_output=$VMSTAT_OUT" "$URL"
