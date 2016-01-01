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
#include <QPointer>
#include <QIcon>

#include <kis_types.h>
#include "KisView.h"

class QLabel;
class QToolButton;
class QPushButton;
class KSqueezedTextLabel;
class KisViewManager;
class KisProgressWidget;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisStatusBar : public QObject
{
    Q_OBJECT

public:

    KisStatusBar(KisViewManager * view);
    ~KisStatusBar();

    void setup();
    void setView(QPointer<KisView> imageView);

    KisProgressWidget *progress();

public Q_SLOTS:

    void documentMousePositionChanged(const QPointF &p);
    void imageSizeChanged();
    void setSelection(KisImageWSP image);
    void setProfile(KisImageWSP image);
    void setHelp(const QString &t);
    void updateStatusBarProfileLabel();
    void updateSelectionToolTip();

private Q_SLOTS:
    void updateSelectionIcon();
    void showMemoryInfoToolTip();

private:
   void updateMemoryStatus();

private:

    QPointer<KisViewManager> m_view;
    QPointer<KisView> m_imageView;
    KisProgressWidget * m_progress;

    QToolButton *m_selectionStatus;
    QPushButton *m_memoryReportBox;
    QLabel *m_pointerPositionLabel;

    KSqueezedTextLabel *m_statusBarStatusLabel;
    KSqueezedTextLabel *m_statusBarProfileLabel;


    QString m_shortMemoryTag;
    QString m_longMemoryTag;
    QIcon m_memoryStatusIcon;
};

#endif
