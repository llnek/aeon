10 INPUT N
11 A = 1
12 B = 1
20 IF N = 1 OR N = 2 THEN 70
30 FOR I = 3 TO N
31   TMP = B
40   B = A + B
50   A = TMP
60 NEXT I
70 PRINT B
80 END