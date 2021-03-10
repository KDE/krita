/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Daniel Novomesky <dnovomesky@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HEIF_EXPORT_H_
#define HEIF_EXPORT_H_

#include <QVariant>

#include <KisImportExportFilter.h>
#include <kis_config_widget.h>
#include "ui_WdgHeifExport.h"

class KisWdgOptionsHeif : public KisConfigWidget, public Ui::WdgHeifExport
{
    Q_OBJECT

public:
    KisWdgOptionsHeif(QWidget *parent)
        : KisConfigWidget(parent)
    {
        setupUi(this);
        connect(chkLossless, SIGNAL(toggled(bool)), SLOT(toggleQualitySlider(bool)));
        connect(chkHLGOOTF, SIGNAL(toggled(bool)), SLOT(toggleQualitySlider(bool)));
        connect(cmbConversionPolicy, SIGNAL(currentIndexChanged(int)), SLOT(toggleExtraHDROptions(int)));
        sliderQuality->setRange(0, 100, 0);
    }

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:

    void toggleQualitySlider(bool toggle);
    // Disable HLG OOTF options when the toggle is off.
    void toggleHLGOptions(bool toggle);
    // Disable all HLG options when the index is not for an HLG option.
    void toggleExtraHDROptions(int index);
private:

    bool m_hasAlpha {false};

};


class HeifExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    HeifExport(QObject *parent, const QVariantList &);
    ~HeifExport() override;

    // This should return true if the library can work with a QIODevice, and doesn't want to open the file by itself
    bool supportsIO() const override { return true; }

    enum ConversionPolicy {
        KeepTheSame,
        ApplyPQ,
        ApplyHLG,
        ApplySMPTE428
    };

    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;
    void initializeCapabilities() override;

    float applyCurveAsNeeded(float value, ConversionPolicy policy);

};

#endif
