/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KIS_Brush_EXPORT_H_
#define _KIS_Brush_EXPORT_H_

#include <QVariant>
#include <QSpinBox>
#include <QPainter>

#include <KisImportExportFilter.h>
#include <ui_wdg_export_gih.h>
#include <kis_config_widget.h>
#include <kis_properties_configuration.h>

class SelectionModeComboBox : public QComboBox
{
    Q_OBJECT

public:
    SelectionModeComboBox(QWidget *parent)
        : QComboBox(parent)
    {
        this->addItem(i18n("Constant"));
        this->addItem(i18n("Random"));
        this->addItem(i18n("Incremental"));
        this->addItem(i18n("Pressure"));
        this->addItem(i18n("Angular"));
        this->addItem(i18n("Velocity"));
    }

};

class BrushPipeSelectionModeHelper : public QWidget
{
    Q_OBJECT

public:
    BrushPipeSelectionModeHelper(QWidget *parent, int dimension)
        : QWidget(parent)
        , cmbSelectionMode(this)
        , rank(this)
        , rankLbl(this)
        , horizLayout(this)
        , dimension(dimension)
    {
        horizLayout.setSpacing(6);
        horizLayout.setMargin(0);

        QSizePolicy sizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);

        this->setSizePolicy(sizePolicy);

        cmbSelectionMode.setSizePolicy(sizePolicy);
        cmbSelectionMode.setCurrentIndex(2);

        rank.setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
        rankLbl.setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

        rankLbl.setText(i18n("Rank"));
        horizLayout.addWidget(&rankLbl);
        horizLayout.addWidget(&rank);
        horizLayout.addWidget(&cmbSelectionMode);

        connect(&rank, SIGNAL(valueChanged(int)), this, SLOT(slotRankChanged()));

        this->hide();
        this->setEnabled(false);
    }

    SelectionModeComboBox cmbSelectionMode;
    QSpinBox rank;
    QLabel rankLbl;
    QHBoxLayout horizLayout;

    int dimension;


Q_SIGNALS:
    void rankChanged(int rankEmitter);

public Q_SLOTS:
    void slotRankChanged()
    {
        emit rankChanged(dimension);
    }

};

#include <KisViewManager.h>
#include <kis_image.h>
#include <KoProperties.h>

class KisWdgOptionsBrush : public KisConfigWidget, public Ui::WdgExportGih
{
    Q_OBJECT

public:
    KisWdgOptionsBrush(QWidget *parent)
        : KisConfigWidget(parent)
        , currentDimensions(0)
        , m_layersCount(0)
        , m_view(0)
    {
        setupUi(this);
        connect(this->brushStyle, SIGNAL(currentIndexChanged(int)), SLOT(enableSelectionMethod(int)));
        connect(this->dimensionSpin, SIGNAL(valueChanged(int)), SLOT(activateDimensionRanks()));

        enableSelectionMethod(brushStyle->currentIndex());

        BrushPipeSelectionModeHelper *bp;
        for (int i = 0; i < this->dimensionSpin->maximum(); i++) {
            bp = new BrushPipeSelectionModeHelper(0, i);
            connect(bp, SIGNAL(rankChanged(int)), SLOT(recalculateRanks(int)));
            dimRankLayout->addWidget(bp);
        }

        activateDimensionRanks();
    }

    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

public Q_SLOTS:

    void enableSelectionMethod(int value) {
        if (value == 0) {
            animStyleGroup->setEnabled(false);
        } else {
            animStyleGroup->setEnabled(true);
        }
    }

    void activateDimensionRanks()
    {
        QLayoutItem *item;
        BrushPipeSelectionModeHelper *bp;
        int dim = this->dimensionSpin->value();
        if(dim >= currentDimensions) {
            for (int i = currentDimensions; i < dim; ++i) {
                if((item = dimRankLayout->itemAt(i)) != 0) {
                    bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
                    bp->setEnabled(true);
                    bp->show();
                }
            }
        }
        else {
            for (int i = currentDimensions -1; i >= dim; --i) {
                if((item = dimRankLayout->itemAt(i)) != 0) {
                   bp = dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget());
                   bp->setEnabled(false);
                   bp->hide();
                }
            }
        }
        currentDimensions = dim;
    }

    void recalculateRanks(int rankDimension = 0) {
//        currentDimensions;
        int rankSum = 0;
        int maxDim = this->dimensionSpin->maximum();

        QVector<BrushPipeSelectionModeHelper *> bp;
        QLayoutItem *item;

        for (int i = 0; i < maxDim; ++i) {
            if((item = dimRankLayout->itemAt(i)) != 0) {
                bp.push_back(dynamic_cast<BrushPipeSelectionModeHelper*>(item->widget()));
                rankSum += bp.at(i)->rank.value();
            }
        }

        BrushPipeSelectionModeHelper *currentBrushHelper;
        BrushPipeSelectionModeHelper *callerBrushHelper = bp.at(rankDimension);
        QVectorIterator<BrushPipeSelectionModeHelper*> bpIterator(bp);

        while (rankSum > m_layersCount && bpIterator.hasNext()) {
            currentBrushHelper = bpIterator.next();

            if(currentBrushHelper != callerBrushHelper) {
                int currentValue = currentBrushHelper->rank.value();
                currentBrushHelper->rank.setValue(currentValue -1);
                rankSum -= currentValue;
            }
        }

        if (rankSum > m_layersCount) {
            callerBrushHelper->rank.setValue(m_layersCount);
        }

        if (rankSum == 0) {
            bp.at(0)->rank.setValue(m_layersCount);
            return;
        }
    }

    void setView(KisViewManager *view) override
    {
        if (view) {
            m_view = view;
            KoProperties properties;
            properties.setProperty("visible", true);
            m_layersCount = m_view->image()->root()->childNodes(QStringList("KisLayer"), properties).count();

            recalculateRanks();
        }
    }


private:
    int currentDimensions;
    int m_layersCount;
    KisViewManager *m_view;
};


class KisBrushExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisBrushExport(QObject *parent, const QVariantList &);
    ~KisBrushExport() override;
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const override;

    void initializeCapabilities() override;
};

#endif
