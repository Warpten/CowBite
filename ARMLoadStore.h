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

#ifndef ARMLOADSTORE_H
#define ARMLOADSTORE_H

//Loads
void ARM_ldrbiaPre();	void ARM_ldrbiaPost();	void ARM_ldrbisPre();	void ARM_ldrbisPost();
void ARM_ldrbaPre();	void ARM_ldrbaPost();	void ARM_ldrbsPre();	void ARM_ldrbsPost();
void ARM_ldrwiaPre();	void ARM_ldrwiaPost();	void ARM_ldrwisPre();	void ARM_ldrwisPost();
void ARM_ldrwaPre();	void ARM_ldrwaPost();	void ARM_ldrwsPre();	void ARM_ldrwsPost();

//Stores
void ARM_strbiaPre();	void ARM_strbiaPost();	void ARM_strbisPre();	void ARM_strbisPost();
void ARM_strbaPre();	void ARM_strbaPost();	void ARM_strbsPre();	void ARM_strbsPost();
void ARM_strwiaPre();	void ARM_strwiaPost();	void ARM_strwisPre();	void ARM_strwisPost();
void ARM_strwaPre();	void ARM_strwaPost();	void ARM_strwsPre();	void ARM_strwsPost();

//Load signed betes
void ARM_ldrsbiaPre();	void ARM_ldrsbiaPost();	void ARM_ldrsbisPre();	void ARM_ldrsbisPost();
void ARM_ldrsbaPre();	void ARM_ldrsbaPost();	void ARM_ldrsbsPre();	void ARM_ldrsbsPost();

//Load signed halfwords
void ARM_ldrshiaPre();	void ARM_ldrshiaPost();	void ARM_ldrshisPre();	void ARM_ldrshisPost();
void ARM_ldrshaPre();	void ARM_ldrshaPost();	void ARM_ldrshsPre();	void ARM_ldrshsPost();

//Load halfwords
void ARM_ldrhiaPre();	void ARM_ldrhiaPost();	void ARM_ldrhisPre();	void ARM_ldrhisPost();
void ARM_ldrhaPre();	void ARM_ldrhaPost();	void ARM_ldrhsPre();	void ARM_ldrhsPost();

//Store halfwords
void ARM_strhiaPre();	void ARM_strhiaPost();	void ARM_strhisPre();	void ARM_strhisPost();
void ARM_strhaPre();	void ARM_strhaPost();	void ARM_strhsPre();	void ARM_strhsPost();


#endif