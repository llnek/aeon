#!/usr/bin/env basic

10 RADIUS=10
20 FOR I=1 TO RADIUS-1
30 W=INT(RADIUS*SIN(180/RADIUS*I*3.1415/180))
40 PRINT SPC(RADIUS-W);:FOR J=1 TO 2*W:PRINT "*";:NEXT J:PRINT
50 NEXT I
