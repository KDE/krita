/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_kernel.h"

#include <kis_convolution_painter.h>

namespace Kross {

namespace KritaCore {

Kernel::Kernel(uint w, uint h)
 : Kross::Api::Class<Kernel>("KritaKernel")
{
/*    m_kernel = new KisKernel();
    m_kernel.width = w;
    m_kernel.height = h;
    m_kernel.offset = 0;
    m_kernel.factor = 0;
    m_kernel.data = new[] Q_INT32[width*height];*/
}


Kernel::~Kernel()
{
//     delete[] m_kernel.data;
    delete m_kernel;
}


}

}
