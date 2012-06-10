/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DISPLAY_FILTER_H
#define KIS_DISPLAY_FILTER_H

#include <QObject>

#include <KoColorSpace.h>

/**
 * @brief The KisDisplayFilter class contains settings pertinent to
 */
class KisDisplayFilter : public QObject
{
    Q_OBJECT
public:
    explicit KisDisplayFilter(QObject *parent = 0);

    virtual void filter(quint8 *src, quint8 *dst, quint32 numPixels) = 0;

};

class KisLcmsDisplayFilter : public KisDisplayFilter
{
    Q_OBJECT
public:

    KisLcmsDisplayFilter(const KoColorSpace *src, QObject *parent = 0);

    virtual void filter(quint8 *src, quint8 *dst, quint32 numPixels);

private slots:

    void resetConfiguration();

private:

    const KoColorSpace *m_srcColorSpace;
    const KoColorSpace *m_dstColorSpace;
    KoColorProfile *m_monitorProfile;
    KoColorConversionTransformation::Intent m_renderingIntent;

};



#endif
