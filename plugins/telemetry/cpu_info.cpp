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
#include "cpu_info.h"

CPUInfo::CPUInfo()
{
    CPUID cpuID(0); // Get CPU vendor
    m_ecx0 = cpuID.ECX();

    m_vendor += std::string((const char*)&cpuID.EBX(), 4);
    m_vendor += std::string((const char*)&cpuID.EDX(), 4);
    m_vendor += std::string((const char*)&cpuID.ECX(), 4);

    CPUID cpuID2(1);
    uint eax, ebx, ecx, edx;
    eax = cpuID2.EAX();
    ebx = cpuID2.EBX();
    ecx = cpuID2.ECX();
    edx = cpuID2.EDX();
    uint temp = cpuID2.EAX();
    m_processorModel = (temp & 0x000000f0) >> 4;
    m_processorFamily = (cpuID2.EAX() & 0x00000f00) >> 8;
    if (isAmd()) {
        if (m_processorFamily >= 0xf) {
            const unsigned char processorFamilyExt = (eax & 0x0ff00000) >> 16;
            m_processorFamily += processorFamilyExt;
            const unsigned char processorModelExt = (eax & 0x000f0000) >> 12;
            m_processorModel += processorModelExt;
        }
    } else if (m_processorFamily == 0xf) {
        const unsigned char processorFamilyExt = (eax & 0x0ff00000) >> 16;
        m_processorFamily += processorFamilyExt;
        const unsigned char processorModelExt = (eax & 0x000f0000) >> 12;
        m_processorModel += processorModelExt;
    } else if (m_processorFamily == 0x6) {
        const unsigned char processorModelExt = (eax & 0x000f0000) >> 12;
        m_processorModel += processorModelExt;
    } else if (m_processorFamily == 0xB) {
        const unsigned char processorFamilyExt = (eax & 0x0ff00000) >> 16;
        m_processorFamily += processorFamilyExt;
        const unsigned char processorModelExt = (eax & 0x000f0000) >> 12;
        m_processorModel += processorModelExt;
    }
}

bool CPUInfo::isIntel() const
{
    return m_ecx0 == 0x6C65746E;
}

bool CPUInfo::isAmd() const
{
    return m_ecx0 == 0x444D4163;
}

unsigned int CPUInfo::processorModel() const
{
    return m_processorModel;
}

unsigned int CPUInfo::processorFamily() const
{
    return m_processorFamily;
}

std::string CPUInfo::vendor() const
{
    return m_vendor;
}

CPUID::CPUID(unsigned int i)
{
#ifdef _WIN32
    __cpuid((int*)regs, (int)i);

#else
    asm volatile("cpuid"
                 : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
                 : "a"(i), "c"(0));
// ECX is set to zero for CPUID function 4
#endif
}

const uint32_t& CPUID::EAX() const
{
    return regs[0];
}

const uint32_t& CPUID::EBX() const
{
    return regs[1];
}

const uint32_t& CPUID::ECX() const
{
    return regs[2];
}

const uint32_t& CPUID::EDX() const
{
    return regs[3];
}
