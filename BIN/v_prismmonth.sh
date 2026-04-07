#!/bin/bash

if [ $# -eq 0 ]; then
  echo "引数は指定されていません。" 1>&2
  exit 1
fi

opt=""
echo $1 | grep -E '^[0-9]{4}[01][0-9]$' > /dev/null
[ $? -eq 0 ] && opt=month
echo $1 | grep -E '^[0-9]{4}[01][0-9][0-3][0-9]$' > /dev/null
[ $? -eq 0 ] && opt=day

if [ -z $opt ]; then
  echo "許されない引数が指定されました。" 1>&2
  exit 1
fi

if [ $opt == "month" ]; then
  stmon=$1
  edmon=$(v_ldate $1m/+1)
  # 出力
  cat $MDWEEK_FILE | v_cutcol 1 END | awk $stmon'01<=$2&&$1<='$edmon'06' | v_horiarray KEY=END | v_cutcol 1 END:5-END
elif [ $opt == "day" ]; then
  v_ldate $1/-1 | v_ldate -w 1 | v_ldate -y2 2w | v_choosecol 3 | cut -c-6
else
  echo option error 1>&2
  exit 1
fi
