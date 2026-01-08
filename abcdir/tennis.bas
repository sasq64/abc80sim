1 REM *********************************
2 REM * T E N N I S   f|r 1 person    *
3 REM * av  PETER STAHL               *
4 REM * f|r ABC 80.                   *
5 REM *********************************
6 ; CHR$(12%,7%) : FOR I=1% TO 500% : NEXT I
7 ; " * T E N N I S   f|r  E N  person * "
8 ; CHR$(151);STRING$(39%,44%)
9 ; : ; " Med detta spel kan EN person spela      Tennis. "
10 ; " Anv{nd f|ljande tangenter f|r man|vrer- ing:"
11 ; : ; " (A) = K|r om spelet och NOLL-st{ller          po{ngen."
12 ; " (X) = H|jer  SPELAREN UPP]T."
13 ; " (Z) = S{nker SPELAREN NED]T."
14 ; " (R) STARTAR om helt och h}llet. [ven    SV]RIGHETSgrad kan v{ljas om."
15 ; : ; " Po{ngen r{knas n{r BOLLen sl}r i        respektive V[GG."
16 ; : ; " Ange HASTIGHET P] BOLLEN:"
17 ; " (1)=SNABBAST    -      (9)=L]NGSAMMAST"
18 ONERRORGOTO 18 : INPUT M1% : IF M1%<1% OR M1%>9% THEN 18
19 ; " Vill du ha EXTRA sv}rt?"
20 ; " (1)-(9) ger flera hinder (1 - 9 st)"
21 ; " (0) = Jag vill INTE ha det sv}rare!"
22 ONERRORGOTO 22 : INPUT M2% : IF M2%<0% OR M2%>9% THEN 22
23 IF M2%=0% THEN 28
24 ; : ; " Vill du ha det {nnu SV]RARE?"
25 ; " (1)=Jag vill ha [nnu SV]RARE."
26 ; " (0)=Jag vill INTE ha sv}rare."
27 ONERRORGOTO 27 : INPUT M4% : IF M4%<0% OR M4%>1% THEN 27
28 DIM M1%(20)
29 ON M1% GOSUB 31,32,33,34,35,36,37,38,39,40
30 M1%=2%*M1%-1 : GOTO 41
31 M1%(1%)=6% : M1%(2%)=0% : RETURN
32 M1%(3%)=8% : M1%(4%)=2% : RETURN
33 M1%(5%)=10% : M1%(6%)=4% : RETURN
34 M1%(7%)=12% : M1%(8%)=5% : RETURN
35 M1%(9%)=14% : M1%(10%)=6% : RETURN
36 M1%(11%)=17% : M1%(12%)=8% : RETURN
37 M1%(13%)=22% : M1%(14%)=11% : RETURN
38 M1%(15%)=25% : M1%(16%)=13% : RETURN
39 M1%(17%)=28% : M1%(18%)=15% : RETURN
40 M1%(19%)=32% : M1%(20%)=17% : RETURN
41 P3%=ASC('X') : P4%=ASC('Z')
42 ; CHR$(12%,7%)
43 FOR I%=1% TO 24% : ; CHR$(151%) : NEXT I%
44 ; CUR(0%,O%)CHR$(135%)
45 FOR I%=2% TO 79% STEP 77% : FOR J%=4% TO 68% : SETDOT J%,I% : NEXT J% : NEXT I%
46 FOR I%=4% TO 68% STEP 64% : FOR J%=2% TO 79% : SETDOT I%,J% : NEXT J% : NEXT I%
47 A%=68% : B1%=30% : B2%=40% : FOR I%=B1% TO B2% : SETDOT I%,A% : NEXT I%
48 X%=40% : Y%=10% : Z%=1% : N%=0% : N1%=0%
49 M3%=25% : GOSUB 50 : IF M4%=0% THEN 64 : M3%=50% : GOSUB 50 : GOTO 64
50 FOR I%=1 TO M2%
51 ON I% GOSUB 53,54,55,56,57,58,59,60,61,62
52 GOTO 63
53 FOR J%=1 TO 3% : SETDOT 35%+J%,M3% : NEXT J% : RETURN
54 FOR J%=1 TO 3% : SETDOT 42%+J%,M3% : NEXT J% : RETURN
55 FOR J%=1 TO 3% : SETDOT 28%+J%,M3% : NEXT J% : RETURN
56 FOR J%=1 TO 3% : SETDOT 49%+J%,M3% : NEXT J% : RETURN
57 FOR J%=1 TO 3% : SETDOT 19%+J%,M3% : NEXT J% : RETURN
58 FOR J%=1 TO 3% : SETDOT 55%+J%,M3% : NEXT J% : RETURN
59 FOR J%=1 TO 3% : SETDOT 15%+J%,M3% : NEXT J% : RETURN
60 FOR J%=1 TO 3% : SETDOT 60%+J%,M3% : NEXT J% : RETURN
61 FOR J%=1 TO 3% : SETDOT 9%+J%,M3% : NEXT J% : RETURN
62 FOR J%=1 TO 3% : SETDOT 64%+J%,M3% : NEXT J% : RETURN
63 NEXT I% : RETURN
64 SETDOT X%,Y%
65 ; CUR(0%,1%);" Tryck n{r du {r KLAR !"; : GET Q$ : GET Q$ : ; CUR(0%,1%);SPACE$(38%)
66 ; CUR(0%,12%)"(R)=OM:(A)=NOLL"
67 REM * *
68 IF INP(56%) AND 128% THEN 69 ELSE 71
69 U%=INP(56%)-128%
70 IF U%=ASC('A') THEN 42 ELSE IF U%=ASC('R') THEN 6 ELSE IF U%=P3% THEN 127 ELSE IF U%=P4% THEN 132
71 FOR I=1% TO M1%(M1%) : NEXT I : U%=0%
72 ON Z% RESTORE 119,120,121,122,123,124,125,126
73 READ Q%,Q1% : X1%=X%+Q% : Y1%=Y%+Q1%
74 G%=Z% : IF DOT(X1%,Y1%) THEN 79
75 GOTO 78
76 IF G%=Z% THEN 77 ELSE 78
77 CLRDOT X%,Y% : X%=X%+2% : Y%=Y%+2%
78 CLRDOT X%,Y% : X%=X1% : Y%=Y1% : SETDOT X%,Y% : GOTO 67
79 ON Z% RESTORE 111,112,113,114,115,116,117,118
80 READ X2%,Y2%,Z2%,Z3%,Z4%,X3%,Y3%,Z5%,Z8%,Z9%,L%
81 P%=DOT(X1%+X2%,Y1%+Y2%) : P1%=DOT(X1%+X3%,Y1%+Y3%) : P2%=U%
82 IF P2%>128% THEN P2%=P2%-128%
83 IF Y1%<>A% THEN 96
84 IF X1%<B1% OR X1%>B2% THEN 96
85 IF X1%<>B1% AND X1%<>B2% THEN 88
86 IF X1%<>B1% THEN 87 : Z%=Z9% : GOTO 109
87 Z%=L% : GOTO 109
88 IF P2%<>P3% AND P2%<>P4% THEN 96
89 IF P%<>-1% THEN 94
90 IF P2%<>P3% THEN 92 : IF Z%=3% OR Z%=5% THEN T%=1% ELSE T%=0%
91 Z%=Z3% : GOTO 109
92 IF Z%=1 OR Z%=7 THEN T%=2% ELSE T%=0%
93 Z%=Z4% : GOTO 109
94 IF P2%=P3% THEN Z%=Z4% ELSE Z%=Z5%
95 GOTO 109
96 IF Y1%<>2% AND Y1%<>79% AND Y1%<>25% AND Y1%<>50% THEN 104
97 IF T%=0% THEN 104 ELSE IF T%=1% THEN 98 ELSE IF T%=2% THEN 101 ELSE 104
98 IF Y1%=79% THEN 100
99 Z%=3% : T%=0% : GOTO 109
100 Z%=5% : T%=0% : GOTO 109
101 IF Y1%<>2% THEN 103
102 Z%=1% : T%=0% : GOTO 109
103 Z%=7% : T%=0% : GOTO 109
104 IF P%<>-1% THEN 105 : Z%=Z2% : GOTO 109
105 IF P1%<>-1% THEN 106 : Z%=Z5% : GOTO 109
106 IF P%=-1% AND P1%=-1% THEN 107 ELSE 108
107 Z%=Z8% : GOTO 109
108 Z%=Z2% : GOTO 109
109 IF Y1%=79% THEN N%=N%+1% ELSE IF Y1%=2% THEN N1%=N1%+1%
110 ; CHR$(7%);CUR(0%,5%)N1%;CUR(0%,30%)N% : GOTO 72
111 DATA -1,0,7,7,6,0,-1,3,5,7,3
112 DATA 0,0,6,7,5,0,0,6,5,1,3
113 DATA -1,0,5,6,5,0,-1,1,7,1,5
114 DATA 0,0,8,8,8,0,0,8,8,8,8
115 DATA 1,0,3,2,3,0,1,7,1,7,3
116 DATA 0,0,2,1,3,0,0,2,3,7,5
117 DATA 1,0,1,1,2,0,1,5,3,1,5
118 DATA 0,0,4,4,4,0,0,4,4,4,4
119 DATA -1,1
120 DATA 0,1
121 DATA 1,1
122 DATA 1,0
123 DATA 1,-1
124 DATA 0,-1
125 DATA -1,-1
126 DATA -1,0
127 FOR I=1% TO M1%(M1%+1%) : NEXT I
128 B3%=B1%-1% : B4%=B2%-1% : IF B3%>4% THEN 130
129 B3%=B1% : B4%=B2%
130 SETDOT B3%,A% : CLRDOT B2%,A% : B1%=B3% : B2%=B4%
131 GOTO 72
132 FOR I=1% TO M1%(M1%+1%) : NEXT I
133 B3%=B1%+1% : B4%=B2%+1% : IF B4%<68% THEN 135
134 B3%=B1% : B4%=B2%
135 SETDOT B4%,A% : CLRDOT B1%,A% : B1%=B3% : B2%=B4%
136 GOTO 72
