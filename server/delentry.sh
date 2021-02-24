#!/bin/bash

if [[ -z "$1" ]]; then
	echo "No idstr specified"
	exit 1
fi


REDIS_KEY="rpihb:entries:$1"
REDIS_CLI="/home/bmink/bin/redis-cli"

set -x

$REDIS_CLI del "$REDIS_KEY"

