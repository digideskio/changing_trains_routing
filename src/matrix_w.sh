#!/usr/bin/env bash

wget -q -O ../data/_ https://adam.noncd.db.de/api/v1.0/facilities
mv ../data/_ ../data/elevator_state.json
./matrix
