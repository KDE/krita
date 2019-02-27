/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISSCREENINFORMATIONADAPTER_H
#define KISSCREENINFORMATIONADAPTER_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include <QSurfaceFormat>

class QScreen;
class QOpenGLContext;

class KRITAUI_EXPORT KisScreenInformationAdapter
{
public:
    struct ScreenInfo {
        QScreen *screen = 0;
        int bitsPerColor = 0;
        QSurfaceFormat::ColorSpace colorSpace = QSurfaceFormat::DefaultColorSpace;
        qreal redPrimary[2] = {0, 0};
        qreal greenPrimary[2] = {0, 0};
        qreal bluePrimary[2] = {0, 0};
        qreal whitePoint[2] = {0, 0};
        qreal minLuminance = 0;
        qreal maxLuminance = 0;
        qreal maxFullFrameLuminance = 0;

        bool isValid() const {
            return screen;
        }
    };

public:
    KisScreenInformationAdapter(QOpenGLContext *context);
    ~KisScreenInformationAdapter();

    bool isValid() const;
    QString errorString() const;

    ScreenInfo infoForScreen(QScreen *screen) const;


private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

QDebug operator<<(QDebug, const KisScreenInformationAdapter::ScreenInfo &);

#endif // KISSCREENINFORMATIONADAPTER_H
