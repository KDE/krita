/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2013-09-07
 * @brief  a command line tool to show libkdcraw info
 *
 * @author Copyright (C) 2013 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QDebug>

// Local includes

#include <KDCRAW/KDcraw>

using namespace KDcrawIface;

int main(int /*argc*/, char** /*argv*/)
{
    qDebug() << "Libkdcraw version : " << KDcraw::version(),
    qDebug() << "Libraw version    : " << KDcraw::librawVersion();
    qDebug() << "Use OpenMP        : " << KDcraw::librawUseGomp();
    qDebug() << "Use RawSpeed      : " << KDcraw::librawUseRawSpeed();
    qDebug() << "Use GPL2 Pack     : " << KDcraw::librawUseGPL2DemosaicPack();
    qDebug() << "Use GPL3 Pack     : " << KDcraw::librawUseGPL3DemosaicPack();
    qDebug() << "Raw files list    : " << KDcraw::rawFilesList();
    qDebug() << "Raw files version : " << KDcraw::rawFilesVersion();
    qDebug() << "Supported camera  : " << KDcraw::supportedCamera();

    return 0;
}
