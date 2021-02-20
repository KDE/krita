/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DLG_IMAGE_PROPERTIES_H_
#define KIS_DLG_IMAGE_PROPERTIES_H_

#include <KoDialog.h>
#include "KisProofingConfiguration.h"
#include <kis_types.h>
#include "ui_wdgimageproperties.h"

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
    KisDlgImageProperties(KisImageWSP image,
                          QWidget *parent = 0,
                          const char *name = 0);
    ~KisDlgImageProperties() override;

    bool convertLayerPixels() const;
    const KoColorSpace * colorSpace() const;

private Q_SLOTS:

    void setAnnotation(const QString& type);
    void setCurrentColor();
    void setProofingConfig();

    void slotSaveDialogState();

    void slotColorSpaceChanged(const KoColorSpace*);
private:

    WdgImageProperties *m_page;
    KisImageWSP m_image;
    KisProofingConfigurationSP m_proofingConfig;
    bool m_firstProofingConfigChange {true};
    QLabel *m_colorWarningLabel {0};
};



#endif // KIS_DLG_IMAGE_PROPERTIES_H_

