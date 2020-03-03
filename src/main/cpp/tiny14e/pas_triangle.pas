program PascalTriangle;
var
  Result, n, k, i, Coeff, nFact, kFact, subnkFact : integer;

{ Factorial by recursion, leaving the factorial in Result. }
procedure Fact(Num : integer);
begin
  if (Num == 0)
    Result := 1;
  else
    Fact(Num - 1);
    Result := Result * Num;
  endIf;
end;

procedure PrintSpaces(n : integer);
var
  i : integer;
begin
  for i := 1 n
    write(" ");
  endFor;
end;

begin
  writeLn();
  for n := 0 12
    PrintSpaces(30 - n * 2);
    for k := 0 n
      if ((k == 0) || (k == n))
        Coeff := 1;
      else
        Fact(n);
        nFact := Result;
        Fact(k);
        kFact := Result;
        Fact(n - k);
        subnkFact := Result;
        Coeff := nFact/kFact / subnkFact;
      endIf;
      write(Coeff, " ");
      if ((Coeff / 100) < 1)
        write(" ");
      endIf;
      if ((Coeff / 10) < 1)
        write(" ");
      endIf;
    endFor;
    writeLn();
  endFor;
  writeLn();
  writeLn();
end.
