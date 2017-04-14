#!/bin/sh

run=`ps w|grep countFiles.sh |wc -l`
#echo $run

if [ "$run" -gt 3 ];then
	#echo "counting ..."
	return
fi

#echo $1
#total=`du $1 -a|wc -l`
#real=$(($total-1))
#echo total=$total $real
path=/tmp/state/status

du /tmp/mnt/USB-disk-1/hack -a | 
while read line
do
	case $line in
	*.mp4) 
		m=$(($m+1))
		sed -e '/^lv_n=.*/d' -i $path
		echo "lv_n=$m">>$path
	 ;;
	esac
done

public_total=`du /tmp/mnt/USB-disk-1/public/ -s |awk -F " " '{print $1}'`
#echo "p=$public_total"
last_p_t=`cat $path |grep p_t |awk -F "=" '{print $2}'`
if [ "$last_p_t" == "$public_total" ];then

        return;

fi
sed -e '/^p_t=.*/d' -i $path
echo "p_t=$public_total">>$path


sed -e '/^p_n=.*/d' -i $path
sed -e '/^v_n=.*/d' -i $path
sed -e '/^o_n=.*/d' -i $path
#$1=/tmp/mnt/USB-disk-1/public
du $1 -a |
while read line
do
	#echo $line
	lowercase=$(echo $line | tr '[A-Z]' '[a-z]');
	#echo $lowercase > /dev/console
	case $lowercase in
	*.jpg) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.jpeg) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.bmp) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.png) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.ai) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.raw) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.tiff) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.gif) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;
	*.cdr) 
		j=$(($j+1))
		sed -e '/^p_n=.*/d' -i $path
		echo "p_n=$j">>$path
	 ;;

	*.rmvb)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.mov)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.wmv)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.mpeg)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.flv)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.avi)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.mkv)
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	 ;;
	*.mp4) 
		i=$(($i+1))
		sed -e '/^v_n=.*/d' -i $path
		echo "v_n=$i">>$path
	;;
	*.*)
		k=$(($k+1))
		sed -e '/^o_n=.*/d' -i $path
		echo "o_n=$k">>$path

	;; 
	esac
done
sync;
