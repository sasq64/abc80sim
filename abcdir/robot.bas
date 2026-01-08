10 ONERRORGOTO 10
20 ; CHR$(12%)"         ROBOT JAKT         "
30 ; "Du har tr{ngt in p} milit{rt omr}de, som bevakas av robotar.";
40 ; "Till f|ljd av detta  m}ste du fly fr}n robotarna."
50 ; " Du flyttar genom att ange riktning med"
60 ; "tal, s} h{r:"
70 ; "       1        som   "CHR$(151%)"5"CHR$(135%)"Tecknenf|rkl.:"
80 ; "    8  .  2  represen-"CHR$(151%)"5"
90 ; "     . . .   terar    "CHR$(151%)"5"CHR$(135%)"'' = Du."
100 ; "      ...    respek-  "CHR$(151%)"5"
110 ; "   7...0...3 tive     "CHR$(151%)"5"CHR$(135%)"'$' = Robot"
120 ; "      ...    riktning."CHR$(151%)"5"
130 ; "     . . .            "CHR$(151%)"5"CHR$(135%)"'#' = Mast"
140 ; "    6  .  4           "CHR$(151%)"5"
150 ; "       5              "CHR$(151%)"5"
160 ; "Robotarna flyttar hela tiden mot dig," : ; "men om de st|ter mot en h|gsp{nnings-"
170 ; "mast blir de pulveriserade och f|rsvin-" : ; "ner helt fr}n omr}det."
180 ; "Det finns 6 sv}righetsgrader"
190 ; "som du anger h{r intill" : ; "ett tal (1-6)"; : INPUT S%
200 IF S%<1% OR S%>6% ; "Skriv 1,2,3,4,5 eller 6...."; : INPUT S% : GOTO 200
210 ; : ; " Hur m}nga robotar............"; : INPUT R% : ; " Hur m}nga h|gsp{nningsmaster.."; : INPUT H%
220 ON S% GOTO 230,240,250,260,270,280
230 S=1.4 : O0%=0% : GOTO 290
240 S=1.6 : O0%=0% : GOTO 290
250 S=1.8 : O0%=0% : GOTO 290
260 S=1.4 : O0%=1% : GOTO 290
270 S=1.6 : O0%=1% : GOTO 290
280 S=1.8 : O0%=1% : GOTO 290
290 Z%=3% : REM IF O0%=1RANDOMIZE
300 DIM H%(H%,2%),R%(R%,2%),M%(1%,2%)
310 ; CHR$(12%)CHR$(151%)"<l"STRING$(32%,44%)"<l"
320 F1%=0%
330 FOR I%=1% TO 17% : ; CUR(I%,0%)CHR$(151%)"5"CHR$(135%)TAB(35%)CHR$(151%)"j" : NEXT I%
340 ; CHR$(151%)"-."STRING$(32%,44%)"-."
350 ; CUR(19%,20%)"8 1 2" : ; CUR(20%,21%)"...";
360 ; CUR(21%,20%)"7.0.3" : ; CUR(22%,21%)"..." : ; CUR(23%,20%)"6 5 4";
370 DIM H%(H%,2%),R%(R%,2%),M%(1%,2%)
380 REM  S[T OP MASTER
390 FOR L%=1% TO H%
400 R=INT(RND*17+1) : IF R>17 GOTO 400
410 K=INT(RND*32+3) : IF K>34 GOTO 410
420 REM  TEST FELT - #
430 GOSUB 1760 : IF F%=1% 400
440 H%(L%,1%)=R : H%(L%,2%)=K
450 ; CUR(R,K)"#"
460 NEXT L%
470 REM  PLACER ROBOTTER
480 FOR L%=1% TO R%
490 R=INT(RND*17+1) : IF R>17 GOTO 490
500 K=INT(RND*32+3) : IF K>34 GOTO 500
510 REM  TEST FELT - $
520 GOSUB 1790 : IF F%=2% 490
530 REM  TEST FELT - #
540 GOSUB 1760 : IF F%=1% 490
550 R%(L%,1%)=R : R%(L%,2%)=K
560 ; CUR(R,K)"$"
570 NEXT L% : REM L%=R%+10T
580 R=INT(RND*17+1) : IF R>17 GOTO 580
590 K=INT(RND*32+3) : IF K>34 GOTO 590
600 REM  TEST FELT - $
610 GOSUB 1790 : IF F%=2% 580
620 REM  TEST FELT - #
630 GOSUB 1760 : IF F%=1% 580
640 M%(1%,1%)=R : M%(1%,2%)=K
650 ; CUR(R,K)""
660 REM *******************************
670 REM * HER BEGYNDER SPILLET
680 REM *******************************
690 A$="012345678"
700 ; CUR(21%,0%)"ROBOTAR"SPACE$(5%);CUR(21%,7%)R%-F1% : ; : ; " FLYTTA (0-8)..."; : GET V$
710 P%=INSTR(1%,A$,V$) : IF P%=0% 700
720 ON P% GOSUB 980,1000,1070,1140,1210,1280,1350,1420,1490
730 R8=F1%^2%*.1
740 F1%=0% : FOR L%=1% TO R%
750 R1=R%(L%,1%) : K1=R%(L%,2%)
760 IF R1=0% F1%=F1%+1 : GOTO 890
770 R9=SQR((R%(L%,1%)-M%(1%,1%))^2%+(R%(L%,2%)-M%(1%,2%))^2%)
780 IF R9>Z%+R8 R=R1 : K=K1 : GOTO 890
790 R=R1-FIX((R%(L%,1%)-M%(1%,1%))/R9*S)
800 K=K1-FIX((R%(L%,2%)-M%(1%,2%))/R9*S)
810 GOSUB 1560 : IF F%=1% OUT 6,137 : F1%=F1%+1% : FOR L2%=1% TO 300% : NEXT L2% : OUT 6,0
820 IF F%=1% THEN ; CUR(R1,K1)" "CUR(R,K)"$" : R%(L%,1%)=0% : IF O0%=1% 1820 ELSE 890
830 IF F%=2% THEN R=R1 : K=K1
840 REM  check spisningen
850 ; CUR(R1,K1)" "CUR(R,K)"$"
860 OUT 6,69 : FOR L2%=1% TO 70% : NEXT L2% : OUT 6,0
870 IF R=M%(1%,1%) AND K=M%(1%,2%) THEN 1680
880 R%(L%,1%)=R : R%(L%,2%)=K
890 NEXT L%
900 FOR L%=1% TO H%
910 IF H%(L%,1%)=0% THEN 930
920 ; CUR(H%(L%,1%),H%(L%,2%))"#"
930 NEXT L%
940 FOR L%=1% TO R% : IF R%(L%,1%)<>0% 700 ELSE NEXT L%
950 ; CUR(22%,0%)"Bravo, du klarade dig!!!" : ; " Vill du f|rs|ka igen (J)....";
960 GET V$ : IF V$="J" 310 ELSE 1730
970 STOP
980 REM  0 FLYT
990 RETURN
1000 REM  1 FLYT
1010 R=M%(1%,1%)-1% : K=M%(1%,2%)
1020 IF R<1% RETURN
1030 GOSUB 1560 : IF F%=1% RETURN
1040 ; CUR(R+1%,K)" "CUR(R,K)""
1050 M%(1%,1%)=R
1060 RETURN
1070 REM  2 FLYT
1080 R=M%(1%,1%)-1% : K=M%(1%,2%)+1%
1090 IF R<1% OR K>34% RETURN
1100 GOSUB 1560 : IF F%=1% RETURN
1110 ; CUR(R+1%,K-1%)" "CUR(R,K)""
1120 M%(1%,1%)=R : M%(1%,2%)=K
1130 RETURN
1140 REM  3 FLYT
1150 R=M%(1%,1%) : K=M%(1%,2%)+1%
1160 IF K>34% RETURN
1170 GOSUB 1560 : IF F%=1% RETURN
1180 ; CUR(R,K-1%)" "CUR(R,K)""
1190 M%(1%,2%)=K
1200 RETURN
1210 REM  4 FLYT
1220 R=M%(1%,1%)+1% : K=M%(1%,2%)+1%
1230 IF R>17% OR K>34% RETURN
1240 GOSUB 1560 : IF F%=1% RETURN
1250 ; CUR(R-1%,K-1%)" "CUR(R,K)""
1260 M%(1%,1%)=R : M%(1%,2%)=K
1270 RETURN
1280 REM  5 FLYT
1290 R=M%(1%,1%)+1% : K=M%(1%,2%)
1300 IF R>17% RETURN
1310 GOSUB 1560 : IF F%=1% RETURN
1320 ; CUR(R-1%,K)" "CUR(R,K)""
1330 M%(1%,1%)=R
1340 RETURN
1350 REM  6 FLYT
1360 R=M%(1%,1%)+1% : K=M%(1%,2%)-1%
1370 IF R>17% OR K<3% RETURN
1380 GOSUB 1560 : IF F%=1% RETURN
1390 ; CUR(R-1%,K+1%)" "CUR(R,K)""
1400 M%(1%,1%)=R : M%(1%,2%)=K
1410 RETURN
1420 REM  7 FLYT
1430 R=M%(1%,1%) : K=M%(1%,2%)-1%
1440 IF K<3% RETURN
1450 GOSUB 1560 : IF F%=1% RETURN
1460 ; CUR(R,K+1%)" "CUR(R,K)""
1470 M%(1%,2%)=K
1480 RETURN
1490 REM  8 FLYT
1500 R=M%(1%,1%)-1% : K=M%(1%,2%)-1%
1510 IF R<1% OR K<3% RETURN
1520 GOSUB 1560 : IF F%=1% RETURN
1530 ; CUR(R+1%,K+1%)" "CUR(R,K)""
1540 M%(1%,1%)=R : M%(1%,2%)=K
1550 RETURN
1560 REM  Check for optaget felt......           F%=0 hvis ikke optaget                 F%=1 optaget af #
1570 REM  F%=2 optaget af $
1580 F%=0%
1590 REM  CHECK FELT
1600 GOSUB 1790 : IF F%=2% RETURN
1610 GOTO 1630
1620 RETURN
1630 REM  Check for optaget felt......           F%=0 hvis ikke optaget                 F%=1 optaget af #
1640 F%=0%
1650 REM  TEST FELT
1660 GOSUB 1760
1670 RETURN
1680 REM  Dette bliver enden p} spillet
1690 ; CUR(R,K)"$"CUR(R1,K1)" "
1700 ; CUR(19%,0%)"Du }kte fast, och kommer att"CUR(20%,0%)"transporteras till n{rmaste "
1710 OUT 6,135 : ; CUR(21%,0%)"rymdbas f|r unders|kning "
1715 ; CUR(22%,0%)STRING$(39%,32%)
1720 ; CUR(23%,0%)" Vill du f|rs|ka igen (J)...."; : GET V$ : IF V$="J" 310 ELSE 1730
1730 REM  AFSLUTNING
1740 IF V$<>"N" ; CUR(23%,0)" Vill du f|rs|ka igen (J)...."; : GET V$ : IF V$="J" THEN 310 ELSE 1740
1750 GOTO 1870
1760 REM  CHECK FELT - MAST
1770 IF DOT(3*R,2*K) AND NOT DOT(3*R+2,2*K) THEN F%=1% ELSE F%=0%
1780 RETURN
1790 REM  CHECK FELT - ROBOT
1800 IF DOT(3*R+1,2*K) AND NOT DOT(3*R+2,2*K) THEN F%=2% ELSE F%=0%
1810 RETURN
1820 REM  ANNULER H\JSP[NDINGSMAST
1830 FOR L1%=1% TO H%
1840 IF R=H%(L1%,1%) AND K=H%(L1%,2%) ; CUR(R,K)" " : H%(L1%,1%)=0% : GOTO 890
1850 NEXT L1% : GOTO 890
1860 IF L%>R% STOP : REM 
1870 END