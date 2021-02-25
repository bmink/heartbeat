#!/bin/sh

URL=http://192.168.1.128/cgi-bin/rpihbsrv.cgi

HOSTNAME=`uname -n`
FREE_OUT=`free -h`
UPTIME_OUT=`uptime`
VMSTAT_OUT=`vmstat -S M`


echo "$POSTDATA"

echo -n "$POSTDATA" | curl -X POST -H "Content-Type: text/plain" --data-urlencode "hostn=$HOSTNAME" --data-urlencode "free_output=$FREE_OUT" --data-urlencode "uptime_output=$UPTIME_OUT" --data-urlencode "vmstat_output=$VMSTAT_OUT" "$URL"
