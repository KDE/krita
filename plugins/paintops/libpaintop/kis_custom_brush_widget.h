/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CUSTOM_BRUSH_H_
#define KIS_CUSTOM_BRUSH_H_

#include <QObject>
#include <QShowEvent>

#include "ui_wdgcustombrush.h"
#include <kis_types.h>
#include <kis_brush.h>
#include <KoResourceServer.h>

const QString TEMPORARY_FILENAME = "/tmp/temporaryKritaBrush.gbr";
const QString TEMPORARY_BRUSH_NAME = "Temporary custom brush";
const double DEFAULT_SPACING = 0.25;


class KoResource;


class KisWdgCustomBrush : public QDialog, public Ui::KisWdgCustomBrush
{
    Q_OBJECT

public:
    KisWdgCustomBrush(QWidget *parent) : QDialog(parent) {
        setupUi(this);
    }
};

class KisCustomBrushWidget : public KisWdgCustomBrush
{
    Q_OBJECT
public:
    KisCustomBrushWidget(QWidget *parent, const QString& caption, KisImageWSP image);
    virtual ~KisCustomBrushWidget();
    KisBrushSP brush();

    void setImage(KisImageWSP image);

protected:
    virtual void showEvent(QShowEvent *);

private Q_SLOTS:
    void slotAddPredefined();
    void slotUpdateCurrentBrush(int i = 0); // To connect with activated(int)
    void slotSpacingChanged();
    void slotUpdateUseColorAsMask(bool useColorAsMask);
    void slotUpdateSaveButton();

Q_SIGNALS:
    void sigNewPredefinedBrush(KoResourceSP );

private:
    void createBrush();
    void updatePreviewImage();

    KisImageWSP m_image;
    KisBrushSP m_brush;

    KoResourceServer<KisBrush> *m_rServer {0};
};


#endif // KIS_CUSTOM_BRUSH_H_
