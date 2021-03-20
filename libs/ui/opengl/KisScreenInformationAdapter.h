/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENINFORMATIONADAPTER_H
#define KISSCREENINFORMATIONADAPTER_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include <QSurfaceFormat>
#include "KisSurfaceColorSpace.h"

class QScreen;
class QOpenGLContext;

class KRITAUI_EXPORT KisScreenInformationAdapter
{
public:
    struct ScreenInfo {
        QScreen *screen = 0;
        int bitsPerColor = 0;
        KisSurfaceColorSpace colorSpace = KisSurfaceColorSpace::DefaultColorSpace;
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
