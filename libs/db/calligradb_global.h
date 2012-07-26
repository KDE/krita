/* This file is part of the KDE project
   Copyright (C) 2007 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KEXIDB_GLOBAL_H_
#define _KEXIDB_GLOBAL_H_

#ifndef WARNING
#ifdef _MSC_VER
/* WARNING preprocessor directive
 Reserved: preprocessor needs two indirections to replace __LINE__ with actual
 string
*/
#define _MSG0(msg)     #msg
/* Preprocessor needs two indirections to replace __LINE__ or __FILE__
 with actual string. */
#define _MSG1(msg)    _MSG0(msg)

/*! Creates message prolog with the name of the source file and the line
   number where a preprocessor message has been inserted.

  Example:
     #pragma KMESSAGE(Your message)
  Output:
     C:\MyCode.cpp(111) : Your message
*/
# define _MSGLINENO __FILE__ "(" _MSG1(__LINE__) ") : warning: "
# define WARNING(msg) message(_MSGLINENO #msg)
#endif /*_MSC_VER*/
#endif /*WARNING*/

#endif
