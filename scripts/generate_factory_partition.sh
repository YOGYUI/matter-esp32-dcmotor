#! /usr/sh

wd=${PWD}
if [[ "$OSTYPE" == "darwin"* ]]; then
    script_path=$(dirname $(realpath $0))
else 
    script_path=$(dirname $(realpath $BASH_SOURCE))
fi
project_path=$(dirname ${script_path})
source ${project_path}/scripts/common.sh
chip_path=${_esp_matter_path}/connectedhomeip/connectedhomeip

VENDOR_ID=FFF2
PRODUCT_ID=8004
PASSCODE=25802580
DISCRIMINATOR=1234

# step.1: create CD(Certification Declaration) file
bin_path=${chip_path}/out/host
key_pem_path=${project_path}/resource/Attestation/PAI-${VENDOR_ID}-${PRODUCT_ID}-Key.pem
cert_pem_path=${project_path}/resource/Attestation/PAI-${VENDOR_ID}-${PRODUCT_ID}-Cert.pem
cd_out_path=${project_path}/resource/Attestation/CD-${VENDOR_ID}-${PRODUCT_ID}.der

cd ${bin_path}
./chip-cert gen-cd \
    -f 1 -l 0 -i 0 -n 1 -t 0 \
    -V 0x${VENDOR_ID} \
    -p 0x${PRODUCT_ID} \
    -d ${DEVICE_TYPE_ID} \
    -c "CSA00000SWC00000-01" \
    -K ${key_pem_path} \
    -C ${cert_pem_path} \
    -O ${cd_out_path}

# step.2: create factory partition
mfg_tool_path=${_esp_matter_path}/tools/mfg_tool
cd ${mfg_tool_path}
./mfg_tool.py \
    -cn "YOGUYI DCMotorFan" \
    -v 0x${VENDOR_ID} \
    -p 0x${PRODUCT_ID} \
    --pai \
    -k ${key_pem_path} \
    -c ${cert_pem_path} \
    -cd ${cd_out_path} \
    --passcode ${PASSCODE} \
    --discriminator ${DISCRIMINATOR}

cd ${wd}