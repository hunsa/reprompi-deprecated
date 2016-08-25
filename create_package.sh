#! /bin/bash

echo "$1 $2"

FNAME=$1
SUF="tar.bz2"
NETGAUGE=netgauge_sync


if [ -e ${FNAME}.${SUF} ]
then
  echo "works great"
else
  echo "no source package"
  exit -1
fi

rm -r $2
mkdir -p $2
cp ${FNAME}.${SUF} $2
cd $2; tar xfvj $2/${FNAME}.${SUF}
rm ${FNAME}.${SUF}
mv -f ${FNAME}/src/reprompi_bench/sync/${NETGAUGE} .
tar cfvj ${FNAME}.${SUF} ${FNAME}
tar cfvj ${NETGAUGE}.${SUF} ${NETGAUGE}
rm -r $2/${FNAME} ${NETGAUGE}


