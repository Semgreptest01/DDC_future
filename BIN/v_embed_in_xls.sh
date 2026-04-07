#!/bin/bash

pushd $PROD_DIR/BIN/lib/ >& /dev/null
java -Xmx1024M ed.embed_in_xls.EmbedInXls $@
if [ $? -ne 0 ] ; then
 exit 1
else
 exit 0
fi
