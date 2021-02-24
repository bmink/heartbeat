#!/bin/sh

# Purges old (>2 minute old) entries from redis

MINDATE=`date --date "2 minutes ago" +%s`
REDIS_KEY="rpihb:entries"
REDIS_CLI="/home/bmink/bin/redis-cli"

$REDIS_CLI ZREMRANGEBYSCORE "$REDIS_KEY" 0 "$MINDATE"

