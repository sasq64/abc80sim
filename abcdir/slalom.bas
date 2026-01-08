1 REM Ins{nd av Peter \hl`n <4659>    1985-04-24 18.18.00
2 REM # Slalom    Ver 1.4
3 REM #  by The Great Programmer <4659>
4 REM #
5 REM #
6 A%=INP(3%)
7 DIM N$(8%)=20%,D9%(8%),F9%(8%),T(8%,2%),A%(500%),B%(500%)
8 RANDOMIZE 
9 GOTO 10000
10 A%=10% : B%=20%
480 K=19% : R1=1
483 G=PI/2 : G1=PI/15
484 K1=COS(G) : F=200%
485 X%=0%
490 ; CHR$(12%)
491 GOSUB 2000
492 ; CUR(9%,19%)''CUR(1%,2%)'Tryck p} RETURN n{r du vill starta';
494 GET A$ : IF ASC(A$)<>13% 494
495 POKE 65008%,255%,255%,255%
500 T%=INP(56%) AND 223%
510 IF T%=220% AND G<PI G=G+G1 : K1=COS(G) : R1=SIN(G)
515 IF T%=198% F=F*.95 : G1=G1*.95
520 IF T%=219% AND G>G1 G=G-G1 : K1=COS(G) : R1=SIN(G)
525 IF T%=196% F=F*1.05 : G1=G1*1.05
530 K=K+K1
535 R=R+ABS(R1)
540 IF R>1% R=R-1% : F7%=0% : GOSUB 2000
542 IF F7% 60000
545 IF K>39 K=39
546 IF K<0% K=0%
547 IF PEEK(31784%+K)=42% OUT 6%,0%,6%,137% : F9%=-1% : FOR F2%=0% TO 900% : NEXT F2% : M%=M%+1% ELSE F9%=0%
550 ; CUR(8%,K);'';
555 OUT 6%,251%
560 FOR F1%=0% TO F : NEXT F1%
562 L9%=0%
563 IF F<100% L9%=3%
564 IF F<75% L9%=255%
566 IF F<50% L9%=249%
568 IF F<30% L9%=1%
569 OUT 6%,L9%
580 IF P1%>500%/A% AND A%>3% A%=A%-1% : P1%=P1%-500%/A%
990 ; CUR(8%,K);' ';
999 GOTO 500
1500 REM  G\R BANA 
1505 B%(0%)=20% : B%=20% : A%(0%)=10% : A%=10%
1506 ; CHR$(12%)
1507 ; 'V{nta ett tag jag ska bara g|ra banan.'
1508 ; '        Kvar att g|ra:'
1510 IF B%>19% 1700
1520 IF B%*RND<6% [%=-[%
1690 GOTO 1900
1700 IF B%*RND>32% [%=-[%
1900 IF RND<.05 [%=-[%
1910 IF B%-A%/2%<5% [%=1%
1920 IF B%+A%/2%>33% [%=-1%
1930 IF S9% IF RND<.06 AND ABS([%)<4% [%=[%+[%
1940 IF RND<.03 AND ABS([%)>=2% [%=[%/2%
1990 B%=B%+[%
1991 IF RND<.95 GOTO 1995
1992 IF RND*A%>7% A%=A%-1% : GOTO 1995
1993 IF RND*A%<4% A%=A%+1% : GOTO 1995
1995 P%=100-FIX(X%*100/]%) : ; CUR(2%,26%-LEN(NUM$(P%)));P%' %'
1997 A%(X%)=A% : B%(X%)=B% : X%=X%+1% : IF X%<=]% 1510
1998 ; CUR(2%,24%);' 0 %'
1999 X%=0% : RETURN 
2000 IF X%>=]% 2020
2003 ; CUR(23%,0%);'****************************************'CUR(22%,B%(X%)-A%(X%)/2%)SPACE$(A%(X%))
2005 X%=X%+1%
2007 IF X%<]% RETURN 
2010 IF X%=]% ; CUR(23%,B%(X%)-1%)'M]L';
2015 RETURN 
2020 IF X%<]%+13% ; CUR(23%,0%) : X%=X%+1% : RETURN 
2022 IF PEEK(31912%+K)=42% F9%=-1%
2025 ; CUR(9%,K)''
2030 F7%=-1% : RETURN 
9999 REM  INSTRUKTIONER
10000 REM  PRESENTATION
10005 ; CHR$(12%);
10010 RESTORE 10900
10020 OUT 6%,121% : READ R%,K%
10040 FOR I%=0% TO 100% : NEXT I%
10050 ; CUR(R%,K%)'*';
10055 OUT 6%,0%
10060 IF R%<>12% OR K%<>37% 10020
10065 IF NOT Z9% Z9$='(Ja/Nej)'
10070 ; CUR(17%,4%)'Beh|ver du instruktioner 'Z9$' ';
10080 GET A$
10085 IF ASC(A$)=13% A$=Z8$
10090 IF A$='N' OR A$='n' Z8$='N' : Z9%=-1% : Z9$='(Nej)' : GOTO 11500
10100 IF A$<>'J' AND A$<>'j' 10080
10110 Z9%=-1% : Z8$='J' : Z9$='(Ja)'
10900 DATA 7,7,6,6,6,5,6,4,7,3,8,3,9,4,9,5,9,6,10,7,11,7,12,6,12,5,12,4,11,3
10910 DATA 6,9,7,9,8,9,9,9,10,9,11,9,12,9,12,10,12,11,12,12,12,13
10920 DATA 12,15,11,15,10,15,9,15,8,15,7,15,6,16,6,17,6,18,7,19,8,19,9,19,10,19,11,19,12,19,9,16,9,17,9,18
10930 DATA 6,21,7,21,8,21,9,21,10,21,11,21,12,21,12,22,12,23,12,24,12,25
10940 DATA 12,28,12,29,12,30,11,31,10,31,9,31,8,31,7,31,6,30,6,29,6,28,7,27,8,27,9,27,10,27,11,27
10950 DATA 12,33,11,33,10,33,9,33,8,33,7,33,6,33,7,34,8,35,9,35,8,35,7,36,6,37,7,37,8,37,9,37,10,37,11,37,12,37
11000 ; CHR$(12%) : ; '            SLALOM'
11010 ; CHR$(23%)'           ######'
11020 ; 'Spelets g}r ut p} att }ka s} fort som   m|jligt fr}n START till M]L. Det g}r'
11030 ; 'fortare att }ka i banan {n ute i skogen.' : ; ' Skog och bana ser ut s} h{r:'
11040 ; : ; '********************        *******************************       **************';
11050 ; '******************      ********************************      ******************';
11060 ; '*** S K O G **** BANA  **** S K O G *********************       ****************';
11070 ; '*******************        *********************************        ************';
11080 ; '**********************          ********'
11100 ; : ; : ; '  Tryck p} RETURN n{r du l{st klart';
11110 GET A$ : IF ASC(A$)<>13% 11110
11120 ; CUR(3%,0%)SPACE$(839%);CUR(3%,0%);
11130 ; 'Sv{nger g|r du med tangenterna:'
11140 ; : ; '         \              ['
11150 ; '        <--            -->'
11160 ; '  I f|rh}llande till f{rdriktningen'
11170 ; : ; ' Tycker Man att det g}r f|r l}ngsamt    eller f|r fort kan man {ndra farten:'
11180 ; : ; '  D - Minskar farten.                     F - \kar farten.'
11190 ; 'Vid h|g fart kan man inte g|ra specielltsn{va sv{ngar, och motsatt f|rh}llande'
11200 ; 'vid l}g fart.'
11210 ; : ; 'Nyb|rjare kan r}das att inte anv{nda    tangenterna D och F.'
11220 ; : ; : ; '   Tryck p} RETURN';
11230 GET Q$ : IF ASC(Q$)<>13% 11230
11500 IF NOT Z1% Z1$='1-9'
11503 ONERRORGOTO 11503 : ; CHR$(12%) : ; 'Hur m}nga }kare ('Z1$') '; : GET S$
11504 IF ASC(S$)=13% AND Z1% S$=Z1$
11505 S%=VAL(S$)
11506 Z1%=-1% : Z1$=S$
11507 ; S$
11508 IF S%<1% OR S%>9% 11500
11510 S%=S%-1%
11512 FOR I%=0% TO S%
11514 ; : ; ']kare no.'I%+1%' heter:    (Max. 20 tkn)'
11516 INPUTLINE N$ : ; : N$=LEFT$(N$,LEN(N$)-2%) : IF LEN(N$)>20% N$=LEFT$(N$,20%)
11518 N$(I%)=N$ : NEXT I%
11520 IF NOT Z2% Z2$='50-1000'
11528 ONERRORGOTO 11528 : ; : ; 'Hur m}nga stegs bana ('Z2$') ';
11530 INPUTLINE A$ : IF ASC(A$)=13% A$=Z2$ ELSE A$=LEFT$(A$,LEN(A$)-2%) : Z2$=''
11535 ]%=VAL(A$)
11540 IF ]%<50% OR ]%>500% ; CHR$(7%) : GOTO 11520
11541 IF Z2% ; Z2$
11542 ONERRORGOTO 0 : Z2%=-1%
11544 Z2$=RIGHT$(NUM$(]%),2%)
11550 IF NOT Z3% Z3$='1-2'
11560 ; : ; : ; 'V{lj sv}righetsgrad ('Z3$') ';
11570 GET A$
11575 IF ASC(A$)=13% IF Z3% A$=Z3$ ELSE 11570
11580 IF A$='1' S9%=0% ELSE IF A$='2' S9%=-1% ELSE 11570
11590 ; A$ : Z3$=A$ : Z3%=-1%
11990 IF RND<.5 [%=1% ELSE [%=-1%
11999 REM  HUVUDPROGRAM
12000 FOR S2%=0% TO 1%
12001 X%=0%
12005 GOSUB 1500
12006 ; : ; : ; : ; 'Ni ska nu bekanta er med banan.          (Tryck RETURN)';
12007 GET A$ : IF ASC(A$)<>13% 12007
12008 GOSUB 2000 : FOR I%=0% TO 200% : NEXT I% : IF X%<]% 12008
12010 FOR S1%=0% TO S%
12015 IF D9%(S1%) 12080
12020 ; : ; '>>> ]K no.'S2%+1%
12030 ; '>>> ]kare no.'S1%+1%'  'N$(S1%)
12040 ; : ; '>>> Tryck p} RETURN n{r du {r redo';
12050 GET A$ : IF ASC(A$)<>13% 12050
12060 GOSUB 10
12070 GOSUB 13000
12080 NEXT S1%
12090 ; CHR$(12%) : ; '>>> SLUTST[LLNING';
12095 S1%=S1%-1%
12100 IF S2%=0% ; '   ]K 1' : ; '>>>' : GOSUB 13005 : NEXT S2% ELSE ; '   TOTALT'
12105 ; '>>>'
12110 GOSUB 13005
12120 ; : ; : ; 'Vill du spela igen (Ja) ';
12130 GET A$
12140 IF A$='n' OR A$='N' ; 'Nej' : POKE 65013%,0% : END 
12150 IF A$<>'j' AND A$<>'J' AND ASC(A$)<>13% 12130
12160 FOR I%=0% TO 8%
12170 D9%(I%)=0% : N$(I%)='' : T(I%,0%)=0% : T(I%,1%)=0% : F9%(I%)=0%
12175 D9%=0%
12180 NEXT I%
12999 GOTO 1
13000 ; : ; 
13005 FOR I%=0% TO S1%-D9%
13006 IF D9%(F9%(I%)) AND I%=0% RETURN 
13007 IF D9%(F9%(I%)) 13900
13010 IF I%>0% IF T(F9%(I%),2%)=T(F9%(I%-1%),2%) ; '>>>    'N$(F9%(I%)); : GOTO 13030
13020 ; '>>>'I%+1%'  'N$(F9%(I%));
13030 S=T(F9%(I%),2%) : GOSUB 14000
13040 ; CUR(PEEK(65011%),39%-LEN(T$))T$
13050 NEXT I%
13900 ; : ; '   Tryck p} RETURN';
13910 GET A$ : IF ASC(A$)<>13% 13910
13980 ; : ; 
13990 RETURN 
14000 REM  TID-UTSKRIFT
14010 M=FIX(S/60) : S=S-M*60
14020 H=FIX(M/60) : M=M-H*60
14030 T$=NUM$(INT(S))+'.'+RIGHT$(LEFT$(NUM$(S-INT(S))+'00',4%),3%)
14040 IF M>0% OR H>0% T$=NUM$(M)+'.'+RIGHT$(T$,2%)
14050 RETURN 
14999 REM  INSORTERA
15000 F9%(S1%)=S1%
15010 IF S1%=0% RETURN 
15020 FOR I%=0% TO S1%-1%
15030 FOR J%=I%+1% TO S1%
15040 IF T(F9%(I%),2%)>T(F9%(J%),2%) X9%=F9%(I%) : F9%(I%)=F9%(J%) : F9%(J%)=X9%
15050 NEXT J% : NEXT I%
15099 RETURN 
60000 REM  SPELET SLUT 
60001 OUT 6%,0%
60005 IF F9% OR K<1 OR K>38 T(S1%,2%)=1E+126 ELSE 60010
60007 ; CUR(12%,1%);'Du }kte aldrig in i m}let, f|ljaktligen blir du diskad...' : D9%(S1%)=-1% : D9%=D9%+1%
60008 GOTO 60100
60010 S=(PEEK(65008%) XOR 255)/50+(PEEK(65009) XOR 255)*5.12+(PEEK(65010) XOR 255)*1310.72
60015 T(S1%,S2%)=S : T(S1%,2%)=T(S1%,0%)+T(S1%,1%)
60020 GOSUB 14000
60050 ; CUR(11%,7%);'Tid:';CUR(11%,25%-LEN(T$));T$
60100 ; : ; : ; '   Tryck p} RETURN';
60110 GET A$ : IF ASC(A$)<>13% 60110
60120 GOSUB 15000
60130 RETURN 
