#!/bin/bash

if [ $# -eq 0 ]; then
  echo "増幅回数の指定は必須です。" 1>&2
  exit 1
fi

cnt=""
echo $1 | grep -E '^[1-9][0-9]*$' > /dev/null
[ $? -eq 0 ] && cnt=$1

if [ -z $cnt ]; then
  echo "許されない引数が指定されました。" 1>&2
  exit 1
fi

file_path=$2
if [ -p /dev/stdin ]; then
  tmp_file_path=$(mktemp)
  trap "rm -f $tmp_file_path" EXIT
  file_path=$tmp_file_path
  cat - > $tmp_file_path
fi

eval seq 1 $cnt | eval sed -e "'s|[[:alnum:]]*|$file_path|g'" | xargs cat 2>/dev/null

if [ $? -ne 0 ]; then
  echo "$file_path: そのようなファイルやディレクトリはありません" 1>&2
  exit 1
fi
