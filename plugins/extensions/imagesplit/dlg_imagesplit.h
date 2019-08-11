/*
 *  dlg_imagesplit.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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
#ifndef DLG_IMAGESPLIT
#define DLG_IMAGESPLIT

#include <KoDialog.h>

#include <kis_types.h>

#include "wdg_imagesplit.h"

class KisViewManager;
/**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgImagesplit: public KoDialog
{

    Q_OBJECT

public:

    DlgImagesplit(KisViewManager* view, const QString &suffix, QStringList listMimeType, int defaultMimeIndex);
    ~DlgImagesplit() override;
    bool autoSave();
    bool sortHorizontal();
    int horizontalLines();
    int verticalLines();
    int cmbIndex;
    QString suffix();
private Q_SLOTS:

    void applyClicked();
    void lineEditEnable();
    void setMimeType(int index);
private:
    WdgImagesplit* m_page;
};

#endif // DLG_IMAGESPLIT
