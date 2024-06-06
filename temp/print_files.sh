#!/bin/bash

function is_valid() {
    
    if [[ $filename == "Makefile" ]] 
    then 
        return 0
    fi 

    list="cpp c h asm ld sh"
    delimiter=" "
    list_whitespaces=`echo $list | tr "$delimiter" " "`
    
    for x in $list_whitespaces; do 
        if [ "$x" = "$extension" ] 
        then 
            return 0
        fi 
    done 

    return 1
}

cd ~/rdk-os
files=$(find . -type f)

for file in $files
do
    filename=$(basename -- $file)
    extension="${filename##*.}"
    filename="${filename%.*}" 

    if is_valid; then 
        echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
        echo $file 
        echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
        cat  $file

    fi

done
