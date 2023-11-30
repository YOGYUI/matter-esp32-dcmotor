#! /usr/sh

if [[ "$OSTYPE" == "darwin"* ]]; then
    _script_path=$(dirname $(realpath $0))
else 
    _script_path=$(dirname $(realpath $BASH_SOURCE))
fi
_project_path=$(dirname ${_script_path})
# sdk_path=~/tools  # change to your own sdk path
_sdk_path=${_project_path}/sdk
_esp_idf_path=${_sdk_path}/esp-idf
_esp_matter_path=${_sdk_path}/esp-matter

# prepare environment (python, idf.py and esptool.py)
if [ -z "$IDF_PATH" ]; then
  source ${_esp_idf_path}/export.sh
fi
if [ -z "$ESP_MATTER_PATH" ]; then
  source ${_esp_matter_path}/export.sh
fi
export IDF_CCACHE_ENABLE=1

function add_to_path()
{
    new_dir=$1
    local IFS=:
    for d in $PATH
    do
        if [[ "$d" == "$new_dir" ]]
        then
            return 0
        fi
    done
    export PATH=$new_dir:$PATH
}
add_to_path ${_esp_matter_path}/connectedhomeip/connectedhomeip/out/host

# serial port configuration
if [[ "$OSTYPE" == "darwin"* ]]; then
    port=/dev/tty.usbserial-0001
else 
    port=/dev/ttyUSB0
fi
baud=921600

port_arg=0
baud_arg=0
monitor=false
for arg in "$@"
do
    if [[ "$arg" == "-p" ]]; then
        port_arg=1
    elif [[ "$arg" == "-b" ]]; then
        baud_arg=1
    elif [[ "$arg" == "-m" ]]; then
        monitor=true
    else
        if [[ $port_arg == 1 ]]; then
            port=$arg
            port_arg=0
        elif [[ $baud_arg == 1 ]]; then
            baud=$arg
            baud_arg=0
        fi
    fi
done

PARTITION_CSV_FILE_PATH=${_project_path}/partitions.csv
PARTITION_TABLE_OFFSET=$(python3 ${_script_path}/get_partition_table_offset.py)

function check_flash_encrypted()
{
    # argument
    # $1: serial port
    local value=$(python3 ${_script_path}/check_flash_encrypted.py -p $1)
    echo ${value}
}

function read_partition_info()
{
    # argument
    # $1: partition name string, $2: info type string (size/offset)
    local value=$(${python} ${_esp_idf_path}/components/partition_table/parttool.py \
                    --partition-table-offset ${PARTITION_TABLE_OFFSET} \
                    --partition-table-file ${PARTITION_CSV_FILE_PATH} \
                    get_partition_info --partition-name $1 --info $2)
    echo ${value}
}

function flash_partition_with_offset()
{
    # argument
    # $1: partition offset (hexa string), $2: binary file path to flash
    # $3: serial port, $4: baud
    echo "[flashing $2 to partition offset $1]"
    esptool.py \
        --chip esp32 \
        --port $3 \
        --baud $4 \
        --before default_reset \
        --after no_reset \
        write_flash \
        --force \
        --flash_mode dio \
        --flash_freq 40m \
        --flash_size keep \
        $5 \
        $1 $2
}

function flash_partition_with_offset_encrypted()
{
    flash_partition_with_offset $1 $2 $3 $4 --encrypt
}

function flash_partition()
{
    # argument
    # $1: partition name string, $2: binary file path to flash
    # $3: serial port, $4: baud
    local offset=$(read_partition_info $1 offset)
    echo "[flashing $2 to partition offset ${offset}]"
    esptool.py \
        --chip esp32 \
        --port $3 \
        --baud $4 \
        --before default_reset \
        --after no_reset \
        write_flash \
        --force \
        --flash_mode dio \
        --flash_freq 40m \
        --flash_size keep \
        $5 \
        ${offset} $2
}

function flash_partition_encrypted() 
{
    flash_partition $1 $2 $3 $4 --encrypt
}

function erase_partition() 
{
    # argument
    # $1: partition name string, $2: serial port
    local offset=$(read_partition_info $1 offset)
    local size=$(read_partition_info $1 size)
    echo "[erasing partition '$1' - offset: ${offset} size: ${size}]"
    esptool.py \
        --chip esp32 \
        --before default_reset \
        --after no_reset \
        --port $2 \
        erase_region \
        ${offset} ${size} \
        --force
}

function erase_partition_with_offset()
{
    # argument
    # $1: partition offset $2: partition size, $3: serial port
    echo "[erasing partition offset: $1 size: $2]"
    esptool.py \
        --chip esp32 \
        --before default_reset \
        --after no_reset \
        --port $3 \
        erase_region \
        $1 $2 \
        --force
}