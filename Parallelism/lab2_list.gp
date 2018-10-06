#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

#Plot 1
set title "Throughput of Synchronized List Operations"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'Mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/$7) \
	title 'Spin-lock' with linespoints lc rgb 'green'

#Plot 2
set title "Wait-for-Lock Time w/ Mutexes"
set xlabel "Threads"
set logscale x 2
set ylabel "Avg Lock Wait Time/Avg Operation Time"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | grep -v 'list-none-m,12,1000'" using ($2):($7) \
	title 'Avg Operation Time' with points lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | grep -v 'list-none-m,12,1000'" using ($2):($8) \
	title 'Avg Lock Wait Time' with points lc rgb 'blue'

#Plot 3
set title "Successful Iterations vs Threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'

plot \
    "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	with points lc rgb "green" title "w/o synchronization", \
    "< grep list-id-m lab2b_list.csv" using ($2):($3) \
        with points lc rgb "red" title "Mutex", \
    "< grep list-id-s lab2b_list.csv" using ($2):($3) \
        with points lc rgb "blue" title "Spin-lock"

#Plot 4
set title "Throughput of List Operations w/ Mutexes"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_4.png'

plot \
    "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv | grep -Ev 'list-none-m,16|list-none-m,24'" using ($2):(1000000000/$7) \
       title '1 List' with linespoints lc rgb 'red', \
    "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv | grep -Ev 'list-none-m,16|list-none-m,24'" using ($2):(1000000000/$7) \
       title '4 Lists' with linespoints lc rgb 'blue', \
    "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv | grep -Ev 'list-none-m,16|list-none-m,24'" using ($2):(1000000000/$7) \
       title '8 Lists' with linespoints lc rgb 'green', \
    "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv | grep -Ev 'list-none-m,16|list-none-m,24'" using ($2):(1000000000/$7) \
        title '16 Lists' with linespoints lc rgb 'violet'

#Plot 5
set title "Throughput of List Operations w/ Spin-Locks"
set xlabel "Threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_5.png'

plot \
    "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv | grep -Ev 'list-none-s,16|list-none-s,24'" using ($2):(1000000000/$7) \
       title '1 List' with linespoints lc rgb 'red', \
    "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv | grep -Ev 'list-none-s,16|list-none-s,24'" using ($2):(1000000000/$7) \
       title '4 Lists' with linespoints lc rgb 'blue', \
    "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv | grep -Ev 'list-none-s,16|list-none-s,24'" using ($2):(1000000000/$7) \
       title '8 Lists' with linespoints lc rgb 'green', \
    "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv | grep -Ev 'list-none-s,16|list-none-s,24'" using ($2):(1000000000/$7) \
	title '16 Lists' with linespoints lc rgb 'violet'