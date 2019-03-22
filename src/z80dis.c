#include "z80.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* const Mnemonics[256] = {
    "NOP",       "LD BC,#h",   "LD (BC),A",  "INC BC",     "INC B",
    "DEC B",     "LD B,*h",    "RLCA",       "EX AF,AF'",  "ADD HL,BC",
    "LD A,(BC)", "DEC BC",     "INC C",      "DEC C",      "LD C,*h",
    "RRCA",      "DJNZ @h",    "LD DE,#h",   "LD (DE),A",  "INC DE",
    "INC D",     "DEC D",      "LD D,*h",    "RLA",        "JR @h",
    "ADD HL,DE", "LD A,(DE)",  "DEC DE",     "INC E",      "DEC E",
    "LD E,*h",   "RRA",        "JR NZ,@h",   "LD HL,#h",   "LD (#h),HL",
    "INC HL",    "INC H",      "DEC H",      "LD H,*h",    "DAA",
    "JR Z,@h",   "ADD HL,HL",  "LD HL,(#h)", "DEC HL",     "INC L",
    "DEC L",     "LD L,*h",    "CPL",        "JR NC,@h",   "LD SP,#h",
    "LD (#h),A", "INC SP",     "INC (HL)",   "DEC (HL)",   "LD (HL),*h",
    "SCF",       "JR C,@h",    "ADD HL,SP",  "LD A,(#h)",  "DEC SP",
    "INC A",     "DEC A",      "LD A,*h",    "CCF",        "LD B,B",
    "LD B,C",    "LD B,D",     "LD B,E",     "LD B,H",     "LD B,L",
    "LD B,(HL)", "LD B,A",     "LD C,B",     "LD C,C",     "LD C,D",
    "LD C,E",    "LD C,H",     "LD C,L",     "LD C,(HL)",  "LD C,A",
    "LD D,B",    "LD D,C",     "LD D,D",     "LD D,E",     "LD D,H",
    "LD D,L",    "LD D,(HL)",  "LD D,A",     "LD E,B",     "LD E,C",
    "LD E,D",    "LD E,E",     "LD E,H",     "LD E,L",     "LD E,(HL)",
    "LD E,A",    "LD H,B",     "LD H,C",     "LD H,D",     "LD H,E",
    "LD H,H",    "LD H,L",     "LD H,(HL)",  "LD H,A",     "LD L,B",
    "LD L,C",    "LD L,D",     "LD L,E",     "LD L,H",     "LD L,L",
    "LD L,(HL)", "LD L,A",     "LD (HL),B",  "LD (HL),C",  "LD (HL),D",
    "LD (HL),E", "LD (HL),H",  "LD (HL),L",  "HALT",       "LD (HL),A",
    "LD A,B",    "LD A,C",     "LD A,D",     "LD A,E",     "LD A,H",
    "LD A,L",    "LD A,(HL)",  "LD A,A",     "ADD B",      "ADD C",
    "ADD D",     "ADD E",      "ADD H",      "ADD L",      "ADD (HL)",
    "ADD A",     "ADC B",      "ADC C",      "ADC D",      "ADC E",
    "ADC H",     "ADC L",      "ADC (HL)",   "ADC,A",      "SUB B",
    "SUB C",     "SUB D",      "SUB E",      "SUB H",      "SUB L",
    "SUB (HL)",  "SUB A",      "SBC B",      "SBC C",      "SBC D",
    "SBC E",     "SBC H",      "SBC L",      "SBC (HL)",   "SBC A",
    "AND B",     "AND C",      "AND D",      "AND E",      "AND H",
    "AND L",     "AND (HL)",   "AND A",      "XOR B",      "XOR C",
    "XOR D",     "XOR E",      "XOR H",      "XOR L",      "XOR (HL)",
    "XOR A",     "OR B",       "OR C",       "OR D",       "OR E",
    "OR H",      "OR L",       "OR (HL)",    "OR A",       "CP B",
    "CP C",      "CP D",       "CP E",       "CP H",       "CP L",
    "CP (HL)",   "CP A",       "RET NZ",     "POP BC",     "JP NZ,$h",
    "JP $h",     "CALL NZ,$h", "PUSH BC",    "ADD *h",     "RST 00h",
    "RET Z",     "RET",        "JP Z,$h",    "DEFB CBh",   "CALL Z,$h",
    "CALL $h",   "ADC *h",     "RST 08h",    "RET NC",     "POP DE",
    "JP NC,$h",  "OUT (*h),A", "CALL NC,$h", "PUSH DE",    "SUB *h",
    "RST 10h",   "RET C",      "EXX",        "JP C,$h",    "IN A,(*h)",
    "CALL C,$h", "DEFB DDh",   "SBC *h",     "RST 18h",    "RET PO",
    "POP HL",    "JP PO,$h",   "EX HL,(SP)", "CALL PO,$h", "PUSH HL",
    "AND *h",    "RST 20h",    "RET PE",     "JP HL",      "JP PE,#h",
    "EX DE,HL",  "CALL PE,$h", "DEFB EDh",   "XOR *h",     "RST 28h",
    "RET P",     "POP AF",     "JP P,$h",    "DI",         "CALL P,$h",
    "PUSH AF",   "OR *h",      "RST 30h",    "RET M",      "LD SP,HL",
    "JP M,$h",   "EI",         "CALL M,$h",  "DEFB FDh",   "CP *h",
    "RST 38h"};

static const char* const MnemonicsCB[256] = {
    "RLC B",      "RLC C",   "RLC D",      "RLC E",   "RLC H",      "RLC L",
    "RLC (HL)",   "RLC A",   "RRC B",      "RRC C",   "RRC D",      "RRC E",
    "RRC H",      "RRC L",   "RRC (HL)",   "RRC A",   "RL B",       "RL C",
    "RL D",       "RL E",    "RL H",       "RL L",    "RL (HL)",    "RL A",
    "RR B",       "RR C",    "RR D",       "RR E",    "RR H",       "RR L",
    "RR (HL)",    "RR A",    "SLA B",      "SLA C",   "SLA D",      "SLA E",
    "SLA H",      "SLA L",   "SLA (HL)",   "SLA A",   "SRA B",      "SRA C",
    "SRA D",      "SRA E",   "SRA H",      "SRA L",   "SRA (HL)",   "SRA A",
    "SLL B",      "SLL C",   "SLL D",      "SLL E",   "SLL H",      "SLL L",
    "SLL (HL)",   "SLL A",   "SRL B",      "SRL C",   "SRL D",      "SRL E",
    "SRL H",      "SRL L",   "SRL (HL)",   "SRL A",   "BIT 0,B",    "BIT 0,C",
    "BIT 0,D",    "BIT 0,E", "BIT 0,H",    "BIT 0,L", "BIT 0,(HL)", "BIT 0,A",
    "BIT 1,B",    "BIT 1,C", "BIT 1,D",    "BIT 1,E", "BIT 1,H",    "BIT 1,L",
    "BIT 1,(HL)", "BIT 1,A", "BIT 2,B",    "BIT 2,C", "BIT 2,D",    "BIT 2,E",
    "BIT 2,H",    "BIT 2,L", "BIT 2,(HL)", "BIT 2,A", "BIT 3,B",    "BIT 3,C",
    "BIT 3,D",    "BIT 3,E", "BIT 3,H",    "BIT 3,L", "BIT 3,(HL)", "BIT 3,A",
    "BIT 4,B",    "BIT 4,C", "BIT 4,D",    "BIT 4,E", "BIT 4,H",    "BIT 4,L",
    "BIT 4,(HL)", "BIT 4,A", "BIT 5,B",    "BIT 5,C", "BIT 5,D",    "BIT 5,E",
    "BIT 5,H",    "BIT 5,L", "BIT 5,(HL)", "BIT 5,A", "BIT 6,B",    "BIT 6,C",
    "BIT 6,D",    "BIT 6,E", "BIT 6,H",    "BIT 6,L", "BIT 6,(HL)", "BIT 6,A",
    "BIT 7,B",    "BIT 7,C", "BIT 7,D",    "BIT 7,E", "BIT 7,H",    "BIT 7,L",
    "BIT 7,(HL)", "BIT 7,A", "RES 0,B",    "RES 0,C", "RES 0,D",    "RES 0,E",
    "RES 0,H",    "RES 0,L", "RES 0,(HL)", "RES 0,A", "RES 1,B",    "RES 1,C",
    "RES 1,D",    "RES 1,E", "RES 1,H",    "RES 1,L", "RES 1,(HL)", "RES 1,A",
    "RES 2,B",    "RES 2,C", "RES 2,D",    "RES 2,E", "RES 2,H",    "RES 2,L",
    "RES 2,(HL)", "RES 2,A", "RES 3,B",    "RES 3,C", "RES 3,D",    "RES 3,E",
    "RES 3,H",    "RES 3,L", "RES 3,(HL)", "RES 3,A", "RES 4,B",    "RES 4,C",
    "RES 4,D",    "RES 4,E", "RES 4,H",    "RES 4,L", "RES 4,(HL)", "RES 4,A",
    "RES 5,B",    "RES 5,C", "RES 5,D",    "RES 5,E", "RES 5,H",    "RES 5,L",
    "RES 5,(HL)", "RES 5,A", "RES 6,B",    "RES 6,C", "RES 6,D",    "RES 6,E",
    "RES 6,H",    "RES 6,L", "RES 6,(HL)", "RES 6,A", "RES 7,B",    "RES 7,C",
    "RES 7,D",    "RES 7,E", "RES 7,H",    "RES 7,L", "RES 7,(HL)", "RES 7,A",
    "SET 0,B",    "SET 0,C", "SET 0,D",    "SET 0,E", "SET 0,H",    "SET 0,L",
    "SET 0,(HL)", "SET 0,A", "SET 1,B",    "SET 1,C", "SET 1,D",    "SET 1,E",
    "SET 1,H",    "SET 1,L", "SET 1,(HL)", "SET 1,A", "SET 2,B",    "SET 2,C",
    "SET 2,D",    "SET 2,E", "SET 2,H",    "SET 2,L", "SET 2,(HL)", "SET 2,A",
    "SET 3,B",    "SET 3,C", "SET 3,D",    "SET 3,E", "SET 3,H",    "SET 3,L",
    "SET 3,(HL)", "SET 3,A", "SET 4,B",    "SET 4,C", "SET 4,D",    "SET 4,E",
    "SET 4,H",    "SET 4,L", "SET 4,(HL)", "SET 4,A", "SET 5,B",    "SET 5,C",
    "SET 5,D",    "SET 5,E", "SET 5,H",    "SET 5,L", "SET 5,(HL)", "SET 5,A",
    "SET 6,B",    "SET 6,C", "SET 6,D",    "SET 6,E", "SET 6,H",    "SET 6,L",
    "SET 6,(HL)", "SET 6,A", "SET 7,B",    "SET 7,C", "SET 7,D",    "SET 7,E",
    "SET 7,H",    "SET 7,L", "SET 7,(HL)", "SET 7,A"};

static const char* const MnemonicsED[256] = {
    "??ED00",     "??ED01",     "??ED02",     "??ED03",     "??ED04",
    "??ED05",     "??ED06",     "??ED07",     "??ED08",     "??ED09",
    "??ED0A",     "??ED0B",     "??ED0C",     "??ED0D",     "??ED0E",
    "??ED0F",     "??ED10",     "??ED11",     "??ED12",     "??ED13",
    "??ED14",     "??ED15",     "??ED16",     "??ED17",     "??ED18",
    "??ED19",     "??ED1A",     "??ED1B",     "??ED1C",     "??ED1D",
    "??ED1E",     "??ED1F",     "??ED20",     "??ED21",     "??ED22",
    "??ED23",     "??ED24",     "??ED25",     "??ED26",     "??ED27",
    "??ED28",     "??ED29",     "??ED2A",     "??ED2B",     "??ED2C",
    "??ED2D",     "??ED2E",     "??ED2F",     "??ED30",     "??ED31",
    "??ED32",     "??ED33",     "??ED34",     "??ED35",     "??ED36",
    "??ED37",     "??ED38",     "??ED39",     "??ED3A",     "??ED3B",
    "??ED3C",     "??ED3D",     "??ED3E",     "??ED3F",     "IN B,(C)",
    "OUT (C),B",  "SBC HL,BC",  "LD (#h),BC", "NEG",        "RETN",
    "IM 0",       "LD I,A",     "IN C,(C)",   "OUT (C),C",  "ADC HL,BC",
    "LD BC,(#h)", "??ED4C",     "RETI",       "??ED4E",     "LD R,A",
    "IN D,(C)",   "OUT (C),D",  "SBC HL,DE",  "LD (#h),DE", "??ED54",
    "??ED55",     "IM 1",       "LD A,I",     "IN E,(C)",   "OUT (C),E",
    "ADC HL,DE",  "LD DE,(#h)", "??ED5C",     "??ED5D",     "IM 2",
    "LD A,R",     "IN H,(C)",   "OUT (C),H",  "SBC HL,HL",  "LD (#h),HL",
    "??ED64",     "??ED65",     "??ED66",     "RRD",        "IN L,(C)",
    "OUT (C),L",  "ADC HL,HL",  "LD HL,(#h)", "??ED6C",     "??ED6D",
    "??ED6E",     "RLD",        "IN F,(C)",   "??ED71",     "SBC HL,SP",
    "LD (#h),SP", "??ED74",     "??ED75",     "??ED76",     "??ED77",
    "IN A,(C)",   "OUT (C),A",  "ADC HL,SP",  "LD SP,(#h)", "??ED7C",
    "??ED7D",     "??ED7E",     "??ED7F",     "??ED80",     "??ED81",
    "??ED82",     "??ED83",     "??ED84",     "??ED85",     "??ED86",
    "??ED87",     "??ED88",     "??ED89",     "??ED8A",     "??ED8B",
    "??ED8C",     "??ED8D",     "??ED8E",     "??ED8F",     "??ED90",
    "??ED91",     "??ED92",     "??ED93",     "??ED94",     "??ED95",
    "??ED96",     "??ED97",     "??ED98",     "??ED99",     "??ED9A",
    "??ED9B",     "??ED9C",     "??ED9D",     "??ED9E",     "??ED9F",
    "LDI",        "CPI",        "INI",        "OUTI",       "??EDA4",
    "??EDA5",     "??EDA6",     "??EDA7",     "LDD",        "CPD",
    "IND",        "OUTD",       "??EDAC",     "??EDAD",     "??EDAE",
    "??EDAF",     "LDIR",       "CPIR",       "INIR",       "OTIR",
    "??EDB4",     "??EDB5",     "??EDB6",     "??EDB7",     "LDDR",
    "CPDR",       "INDR",       "OTDR",       "??EDBC",     "??EDBD",
    "??EDBE",     "??EDBF",     "??EDC0",     "??EDC1",     "??EDC2",
    "??EDC3",     "??EDC4",     "??EDC5",     "??EDC6",     "??EDC7",
    "??EDC8",     "??EDC9",     "??EDCA",     "??EDCB",     "??EDCC",
    "??EDCD",     "??EDCE",     "??EDCF",     "??EDD0",     "??EDD1",
    "??EDD2",     "??EDD3",     "??EDD4",     "??EDD5",     "??EDD6",
    "??EDD7",     "??EDD8",     "??EDD9",     "??EDDA",     "??EDDB",
    "??EDDC",     "??EDDD",     "??EDDE",     "??EDDF",     "??EDE0",
    "??EDE1",     "??EDE2",     "??EDE3",     "??EDE4",     "??EDE5",
    "??EDE6",     "??EDE7",     "??EDE8",     "??EDE9",     "??EDEA",
    "??EDEB",     "??EDEC",     "??EDED",     "??EDEE",     "??EDEF",
    "??EDF0",     "??EDF1",     "??EDF2",     "??EDF3",     "??EDF4",
    "??EDF5",     "??EDF6",     "??EDF7",     "??EDF8",     "??EDF9",
    "??EDFA",     "??EDFB",     "??EDFC",     "??EDFD",     "??EDFE",
    "??EDFF"};

static const char* const MnemonicsXX[256] = {
    "NOP",         "LD BC,#h",    "LD (BC),A",   "INC BC",      "INC B",
    "DEC B",       "LD B,*h",     "RLCA",        "EX AF,AF'",   "ADD I%,BC",
    "LD A,(BC)",   "DEC BC",      "INC C",       "DEC C",       "LD C,*h",
    "RRCA",        "DJNZ @h",     "LD DE,#h",    "LD (DE),A",   "INC DE",
    "INC D",       "DEC D",       "LD D,*h",     "RLA",         "JR @h",
    "ADD I%,DE",   "LD A,(DE)",   "DEC DE",      "INC E",       "DEC E",
    "LD E,*h",     "RRA",         "JR NZ,@h",    "LD I%,#h",    "LD (#h),I%",
    "INC I%",      "INC I%H",     "DEC I%H",     "LD I%Xh,*h",  "DAA",
    "JR Z,@h",     "ADD I%,I%",   "LD I%,(#h)",  "DEC I%",      "INC I%L",
    "DEC I%L",     "LD I%L,*h",   "CPL",         "JR NC,@h",    "LD SP,#h",
    "LD (#h),A",   "INC SP",      "INC (I%+h)",  "DEC (I%+h)",  "LD (I%+h),*h",
    "SCF",         "JR C,@h",     "ADD I%,SP",   "LD A,(#h)",   "DEC SP",
    "INC A",       "DEC A",       "LD A,*h",     "CCF",         "LD B,B",
    "LD B,C",      "LD B,D",      "LD B,E",      "LD B,I%H",    "LD B,I%L",
    "LD B,(I%+h)", "LD B,A",      "LD C,B",      "LD C,C",      "LD C,D",
    "LD C,E",      "LD C,I%H",    "LD C,I%L",    "LD C,(I%+h)", "LD C,A",
    "LD D,B",      "LD D,C",      "LD D,D",      "LD D,E",      "LD D,I%H",
    "LD D,I%L",    "LD D,(I%+h)", "LD D,A",      "LD E,B",      "LD E,C",
    "LD E,D",      "LD E,E",      "LD E,I%H",    "LD E,I%L",    "LD E,(I%+h)",
    "LD E,A",      "LD I%H,B",    "LD I%H,C",    "LD I%H,D",    "LD I%H,E",
    "LD I%H,I%H",  "LD I%H,I%L",  "LD H,(I%+h)", "LD I%H,A",    "LD I%L,B",
    "LD I%L,C",    "LD I%L,D",    "LD I%L,E",    "LD I%L,I%H",  "LD I%L,I%L",
    "LD L,(I%+h)", "LD I%L,A",    "LD (I%+h),B", "LD (I%+h),C", "LD (I%+h),D",
    "LD (I%+h),E", "LD (I%+h),H", "LD (I%+h),L", "HALT",        "LD (I%+h),A",
    "LD A,B",      "LD A,C",      "LD A,D",      "LD A,E",      "LD A,I%H",
    "LD A,I%L",    "LD A,(I%+h)", "LD A,A",      "ADD B",       "ADD C",
    "ADD D",       "ADD E",       "ADD I%H",     "ADD I%L",     "ADD (I%+h)",
    "ADD A",       "ADC B",       "ADC C",       "ADC D",       "ADC E",
    "ADC I%H",     "ADC I%L",     "ADC (I%+h)",  "ADC,A",       "SUB B",
    "SUB C",       "SUB D",       "SUB E",       "SUB I%H",     "SUB I%L",
    "SUB (I%+h)",  "SUB A",       "SBC B",       "SBC C",       "SBC D",
    "SBC E",       "SBC I%H",     "SBC I%L",     "SBC (I%+h)",  "SBC A",
    "AND B",       "AND C",       "AND D",       "AND E",       "AND I%H",
    "AND I%L",     "AND (I%+h)",  "AND A",       "XOR B",       "XOR C",
    "XOR D",       "XOR E",       "XOR I%H",     "XOR I%L",     "XOR (I%+h)",
    "XOR A",       "OR B",        "OR C",        "OR D",        "OR E",
    "OR I%H",      "OR I%L",      "OR (I%+h)",   "OR A",        "CP B",
    "CP C",        "CP D",        "CP E",        "CP I%H",      "CP I%L",
    "CP (I%+h)",   "CP A",        "RET NZ",      "POP BC",      "JP NZ,$h",
    "JP $h",       "CALL NZ,$h",  "PUSH BC",     "ADD *h",      "RST 00h",
    "RET Z",       "RET",         "JP Z,$h",     "DEFB CBh",    "CALL Z,$h",
    "CALL $h",     "ADC *h",      "RST 08h",     "RET NC",      "POP DE",
    "JP NC,$h",    "OUT (*h),A",  "CALL NC,$h",  "PUSH DE",     "SUB *h",
    "RST 10h",     "RET C",       "EXX",         "JP C,$h",     "IN A,(*h)",
    "CALL C,$h",   "DEFB DDh",    "SBC *h",      "RST 18h",     "RET PO",
    "POP I%",      "JP PO,$h",    "EX I%,(SP)",  "CALL PO,$h",  "PUSH I%",
    "AND *h",      "RST 20h",     "RET PE",      "JP I%",       "JP PE,$h",
    "EX DE,I%",    "CALL PE,$h",  "DEFB EDh",    "XOR *h",      "RST 28h",
    "RET P",       "POP AF",      "JP P,$h",     "DI",          "CALL P,$h",
    "PUSH AF",     "OR *h",       "RST 30h",     "RET M",       "LD SP,I%",
    "JP M,$h",     "EI",          "CALL M,$h",   "DEFB FDh",    "CP *h",
    "RST 38h"};

/* The XXCB instructions are odd, indeed... */

#define XXCBip(i) #i " (I%+h)",
#define XXCBir(i, r) #i " (I%+h) and LD " #r ",(I%+h)",
#define XXCBi(i)                                                               \
    XXCBir(i, B) XXCBir(i, C) XXCBir(i, D) XXCBir(i, E) XXCBir(i, H)           \
        XXCBir(i, L) XXCBip(i) XXCBir(i, A)

#define XXCBbp(i, b) #i " " #b ",(I%+h)",
#define XXCBbr(i, b, r) #i " " #b ",(I%+h) and LD " #r ",(I%+h)",
/* RES or SET */
#define XXCBrs1(i, b)                                                          \
    XXCBbr(i, b, B) XXCBbr(i, b, C) XXCBbr(i, b, D) XXCBbr(i, b, E)            \
        XXCBbr(i, b, H) XXCBbr(i, b, L) XXCBbp(i, b) XXCBbr(i, b, A)
#define XXCBrs(i)                                                              \
    XXCBrs1(i, 0) XXCBrs1(i, 1) XXCBrs1(i, 2) XXCBrs1(i, 3) XXCBrs1(i, 4)      \
        XXCBrs1(i, 5) XXCBrs1(i, 6) XXCBrs1(i, 7)
/* BIT (always "plain" */
#define XXCBb1(i, b)                                                           \
    XXCBbp(i, b) XXCBbp(i, b) XXCBbp(i, b) XXCBbp(i, b) XXCBbp(i, b)           \
        XXCBbp(i, b) XXCBbp(i, b) XXCBbp(i, b)
#define XXCBb(i)                                                               \
    XXCBb1(i, 0) XXCBb1(i, 1) XXCBb1(i, 2) XXCBb1(i, 3) XXCBb1(i, 4)           \
        XXCBb1(i, 5) XXCBb1(i, 6) XXCBb1(i, 7)

static const char* const MnemonicsXXCB[256] = {
    XXCBi(RLC) XXCBi(RRC) XXCBi(RL) XXCBi(RR) XXCBi(SLA) XXCBi(SRA) XXCBi(SLL)
        XXCBi(SRL) XXCBb(BIT) XXCBrs(RES) XXCBrs(SET)};

/* XXED is never valid */

static const char* const* const mtable[8] = {
    Mnemonics,   MnemonicsCB,   MnemonicsED, NULL,
    MnemonicsXX, MnemonicsXXCB, NULL,        NULL};

int disassemble(int pc)
{
    char buffer[80];
    int n;

    n = DAsm(pc, buffer, NULL);
    fputs(buffer, tracef);

    return n;
}

int DAsm(uint16_t pc, char* T, int* target)
{
    const char *S, *P;
    char Sbuf[8];
    char PP, *R;
    int I, D;
    int table;
    char XReg;
    uint16_t pc0 = pc;

    if (target)
        *target = -1;

    XReg = '?';

    I = mem_fetch(pc++);

    D = table = 0;

    S = NULL;

    while (!S) {
        switch (I) {
        case 0xCB:
            if (table & 4)
                D = mem_fetch(pc++); /* Displacement before extended opcode! */
            I = mem_fetch(pc++);
            table = (table & ~3) | 1;
            break;
        case 0xED:
            I = mem_fetch(pc++);
            table = (table & ~3) | 2;
            break;
        case 0xDD:
            I = mem_fetch(pc++);
            XReg = 'X';
            table |= 4;
            break;
        case 0xFD:
            I = mem_fetch(pc++);
            XReg = 'Y';
            table |= 4;
            break;
        default:
            if (!mtable[table][I]) {
                sprintf(Sbuf, "??%s%02X",
                        (table & 4) == 0 ? "" : XReg == 'X' ? "DD" : "FD", I);
                S = Sbuf;
            } else {
                S = mtable[table][I];
            }
            break;
        }
    }

    P = S;
    R = T;
    while ((PP = *P++)) {
        switch (PP) {
        case '%':
            *R++ = XReg;
            break;
        case '*':
            I = mem_fetch(pc++);
            R += sprintf(R, "%02X", I);
            break;
        case '#':
        case '$':
            I = mem_fetch_word(pc);
            pc += 2;
            R += sprintf(R, "%04X", I);
            if (PP == '$' && target)
                *target = I;
            break;
        case '@':
            I = mem_fetch(pc++);
            if (I >= 0x80)
                I -= 256;
            I = (I + pc) & 0xFFFF;
            R += sprintf(R, "%04X", I);
            if (target)
                *target = I;
            break;
        case '+':
            if (table == 5)
                I = D;
            else
                I = mem_fetch(pc++);
            if (I >= 0x80) {
                PP = '-';
                I = -(I - 256);
            }
            R += sprintf(R, "%c%02X", PP, I);
            break;
        default:
            *R++ = PP;
            break;
        }
    }
    *R = '\0';

    /* Return the number of consumed bytes */
    return (uint16_t)(pc - pc0);
}
