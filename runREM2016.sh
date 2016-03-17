#!/bin/bash

PS=" REM2016/FH24PEV3TwoResidential1
     REM2016/FH24PEV10OneResidential
     REM2016/FH24PEV10REDD1
     REM2016/FH24PEV10TwoResidential1
     REM2016/FH24PEV20OneResidential
     REM2016/FH24PEV20TwoResidential1
     REM2016/FH24PEV50OneResidential
     REM2016/FH24PEV50TwoResidential1
"

NINTERVALS="2
3
5
"

TIMELIMIT=" 5
10
20
"

for tL in $TIMELIMIT
do
	for nIntervals in $NINTERVALS
	do
	
	  for probl in $PS
	  do 
             echo "Resolvendo o problema $probl without MIP Start $nIntervals $tL"		
	    ./main $probl 0 $nIntervals $tL

            echo "Resolvendo o problema $probl with MIP Start $nIntervals $tL"	
	    ./main $probl 1 $nIntervals $tL
	  
	done
  done
done


