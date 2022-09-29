#!/bin/bash
#
cnt=0
max=$1
testUrl=$2
#
if [[ $max == "" ]]; then
    max=3
fi
if [[ $testUrl == "" ]]; then
    testUrl="http://192.168.4.1/"
fi
#
echo "Starting testlagserv with max: "$max" and testUrl: "$testUrl
#
while [ 1 ]; do
    echo $cnt".) testlagserv running..."
    #
    #curl -I $testUrl # Test HEAD only
    #curl $testUrl    # Test with a lot of infos similar to browser
    # Test POST request with body
    curl -X POST http://192.168.4.1/up=true -H "Content-Type: application/json" -d '{"id":123,"name":"test"}'
    #curl -X POST http://192.168.4.1/reset -H "Content-Type: application/json" -d '{"id":123,"name":"test"}'
    #
    let cnt=$cnt+1
    if [[ $cnt -ge $max ]]; then
        exit 1
    fi
    sleep 1
done
