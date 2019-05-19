#!/bin/sh

TMP_FOLDER=$(mktemp -d)
NAME_IN=test
NAME_OUT=test_out
URI=https://github.com/todoesverso/lv2s/todoes-amp#mono
LV2FILE=/home/todoesverso/repos/lv2file/lv2file


# Create files
FILE_IN=${TMP_FOLDER}/${NAME_IN}.wav
FILE_OUT=${TMP_FOLDER}/${NAME_OUT}.wav
sox -V -r 48000 -n -b 16 ${FILE_IN} synth 0.1 sin 100 vol -3dB
sox -V ${FILE_IN} -n stats

# Process
LV2_PATH=$(pwd)/plugin ${LV2FILE} --list 
LV2_PATH=$(pwd)/plugin ${LV2FILE} -n ${URI}
LV2_PATH=$(pwd)/plugin ${LV2FILE} -i ${FILE_IN} -o ${FILE_OUT} -c1:in -pgain:3 ${URI}
sox ${FILE_IN} -n stats
sox ${FILE_OUT} -n stats

#audacity ${FILE_IN} ${FILE_OUT}

# Clean 
rm -fr ${TMP_FOLDER}
