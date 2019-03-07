#!/bin/ksh

usage() {
  echo $0 "-f fileName -i idev -o odev"
  echo "  -f fileName  file to send"
  echo "  -i idev      input device"
  echo "  -o odev      output device"
  echo "example :"
  echo "send the file testFile.txt on the device ser2 and receive it on ser3"
  echo $0 -f testFile.txt -i ser2 -o ser3
}

# read options
while getopts "f:i:o:" switch; do
  case $switch in
  f)
    fileName=$OPTARG
    ;;
  i)
    inputName=$OPTARG
    inputDev="/dev/"$OPTARG
    ;;
  o)
    outputName=$OPTARG
    outputDev="/dev/"$OPTARG
    ;;
  *)
    usage
    exit 1;;
  esac
done

# exit if one of the three options is not defined
if [ X$fileName = "X" -o X$inputDev = "X" -o X$outputDev = "X" ] ; then
  usage
  exit 1
fi

tmpFile="/tmp/file_${outputName}_${inputName}.txt"
if [ ! -e $fileName ] ; then
  echo "file" $fileName "not found"
  usage
  exit 1
fi

if [ ! -c $inputDev ] ; then
  echo "device" $inputDev "not found or is not a char device"
  usage
  exit 1
fi

if [ ! -c $outputDev ] ; then
  echo "device" $outputDev "not found or is not a char device"
  usage
  exit 1
fi

#set min and time then the cat will end if there are no more input on the line
stty min=00 time=10 < $inputDev
stty min=00 time=10 < $outputDev

stty +flush < $inputDev
stty +flush < $outputDev

# get datas from input device and store them in temporary file
# store the process ID to kill this task after all the data have been send
cat < $inputDev > $tmpFile &

# send the datas on the output device
cat $fileName > $outputDev

# wait the end of cat
wait

stty +sane < $inputDev
stty +sane < $outputDev

# check if we received what we send or not
diff -q $tmpFile $fileName
returnDiff=$?
if [ $returnDiff != 0 ] ; then
  exit 1
fi

rm $tmpFile

# we are here ? Then it's a good news
echo $inputName $outputName ok
exit 0

