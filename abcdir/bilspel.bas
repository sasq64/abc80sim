1 REM Ins{nt av 2839
100 REM +---------------------------+
110 REM +------....BILBANA....------+
120 REM +---------------------------+
130 REM +-- Ett litet bilspel med --+
140 REM +-- en kamp mot datorn.   --+
150 REM +---------------------------+
160 REM +--  K|rs p} grundutr.    --+
170 REM +---------------------------+
180 REM +-- (c) Lars B|rje Cid    --+
190 REM +---------------------------+
200 REM +-- Duveholm  1983-06-14  --+
210 REM +---------------------------+
220 REM 
230 REM Detta spel {r bara under ut-
240 REM veckling. Jag kommer troligen
250 REM att skicka in en nyare variant
260 REM inom n}gon vecka.
270 REM 
280 REM -    Lars B|rje Cid 
290 REM -    Bondegatan 24 B
300 REM -    641 45  Katrineholm
310 REM 
320 REM -----------------------------
330 REM 
340 REM 
350 GOSUB 360 : GOTO 370
360 ; CHR$(12%)TAB(12%)"B I L B A N A" : ; TAB(11%)CHR$(23%)STRING$(13%,35%) : RETURN 
370 ; CUR(7%,5%)"Instruktioner (Nej) ? "; : GET Z$
380 IF Z$="J" OR Z$="j" THEN GOSUB 1100
390 P%=3%
400 ONERRORGOTO 400
410 ; CUR(12%-P%,4%)"Sv}rigetsgrad (1-20) "; : INPUT A%
411 IF A%<1% OR A%>20% THEN 410 ELSE P%=0%
420 T1%=5%+A%*5%
430 R%=63% : K%=37% : Y%=67% : X%=37% : R1%=R% : K1%=K% : Y1%=Y% : X1%=X%
440 H%=0% : M%=0% : S%=0% : V%=0% : F%=4% : GOSUB 1970
450 F%(0%)=0% : F%(1%)=0% : S%(0%)=0% : S%(1%)=0%
460 F1%(0%)=0% : F1%(1%)=0%
470 REM 
480 REM -- INIT --
490 REM 
500 GOSUB 1390 : RESTORE 1730
510 SETDOT R%,K% : SETDOT Y%,X%
520 FOR P=1 TO 10 : FOR I=1 TO 260 : NEXT I : ; CHR$(7%); : NEXT P
530 CLRDOT R1%,K1% : SETDOT R%,K% : R1%=R% : K1%=K%
540 REM 
550 REM -- DATORNS BIL --
560 REM 
570 IF T1%=0% THEN T1%=20%+A%*3% : GOTO 730 ELSE T1%=T1%-1%
580 IF F%(0%) THEN 730
590 IF L%>0% THEN 610
600 READ Y%,X%,L%,S2%
610 CLRDOT Y1%,X1% : SETDOT Y%,X% : L%=L%-1% : Y1%=Y% : X1%=X%
620 ONERRORGOTO 630 : GOTO 650
630 RESTORE 1730 : OUT 6,137% : S%(0%)=S%(0%)+1% : GOSUB 1060 : GOSUB 2100 : IF S%(0%)=3% THEN F%(0%)=-1%
640 IF F%(0%) AND NOT F1%(1%) THEN F1%(0%)=-1% : REM DATORN VANN
650 ON S2% GOTO 660,670,680,690
660 Y%=Y%-1% : GOTO 730
670 X%=X%+1% : GOTO 730
680 Y%=Y%+1% : GOTO 730
690 X%=X%-1%
700 REM 
710 REM -- SPELARENS BIL --
720 REM 
730 IF L9%>0% THEN 740 ELSE GOSUB 1810 : L9%=L9%-1%
740 IF F%(1%) THEN 1000
750 IF L9%=0% THEN L9%=10%+S%(1%)*10%
760 IF G%>0% THEN G%=G%-1% : GOSUB 1010 : GOTO 1000
770 T%=INP(56%)-128%
780 GOSUB 1010
790 FOR P=1 TO 0 : NEXT P : OUT 6%,0%
800 IF T%=ASC("Y") THEN F%=1%
810 IF T%=ASC("H") THEN F%=2%
820 IF T%=ASC("B") THEN F%=3%
830 IF T%=ASC("G") THEN F%=4%
840 ON F% GOTO 850,870,890,910
850 R%=R%-1% : IF DOT(R%,K%) THEN R%=R%+1% : OUT 6%,153% : G%=15%
860 GOTO 950
870 K%=K%+1% : IF DOT(R%,K%) THEN K%=K%-1% : OUT 6%,153% : G%=15%
880 GOTO 950
890 R%=R%+1% : IF DOT(R%,K%) THEN R%=R%-1% : OUT 6%,153% : G%=15%
900 GOTO 950
910 K%=K%-1% : IF DOT(R%,K%) THEN K%=K%+1% : OUT 6%,153% : G%=15%
920 REM 
930 REM -- KOLLA POSITION  --
940 REM 
950 IF K%=36% AND R%<45% AND R%>33% THEN G%(1%)=-1%
960 IF R%>40% AND K%=38% THEN E%=-1%
970 IF E% E%=0% : IF G%(1%) S%(1%)=S%(1%)+1% : E%=-1%
980 IF E% E%=0% : OUT 6,135% : GOSUB 1060 : GOSUB 2100 : G%(1%)=0% : IF S%(1%)=3% F%(1%)=-1
990 IF F%(1%) AND NOT F1%(0%) THEN F1%(1%)=-1% : REM SPELAREN VANN
1000 GOTO 530
1010 CLRDOT R%,K%
1020 OUT 6%,0% : RETURN 
1030 REM 
1040 REM -- PRINT ON SCREEN --
1050 REM 
1060 ; CUR(16%,12%)S%(0%) : ; CUR(17%,12%)S%(1%) : RETURN 
1070 REM 
1080 REM -- INSTRUKTIONER --
1090 REM 
1100 GOSUB 360 : RESTORE 1240
1110 FOR I%=1% TO 13% : READ A$ : FOR J%=1% TO LEN(A$)
1120 ; CUR(I%+4%,J%)MID$(A$,J%,1%)
1130 FOR P=1 TO 70 : NEXT P : NEXT J% : NEXT I% : GOSUB 1210
1140 FOR I%=1% TO 11% : READ A$ : FOR J%=1% TO LEN(A$)
1150 ; CUR(I%+4%,J%)MID$(A$,J%,1%)
1160 FOR P=1 TO 70 : NEXT P : NEXT J% : NEXT I% : GOSUB 1210
1170 FOR I%=1% TO 13% : READ A$ : FOR J%=1% TO LEN(A$)
1180 ; CUR(I%+4%,J%)MID$(A$,J%,1%)
1190 FOR P=1 TO 70 : NEXT P : NEXT J% : NEXT I%
1200 GOSUB 1210 : RETURN 
1210 POKE 65013%,0% : ; CUR(22%,5%)"Tryck p} RETURN !"CHR$(8%);
1220 GET Z$ : IF Z$<>CHR$(13%) THEN 1220
1230 GOSUB 360 : RETURN 
1240 DATA "Detta spel g}r ut p} att k|ra","runt en bana i 3 varv.
1250 DATA "Det finns tv} bilar :   ",,"Den ena (den som blinkar) {r din.","Den andra {r datorns.
1260 DATA ,"Tid, antal k|rda varv samt","varvtid visas p} sk{rmen.
1270 DATA ,"N{r du krockar stannar din","bil i cirka 1 sekund medans","datorns bil forts{tter.
1280 DATA ,"Tiden som visas p} sk{rmen","{r realtid. N{r datorn eller","du har k|rt runt ett","varv kommer varvtiden
1290 DATA "att visas till h|ger. F|r","Den som kommer f|rst runt visas","hela varvtiden och"
1300 DATA "f|r den som kommer tv} :   a"
1310 DATA "visas tiden som motst}ndaren","ligger f|re.",
1320 DATA "         T A N G E N T E R
1330 DATA "         = = = = = = = = =
1340 DATA ,,"    Y -- Upp.","    B -- Ned.","    G -- V{nster.","    H -- H|ger.
1350 DATA ,,,,"      L Y C K A   T I L L !
1360 REM 
1370 REM -- BANA --
1380 REM 
1390 GOSUB 360 : FOR I%=1% TO 24% : ; : ; CHR$(23%); : NEXT I%
1400 RESTORE 1430 : FOR I%=1% TO 21% : READ A$ : FOR J%=1% TO LEN(A$)
1410 ; CUR(I%+2%,J%)MID$(A$,J%,1%); : NEXT J% : IF I%>16% THEN GOSUB 1650
1420 NEXT I%
1430 DATA '?/////////o//////////////o'
1440 DATA '?          j7              "'
1450 DATA '!          j?                k'
1460 DATA '%     p     j5   h|p         "'
1470 DATA '7     zt   "5   ~|tpp0   '
1480 DATA '5    `    k%  h5   '
1490 DATA '5    z    "   ~5   '
1500 DATA '5   `5   "oo5   o5   '
1510 DATA '5   j0    !      o5   '
1520 DATA '5   z5            {!   '
1530 DATA '5      `ppp    `?    '
1540 DATA '5   5    '
1550 DATA '5  j TID :   00.00 :  00  VARVTID  5    '
1560 DATA 'u    V.D :   0         00.00 :  00     '
1570 DATA '   "o V.S :   0        5   '
1580 DATA '4    #o////o?   '
1590 DATA '5      #o?!     ##   '
1600 DATA '        #####///////####!           '
1610 DATA 'p                                   '
1620 DATA '}|p0                             `'
1630 DATA '||||||||||||||||||||||||||||'
1640 POKE 32759%,127% : RETURN 
1650 ; CUR(15%,7%)CHR$(1%)CUR(15%,21%)CHR$(23%)
1660 ; CUR(16%,7%)CHR$(1%)CUR(16%,21%)CHR$(23%)
1670 ; CUR(16%,32%)CHR$(23%)CUR(16%,23%)CHR$(1%)
1680 ; CUR(17%,7%)CHR$(1%)CUR(17%,21%)CHR$(23%)
1690 RETURN 
1700 REM 
1710 REM -- DATA F\R DATORNS BIL --
1720 REM 
1730 DATA 65,37,11,04,64,26,05,04,63,22,05,04
1740 DATA 62,17,03,01,59,17,10,04,59,07,40,01
1750 DATA 19,07,20,02,19,27,20,03,39,28,18,02
1760 DATA 39,46,25,01,14,46,27,02,14,73,46,03
1770 DATA 60,73,11,04,60,62,05,03,65,62,25,04
1780 REM 
1790 REM -- L[S KLOCKA --
1800 REM 
1810 IF C% C%=0% : RETURN ELSE C%=-1%
1820 IF F%(1%) AND F%(0%) THEN 2230
1830 Q$=""
1840 FOR I%=0% TO 2% : Z%(I%)=255 XOR PEEK(65008%+I%) : NEXT I%
1850 Z=((Z%(2%)*256%)+Z%(1%))*5.12+Z%(0%)/50
1860 H%=Z/3600 : Z=Z-3600*H% : M%=Z/60% : S%=Z-60%*M%
1870 IF ABS(H%)<10% Q$="0"
1880 Q$=Q$+RIGHT$(NUM$(H%),2%)+"."
1890 IF ABS(M%)<10% Q$=Q$+"0"
1900 Q$=Q$+RIGHT$(NUM$(M%),2%)+":"
1910 IF ABS(S%)<10% Q$=Q$+"0"
1920 Q$=Q$+RIGHT$(NUM$(S%),2%)
1930 ; CUR(15%,13%)Q$ : RETURN 
1940 REM 
1950 REM -- NOLLST[LL KOCKA --
1960 REM 
1970 H%=0% : M%=0% : S%=0% : H1%=0% : M1%=0% : S1%=0%
1980 Z=H%*3600+M%*60+S% : Z1%=Z*50/256
1990 Z%= NOT (50*(Z-Z1%/50*256)) : Z1%= NOT Z1% : ONERRORGOTO 0
2000 POKE 65008%,Z%,Z1%,SWAP%(Z1%) : RETURN 
2010 REM 
2020 REM -- PAUS --
2030 REM 
2040 FOR P=1 TO 40
2050 FOR J=1 TO 20 : OUT 6,137 : NEXT J : FOR J=1 TO 20 : OUT 6,251 : NEXT J : OUT 6,0 : NEXT P
2060 RETURN 
2070 REM 
2080 REM -- BER[KNA VARVTID --
2090 REM 
2100 GOSUB 1830 : V0%=H%-H1% : V1%=M1%-M% : V2%=ABS(S1%-S%)
2110 IF ABS(V0%)<9% THEN Q2$="0"
2120 Q2$=Q2$+RIGHT$(NUM$(V0%),2%)+"."
2130 IF ABS(V1%)<9% THEN Q2$=Q2$+"0"
2140 Q2$=Q2$+RIGHT$(NUM$(V1%),2%)+":"
2150 IF ABS(V2%)<9% THEN Q2$=Q2$+"0"
2160 Q2$=Q2$+RIGHT$(NUM$(V2%),2%)
2170 ; CUR(16%,23%)CHR$(1%)Q2$CHR$(23%)
2180 GOSUB 1830 : H1%=H% : M1%=M% : S1%=S% : RETURN 
2190 REM 
2200 REM -- END --
2210 REM 
2220 REM 
2230 GOSUB 1830 : H1%=H% : M1%=M% : S1%=S%
2240 GOSUB 2040 : GOSUB 360 : ; CUR(6%,7%);
2250 IF F1%(0%) THEN ; "Tyv{rr Datorn vann!" : GOTO 2280
2260 IF F1%(1%) THEN ; "Grattis  D U  vann!!"
2270 ; TAB(7%)"         = ="
2280 POKE 65013%,0% : ; CUR(9%,3%)"Vill du spel igen (JA/NEJ) ?"; : INPUTLINE A$
2290 A$=LEFT$(A$,LEN(A$)-2%) : ; 
2300 IF NOT (A$="JA" OR A$="ja" OR A$="NEJ" OR A$="nej") THEN 2280
2310 IF A$="JA" OR A$="ja" THEN 400
2320 ; : ; "Ok."; : END 
