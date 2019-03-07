#!/bin/ksh

usage() {
  echo $0 "-f <file> [-c] [-n=<num>]"
  echo "  -c         continue on error"
  echo "  -f <file>  use <file> for test configuration"
  echo "  -n=<num>   number of runs (default endless)"
}

endTest() {
    loopTest=FALSE
    wait
}

testError() {
    if [[ $stopOnError == TRUE ]]; then
        loopTest=FALSE
    fi
}

trap 'endTest'   INT   #traps Interrupt (signal 2)
trap 'endTest'   TERM  #traps Terminated (signal 15)
trap 'endTest'   USR2  #traps User defined signal 2 (signal 17)

trap 'testError' USR1  #traps User defined signal 1 (signal 16)

loopTest=TRUE
stopOnError=TRUE
nbLoop=0

# read options
while getopts "cf:n:" switch; do
  case $switch in
  c)
    stopOnError=FALSE
    ;;
  f)
    cfgFileName=$OPTARG
    ;;
  n)
    nbLoop=$OPTARG
    ;;
  *)
    usage
    exit 1;;
  esac
done

# exit config file name is not defined
if [ X$cfgFileName = "X" ] ; then
  usage
  exit 1
fi

if [ ! -e $cfgFileName ] ; then
  echo "config file" $cfgFileName "not found"
  usage
  exit 1
fi

. ./$cfgFileName

loop=0
while [[ $loopTest == TRUE ]]; do 

  echo "test #" $loop

  cfgLoop=0
  while [[ $cfgLoop -lt ${#serTest1[*]} ]];do
    ./serialtestdir.sh $cfgLoop $cfgFileName &
    (( cfgLoop += 1 ))
  done
  wait

  (( loop += 1 ))
  if (( $nbLoop != 0 && $loop >= nbLoop )) ; then
      echo fin
      loopTest=FALSE
  fi

done



#while [[ $loopTest == TRUE ]]; do 

#  loop=0
#  while [[ $loop -lt ${#serTest1[*]} ]];do
#   ./serialtestdir.sh ${serTest1[$loop]} ${serTest2[$loop]} ${serTestDir[$loop]} "${serTestStty[$loop]}" &
#    (( loop += 1 ))
#  done
#  wait

#done
