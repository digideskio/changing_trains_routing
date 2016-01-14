#!/usr/bin/env bash


refresh_data()
{
  DATE=`date '+%s'`
  
  ./rewrite_request "`cat ../station_$ID/name.txt`" <all_data.ql >../station_$ID/request.ql
  wget -nv --post-file=../station_$ID/request.ql -O ../station_$ID/_$DATE http://dev.overpass-api.de/api_drolbr/interpreter

  FILESIZE=`stat -c '%s' ../station_$ID/_$DATE`
  if [[ $FILESIZE -lt 1000 ]]; then
    ./rewrite_request 1 "`cat ../station_$ID/name.txt`" <all_data_area.ql >../station_$ID/request_1.ql
    wget -nv --post-file=../station_$i/request_1.ql -O ../station_$ID/_$DATE http://overpass-api.de/api/interpreter
  fi

  FILESIZE=`stat -c '%s' ../station_$ID/_$DATE`
  if [[ $FILESIZE -gt 1000 ]]; then
    mv ../station_$ID/_$DATE ../station_$ID/data.osm
  else
    rm ../station_$ID/_$DATE
  fi
};


while [[ true ]]; do
  ID=1
  while [[ $ID -le 1015 ]]; do
    if [[ -a ../station_$ID/refresh ]]; then
      refresh_data
      rm ../station_$ID/refresh
    fi
    ID=$(($ID + 1))
  done
  sleep 30
done
