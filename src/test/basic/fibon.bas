10 INPUT N
11 A = 1
12 B = 1
20 IF N = 1 THEN goto 100
21 if n = 2 then goto 200
22 print A; " "; B; " ";
30 FOR I = 3 TO N
31   TMP = B
40   B = A + B
50   A = TMP
55   print b; " ";
60 NEXT I:print:END
100 PRINT A:END
200 print A; " "; B:END


