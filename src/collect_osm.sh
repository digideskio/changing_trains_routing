#!/usr/bin/env bash

i=1
read line
while [[ -n $line ]]; do
  mkdir -p station_$i
  echo $line >station_$i/name.txt
  ./rewrite_request "$line" <all_data.ql >station_$i/request.ql
  wget --post-file=station_$i/request.ql -O station_$i/data.osm http://dev.overpass-api.de/api_drolbr/interpreter
  filesize=`stat -c '%s' station_$i/data.osm`
  if [[ $filesize -lt 1000 ]]; then
    ./rewrite_request 1 "$line" <all_data_area.ql >station_$i/request_1.ql
    wget --post-file=station_$i/request_1.ql -O station_$i/data.osm http://overpass-api.de/api/interpreter
  fi
  i=$(($i + 1))
  read line
done
