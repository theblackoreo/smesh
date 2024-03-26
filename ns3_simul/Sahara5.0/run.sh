#!/bin/bash


cp /media/psf/iCloud/smesh/ns3_simul/Sahara5.0/alphaTest.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/alphaTest.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/src/sahara/examples/


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaRouting.h /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaRouting.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Helper/saharaHelper.h /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/src/sahara/helper/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Helper/saharaHelper.cc /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/src/sahara/helper/


cd /home/parallels/Desktop/ns-allinone-3.40/ns-3.40/


./ns3 run scratch/alphaTest.cc

