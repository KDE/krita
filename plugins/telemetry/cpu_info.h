/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KIS_CPU_INFO_H
#define KIS_CPU_INFO_H

#ifdef _WIN32
#include <intrin.h> \
#include <limits.h> \
typedef unsigned __int32 uint32_t;

#else
#include <stdint.h>
#endif
#include <iostream>
#include <string>

class CPUID {
    uint32_t regs[4];

public:
    explicit CPUID(unsigned int i);

    const uint32_t& EAX() const;
    const uint32_t& EBX() const;
    const uint32_t& ECX() const;
    const uint32_t& EDX() const;
};
class CPUInfo {
public:
    CPUInfo();
    bool isIntel() const;
    bool isAmd() const;
    unsigned int processorModel() const;

    unsigned int processorFamily() const;

    std::string vendor() const;

private:
    unsigned int m_ecx0;
    unsigned int m_processorModel;
    unsigned int m_processorFamily;
    std::string m_vendor;
};

#endif
