#!/bin/bash


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/alphaTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/alphaTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/examples/


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaRouting.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaRouting.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Helper/saharaHelper.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/helper/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Helper/saharaHelper.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/helper/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/routingTable.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/routingTable.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/


cd /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/


./ns3 run scratch/alphaTest.cc --vis
