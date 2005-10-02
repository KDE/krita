/*
 *  kis_previewdialog.h -- part of Krita
 *
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PREVIEWDIALOG_H
#define KIS_PREVIEWDIALOG_H

#include <kdialogbase.h>

class KisPreviewWidget;
class QFrame;

class KisPreviewDialog: public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:
    KisPreviewDialog( QWidget* parent = 0, const char* name = 0, bool modal = false, const QString &caption=QString::null);
    ~KisPreviewDialog();

    KisPreviewWidget* previewWidget() { return m_preview; }
    QFrame* container() { return m_containerFrame; }
private:
    KisPreviewWidget* m_preview;
    QFrame* m_containerFrame;
};

#endif
