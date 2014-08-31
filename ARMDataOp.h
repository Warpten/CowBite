/*
    CowBite GBA Emulator/Debugger
    Copyright (C) 2002 Thomas Happ

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    CowBite contact info:
    Thomas Happ
    SorcererXIII@yahoo.com
    http://cowbite.emuunlim.com
*/

#ifndef ARMDATAOP_H
#define ARMDATAOP_H


//////////////////////////////////////////////////////
//Our plethora of functions.
void ARM_dprint();
void ARM_and();	void ARM_andi(); void ARM_ands(); void ARM_andsi();
void ARM_andRegShift(); void ARM_andImmShift(); void ARM_andiShift();
void ARM_andsRegShift(); void ARM_andsImmShift(); void ARM_andsiShift();

void ARM_eor();	void ARM_eori(); void ARM_eors(); void ARM_eorsi();
void ARM_eorRegShift(); void ARM_eorImmShift(); void ARM_eoriShift();
void ARM_eorsRegShift(); void ARM_eorsImmShift(); void ARM_eorsiShift();

void ARM_sub();	void ARM_subi(); void ARM_subs(); void ARM_subsi();
void ARM_subRegShift(); void ARM_subImmShift(); void ARM_subiShift();
void ARM_subsRegShift(); void ARM_subsImmShift(); void ARM_subsiShift();

void ARM_rsb();	void ARM_rsbi(); void ARM_rsbs(); void ARM_rsbsi();
void ARM_rsbRegShift(); void ARM_rsbImmShift(); void ARM_rsbiShift();
void ARM_rsbsRegShift(); void ARM_rsbsImmShift(); void ARM_rsbsiShift();

void ARM_add();	void ARM_addi(); void ARM_adds(); void ARM_addsi();
void ARM_addRegShift(); void ARM_addImmShift(); void ARM_addiShift();
void ARM_addsRegShift(); void ARM_addsImmShift(); void ARM_addsiShift();

void ARM_adc();	void ARM_adci(); void ARM_adcs(); void ARM_adcsi();
void ARM_adcRegShift(); void ARM_adcImmShift(); void ARM_adciShift();
void ARM_adcsRegShift(); void ARM_adcsImmShift(); void ARM_adcsiShift();

void ARM_sbc();	void ARM_sbci(); void ARM_sbcs(); void ARM_sbcsi();
void ARM_sbcRegShift(); void ARM_sbcImmShift(); void ARM_sbciShift();
void ARM_sbcsRegShift(); void ARM_sbcsImmShift(); void ARM_sbcsiShift();

void ARM_rsc();	void ARM_rsci(); void ARM_rscs(); void ARM_rscsi();
void ARM_rscRegShift(); void ARM_rscImmShift(); void ARM_rsciShift();
void ARM_rscsRegShift(); void ARM_rscsImmShift(); void ARM_rscsiShift();

void ARM_tsts(); void ARM_tstsi();
void ARM_tstsRegShift(); void ARM_tstsImmShift(); void ARM_tstsiShift();

void ARM_teqs(); void ARM_teqsi();
void ARM_teqsRegShift(); void ARM_teqsImmShift(); void ARM_teqsiShift();

void ARM_cmps(); void ARM_cmpsi();
void ARM_cmpsRegShift(); void ARM_cmpsImmShift(); void ARM_cmpsiShift();

void ARM_cmns(); void ARM_cmnsi();
void ARM_cmnsRegShift(); void ARM_cmnsImmShift(); void ARM_cmnsiShift();

void ARM_orr();	void ARM_orri(); void ARM_orrs(); void ARM_orrsi();
void ARM_orrRegShift(); void ARM_orrImmShift(); void ARM_orriShift();
void ARM_orrsRegShift(); void ARM_orrsImmShift(); void ARM_orrsiShift();

void ARM_mov();	void ARM_movi(); void ARM_movs(); void ARM_movsi();
void ARM_movRegShift(); void ARM_movImmShift(); void ARM_moviShift();
void ARM_movsRegShift(); void ARM_movsImmShift(); void ARM_movsiShift();

void ARM_bic();	void ARM_bici(); void ARM_bics(); void ARM_bicsi();
void ARM_bicRegShift(); void ARM_bicImmShift(); void ARM_biciShift();
void ARM_bicsRegShift(); void ARM_bicsImmShift(); void ARM_bicsiShift();

void ARM_mvn();	void ARM_mvni(); void ARM_mvns(); void ARM_mvnsi();
void ARM_mvnRegShift(); void ARM_mvnImmShift(); void ARM_mvniShift();
void ARM_mvnsRegShift(); void ARM_mvnsImmShift(); void ARM_mvnsiShift();

void ARM_msr();	void ARM_msri();	void ARM_msriShift();	void ARM_mrs();

void ARM_mul();
void ARM_mull();

#endif