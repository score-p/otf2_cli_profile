#!/usr/bin/gnuplot -persist

set key left top Right

### small ###

set terminal png enhanced 18
set output "dat.png"

set xlabel "time [s]"
set ylabel "OTF read progress [%]"

# set logscale x
# set logscale y

plot [:][0:] \
	'8000x1300K_32_10000.txt'  using ($3):($4)  t "byte progress with 32 file handles [%]" w l, \
	'8000x1300K_32_10000.txt'  using ($3):($5)  t "time progress 32 file handles [%]" w l, \
	'8000x1300K_1000_10000.txt'  using ($3):($4)  t "byte progress 1000 file handles [%]" w l, \
	'8000x1300K_1000_10000.txt'  using ($3):($5)  t "time progress 1000 file handles [%]" w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($4) w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($5) w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($4) w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($5) w l

set terminal png enhanced 18
set output "dat2.png"

set xlabel "time [s]"
set ylabel "OTF read progress [%]"

# set logscale x
# set logscale y

plot [:][0:] \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($4) w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($5) w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($4) w l, \
	'2000x248Kzipped_1000_10000.txt'  using ($3):($5) w l

