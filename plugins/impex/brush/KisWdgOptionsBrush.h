/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2019 Iván SantaMaría <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWDGOPTIONSBRUSH_H
#define KISWDGOPTIONSBRUSH_H

#include <QVariant>
#include <QSpinBox>
#include <QPainter>

#include <KisImportExportFilter.h>
#include <ui_wdg_export_gih.h>
#include <kis_config_widget.h>
#include <kis_properties_configuration.h>

class BrushPipeSelectionModeHelper : public QWidget
{
    Q_OBJECT

public:
    BrushPipeSelectionModeHelper(QWidget *parent, int dimension)
        : QWidget(parent)
        , cmbSelectionMode(this)
        , rankSpinBox(this)
        , rankLbl(this)
        , horizLayout(this)
        , dimension(dimension)
    {
        cmbSelectionMode.addItem(i18n("Constant"));
        cmbSelectionMode.addItem(i18n("Random"));
        cmbSelectionMode.addItem(i18n("Incremental"));
        cmbSelectionMode.addItem(i18n("Pressure"));
        cmbSelectionMode.addItem(i18n("Angular"));
        cmbSelectionMode.addItem(i18n("Velocity"));

        horizLayout.setSpacing(6);
        horizLayout.setMargin(0);

        QSizePolicy sizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);

        this->setSizePolicy(sizePolicy);

        cmbSelectionMode.setSizePolicy(sizePolicy);
        cmbSelectionMode.setCurrentIndex(2);

        rankSpinBox.setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
        rankLbl.setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

        rankLbl.setText(i18n("Rank"));
        horizLayout.addWidget(&rankLbl);
        horizLayout.addWidget(&rankSpinBox);
        horizLayout.addWidget(&cmbSelectionMode);

        connect(&rankSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotRankChanged()));

        this->hide();
        this->setEnabled(false);
    }

    QComboBox cmbSelectionMode;
    QSpinBox rankSpinBox;
    QLabel rankLbl;
    QHBoxLayout horizLayout;

    int dimension;


Q_SIGNALS:
    void sigRankChanged(int rankEmitter);

public Q_SLOTS:
    void slotRankChanged()
    {
        emit sigRankChanged(dimension);
    }

};

class KisWdgOptionsBrush : public KisConfigWidget, public Ui::WdgExportGih
{
    Q_OBJECT

public:
    KisWdgOptionsBrush(QWidget *parent);

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

    void setView(KisViewManager *view) override;

public Q_SLOTS:

    void slotEnableSelectionMethod(int value);
    void slotActivateDimensionRanks();
    void slotRecalculateRanks(int rankDimension = 0);

private:
    int m_currentDimensions;
    int m_layersCount;
    KisViewManager *m_view;
};

#endif // KISWDGOPTIONSBRUSH_H
