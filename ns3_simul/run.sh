#!/bin/bash


cd /home/fabiotagliani/Desktop/smesh
git pull

cp /home/fabiotagliani/Desktop/smesh/ns3_simul/test_ns3_wifi.cc /home/fabiotagliani/Desktop/ns-allinone-3.33/ns-3.33/scratch/

cd /home/fabiotagliani/Desktop/ns-allinone-3.33/ns-3.33/

./waf --run scratch/test_ns3_wifi.cc


