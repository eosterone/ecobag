#!/bin/bash

if [[ $# -ne 1 ]]; then
	echo "USAGE: buildContract.sh <name> within the directory"
	exit 1
fi

NAME=$1

eosiocpp -o ${NAME}.wast ${NAME}.cpp &&
eosiocpp -g ${NAME}.abi ${NAME}.hpp &&
cleos set contract ${NAME} ../${NAME}

