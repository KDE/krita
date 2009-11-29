/* This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2003-200^ Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_STATUSBAR_H
#define KIS_STATUSBAR_H

#include <QObject>

#include <kis_types.h>

class QLabel;
class KSqueezedTextLabel;
class KisView2;
class KisProgressWidget;

#include "krita_export.h"

class KRITAUI_EXPORT KisStatusBar : public QObject
{
    Q_OBJECT

public:

    KisStatusBar(KisView2 * view);
    ~KisStatusBar();

public slots:

    void setZoom(int percentage);
    void documentMousePositionChanged(const QPointF &p);
    void imageSizeChanged(qint32 w, qint32 h);
    void setSelection(KisImageWSP image);
    void setProfile(KisImageWSP image);
    void setHelp(const QString &t);
    void updateStatusBarProfileLabel();

    KisProgressWidget * progress();

private:

    KisView2 * m_view;
    KisProgressWidget * m_progress;

    QLabel *m_selectionStatusLabel;
    QLabel *m_statusBarZoomLabel; // Make interactive line edit
    QLabel *m_statusBarPositionLabel;
    QLabel *m_sizeLabel;
    QLabel *m_imageSizeLabel;
    QLabel *m_pointerPositionLabel;

    KSqueezedTextLabel *m_statusBarStatusLabel;
    KSqueezedTextLabel *m_statusBarProfileLabel;
    KSqueezedTextLabel *m_statusBarHelpLabel;

};

#endif
