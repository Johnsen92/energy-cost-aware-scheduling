\ENCODING=ISO-8859-1
\Problem name: IloCplex

Maximize
 obj: x1 + 3 x2 + x3
Subject To
 c1: x1 + x2 - Rgc1  = 0
 c2: x3 - Rgc2  = 0
Bounds
      x1 Free
      x2 Free
      x3 Free
 0 <= Rgc1 <= 20
 0 <= Rgc2 <= 30
End
