#!/usr/bin/env bash


refresh_data()
{
  DATE=`date '+%s'`
  
  ./rewrite_request "`cat ../station_$ID/name.txt`" <all_stations.ql >../station_$ID/request_nodes.ql
  wget -nv --post-file=../station_$ID/request_nodes.ql -O ../station_$ID/_$DATE http://dev.overpass-api.de/api_drolbr/interpreter
  cat ../station_$ID/_$DATE | awk '{ if (result == "" || length(result) > length($0)) result = $0; } END { if (result != "") print result; }' >../station_$ID/osm_name.txt
  
  if [[ -s ../station_$ID/osm_name.txt ]]; then
    ./rewrite_request 100 "`cat ../station_$ID/osm_name.txt`" <all_data.ql >../station_$ID/request.ql
    wget -nv --post-file=../station_$ID/request.ql -O ../station_$ID/_$DATE http://dev.overpass-api.de/api_drolbr/interpreter
  else
    ./rewrite_request 1 "`cat ../station_$ID/name.txt`" <all_stations_area.ql >../station_$ID/request_nodes_1.ql
    wget -nv --post-file=../station_$ID/request_nodes_1.ql -O ../station_$ID/_$DATE http://overpass-api.de/api/interpreter
    cat ../station_$ID/_$DATE | awk '{ if (result == "" || length(result) > length($0)) result = $0; } END { if (result != "") print result; }' >../station_$ID/osm_name.txt
    
    ./rewrite_request 101 "`cat ../station_$ID/name.txt`" "`cat ../station_$ID/osm_name.txt`" <all_data_area.ql >../station_$ID/request_1.ql
    wget -nv --post-file=../station_$ID/request_1.ql -O ../station_$ID/_$DATE http://overpass-api.de/api/interpreter    
  fi

  FILESIZE=`stat -c '%s' ../station_$ID/_$DATE`
  if [[ $FILESIZE -gt 1000 ]]; then
    mv ../station_$ID/_$DATE ../station_$ID/data.osm
  else
    rm ../station_$ID/_$DATE
  fi
  
  ./matrix --output=stats --id=$ID >../station_$ID/stats.tsv
};


while [[ true ]]; do
  ID=1
  OLDEST_ID=1
  while [[ $ID -le 1015 ]]; do
    if [[ -a ../station_$ID/refresh ]]; then
      refresh_data
      rm ../station_$ID/refresh
    fi
    if [[ ../station_$ID/data.osm -ot ../station_$OLDEST_ID/data.osm ]]; then
      OLDEST_ID=$ID
    fi
    ID=$(($ID + 1))
  done
  ID=$OLDEST_ID
  refresh_data
  sleep 30
done
