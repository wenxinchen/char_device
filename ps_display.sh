#!/bin/sh

PS=/bin/ps
PIDOF=/sbin/pidof
CHRT=/usr/bin/chrt
EXPR=/usr/bin/expr

if [ x"$1" = x"-a" ];then
	pid_array=`$PS -ef | awk '{print $2}' | grep -v "PID"`
elif [ x"$1" = x"-m" ];then
	$CHRT $2 -p $3 $4
	exit
else
	pid_array=`$PIDOF $1`
fi

if [ x"$pid_array" = x"" ];then
	printf "Usage: 1) %s -a                    (Display information of all the processes!)\n" $0
	printf "       2) %s -m -f/r priority tid  (Mofify the process/thread's priority)\n" $0
	printf "          f:SCHED_FIFO, r:SCHED_RR\n"
	printf "       3) %s process_name          (Display information of Only such process!)\n" $0
	exit
fi

printf "%-10s %-10s %-20s %-15s %-15s\n" "PID" "TID" "CMD" "Policy" "Priority"
for pid in ${pid_array}
do
	count=`$PS -L -p $pid | awk '{print NR}' | tail -n1`
	count=`$EXPR $count - 1`
	
	i=0;
	for line in `$PS -L -p $pid | awk '{print $1}'`
	do
		ppid[$i]=$line
		i=`$EXPR $i + 1`
	done
	
	i=0;
	for line in `$PS -L -p $pid | awk '{print $2}'`
	do
		ttid[$i]=$line
		i=`$EXPR $i + 1`
	done
		
	i=0;
	for line in `$PS -L -p $pid | awk '{print $5}'`
	do
		cmd[$i]=$line
		i=`$EXPR $i + 1`
	done
		
	for ((i=1; i<=count; i=i+1))
	do
		policy[$i]=`$CHRT -p ${ttid[$i]} | awk -F: '{print $2}' | sed -e 's/^ //g' | sed -n '1p'`
	done
	
	for ((i=1; i<=count; i=i+1))
	do
		priority[$i]=`$CHRT -p ${ttid[$i]} | awk -F: '{print $2}' | sed -e 's/^ //g' | sed -n '2p'`
	done
	
	
#	printf "%-10s %-10s %-20s %-15s %-15s\n" ${ppid[0]} "TID" ${cmd[0]} "Policy" "Priority"
	for ((i=1; i<=count; i=i+1))
	do
		printf "%-10s %-10s %-20s %-15s %-15s\n" ${ppid[$i]} ${ttid[$i]} ${cmd[$i]} ${policy[$i]} ${priority[$i]}
	done
	
done
#$CHRT -f/-r -p priority pid
