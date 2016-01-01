#!/usr/bin/env bash

i=1
read line
while [[ -n $line ]]; do
  mkdir -p station_$i
  echo $line >station_$i/name.txt
  ./rewrite_request "$line" <alle_daten.ql >station_$i/request.ql
  wget --post-file=station_$i/request.ql -O station_$i/data.osm http://dev.overpass-api.de/api_drolbr/interpreter
  i=$(($i + 1))
  read line
done
