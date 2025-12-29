1 REM Ins{nd av Lennart Einarsson <6554>    1986-05-30 17.58.54
10 REM *** DOTRACE ***
20 REM * LARS G R\STLUND * 790430 *
30 REM * ABC80 och 40tkn
40 IF PEEK(590%)<>40% ; INP(3%)
50 ; CHR$(12)
60 ; CUR(3,6)"**************************"
70 ; CUR(4,6)"*                        *"
80 ; CUR(5,6);"* V[LKOMMEN TILL DOTRACE *"
90 ; CUR(6,6);"*                        *"
100 ; CUR(7,6);"**************************"
110 ; CUR(10,1);"V[NSTRA SPELAREN HAR DEN \VRE DOTTEN"
120 ; CUR(14,6);"V[NSTER             H\GER"
130 ; CUR(17,9)"W                  ]"
140 ; "        A S                \ ["
150 ; "         Z                  -"
160 ; : ; : ; TAB(12)"TRYCK P] RET.";
170 GET A$
180 R1%=8% : K1%=40% : H%=0% : M%=0% : S%=0% : G%=0% : V1%=0% : V2%=0% : V3%=0% : D1%=0% : D2%=0% : ; CHR$(12)
190 ; CUR(2,17);"V[LJ !"
200 ; CUR(8,5);"EN H\GER-SPELARE MED"
210 ; CUR(9,5);"BANA A,B,C   (J/N) ? ";
220 ; CUR(13,5);"BANA              HASTIGHET"
230 ; CUR(15,5);"* A.  BANA A      * 1.  PROFFS"
240 ; "     * B.  BANA B      * 2.  NORMAL"
250 ; "     * C.  BANA C      * 3.  AMAT\R"
260 ; "     * D.  EGEN        * 4.  NYB\RJARE"
270 ; CUR(22,5);"ANTAL VARV ";
280 ; CUR(9,26); : GET W$
290 W1%=ASC(W$)
300 IF W1%>95% THEN PRINT CUR(5,12);"                 " : GOTO 330
310 ; CUR(5,14);"SM] BOKST[VER"
320 POKE 32396,164 : POKE 32412,164 : GOTO 280
330 IF W$="n" THEN 390
340 IF W$<>"j" THEN 280
350 W%=1% : R2%=8% : K2%=40%
360 ; CUR(9,26);"JA"; : ; CUR(13,10);"A,B,C";
370 V%=3% : ; CUR(22,15);V%;
380 GOTO 430
390 ; CUR(9,26);"NEJ"; : W%=0% : R1%=7% : K1%=40%
400 R2%=9% : K2%=40%
410 ; CUR(13,10); : GET B$
420 IF B$="a" OR B$="b" OR B$="c" OR B$="d" THEN ; CHR$(ASC(B$)-32); ELSE 410
430 ; CUR(13,33); : GET H$
440 IF H$>"0" AND H$<"5" THEN ; H$; ELSE 430
450 IF W%=1% THEN 500
460 ; CUR(22,16); : GET V$
470 IF V$<"0" OR V$>"9" THEN 460
480 ; V$; : ; CUR(23,39);
490 LET V%=VAL(V$)
500 FOR T%=1% TO 2500% : NEXT T%
510 IF W%=1% THEN K%=(4%+VAL(H$))^3% ELSE K%=(2%+VAL(H$))^3%
520 ; CHR$(12)
530 FOR I%=1% TO 20% : ; CHR$(151) : NEXT I%
540 IF W%=1% THEN 1110 ELSE 570
550 ; CUR(22,3);"TRYCK P] RET."; : GET D$ : IF D$<>CHR$(13) THEN 550
560 GOTO 180
570 IF B$="a" THEN 1110
580 IF B$="b" THEN 1320
590 IF B$="c" THEN 1540
600 IF PEEK(62959)=66 THEN ; CHR$(12);CUR(4,15);"EGEN BANA" ELSE 650
610 ; CUR(12,12);"* F.  F\RRA BANAN";CUR(13,12);'* G.  G\RA OM "F."';CUR(14,12);"* N.  G\RA NY BANA";
620 ; CUR(18,12)"F/G/N ?"; : GET C$
630 IF C$="f" THEN ; CHR$(12) : GOTO 2540
640 IF C$<>"n" AND C$<>"g" THEN 620
650 ; CHR$(12)
660 ; CUR(9,3);"RITA/SUDDA MED.      HOPPA \VER MED."
670 ; CUR(12,10);"W";CUR(12,31);"]"
680 ; CUR(13,9);"A S"; : ; CUR(13,30);"\ ["
690 ; CUR(14,10);"Z";CUR(14,31);"-"
700 ; CUR(19,0);"TRYCK P] 'R' OM DU VILL RITA OM"
710 ; "TRYCK P] 'B' OM DU VILL BYTA BANA"
720 ; "TRYCK P] 'K' N[R DU [R KLAR"
730 ; CUR(23,13);"TRYCK P] 'S'"; : GET S$ : IF S$<>"s" THEN 730
740 ; CHR$(12)
750 IF C$="g" AND PEEK(62959)=66 THEN 2540
760 FOR I%=1% TO 20% : ; CHR$(151) : NEXT I%
770 FOR R%=2% TO 30% : SETDOT 3%,R% : NEXT R%
780 FOR R%=51% TO 74% : SETDOT 3%,R% : NEXT R%
790 FOR R%=3% TO 5% : SETDOT R%,31% : SETDOT R%,50% : NEXT R%
800 FOR R%=32% TO 49% : SETDOT 5%,R% : SETDOT 11%,R% : NEXT R%
810 FOR R%=3% TO 61% : SETDOT R%,2% : SETDOT R%,75% : NEXT R%
820 FOR R%=2% TO 75% : SETDOT 62%,R% : NEXT R%
830 Y%=8% : X%=50% : SETDOT Y%,X%
840 B%=INP(56)
850 IF B%=226% THEN 180
860 IF B%=242% THEN 740
870 IF B%=235% THEN 2600
880 X1%=X% : Y1%=Y%
890 IF B%=247% THEN Y%=Y%-1% : IF Y%<4% THEN Y%=Y1% : GOTO 840 ELSE 940
900 IF B%=250% THEN Y%=Y%+1% : IF Y%>61% THEN Y%=Y1% : GOTO 840 ELSE 940
910 IF B%=225% THEN X%=X%-1% : IF X%<3% THEN X%=X1% : GOTO 840 ELSE 940
920 IF B%=243% THEN X%=X%+1% : IF X%>74% THEN X%=X1% : GOTO 840 ELSE 940
930 GOTO 970
940 IF DOT(Y%,X%) THEN CLRDOT Y%,X% : CLRDOT Y1%,X1% ELSE SETDOT Y%,X% : SETDOT Y1%,X1%
950 FOR H4%=1% TO 300% : NEXT H4%
960 GOTO 840
970 IF B%=253% THEN Y%=Y%-1% : IF Y%<4% THEN Y%=Y1% : GOTO 840 ELSE GOTO 1020
980 IF B%=173% THEN Y%=Y%+1% : IF Y%>61% THEN Y%=Y1% : GOTO 840 ELSE 1040
990 IF B%=252% THEN X%=X%-1% : IF X%<3% THEN X%=X1% : GOTO 840 ELSE 1060
1000 IF B%=251% THEN X%=X%+1% : IF X%>74% THEN X%=X1% : GOTO 840 ELSE 1080
1010 GOTO 840
1020 IF DOT(Y%,X%) THEN 970 ELSE CLRDOT Y1%,X1% : SETDOT Y%,X%
1030 GOTO 1090
1040 IF DOT(Y%,X%) THEN 980 ELSE CLRDOT Y1%,X1% : SETDOT Y%,X%
1050 GOTO 1090
1060 IF DOT(Y%,X%) THEN 990 ELSE CLRDOT Y1%,X1% : SETDOT Y%,X%
1070 GOTO 1090
1080 IF DOT(Y%,X%) THEN 1000 ELSE CLRDOT Y1%,X1% : SETDOT Y%,X%
1090 FOR H4%=1% TO 300% : NEXT H4%
1100 GOTO 840
1110 ; CUR(0,0);CHR$(151);"j#####k#####k  DOT RACE  j#####k#####k"
1120 ; CUR(1,1);"j     j     jppppppppppppz     j     j"
1130 ; CUR(2,1);"j  j     j                  j     j  j"
1140 ; CUR(3,1);"j  jpppppzppppppppppppppppppzpppppz  j"
1150 ; CUR(4,1);"j           5            5        j  j"
1160 ; CUR(5,1);"jpppppppp0  5  ppppppp0  5  pppp  j  j"
1170 ; CUR(6,1);"j           5     j      5     j  j  j"
1180 ; CUR(7,1);"j  `ppppppppuppp  j  `pppuppp  j  j  j"
1190 ; CUR(8,1);"j  j           j  j  j         j     j"
1200 ; CUR(9,1);"j  j  `pppppp  j  j  j  `ppppppzpppppz"
1210 ; CUR(10,1);"j  j        j  j  j  j  j   BANA A   j"
1220 ; CUR(11,1);'j  jpppppp  j  j  j  j  "############k'
1230 ; CUR(12,1);'j  j     j  j  j  j  j               j'
1240 ; CUR(13,1);'j  j  `  j  j  j  j  "#########k###  j'
1250 ; CUR(14,1);'j  j  j  j  j  j  *t           j     j'
1260 ; CUR(15,1);'j  j  j  j  j  *t  "########k  j  "##k'
1270 ; CUR(16,1);'j  j  j  j  j0  "m0         j  j     j'
1280 ; CUR(17,1);'j  "  j  "  j+t   ########  j  "###  j'
1290 ; CUR(18,1);'j     j     j "m0           j        j'
1300 ; CUR(19,1);'"#####################################'
1310 GOTO 1750
1320 ; CUR(0,0);CHR$(151);'7########5      DOTRACE       7#######k'
1330 ; CUR(1,1);'5        upppppppppppppppppppp5       j'
1340 ; CUR(2,1);'5  7##!  5                    5  ##k  j'
1350 ; CUR(3,1);'5  5     5  ppppppppppppppp0  5    j  j'
1360 ; CUR(4,1);'5  5  7##!  5                 7##  j  j'
1370 ; CUR(5,1);'5  5  5     5  ppppppppppppppp5    j  j'
1380 ; CUR(6,1);'5  5  !  7##!  5     5     5  5  ##k  j'
1390 ; CUR(7,1);'5  5     5     5  0  5  0  5  5    j  j'
1400 ; CUR(8,1);'5  ######5  7##!  5  5  5  7#####  j  j'
1410 ; CUR(9,1);'5        5  5     5  5  5  5       j  j'
1420 ; CUR(10,1);'7#####!  5  !  7##!  5  5  5  7#####  j'
1430 ; CUR(11,1);'5        5     5     5  5  5  5       j'
1440 ; CUR(12,1);'5  7#####7#####5  ###!  5  5  5  #####k'
1450 ; CUR(13,1);'5  5     5     5        5  5  5       j'
1460 ; CUR(14,1);'5  5  5  5  5  7########5  !  7#####  j'
1470 ; CUR(15,1);'5  5  5  5  5  5        5     5       j'
1480 ; CUR(16,1);'5  5  5  5  5     7##5  ######5  #####k'
1490 ; CUR(17,1);'5  5  5  5  5     5  5        5       j'
1500 ; CUR(18,1);'5  !  5  !  7#####!  ######5  ######  j'
1510 ; CUR(19,1);'5     5     5   BANA  B    5          j'
1520 ; CUR(20,1);'############!              ############'
1530 GOTO 1750
1540 ; CUR(0,0);CHR$(151);'7##########k    DOTRACE    BANA C      '
1550 ; CUR(1,1);'5  pppppp  jpppppppppppppppppppppppp   '
1560 ; CUR(2,1);'5  5       j                       j   '
1570 ; CUR(3,1);'5  5  pppppz  `pppppppppppppppppp  j   '
1580 ; CUR(4,1);'5  5       j  j           j     j  j   '
1590 ; CUR(5,1);'5  uppppp  j  j  `pppppp  j  `  j  j   '
1600 ; CUR(6,1);'5       j     j  j        j  j  j  j   '
1610 ; CUR(7,1);'uppppp  jpppppz  j  `pppppz  j  j  j   '
1620 ; CUR(8,1);'5    j           j  j     j  j  j  j   '
1630 ; CUR(9,1);'5  0 jpppppppppppz  j  `  j  j  j  j   '
1640 ; CUR(10,1);'5  5 j              j  j  j  j     j   '
1650 ; CUR(11,1);'5  5 j  `pppppppppppz  j  j  jpppppz   '
1660 ; CUR(12,1);'5  5 j  j              j  j     j      '
1670 ; CUR(13,1);'5  5 j  j  `pppppppppppz  jppp  j      '
1680 ; CUR(14,1);'5  5 j  j  j  DOTRACE  j     j  j      '
1690 ; CUR(15,1);'5  5 j  j  jpppppppppppzppp  j  j      '
1700 ; CUR(16,1);'5  5    j                 j  j  j      '
1710 ; CUR(17,1);'5  uppppzppppppppppppppp  j  j  j      '
1720 ; CUR(18,1);'5                         j     j      '
1730 ; CUR(19,1);'upppppppppppppppppppppppppzpppppz      '
1740 ; CUR(20,1);'                                       '
1750 SETDOT R2%,K2%
1760 IF W%=0% THEN SETDOT R1%,K1%
1770 IF G%=1% THEN 1890
1780 ; CUR(22,0);'Tryck p} return vid start!'; : GET A$ : IF A$<>CHR$(13) THEN 1780
1790 ; CUR(22,0);'                          '
1800 FOR X%=10% TO 0% STEP -1
1810 ; CUR(22,18);X%;
1820 FOR I%=1% TO 2000% : NEXT I%
1830 ; CUR(22,18);'   '
1840 NEXT X%
1850 ; CUR(22,18);'   '
1860 IF W%=0% THEN ; CUR(22,2);'ANTAL VARV: 0'; : ; CUR(23,2);'VARVTID:  ';
1870 ; CUR(22,22);'ANTAL VARV: 0'; : ; CUR(23,22);'VARVTID: ';
1880 GOSUB 2270
1890 CLRDOT R2%,K2% : CLRDOT R1%,K1%
1900 GOSUB 2100
1910 IF W%=1% THEN 2000
1920 R3%=R1% : K3%=K1%
1930 IF D1%=1% THEN R1%=R1%-1%
1940 IF D1%=-1% THEN R1%=R1%+1%
1950 IF D1%=2% THEN K1%=K1%-1%
1960 IF D1%=-2% THEN K1%=K1%+1%
1970 CLRDOT R3%,K3%
1980 IF DOT(R1%,K1%) THEN OUT 6,0 : OUT 6,137 : D1%=0% : R1%=R3% : K1%=K3% ELSE SETDOT R1%,K1% : GOSUB 2210
1990 IF V1%=V% THEN 2500
2000 GOSUB 2100
2010 R3%=R2% : K3%=K2%
2020 IF D2%=1% THEN R2%=R2%-1%
2030 IF D2%=-1% THEN R2%=R2%+1%
2040 IF D2%=2% THEN K2%=K2%-1%
2050 IF D2%=-2% THEN K2%=K2%+1%
2060 CLRDOT R3%,K3%
2070 IF DOT(R2%,K2%) THEN OUT 6,0 : OUT 6,137 : D2%=0% : R2%=R3% : K2%=K3% ELSE SETDOT R2%,K2% : GOSUB 2240
2080 IF V2%=V% THEN 2500
2090 GOTO 1900
2100 C1%=C% : C%=INP(56)
2110 IF C1%=C% THEN RETURN 
2120 IF C%=247% AND D1%<>-1% THEN D1%=1%
2130 IF C%=225% AND D1%<>-2% THEN D1%=2%
2140 IF C%=243% AND D1%<>2% THEN D1%=-2%
2150 IF C%=250% AND D1%<>1% THEN D1%=-1%
2160 IF C%=253% AND D2%<>-1% THEN D2%=1%
2170 IF C%=252% AND D2%<>-2% THEN D2%=2%
2180 IF C%=251% AND D2%<>2% THEN D2%=-2%
2190 IF C%=173% AND D2%<>1% THEN D2%=-1%
2200 RETURN 
2210 FOR I%=1% TO K% : NEXT I%
2220 IF R1%>4% AND R1%<11% AND K1%=39% THEN GOSUB 2330 : V1%=V1%+1% : ; CUR(23,11);RIGHT$(T$,4); : ; CUR(22,13);V1%;
2230 RETURN 
2240 FOR I%=1% TO K% : NEXT I%
2250 IF R2%>4% AND R2%<11% AND K2%=39% THEN GOSUB 2330 : V2%=V2%+1% : ; CUR(23,31);RIGHT$(T$,4); : ; CUR(22,33);V2%;
2260 RETURN 
2270 Z=H%*3600+M%*60+S%
2280 Z1%=Z*50/256
2290 Z%= NOT (50*(Z-Z1%/50*256))
2300 Z1%= NOT Z1%
2310 POKE 65008%,Z%,Z1%,SWAP%(Z1%)
2320 RETURN 
2330 D%=0
2340 T1%=65008%
2350 IF (PEEK(T1%) AND 4%)=0 THEN 2330
2360 FOR I%=0% TO 2%
2370 Z%(I%)=255% XOR PEEK(T1%+I%)
2380 NEXT I%
2390 Z=((Z%(2)*256)+Z%(1))*5.12+Z%(0)/50
2400 IF Z>86400 THEN Z=Z-86400 : D%=D%+1 : GOTO 2400
2410 H%=Z/3600 : Z=Z-3600*H%
2420 M%=Z/60 : S%=Z-60%*M%
2430 IF D%<>0 THEN GOSUB 2280
2440 T$=RIGHT$(NUM$(100+M%),3)+RIGHT$(NUM$(100+S%),3)
2450 T=VAL(T$)
2460 T$=RIGHT$(NUM$(100+H%),3)+':'+RIGHT$(NUM$(100+M%),3)+':'+RIGHT$(NUM$(100+S%),3)
2470 IF W%=1% AND V2%=0% THEN G%=1% : V2%=V2%+1% : ; CUR(23,31);RIGHT$(T$,4); : ; CUR(22,33);V2%; : GOTO 1320
2480 IF W%=1% AND V2%=1% THEN V2%=V2%+1% : ; CUR(23,31);RIGHT$(T$,4); : ; CUR(22,33);V2%; : GOTO 1540
2490 RETURN 
2500 IF W%=1% THEN ; CUR(22,5); ELSE ; CUR(21,12);
2510 ; "Mera ? (j)"; : GET D$
2520 IF D$="J" OR D$="j" THEN 180
2530 IF D$="N" OR D$="n" THEN 2660 ELSE 2500
2540 \%=62960% : [%=31784%
2550 FOR G1%=1% TO 936%
2560 POKE [%,PEEK(\%)
2570 \%=\%+1% : [%=[%+1%
2580 NEXT G1%
2590 IF C$="g" THEN 830 ELSE 1750
2600 \%=62960% : [%=31784% : POKE 62959,66
2610 FOR G1%=1% TO 936%
2620 POKE \%,PEEK([%)
2630 \%=\%+1% : [%=[%+1%
2640 NEXT G1%
2650 GOTO 1750
2660 ; CALL(0)
