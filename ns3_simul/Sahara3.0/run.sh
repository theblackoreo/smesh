#!/bin/bash


cp /media/psf/iCloud/smesh/ns3_simul/Sahara3.0/test.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara3.0/Model/Sahara.h /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara3.0/Model/Sahara.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara3.0/Model/SaharaHelper.h /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara3.0/Model/SaharaHelper.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/




cd /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/

./ns3 run scratch/test.cc



