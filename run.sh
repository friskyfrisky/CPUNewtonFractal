#!/bin/bash

#set dir to where script is
dir=$(cd -P -- "$(dirname -- "$0")" && pwd -P)

if [ -d "EasyBMP" ] 
then
	echo "EasyBMP installed in dir"
else
	echo "installing EasyBMP in dir"
	git clone "https://github.com/izanbf1803/EasyBMP"
fi

if [ -d "frames" ]
then
	echo "frames folder exists..."
else
	echo "making frames folder"
	mkdir frames
fi
if ! command -v mogrify
then
	echo "install imagemagick"
	exit
fi

#recomplie every time
g++ newt.cpp -lpthread -O3
c=0
for ((i=1000; i<9000; i++)); do
	time ./a.out $i
	mogrify -resize 1920x1080 -format jpg out.bmp
	rm out.bmp
	#mv out.jpg $(printf "%02d"$i)out.jpg
	mv "out.jpg" "./frames/out$(echo $c).jpg"
	let "c++"
done

rm a.out
#we will recompile anyway
