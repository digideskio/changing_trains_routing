#!/usr/bin/env bash

DATE=`date '+%s'`
wget -q -O ../data/_$DATE https://adam.noncd.db.de/api/v1.0/facilities
mv ../data/_$DATE ../data/elevator_state.json
./matrix

ID=`./matrix --output=id`
touch ../station_$ID/refresh
chmod 666 ../station_$ID/refresh
