#!/bin/bash

#ffmpeg -i out.mp4 -ss 0 -t 58 seg1.mp4

end="$2"
begin="0"
length="66"
# 66 / 3
iter="22"

while [[ $begin -lt $end ]];
do
    video_name=`uuidgen`
    ffmpeg -i "$1" -ss $begin -t $length "$video_name.mp4"
    echo "$video_name.mp4, $begin" >> files.txt
    begin=`echo "$begin + $iter" | bc`
done
