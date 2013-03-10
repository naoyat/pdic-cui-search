#!/bin/sh
PART=part-$1
if [ ! -e $PART ] ; then
    cat ALL-pre.xml $1 ALL-post.xml > $PART
fi
MEM=1200m
java -Xms$MEM -Xmx$MEM -jar jing/bin/jing.jar ~/work/ddk/documents/DictionarySchema/AppleDictionarySchema.rng $PART # &> jing.out
