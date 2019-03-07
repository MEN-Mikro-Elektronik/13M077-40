#!/bin/ksh

if [ ! -e $2 ] ; then
  echo "config file" $2 "not found"
  kill -s USR2 $PPID
  exit 1
fi

. ./$2

ser1=${serTest1[$1]}
ser2=${serTest2[$1]}
serDir=${serTestDir[$1]}

sttyLoop=0
while [[ $sttyLoop -lt ${#serTestStty[*]} ]];do

  stty ${serTestStty[$sttyLoop]} < /dev/ser$ser1
  stty ${serTestStty[$sttyLoop]} < /dev/ser$ser2

  case $serDir in
  0)
    echo "serial test ${serialArray[$ser1]} --> ${serialArray[$ser2]} ${serTestStty[$sttyLoop]}"
    ./serialtest.sh -o ser$ser1 -i ser$ser2 -f $testFile
    testReturn=$?
    ;;
  1)
    echo "serial test ${serialArray[$ser1]} --> ${serialArray[$ser2]} ${serTestStty[$sttyLoop]}"
    ./serialtest.sh -o ser$ser1 -i ser$ser2 -f $testFile
    testReturn=$?
    if [ $testReturn == 0 ] ; then
      echo "serial test ${serialArray[$ser2]} --> ${serialArray[$ser1]} ${serTestStty[$sttyLoop]}"
      ./serialtest.sh -o ser$ser2 -i ser$ser1 -f $testFile
      testReturn=$?
    fi
    ;;
  2)
    echo "serial test ${serialArray[$ser1]} <-> ${serialArray[$ser2]} ${serTestStty[$sttyLoop]}"
    { ./serialtest.sh -o ser$ser1 -i ser$ser2 -f $testFile; echo $? > /tmp/res.$$.tst1; }&
    { ./serialtest.sh -o ser$ser2 -i ser$ser1 -f $testFile; echo $? > /tmp/res.$$.tst2; }&
    wait
    tst1=`cat /tmp/res.$$.tst1`
    tst2=`cat /tmp/res.$$.tst2`
    rm /tmp/res.$$.tst1 /tmp/res.$$.tst2
    (( testReturn = tst1 + tst2 ))
    ;;
  esac

  if [ $testReturn != 0 ] ; then
    echo "!! Error during serial test !!"
    kill -s USR1 $PPID
    exit 1
  fi

  (( sttyLoop += 1 ))
done
