/*
 * cmd_check.h  SYSTEM35のコマンド解析
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
/* $Id: cmd_check.h,v 1.21 2003/04/22 16:34:28 chikama Exp $ */

#ifndef __CMD_CHECK__
#define __CMD_CHECK__

/* define by cmdb.c */
extern void commandB0();
extern void commandB1();
extern void commandB2();
extern void commandB3();
extern void commandB4();
extern void commandB10();
extern void commandB11();
extern void commandB12();
extern void commandB13();
extern void commandB14();
extern void commandB21();
extern void commandB22();
extern void commandB23();
extern void commandB24();
extern void commandB31();
extern void commandB32();
extern void commandB33();
extern void commandB34();

/* defined by cmdc.c */
extern void commandCB();
extern void commandCC();
extern void commandCD();
extern void commandCE();
extern void commandCF();
extern void commandCK();
extern void commandCL();
extern void commandCM();
extern void commandCP();
extern void commandCS();
extern void commandCT();
extern void commandCU();
extern void commandCV();
extern void commandCX();
extern void commandCY();
extern void commandCZ();

/* defined by cmdd.c */
extern void commandDC();
extern void commandDI();
extern void commandDS();
extern void commandDR();
extern void commandDF();

/* defined by cmde.c */
extern void commandES();
extern void commandEC();
extern void commandEG();
extern void commandEM();
extern void commandEN();

/* defined by cmdf.c */
extern void commandF();

/* defined by cmdg.c */
extern void commandG0();
extern void commandG1();
extern void commandGS();
extern void commandGX();

/* defined by cmdh.c */
extern void commandH();
extern void commandHH();

/* defined by cmdi.c */
extern void commandIK();
extern void commandIM();
extern void commandIC();
extern void commandIZ();
extern void commandIX();
extern void commandIY();
extern void commandIG();
extern void commandIE();

/* defined by cmdj.c */
extern void commandJ0();
extern void commandJ1();
extern void commandJ2();
extern void commandJ3();
extern void commandJ4();

/* defined by cmdk.c */
extern void commandKI();
extern void commandKK();
extern void commandKN();
extern void commandKP();
extern void commandKQ();
extern void commandKR();
extern void commandKW();

/* defined by cmdl.c */
extern void commandLC();
extern void commandLD();
extern void commandLE();
extern void commandLHD();
extern void commandLHG();
extern void commandLHM();
extern void commandLHS();
extern void commandLHW();
extern void commandLL();
extern void commandLP();
extern void commandLT();
extern void commandLXG();
extern void commandLXO();
extern void commandLXC();
extern void commandLXL();
extern void commandLXS();
extern void commandLXP();
extern void commandLXR();
extern void commandLXW();


/* defined by cmdm.c */
extern void commandMA();
extern void commandMC();
extern void commandMD();
extern void commandME();
extern void commandMF();
extern void commandMG();
extern void commandMH();
extern void commandMI();
extern void commandMJ();
extern void commandML();
extern void commandMM();
extern void commandMN();
extern void commandMP();
extern void commandMS();
extern void commandMT();
extern void commandMV();
extern void commandMZ0();

/* defined by cmdn.c */
extern void commandN_ADD();
extern void commandN_SUB();
extern void commandN_MUL();
extern void commandN_DIV();
extern void commandN_GT();
extern void commandN_LT();
extern void commandN_EQ();
extern void commandN_NE();
extern void commandN_AND();
extern void commandN_OR();
extern void commandN_XOR();
extern void commandN_NOT();
extern void commandNB();
extern void commandNC();
extern void commandNI();
extern void commandNP();
extern void commandNR();
extern void commandNO();
extern void commandNT();
extern void commandND_ADD();
extern void commandND_SUB();
extern void commandND_MUL();
extern void commandND_DIV();
extern void commandNDC();
extern void commandNDD();
extern void commandNDM();
extern void commandNDA();
extern void commandNDH();

/* defined by cmdo.c */
extern void commandO();

/* defined by cmdp.c */
extern void commandPC();
extern void commandPD();
extern void commandPF();
extern void commandPG();
extern void commandPN();
extern void commandPP();
extern void commandPS();
extern void commandPT0();
extern void commandPT1();
extern void commandPT2();
extern void commandPW();

/* defined by cmdq.c */
extern void commandQC();
extern void commandQD();
extern void commandQE();
extern void commandQP();

/* defined by cmds.c */
extern void commandSC();
extern void commandSG();
extern void commandSL();
extern void commandSM();
extern void commandSO();
extern void commandSP();
extern void commandSQ();
extern void commandSR();
extern void commandSS();
extern void commandST();
extern void commandSU();
extern void commandSW();
extern void commandSX();
extern void commandSI();

/* defined by cmdt.c */
extern void commandT();

/* defined by cmdu.c */
extern void commandUC();
extern void commandUD();
extern void commandUR();
extern void commandUS();
extern void commandUG();
extern void commandUP0();
extern void commandUP1();
extern void commandUP3();

/* defined by cmdv.c */
extern void commandVC();
extern void commandVP();
extern void commandVS();
extern void commandVG();
extern void commandVH();
extern void commandVF();
extern void commandVV();
extern void commandVR();
extern void commandVW();
extern void commandVE();
extern void commandVZ();
extern void commandVX();
extern void commandVT();
extern void commandVB();
extern void commandVIC();
extern void commandVIP();
extern void commandVA();
extern void commandVJ();

/* defined by cmdw.c */
extern void commandWW();
extern void commandWV();
extern void commandWZ();
extern void commandWX();

/* defined by cmdy.c */
extern void commandY();

/* defined by cmdz.c */
extern void commandZC();
extern void commandZM();
extern void commandZS();
extern void commandZB();
extern void commandZH();
extern void commandZW();
extern void commandZL();
extern void commandZE();
extern void commandZF();
extern void commandZD();
extern void commandZT0();
extern void commandZT1();
extern void commandZT2();
extern void commandZT3();
extern void commandZT4();
extern void commandZT5();
extern void commandZT10();
extern void commandZT11();
extern void commandZT20();
extern void commandZT21();
extern void commandZZ0();
extern void commandZZ1();
extern void commandZZ2();
extern void commandZZ3();
extern void commandZZ4();
extern void commandZZ5();
extern void commandZZ7();
extern void commandZZ8();
extern void commandZZ9();
extern void commandZZ10();
extern void commandZZ13();
extern void commandZZ14();
extern void commandZG();
extern void commandZI();
extern void commandZA();
extern void commandZK();
extern void commandZR();

/* define in cmd2F.c */
extern void commands2F00();
extern void commands2F01();
extern void commands2F02();
extern void commands2F03();
extern void commands2F04();
extern void commands2F05();
extern void commands2F08();
extern void commands2F09();
extern void commands2F0A();
extern void commands2F0B();
extern void commands2F0C();
extern void commands2F0D();
extern void commands2F0E();
extern void commands2F0F();
extern void commands2F10();
extern void commands2F11();
extern void commands2F12();
extern void commands2F13();
extern void commands2F14();
extern void commands2F15();
extern void commands2F16();
extern void commands2F17();
extern void commands2F18();
extern void commands2F19();
extern void commands2F1A();
extern void commands2F1B();
extern void commands2F1C();
extern void commands2F1D();
extern void commands2F1E();
extern void commands2F1F();
extern void commands2F20();
extern void commands2F21();
extern void commands2F23();
extern void commands2F24();
extern void commands2F25();
extern void commands2F26();
extern void commands2F27();
extern void commands2F28();
extern void commands2F29();
extern void commands2F2A();
extern void commands2F2B();
extern void commands2F2D();
extern void commands2F2E();
extern void commands2F2F();
extern void commands2F30();
extern void commands2F31();
extern void commands2F32();
extern void commands2F33();
extern void commands2F34();
extern void commands2F35();
extern void commands2F36();
extern void commands2F37();
extern void commands2F38();
extern void commands2F39();
extern void commands2F3A();
extern void commands2F3B();
extern void commands2F3C();
extern void commands2F3D();
extern void commands2F3E();
extern void commands2F3F();
extern void commands2F40();
extern void commands2F41();
extern void commands2F42();
extern void commands2F43();
extern void commands2F44();
extern void commands2F45();
extern void commands2F46();
extern void commands2F47();
extern void commands2F48();
extern void commands2F49();
extern void commands2F4A();
extern void commands2F4B();
extern void commands2F4C();
extern void commands2F4D();
extern void commands2F4E();
extern void commands2F4F();
extern void commands2F50();
extern void commands2F51();
extern void commands2F52();
extern void commands2F53();
extern void commands2F54();
extern void commands2F55();
extern void commands2F56();
extern void commands2F57();
extern void commands2F58();
extern void commands2F59();
extern void commands2F5A();
extern void commands2F5B();
extern void commands2F5C();
extern void commands2F5D();
extern void commands2F5E();
extern void commands2F5F();
extern void commands2F61();
extern void commands2F62();
extern void commands2F63();
extern void commands2F64();
extern void commands2F65();
extern void commands2F66();
extern void commands2F67();
extern void commands2F68();
extern void commands2F69();
extern void commands2F6A();
extern void commands2F6B();
extern void commands2F6C();
extern void commands2F6D();
extern void commands2F6E();
extern void commands2F6F();
extern void commands2F70();
extern void commands2F71();
extern void commands2F72();
extern void commands2F73();
extern void commands2F74();
extern void commands2F75();
extern void commands2F76();
extern void commands2F77();
extern void commands2F78();
extern void commands2F79();
extern void commands2F7A();
extern void commands2F7B();
extern void commands2F7C();
extern void commands2F7D();
extern void commands2F7E();
extern void commands2F7F();
extern void commands2F80();
extern void commands2F81();
extern void commands2F82();
extern void commands2F83();
extern void commands2F84();
extern void commands2F85();
extern void commands2F86();
extern void commands2F87();
extern void commands2F88();
extern void commands2F89();
extern void commands2F8A();
extern void commands2F8B();
extern void commands2F8C();

/* define in cmd2F60.c */
extern void commands2F60();

#endif /* !__CMD_CHECK__ */
