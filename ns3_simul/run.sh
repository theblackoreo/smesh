#!/bin/bash

cp /Users/fabiotagliani/Desktop/smesh/ns3_simul/test_ns3_wifi.cc /Users/fabiotagliani/Desktop/workspace/ns-3-dev/scratch/

cd /Users/fabiotagliani/Desktop/workspace/ns-3-dev/

./ns3 build

./ns3 run scratch/test_ns3_wifi.cc --vis

