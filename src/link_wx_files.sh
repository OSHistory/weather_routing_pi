#!/bin/bash 

WX_DIR="/usr/include/wx-3.0/wx"

for header in $(ls $WX_DIR); do 
    full_path=$WX_DIR"/"$header
    ln -s $full_path "wx/"$header_base
done 
