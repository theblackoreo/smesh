#!/bin/bash


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/alphaTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/alphaStatic.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/betaStatic.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/manet-routing-compare.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/ddos.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/mobilityTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/mobility.csv /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/mob.py /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/



cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaMobility.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaMobility.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/alphaTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/examples/


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaRouting.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaRouting.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Helper/saharaHelper.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/helper/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Helper/saharaHelper.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/helper/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/routingTable.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/routingTable.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaPacket.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaPacket.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/


cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaQueue.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaQueue.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaSync.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaSync.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaBlock.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaBlock.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaCrypto.h /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/Model/saharaCrypto.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/src/sahara/model/

cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/nodeCompromizedTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/
cp /media/psf/iCloud/smesh/ns3_simul/Sahara4.0/nodeCompromizedTest.cc /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/scratch/

cd /home/parallels/Downloads/ns-allinone-3.40/ns-3.40/

clear
cd scratch/
python3 mob.py
cd ..
./ns3 run scratch/alphaTest.cc --vis