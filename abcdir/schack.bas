20 DIM V%(70%),A%(8%,8%),C%(9%,9%),A$(13%)=12%,N%(160%),\$=1%,A$=20%,A1$=8%,A2$=8%
30 DIM G2$=11%,N2%(70%)
40 N5%=0% : Z%=0% : N%=0% : G2$="           "
50 GOSUB 3060 : GOSUB 2280
60 DEFFNX=(N3% AND 7%)+1%
70 GOSUB 2580 : ; : ; "Vilken f{rg vill du ha (Vitt/Svart)";
80 INPUT A$ : A$=LEFT$(A$+" ",1%)
90 ; : ; G2$"D} tar jag ";
100 IF A$="s" OR A$="S" THEN F7%=0% : A2$="87654321" ELSE F7%=-1% : A2$="12345678"
110 IF F7% THEN ; "svart" : ; : ; G2$"Du"; ELSE ; "vitt" : ; : ; G2$"Jag";
120 ; " b|rjar"
130 GOSUB 730 : GOSUB 730
140 A1$="ABCDEFGH"
150 FOR X%=0% TO 9% : FOR Y%=0% TO 9% : C%(X%,Y%)=-1% : NEXT Y% : NEXT X%
160 GOSUB 170 : GOTO 290
170 RESTORE
180 FOR Y%=1% TO 8%
190 READ B%
200 C%(1%,Y%)=B%+1000%
210 C%(2%,Y%)=1001%
220 C%(7%,Y%)=1%
230 C%(8%,Y%)=B%
240 FOR X%=3% TO 6%
250 C%(X%,Y%)=0%
260 NEXT X% : NEXT Y%
270 RETURN
280 DATA 5,3,103,9,99,103,3,5
290 GOSUB 2390
300 FOR A%=1% TO 8%
310 FOR B%=1% TO 4%
320 A%(A%,B%)=(9%-A%+B%)*5%
330 A%(A%,B%+4%)=(14%-A%-B%)*5%
340 NEXT B% : NEXT A%
350 FOR A%=3% TO 6% : FOR B%=3% TO 6%
360 A%(A%,B%)=A%(A%,B%)+10%
370 IF (A%=4% OR A%=5%) AND (B%=4% OR B%=5%) THEN A%(A%,B%)=A%(A%,B%)+20%
380 NEXT B% : NEXT A%
390 IF NOT F7% THEN 620
400 R2%=14% : GOSUB 2630 : ; "Ditt" : GOSUB 2630 : ; "drag" : GOSUB 2630 : ; "(eller" : GOSUB 2630
410 ; "I/]/P)" : GOSUB 2620
420 INPUT A$
430 \$=CHR$(ASC(A$) AND 223%)
440 IF LEN(A$)=0% THEN 400
450 X2%=INSTR(1%,"]PI",\$)
460 IF X2% THEN ON X2% GOSUB 2680,2790,3060 : GOSUB 2390 : GOTO 400
470 IF A$<>"0-0-0" THEN 490
480 H%=1% : I%=3% : F%=1% : G%=5% : GOSUB 650 : I%=4% : G%=1% : GOSUB 650 : N%=N%-1% : N%(N%)=4097% : GOTO 620
490 IF A$<>"0-0" THEN 510
500 H%=1% : I%=7% : F%=1% : G%=5% : GOSUB 650 : I%=6% : G%=8% : GOSUB 650 : N%=N%-1% : N%(N%)=4096% : GOTO 620
510 IF LEN(A$)=5% THEN A$=LEFT$(A$,2%)+RIGHT$(A$,4%)
520 IF LEN(A$)<>4% THEN 740
530 G%=INSTR(1%,A1$,\$)
540 F%=INSTR(1%,A2$,MID$(A$,2%,1%))
550 I%=INSTR(1%,A1$,CHR$(ASC(MID$(A$,3%,1%)) AND 223%))
560 H%=INSTR(1%,A2$,RIGHT$(A$,4%))
570 IF F%=0% OR G%=0% OR H%=0% OR I%=0% THEN 740
580 C%=C%(F%,G%)
590 IF C%=0% THEN GOSUB 2580 : ; "Det finns ingen pj{s p} ruta "LEFT$(A$,2%) : GOSUB 2590 : GOTO 400
600 IF C%<1000% THEN GOSUB 2580 : ; "Du f}r inte flytta mina pj{ser" : GOSUB 2590 : GOTO 400
610 GOSUB 650
620 R2%=5% : GOSUB 2630 : GOSUB 2630 : ; "Tyst" : GOSUB 2630 : ; "jag" : GOSUB 2630 : ; "t{nker"
630 FOR \7%=1% TO 13% : GOSUB 2630 : NEXT \7%
640 GOTO 850
650 C%=C%(F%,G%)
660 IF (C%=1% AND H%=1%) OR (C%=1001% AND H%=8%) THEN C%=C%+8%
670 C%(H%,I%)=C% : C%(F%,G%)=0%
680 IF N5% THEN RETURN
690 N%=N%+1%
700 N%(N%)=((I%*8%+H%)*8%+G%)*8%+F%-585%
710 GOSUB 730 : Y%=H% : X%=I% : GOSUB 2450 : GOSUB 730 : Y%=F% : X%=G% : GOSUB 2450 : GOTO 730
720 GOTO 2390
730 FOR \=1% TO 1500% : NEXT \ : RETURN
740 ; CHR$(12%);A$;"   {r inget till}tet drag."
750 ; : ; "F|ljande typer av drag {r till}tna:" : ;
760 GOSUB 3140
770 ; "Dessutom kan du svara n}got av f|ljande"
780 ; "(F|rsta bokstaven r{cker):" : ;
790 ; "   d)  ]ngra    Backa ett drag" : ;
800 ; "   e)  Proto    Skriv matchprotokoll" : ;
810 ; "   f)  Instr    Ge instrukioner" : ;
820 ; "Observera att rockaderna anges med"
830 ; "nollor."
840 GOSUB 2590 : GOTO 400
850 Q%=0% : W%=0% : M1%=0% : M3%=0% : E%=33%
860 FOR Z9%=1% TO 70% : V%(Z9%)=0% : NEXT Z9%
870 D%=1% : M%=1% : M2%=-1% : M4%=1% : M5%=6% : M6%=5%
880 S1%=-32767%
890 FOR X5%=1% TO 8%
900 FOR Y5%=1% TO 8%
910 O%=0% : V%=0% : P%=0%
920 G%=C%(X5%,Y5%)
930 IF G%=0% THEN 1740
940 IF G%>1000% XOR M%=2% THEN 1740
950 G%=G%-M3%
960 IF G%=99% THEN 1000
970 IF G%=103% THEN 1590
980 ON G% GOTO 1100,990,1240,990,1390,990,990,990,1580
990 STOP
1000 FOR X%=-1% TO 1%
1010 FOR Y%=-1% TO 1%
1020 IF X% OR Y% THEN GOSUB 1050
1030 NEXT Y% : NEXT X%
1040 GOTO 1740
1050 H%=X5%+X% : I%=Y5%+Y% : C%=C%(H%,I%)
1060 IF C%<0% OR O% THEN RETURN
1070 V%=-60%
1080 IF C%=0% THEN 1440
1090 IF M%=1% EQV C%>1000% THEN 1440 ELSE RETURN
1100 H%=X5%+M2% : I%=Y5%-1%
1110 IF H%=M4% THEN P%=800% : V%=-20%
1120 GOSUB 1150
1130 I%=Y5%+1% : GOSUB 1150
1140 I%=Y5% : GOSUB 1190 : GOTO 1740
1150 C%=C%(H%,I%)
1160 IF C%<=0% OR O% THEN RETURN
1170 IF M%=1% EQV C%<1000% THEN V%=-30 : RETURN
1180 GOTO 1710
1190 C%=C%(H%,I%)
1200 IF C% OR O% THEN RETURN ELSE GOSUB 1710
1210 IF H%<>M5% THEN RETURN
1220 H%=M6% : V%=-5% : C%=C%(H%,I%)
1230 IF C% OR O% THEN RETURN ELSE 1440
1240 H%=X5%+1% : I%=Y5%+2% : GOSUB 1330
1250 H%=X5%-1% : GOSUB 1330
1260 I%=Y5%-2% : GOSUB 1330
1270 H%=X5%+1% : GOSUB 1330
1280 H%=X5%+2% : I%=Y5%+1% : GOSUB 1330
1290 I%=Y5%-1% : GOSUB 1330
1300 H%=X5%-2% : GOSUB 1330
1310 I%=Y5%+1% : GOSUB 1330
1320 GOTO 1740
1330 IF H%<1% OR H%>8% OR O% THEN RETURN
1340 IF H%<1% OR H%>8% OR O% THEN RETURN
1350 IF I%<1% OR I%>8% THEN RETURN
1360 C%=C%(H%,I%)
1370 IF C%=0% THEN 1710
1380 IF M%=1% EQV C%>1000% THEN 1710 ELSE RETURN
1390 X%=1% : Y%=0% : GOSUB 1640
1400 X%=-1% : GOSUB 1640
1410 X%=0% : Y%=1% : GOSUB 1640
1420 Y%=-1% : GOSUB 1640
1430 IF Q%=1% THEN 1590 ELSE 1740
1440 IF M%=2% THEN 1520
1450 N2%(D%)=((I%*8%+H%)*8%+Y5%)*8%+X5%-585%
1460 IF C%>1000% THEN C%=C%-1000%
1470 IF C%=103% THEN C%=3%
1480 V%(D%)=C%*100%+A%(H%,I%)-A%(X5%,Y5%)+P%+V%
1490 D%=D%+1%
1500 V%(D%)=-32767%
1510 RETURN
1520 IF C%=103% THEN C%=3%
1530 S2%=C%*100%+A%(9%-H%,I%)-A%(9%-X5%,Y5%)+P%+V%+Z%
1540 Z%=0%
1550 IF S1%<S2% THEN S1%=S2%
1560 IF V%(W%)-S1%<V3% THEN O%=-1%
1570 RETURN
1580 Q%=1% : GOTO 1390
1590 Q%=0% : X%=1% : Y%=1% : GOSUB 1640
1600 X%=-1% : GOSUB 1640
1610 Y%=-1% : GOSUB 1640
1620 X%=1% : GOSUB 1640
1630 GOTO 1740
1640 H%=X5% : I%=Y5%
1650 I%=I%+Y% : H%=H%+X%
1660 C%=C%(H%,I%)
1670 IF C%<0% OR O% THEN RETURN
1680 IF C%=0% THEN GOSUB 1440 : GOTO 1650
1690 IF M%=1% EQV C%<1000% THEN RETURN
1700 Z%=-10%
1710 IF M%=1% THEN 1440
1720 IF C%(H%+1%,I%-1%)=1% OR C%(H%+1%,I%+1%)=1% THEN IF G%=103% THEN Z%=-300% ELSE Z%=-100%*G%
1730 GOTO 1440
1740 IF O% THEN 1760
1750 NEXT Y5% : NEXT X5%
1760 IF M%=2% THEN 1910
1770 M%=2% : M1%=8% : M2%=1% : M3%=1000% : M4%=8% : M5%=2% : M6%=3%
1780 FOR L%=1% TO 60%
1790 IF V%(L%)=-32767% THEN 1900
1800 GOSUB 1810 : GOTO 1860
1810 J%=L%
1820 FOR K%=L% TO 99%
1830 IF V%(K%)=-32767% THEN RETURN
1840 IF V%(K%)>V%(J%) THEN J%=K%
1850 NEXT K%
1860 IF J%=L% THEN 1890
1870 \%=V%(L%) : V%(L%)=V%(J%) : V%(J%)=\%
1880 \%=N2%(L%) : N2%(L%)=N2%(J%) : N2%(J%)=\%
1890 NEXT L%
1900 IF W%=0% THEN V3%=-32767% : GOTO 1970
1910 V%(W%)=V%(W%)-S1%
1920 IF V%(W%)>V3% THEN V3%=V%(W%)
1930 S1%=-32767%
1940 IF V%(W%+1%)=-32767% THEN 2060
1950 C%(F1%,G1%)=U%
1960 C%(H1%,I1%)=U1%
1970 W%=W%+1%
1980 N3%=N2%(W%) : GOSUB 2640
1990 F1%=F% : G1%=G% : H1%=H% : I1%=I%
2000 U%=C%(F1%,G1%)
2010 U1%=C%(H1%,I1%)
2020 C%(H1%,I1%)=U%
2030 IF U%=1% AND H1%=1% THEN C%(H1%,I1%)=9%
2040 C%(F1%,G1%)=0%
2050 GOTO 880
2060 C%(F1%,G1%)=U%
2070 C%(H1%,I1%)=U1%
2080 L%=1%
2090 GOSUB 1810
2100 N3%=N2%(J%) : GOSUB 2640
2110 IF V%(J%)>-3000% THEN 2140
2120 R2%=5% : GOSUB 2630 : ; "OK, du" : GOSUB 2630 : ; "vinner" : GOSUB 2630
2130 GOTO 2160
2140 IF V%(J%)<3000% THEN 2220
2150 R2%=4% : GOSUB 2630 : ; "Jag" : GOSUB 2630 : ; "har" : GOSUB 2630 : ; "vunnit" : GOSUB 2630
2160 R2%=8% : GOSUB 2630 : ; "Vill" : GOSUB 2630 : ; "du" : GOSUB 2630 : ; "spela"
2170 GOSUB 2630 : ; "igen" : GOSUB 2630
2180 INPUT A$ : A$=LEFT$(A$+" ",1%)
2190 IF A$="j" OR A$="J" THEN 20
2200 GOSUB 2580 : ; "OK, tack f|r god match";CUR(23%,0%)
2210 END
2220 R2%=6% : GOSUB 2630 : ; "Mitt" : GOSUB 2630 : ; "drag" : GOSUB 2620
2230 GOSUB 730 : GOSUB 2250 : GOSUB 650 : GOTO 400
2240 END
2250 ; MID$(A1$,G%,1%)MID$(A2$,F%,1%)"-";
2260 ; MID$(A1$,I%,1%)MID$(A2$,H%,1%)" ";
2270 RETURN
2280 FOR I%=0% TO 13% : A$(I%)="" : NEXT I%
2290 RESTORE 2370
2300 FOR ]%=0% TO 5%
2310 READ A$(]%)
2320 FOR [%=1% TO 12%
2330 A$(]%+6%)=A$(]%+6%)+CHR$( NOT ASC(MID$(A$(]%),[%,1%)) OR 32%)
2340 NEXT [% : NEXT ]%
2350 A$(12%)=STRING$(12%,127%)
2360 A$(13%)=SPACE$(12%)
2370 DATA "     j5 (.-$"," `t  / (,/$","hxt4  (//$"," h4  ~} (.-$"," tx  ~} (.-$"," ||  ~} (.-$"
2380 RETURN
2390 GOSUB 2580
2400 FOR \7%=0% TO 23% : ; CUR(\7%,0%)CHR$(151%)TAB(33%)CHR$(135%); : NEXT \7%
2410 FOR Y%=1% TO 8%
2420 FOR X%=1% TO 8%
2430 GOSUB 2450
2440 NEXT X% : NEXT Y% : RETURN
2450 IF F7% THEN Y1%=9%-Y% ELSE Y1%=Y%
2460 R%=(Y1%-1%)*3% : K%=(X%-1%)*4%+1%
2470 FOR \9%=1% TO 3%
2480 C%=C%(Y%,X%)
2490 GOSUB 2530
2500 IF C%=0% THEN P%=12%+((X%+Y1%) AND 1%)
2510 ; CUR(R%+\9%-1%,K%)MID$(A$(P%),4*\9%-3%,4%);
2520 NEXT \9% : RETURN
2530 P%=-F7%*6%
2540 IF C%>1000% THEN C%=C%-1000% : P%=6%-P%
2550 IF C%>100% THEN P%=P%+3% : RETURN
2560 IF C%=99% THEN P%=P%+5% : RETURN
2570 P%=P%+C%/2% : RETURN
2580 GOSUB 2610 : ; CUR(10%,3%); : RETURN
2590 GOSUB 2600 : GOTO 2390
2600 ; CUR(23%,0%)"Tryck p} return n{r du l{st f{rdigt"; : GET \$ : RETURN
2610 ; CHR$(12%); : RETURN
2620 GOSUB 2630
2630 ; CUR(R2%,34%);"      ";CUR(R2%,34%); : R2%=R2%+1% : RETURN
2640 F%=FNX : N3%=N3%/8%
2650 G%=FNX : N3%=N3%/8%
2660 H%=FNX : N3%=N3%/8%
2670 I%=FNX : RETURN
2680 IF N%<=2% THEN 20
2690 N%=N%-2%
2700 GOSUB 170
2710 FOR N1%=1% TO N%
2720 N3%=N%(N1%)
2730 IF N3%=4096% THEN C%(1%,5%)=0% : C%(1%,6%)=1005% : C%(1%,7%)=1099% : C%(1%,8%)=0% : GOTO 2770
2740 IF N3%=4097% THEN C%(1%,5%)=0% : C%(1%,4%)=1005% : C%(1%,3%)=1099% : C%(1%,1%)=0% : GOTO 2770
2750 GOSUB 2640
2760 N5%=-1% : GOSUB 650 : N5%=0%
2770 NEXT N1%
2780 RETURN
2790 FOR N1%=0% TO 199% STEP 40%
2800 GOSUB 2610
2810 IF N%<=N1%*2% THEN RETURN
2820 ; G2$" Matchprotokoll"TAB(30%)"Sid ";
2830 I1%=N1%/40%+1% : GOSUB 3030 : ; "(";
2840 I1%=(N%-1%)/80%+1% : GOSUB 3030 : ; ")"
2850 ; CHR$(151%)G2$"##############"
2860 ; "    Vit     Svart        Vit     Svart"
2870 FOR I1%=1% TO 20%
2880 ; : N2%=I1%+N1%
2890 GOSUB 2920
2900 ; TAB(21%); : N2%=N2%+20% : GOSUB 2920
2910 NEXT I1% : GET \$ : NEXT N1%
2920 IF 2%*N2%>N%+1% THEN RETURN
2930 A$=NUM$(N2%)
2940 ; RIGHT$(A$,LEN(A$)-1%)"  ";
2950 N3%=N%(2%*N2%-1%) AND 8191%
2960 GOSUB 2990 : ; "  ";
2970 IF 2%*N2%>N% THEN RETURN
2980 N3%=N%(2%*N2%) AND 8191%
2990 IF N3%=4096% THEN ; "0-0   "; : RETURN
3000 IF N3%=4097% THEN ; "0-0-0 "; : RETURN
3010 GOSUB 2640
3020 GOSUB 2250 : RETURN
3030 A$=NUM$(I1%)
3040 ; RIGHT$(A$,LEN(A$));
3050 RETURN
3060 GOSUB 2610 : ; G2$"    SCHACK" : ; CHR$(151%)G2$"   ######" : ;
3070 ; "Detta {r spelet schack. Du kan spela"
3080 ; "antingen svart eller vitt. Den som har"
3090 ; "vitt b|rjar." : ;
3100 ; "Alla schackdrag {r till}tna utom en"
3110 ; "passant." : ;
3120 ; "Dragen anges s} h{r:" : ;
3130 GOSUB 3140 : GOTO 3190
3140 ; "   D2-D4   Flytta pj{sen p} ruta"
3150 ; G2$"D2 till ruta D4" : ;
3160 ; "   0-0     Kort rokad" : ;
3170 ; "   0-0-0   L}ng rokad" : ;
3180 RETURN
3190 GOSUB 2600
3200 ; CHR$(12%) : ; "N{r det {r din tur kan du ocks} ange"
3210 ; "n}got av f|ljande:" : ;
3220 ; "   ]ngra   Flyttar tillbaka partiet"
3230 ; G2$"ett drag. Anv{nds om du"
3240 ; G2$"om du skrivit fel eller om"
3250 ; G2$"du vill prova olika drag" : ;
3260 ; "   Instr   Avbryter tillf{lligt partiet"
3270 ; G2$"och visar dessa instruktioner";
3280 ; G2$"Du kan forts{tta partiet"
3290 ; G2$"n{r du l{st f{rdigt" : ;
3300 ; "   Proto   Avbryter tillf{lligt partiet"
3310 ; G2$"och skriver ut ett protokoll"
3320 ; G2$"f|r partiet" : ;
3330 ; : ; "Du beh|ver bara ge f|rsta bokstaven."
3340 ; "Dvs du kan skriva ] i st{llet f|r ]ngra."
3350 GOSUB 2600
3360 RETURN