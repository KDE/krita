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

#include <QLabel>
#include <QLineEdit>

#include <kstatusbar.h>
#include <ksqueezedtextlabel.h>

#include "kis_label_progress.h"

class KisStatusBar : public QObject
{
    Q_OBJECT

public:

    KisStatusBar(KStatusBar * statusBar );
    ~KisStatusBar();

public:

    void setZoom( int percentage );
    void setPosition( int x, int y );
    void setSize( int w, int h );
    void setSelection( const QString & t );
    void setProfile( const QString & t );
    void setHelp( const QString &t );

private:

    KStatusBar * m_statusbar;

    QLineEdit *m_statusBarZoomLabel;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_sizeLabel;

    KSqueezedTextLabel *m_statusBarSelectionLabel;
    KSqueezedTextLabel *m_statusBarProfileLabel;
    KSqueezedTextLabel *m_statusBarHelpLabel;

    KisLabelProgress *m_progress;


};

#endif
