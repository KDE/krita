/*
 *  dlg_imagesplit.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    DlgImagesplit(KisViewManager* view
                  , const QString &suffix
                  , QStringList listMimeType
                  , int defaultMimeIndex
                  , QImage thumbnail
                  , QList<qreal> hGuides
                  , QList<qreal> vGuides
                  , qreal thumbnailRatio);
    ~DlgImagesplit() override;
    bool autoSave();
    bool sortHorizontal();
    int horizontalLines();
    int verticalLines();
    bool useHorizontalGuides();
    bool useVerticalGuides();
    int cmbIndex;
    QString suffix();
private Q_SLOTS:

    void applyClicked();
    void lineEditEnable();
    void setMimeType(int index);
    void updatePreview();
private:
    WdgImagesplit* m_page;
    QImage m_thumbnail;
    QList<qreal> m_horizontalGuides;
    QList<qreal> m_verticalGuides;
    qreal m_thumbnailRatio;
};

#endif // DLG_IMAGESPLIT
