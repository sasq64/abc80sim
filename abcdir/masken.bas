10 ; CHR$(12%)
20 ; CUR(0%,15%)"MASKEN"CUR(1%,14%)CHR$(23)"######"
30 ; : ; "Hej! I det h|r spelet {r du"
40 ; "'maskf|rare'"
50 ; : ; "Du ska styra masken runt p} sk{rmen"
60 ; "p} jakt efter rutgrupperna."
70 ; : ; "Du styr }t ena h}llet med Z"
80 ; "och }t andra h}llet med -"
90 ; : ; "Om du styr in i din egen svans,"
100 ; "eller om du krockar med ramen"
110 ; "har du f|rlorat!"
120 ; : ; "OBS n{r masken g}r mot dig p} "
130 ; "sk{rmen ger - en sv{ng }t v{nster osv."
140 ; : ; "Gl|m inte UPPER CASE!"
150 ; : ; : ; "TRYCK N[R DU [R KLAR"; : GET A$
160 X%=26% : Y%=15%
170 DIM M1%(500%),M2%(500%),B$=30
180 DIM S%(5%),T%(1000%),D%(1000%)
190 F9%=75%
200 T3%=0% : P%=0% : R%=0%
210 FOR I%=65081% TO 65085%
220 S%(I%-65081%)=PEEK(I%) : NEXT I%
230 ; CHR$(12%)
240 FOR I%=1% TO 24% : ; : ; CHR$(23%); : NEXT I%
250 FOR I%=3% TO 78% : SETDOT 2%,I% : SETDOT 69%,I% : NEXT I%
260 FOR I%=2% TO 68% : SETDOT I%,3% : SETDOT I%,78% : NEXT I%
270 ; CUR(11%,5%)"OOOO\"
280 FOR I%=1% TO 5% : M1%(I%+250)=11% : M2%(256%-I%)=4%+I% : NEXT I%
290 B%=251% : E%=255% : F%=0%
300 FOR D=-2000*R% TO 2000 : NEXT D
310 T2%=-1% : GOSUB 570
320 T3%=1%+T3%
330 IF T3%=T7% T7%=0% : R%=0% : GOSUB 1010
340 IF NOT R% THEN 380
350 IF T3%<>T%(P%) THEN 430
360 A%=D%(P%) : P%=1%+P%
370 GOTO 420
380 A%=PEEK(65013%)
390 IF A%=0% THEN 430
400 A%=INP(56%) AND 127% : POKE 65013%,0%
410 T%(P%)=T3% : D%(P%)=A% : P%=1%+P%
420 F%=(F%+(A%=45%)-(A%=90%)) AND 3%
430 M1%(B%-1%)=M1%(B%)+(F%=1%)-(F%=3%)
440 M2%(B%-1%)=M2%(B%)+(F%=2%)-(F%=0%)
450 W%=M1%(B%-1%) : Z%=M2%(B%-1%)
460 IF (W%=Y% OR W%=Y%+1%) AND (Z%=X% OR Z%=X%+1%) THEN GOSUB 580 : GOTO 460
470 IF DOT(W%*3%,Z%*2%) 660
480 IF W%<1% OR W%>22% OR Z%<2% OR Z%>38% THEN 660
490 ; CUR(M1%(B%),M2%(B%));"O";
500 ; CUR(M1%(B%-1%),M2%(B%-1%));"\"
510 IF RND>.1 ; CUR(M1%(E%),M2%(E%));" " : E%=E%-1%
520 IF E%=-1% E%=499%
530 B%=B%-1%
540 IF B%=0% THEN M1%(500%)=M1%(0%) : M2%(500%)=M2%(0%) : B%=500%
550 FOR D=-F9%*R% TO F9% : NEXT D
560 GOTO 320
570 REM  TR[FF 
580 OUT 6,7 : A%=0% : T2%=T2%+1%
590 ; CUR(Y%,X%)"  " : ; CUR(Y%+1%,X%)"  "
600 X%=RND*36+2% : Y%=RND*21+1%
610 FOR G%=X%*2% TO X%*2%+2% : FOR T%=Y%*3% TO Y%*3%+3% : IF DOT(T%,G%) 600
620 NEXT T% : NEXT G%
630 ; CUR(Y%,X%);"ff";
640 ; CUR(Y%+1%,X%);"99";
645 ; CUR(0%,0%)T2%CHR$(23%);
650 OUT 6,0 : OUT 6,17 : RETURN
660 REM    
670 FOR D=1 TO 300 : OUT 6,11 : NEXT D : OUT 6,0
680 FOR D=1 TO 1000 : NEXT D
690 ; CHR$(12%) : ; : ;
700 ; "Du tr{ffade"T2%" m}l."
710 B$="URKASST..."
720 IF T2%>0% B$="Inte riktigt bra."
730 IF T2%>5% B$="Bravo..."
740 IF T2%>10% B$="Mycket bra..."
750 IF T2%>25% B$="Fenomenalt..."
760 IF T2%>50% B$="Du {r ju PROFFS..."
770 IF T2%>100% B$="*** SUPERBRA ***"
780 ; : ; B$
789 ;
790 IF T2%>30% ; "Du borde bli maskf|rare p} heltid."
800 IF A8% THEN 900
810 IF R% THEN 930
820 ; : ; "Vill du ha repris"; : POKE 65013%,0% : INPUT A$
830 T7%=0%
840 ONERRORGOTO 860
850 T7%=T3%-VAL(A$)
860 ONERRORGOTO 0
870 IF T7% 900
880 IF INSTR(1,"Jj",CHR$(ASC(A$))) 900
890 GOTO 930
900 FOR I%=65081% TO 65085%
910 POKE I%,S%(I%-65081%) : NEXT I%
920 R%=-1% : T3%=0% : P%=0% : GOTO 230
930 ; : ; "Vill du {ndra fart"; : POKE 65013%,0% : INPUT A$
940 IF INSTR(1,"Jj",CHR$(ASC(A$))) 980
950 ; : ; "Vill du f|rs|ka en g}ng till"; : POKE 65013%,0% : INPUT A$
960 IF INSTR(1,"Nn",CHR$(ASC(A$))) END
970 GOTO 200
980 ; "fart 1,2,3 eller 4 "; : INPUT A9%
990 IF A9%<5% AND A9%>0% THEN F9%=(5%-A9%)*25% : GOTO 200
1000 A8%=-1% : GOTO 900
1010 POKE 65013%,0
1020 IF PEEK(65013%)=0% 1020
1030 RETURN