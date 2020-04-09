rem 100 if 10 > 20 and 10 > 3 then a = 3 + 4 else a = 999
rem 200 gosub 400
rem 300 print "a= "; a
rem 350 end
rem 400 print "called sub"
rem 500 return

100 dim a(1,2)
200 a(0,0)=10:a(0,1)=20:a(0,2)=30:a(1,0)=40:a(1,1)=50:a(1,2)=60
rem 300 print "poo =", a(0,0),a(0,1),a(0,2),a(1,0),a(1,1),a(1,2)
300 print "poo =", a(0,2) + a(1,1) * a(1,2)

