/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DLG_IMAGE_PROPERTIES_H_
#define KIS_DLG_IMAGE_PROPERTIES_H_

#include <KoDialog.h>
#include <kis_types.h>
#include "ui_wdgimageproperties.h"

class KisDisplayColorConverter;

class KoColorSpace;
class WdgImageProperties : public QWidget, public Ui::WdgImageProperties
{
    Q_OBJECT

public:
    WdgImageProperties(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisDlgImageProperties : public KoDialog
{

    Q_OBJECT

public:
    KisDlgImageProperties(KisImageWSP image, KisDisplayColorConverter *colorConverter,
                          QWidget *parent = 0,
                          const char *name = 0);
    ~KisDlgImageProperties() override;

    bool convertLayerPixels() const;
    const KoColorSpace * colorSpace() const;

    virtual int exec() override;

private Q_SLOTS:
    void setAnnotation(const QString& type);
    void setCurrentColor();
    void setProofingConfigToImage();
    void updateDisplayConfigInfo();

    void slotColorSpaceChanged(const KoColorSpace*);
private:

    struct Private;
    QScopedPointer<Private> d;
    WdgImageProperties *m_page;
};



#endif // KIS_DLG_IMAGE_PROPERTIES_H_

