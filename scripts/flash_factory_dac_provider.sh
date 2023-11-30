#! /usr/sh

wd=${PWD}
if [[ "$OSTYPE" == "darwin"* ]]; then
    script_path=$(dirname $(realpath $0))
else
    script_path=$(dirname $(realpath $BASH_SOURCE))
fi
project_path=$(dirname ${script_path})
source ${project_path}/scripts/common.sh

binary_path=${project_path}/resource/DACProvider/partition.bin
flash_partition nvs ${binary_path} ${port} ${baud}

cd ${wd}