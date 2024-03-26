#!/bin/bash


cp /media/psf/iCloud/smesh/ns3_simul/test_ns3_wifi.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/

cd /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/

./ns3 run scratch/test_ns3_wifi.cc --vis




