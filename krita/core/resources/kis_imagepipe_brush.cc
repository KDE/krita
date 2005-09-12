/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <netinet/in.h>
#include <limits.h>
#include <stdlib.h>

#include <qimage.h>
#include <qpoint.h>
#include <qvaluevector.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_global.h"
#include "kis_imagepipe_brush.h"
#include "kis_brush.h"
#include "kis_alpha_mask.h"

KisImagePipeBrush::KisImagePipeBrush(const QString& filename) : super(filename)
{
    m_brushType = INVALID;
    m_numOfBrushes = 0;
    m_currentBrush = 0;

}

KisImagePipeBrush::~KisImagePipeBrush()
{
    m_brushes.setAutoDelete(true);
    m_brushes.clear();
}

bool KisImagePipeBrush::load()
{
    QFile file(filename());
    file.open(IO_ReadOnly);
    m_data = file.readAll();
    file.close();
    return init();
}

bool KisImagePipeBrush::init()
{
    // XXX: this doesn't correctly load the image pipe brushes yet.

    // XXX: This stuff is in utf-8, too.
    // The first line contains the name -- this means we look until we arrive at the first newline
    QValueVector<char> line1;

    Q_UINT32 i = 0;

    while (m_data[i] != '\n' && i < m_data.size()) {
        line1.append(m_data[i]);
        i++;
    }
    setName(i18n(QString::fromUtf8(&line1[0], i).ascii()));

    i++; // Skip past the first newline

    // The second line contains the number of brushes, separated by a space from the parasite

    // XXX: This stuff is in utf-8, too.
     QValueVector<char> line2;
     while (m_data[i] != '\n' && i < m_data.size()) {
        line2.append(m_data[i]);
         i++;
     }

    QString paramline = QString::fromUtf8((&line2[0]), line2.size());
    Q_UINT32 m_numOfBrushes = paramline.left(paramline.find(' ')).toUInt();
    m_parasite = paramline.mid(paramline.find(' ') + 1);
    
    i++; // Skip past the second newline

     Q_UINT32 numOfBrushes = 0;
      while (numOfBrushes < m_numOfBrushes && i < m_data.size()){
        KisBrush * brush = new KisBrush(name() + "_" + numOfBrushes,
                        m_data,
                        i);
        Q_CHECK_PTR(brush);

        m_brushes.append(brush);
        
         numOfBrushes++;
     }

    if (!m_brushes.isEmpty()) {
        setValid(true);
        if (m_brushes.at( 0 ) -> brushType() == MASK) {
            m_brushType = PIPE_MASK;
        }
        else {
            m_brushType = PIPE_IMAGE;
        }
        setSpacing(m_brushes.at(m_brushes.count() - 1) -> spacing());
        setWidth(m_brushes.at(0) -> width());
        setHeight(m_brushes.at(0) -> height());
    }

    m_data.resize(0);
    return true;
}

bool KisImagePipeBrush::save()
{
    return false;
}

QImage KisImagePipeBrush::img()
{
    if (m_brushes.isEmpty()) {
        return 0;
    }
    else {
        return m_brushes.at(0) -> img();
    }
}

KisAlphaMaskSP KisImagePipeBrush::mask(double pressure, double subPixelX, double subPixelY) const
{
    if (m_brushes.isEmpty()) return 0;
    // XXX: This does not follow the instructions in the 'parasite'
    if (m_currentBrush == m_brushes.count()) {
        m_currentBrush = 0;
    }
    m_currentBrush++;
    return m_brushes.at(m_currentBrush - 1) -> mask(pressure, subPixelX, subPixelY);
}

KisLayerSP KisImagePipeBrush::image(KisColorSpace * colorSpace, double pressure, double subPixelX, double subPixelY) const
{
    if (m_brushes.isEmpty()) return 0;
    // XXX: This does not follow the instructions in the 'parasite'
    if (m_currentBrush == m_brushes.count()) {
        m_currentBrush = 0;
    }
    m_currentBrush++;
    return m_brushes.at(m_currentBrush - 1) -> image(colorSpace, pressure, subPixelX, subPixelY);
}

void KisImagePipeBrush::setParasite(const QString& parasite)
{
    m_parasite = parasite;
}


enumBrushType KisImagePipeBrush::brushType() const
{
    if (m_brushType == PIPE_IMAGE && useColorAsMask()) {
        return PIPE_MASK;
    }
    else {
        return m_brushType;
    }
}

bool KisImagePipeBrush::useColorAsMask() const
{
    if (m_brushes.count() > 0) {
        return m_brushes.at(0) -> useColorAsMask();
    }
    else {
        return false;
    }
}

void KisImagePipeBrush::setUseColorAsMask(bool useColorAsMask)
{
    for (uint i = 0; i < m_brushes.count(); i++) {
        m_brushes.at(i) -> setUseColorAsMask(useColorAsMask);
    }
}

bool KisImagePipeBrush::hasColor() const
{
    if (m_brushes.count() > 0) {
        return m_brushes.at(0) -> hasColor();
    }
    else {
        return false;
    }
}

KisBoundary KisImagePipeBrush::boundary() {
    Q_ASSERT(!m_brushes.isEmpty());
    return m_brushes.at(0) -> boundary();
}
        
#include "kis_imagepipe_brush.moc"

