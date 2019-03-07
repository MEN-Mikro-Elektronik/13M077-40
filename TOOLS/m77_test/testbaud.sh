#!/bin/ksh

usage() {
  echo $0 "device [-h] -o odev"
  echo "  -h       add high baudrate"
  echo "  -o odev  output device"
}

end() {
# restore saved config
    stty $currentConfig < $outputDev
    exit 1
}

trap 'end' 2 #traps Ctrl-C (signal 2)

baudList="300 1200 2400 4800 9600 19200 38400 57600 115200"

# read options
while getopts "ho:" switch; do
  case $switch in
  h)
    baudList=$baudList" 230400 576000 1152000"
    ;;
  o)
    outputDev="/dev/"$OPTARG
    ;;
  *)
    usage
    exit 1
    ;;
  esac
done

if [ ! -c $outputDev ] ; then
  echo "device" $outputDev "not found or is not a char device"
  exit 1
fi

# save current config
currentConfig=`stty -g < $outputDev`

for baudrate in $baudList ; do
    echo -n "set baudrate to $baudrate (1bit = "
    if (( $baudrate < 115200 )) ; then
        echo -n $((1000000/$baudrate))
        echo "us)"
    else
        echo -n $((1000000000/$baudrate))
        echo "ns)"
    fi
    stty baud=$baudrate < $outputDev
    read
    echo -n U > $outputDev
done

end
