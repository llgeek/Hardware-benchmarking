#! /bin/bash
#run_all.sh
#run all benchmark scripts
#

cpu=./cpu 
cpuavx=./cpuavx
gpu=./gpu 
memory=./memory
disk=./disk
network=./network

ignore=false

echo " start all benchmarks in default settings.."

echo " benchmark CPU.."

for opt in 0 1
do
	for thread in 1 2 4 8
	do
		$cpu -o$opt -t$thread
	done
	echo ""
done

echo " CPU done.."


echo " benchmark CPU with AVX.."
for opt in 0 1
do
	for thread in 1 2 4 8
	do
		$cpuavx -o$opt -t$thread
	done
	echo " "
done

echo " CPU AVX done.."



# echo " benchmark GPU.."

# echo " GPU done.."


# echo " benchmark memory.."
	
# for opt in 0 1 2
# do
# 	for thread in 1 2 4 8
# 	do 
# 		for blocksize in 8B 8KB 8MB 80MB
# 		do
# 			$memory -o$opt -t$thread -b$blocksize
# 		done
# 		echo " "
# 	done
# 	echo " "
# done

# echo " memory done.."


echo " benchmark disk.."
for opt in 0 1 2
do
	for thread in 1 2 4 8
	do
		for blocksize in 8B 8KB 8MB 80MB
		do
			if $ignore; then
				$disk -i -o$opt -t$thread -b$blocksize
			else
				$disk -o$opt -t$thread -b$blocksize
				let $ignore=true
			fi
		done
		echo " "
	done
	echo " "
done

echo " disk done.."



## Because you have to start the server and client separately, 
# so the bash cannot support batch processing
# 
#
# echo " benchmark network.."
# for opt in 0 1
# do
# 	for thread in 1 2 4 8
# 	do
# 		for metric in 0 1
# 		do
# 			$network -o$opt -t$thread -m$metric
# 		done
# 		echo " "
# 	done
# 	echo " "
# done

# echo "network done.."

 echo "Pass all test cases..."
 echo "Done!"


