/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisTemplate.h"

#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <kis_debug.h>
#include <KoResourcePaths.h>
#include <kis_icon.h>


KisTemplate::KisTemplate(const QString &name, const QString &description, const QString &file,
                       const QString &picture, const QString &fileName, const QString &_measureSystem,
                       bool hidden, bool touched)
    : m_name(name)
    , m_descr(description)
    , m_file(file)
    , m_picture(picture)
    , m_fileName(fileName)
    , m_hidden(hidden)
    , m_touched(touched)
    , m_cached(false)
    , m_measureSystem(_measureSystem)
{
}

const QPixmap &KisTemplate::loadPicture()
{
    if (m_cached)
        return m_pixmap;

    m_cached = true;

    if (QFile::exists(m_picture)) {
        QImage img(m_picture);
        if (img.isNull()) {
            dbgKrita << "Couldn't find icon " << m_picture;
            m_pixmap = QPixmap();
            return m_pixmap;
        }
        const int maxHeightWidth = 128; // ### TODO: some people would surely like to have 128x128
        if (img.width() > maxHeightWidth || img.height() > maxHeightWidth) {
            img = img.scaled(maxHeightWidth, maxHeightWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        m_pixmap = QPixmap::fromImage(img);
        return m_pixmap;
    }
    else { // relative path


        // each template folder should have a light and dark version of the icon that will be for light and dark themes
        QString themePrefix;
        if( KisIconUtils::useDarkIcons()  ) {
            themePrefix = "dark_";
        } else {
            themePrefix = "light_";
        }


        QString filenameBuilder = themePrefix.append(m_picture).append(".png");
        QString filename = KoResourcePaths::findResource("kis_pics", filenameBuilder);

        if (filename.isEmpty()) {

        }
        m_pixmap = QPixmap(filename);
        return m_pixmap;
    }
}

