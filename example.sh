#!/bin/bash

# Example script to use ezmlm-stats to generate an html stats page

es=/usr/local/bin/ezmlm-stats
listdir=/home/ngwlist/list
htmloutput=stats.html
pngdir=.

lastmonth=`date -u -d "1 month ago" +"%d %b %Y %H:%M:%S -0000"`

tmpdir=.
full_trafficfile=${tmpdir}/full_traffic.txt
full_senderfile=${tmpdir}/full_senders.txt
last_trafficfile=${tmpdir}/lastmonth_traffic.txt
last_senderfile=${tmpdir}/lastmonth_senders.txt


# note: I use the startdate option to skip an initial spike of subscriptions when the list was created
# when I imported the subscriber list from the older package I was using.

$es --startdate "13 May 2000 00:00:00 -0000" --traffic $full_trafficfile --senders $full_senderfile $listdir

# Import the ##VAR=value items from the traffic file, strip the ## and evaluate.

eval `grep "^##" $full_trafficfile  | sed "s/^##//"`

gnuplot <<EOF
set timefmt "%m/%d/%Y"
set key left
set xtics rotate
set y2tics autofreq
set terminal png small
set size 1.5,.75

# Do full daily stats
set format x "  %m/%d/%Y"
set xdata time
set xrange ["$GROUP_DAILY_STARTDATE":"$GROUP_DAILY_ENDDATE"]
#set xtics "$GROUP_DAILY_STARTDATE"
set xtics 2592000
set mxtics 2
set output "${pngdir}/daily_full.png"
plot	"$full_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_MSGCOUNT smooth bezier axes x1y1 title "Message count <-left scale" with lines lt 0, \
	"$full_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_POP smooth bezier axes x1y2 title "NGW List Population right scale->" with lines lt 1, \
	"$full_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_REG_POP smooth bezier axes x1y2 title "Regular subscribers right scale->" with line lt 2, \
	"$full_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_DIG_POP smooth bezier axes x1y2 title "Digest subscribers right scale->" with line lt 3


# Do the hourly stats
set xdata
set xrange [0:23]
set xtics ("  Midnight" 0, "1 am" 1, "2 am" 2, "3 am" 3, "4 am" 4, "5 am" 5, "6 am" 6, "7 am" 7, "8 am" 8, "9 am" 9, "10 am" 10, "11 am" 11, "Noon" 12, "1 pm" 13, "2 pm" 14, "3 pm" 15, "4 pm" 16, "5 pm" 17, "6 pm" 18, "7 pm" 19, "8 pm" 20, "9 pm" 21, "10 pm" 22, "11 pm" 23)
set format x "  %g"
set output "${pngdir}/hour.png"
plot	"$full_trafficfile" every :::$GROUP_HOUR_START::$GROUP_HOUR_END using $COL_DATE:$COL_MSGCOUNT title "Message traffic <-left scale" with boxes lt 0, \
	"$full_trafficfile" every :::$GROUP_HOUR_START::$GROUP_HOUR_END using $COL_DATE:$COL_POP axes x1y2 title "Subscriber deltas right scale->" with lines


set xrange [0:6]
set xtics ("  Sun" 0, "Mon" 1, "Tue" 2, "Wed" 3, "Thu" 4, "Fri" 5, "Sat" 6)
set format x "  %g"
set output "${pngdir}/week.png"
plot	"$full_trafficfile" every :::$GROUP_WEEK_START::$GROUP_WEEK_END using $COL_DATE:$COL_MSGCOUNT title "Message traffic <-left scale" with boxes lt 0, \
	"$full_trafficfile" every :::$GROUP_WEEK_START::$GROUP_WEEK_END using $COL_DATE:$COL_POP axes x1y2 title "Subscriber deltas right scale->" with lines


EOF

#
# Do last month stats
#
$es --startdate "$lastmonth" --traffic $last_trafficfile --senders $last_senderfile $listdir

eval `grep "^##" $last_trafficfile  | sed "s/^##//"`

gnuplot <<EOF
set timefmt "%m/%d/%Y"
set key left
set xtics rotate
#set mytics 10
set y2tics autofreq
set terminal png small
set size 1.5,.75

# Do full daily stats
set format x "  %m/%d/%Y"
set xdata time
set xrange ["$GROUP_DAILY_STARTDATE":"$GROUP_DAILY_ENDDATE"]
#set xtics "$GROUP_DAILY_STARTDATE"
#set xtics 2592000
set mxtics 2
set y2range [0:*]
set output "${pngdir}/daily_lastmonth.png"
plot	"$last_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_MSGCOUNT axes x1y1 title "Message count <-left scale" with lines lt 0, \
	"$last_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_POP smooth bezier axes x1y2 title "NGW List Population right scale->" with lines lt 1, \
	"$last_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_REG_POP smooth bezier axes x1y2 title "Regular subscribers right scale->" with line lt 2, \
	"$last_trafficfile" every :::$GROUP_DAILY_START::$GROUP_DAILY_END using $COL_DATE:$COL_DIG_POP smooth bezier axes x1y2 title "Digest subscribers right scale->" with line lt 3


# Do the hourly stats
set xdata
set xrange [0:23]
set xtics ("  Midnight" 0, "1 am" 1, "2 am" 2, "3 am" 3, "4 am" 4, "5 am" 5, "6 am" 6, "7 am" 7, "8 am" 8, "9 am" 9, "10 am" 10, "11 am" 11, "Noon" 12, "1 pm" 13, "2 pm" 14, "3 pm" 15, "4 pm" 16, "5 pm" 17, "6 pm" 18, "7 pm" 19, "8 pm" 20, "9 pm" 21, "10 pm" 22, "11 pm" 23)
set format x "  %g"
set output "${pngdir}/hour_lastmonth.png"
plot	"$last_trafficfile" every :::$GROUP_HOUR_START::$GROUP_HOUR_END using $COL_DATE:$COL_MSGCOUNT title "Message traffic <-left scale" with boxes lt 0, \
	"$last_trafficfile" every :::$GROUP_HOUR_START::$GROUP_HOUR_END using $COL_DATE:$COL_POP axes x1y2 title "Subscriber deltas right scale->" with lines


set xrange [0:6]
set xtics ("  Sun" 0, "Mon" 1, "Tue" 2, "Wed" 3, "Thu" 4, "Fri" 5, "Sat" 6)
set format x "  %g"
set output "${pngdir}/week_lastmonth.png"
plot	"$last_trafficfile" every :::$GROUP_WEEK_START::$GROUP_WEEK_END using $COL_DATE:$COL_MSGCOUNT title "Message traffic <-left scale" with boxes lt 0, \
	"$last_trafficfile" every :::$GROUP_WEEK_START::$GROUP_WEEK_END using $COL_DATE:$COL_POP axes x1y2 title "Subscriber deltas right scale->" with lines


EOF

topten_full=`head -10 $full_senderfile | sed "s/^\([0-9]*\) \(.*\)$/<tr><td>\1<\/td><td>\2<\/td><\/tr>/"`
topten_last=`head -10 $last_senderfile | sed "s/^\([0-9]*\) \(.*\)$/<tr><td>\1<\/td><td>\2<\/td><\/tr>/"`

cat - > $htmloutput <<EOF
<html>
<head>
<title>My ezmlm-stats</title>
</head>
<body>
<p>
Last month message and population stats:<p>
<table border=0>
<tr><th>Msg Count</th><th>Name</th></tr>
${topten_last}
</table>
<p>
<img src="daily_lastmonth.png"><p>
<hr>
<img src="hour_lastmonth.png"><p>
<hr>
<img src="week_lastmonth.png"><p>
<hr>
Total message and population stats:<p>
<table border=0>
<tr><th>Msg Count</th><th>Name</th></tr>
${topten_full}
</table>
<p>
<img src="daily_full.png"><p>
<hr>
<img src="hour.png"><p>
<hr>
<img src="week.png"><p>
</body>
</html>

EOF