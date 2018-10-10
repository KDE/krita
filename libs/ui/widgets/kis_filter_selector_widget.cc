    /*
     *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
     *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

    #include "kis_filter_selector_widget.h"

    #include <QHeaderView>
    #include <QTreeView>
    #include <QLabel>
    #include <QComboBox>
    #include <QPushButton>
    #include <QScrollArea>
    #include <QLayout>
    #include <QDialogButtonBox>
    #include <QPlainTextEdit>
    #include <QDomDocument>
    #include <QDomElement>

    #include "ui_wdgfilterselector.h"

    #include <kis_layer.h>
    #include <kis_paint_device.h>
    #include <filter/kis_filter.h>
    #include <kis_config_widget.h>
    #include <filter/kis_filter_configuration.h>
    #include "kis_default_bounds.h"

    // From krita/ui
    #include "kis_bookmarked_configurations_editor.h"
    #include "kis_bookmarked_filter_configurations_model.h"
    #include "kis_filters_model.h"
    #include "kis_config.h"

    class ThumbnailBounds : public KisDefaultBounds {
    public:
        ThumbnailBounds() : KisDefaultBounds() {}
        ~ThumbnailBounds() override {}

        QRect bounds() const override
        {
            return QRect(0, 0, 100, 100);
        }
    private:
        Q_DISABLE_COPY(ThumbnailBounds)
    };


    struct KisFilterSelectorWidget::Private {
        QWidget *currentCentralWidget {0};
        KisConfigWidget *currentFilterConfigurationWidget {0};
        KisFilterSP currentFilter;
        KisPaintDeviceSP paintDevice;
        Ui_FilterSelector uiFilterSelector;
        KisPaintDeviceSP thumb;
        KisBookmarkedFilterConfigurationsModel *currentBookmarkedFilterConfigurationsModel {0};
        KisFiltersModel *filtersModel {};
        QGridLayout *widgetLayout {};
        KisViewManager *view{};
        bool showFilterGallery {true};
    };

    KisFilterSelectorWidget::KisFilterSelectorWidget(QWidget* parent)
        : d(new Private)
    {
        Q_UNUSED(parent);
        setObjectName("KisFilterSelectorWidget");
        d->uiFilterSelector.setupUi(this);

        d->widgetLayout = new QGridLayout(d->uiFilterSelector.centralWidgetHolder);
        d->widgetLayout->setContentsMargins(0,0,0,0);
        d->widgetLayout->setHorizontalSpacing(0);

        showFilterGallery(false);

        connect(d->uiFilterSelector.filtersSelector, SIGNAL(clicked(const QModelIndex&)), SLOT(setFilterIndex(const QModelIndex &)));
        connect(d->uiFilterSelector.filtersSelector, SIGNAL(activated(const QModelIndex&)), SLOT(setFilterIndex(const QModelIndex &)));

        connect(d->uiFilterSelector.comboBoxPresets, SIGNAL(activated(int)),SLOT(slotBookmarkedFilterConfigurationSelected(int)));
        connect(d->uiFilterSelector.pushButtonEditPressets, SIGNAL(pressed()), SLOT(editConfigurations()));
        connect(d->uiFilterSelector.btnXML, SIGNAL(clicked()), this, SLOT(showXMLdialog()));

        KisConfig cfg(true);
        d->uiFilterSelector.chkRememberPreset->setChecked(cfg.readEntry<bool>("filterdialog/rememberlastpreset", false));

    }

    KisFilterSelectorWidget::~KisFilterSelectorWidget()
    {
        KisConfig cfg(false);
        cfg.writeEntry<bool>("filterdialog/rememberlastpreset", d->uiFilterSelector.chkRememberPreset->isChecked());
        delete d->filtersModel;
        delete d->currentBookmarkedFilterConfigurationsModel;
        delete d->currentCentralWidget;
        delete d->widgetLayout;
        delete d;
    }

    void KisFilterSelectorWidget::setView(KisViewManager *view)
    {
        d->view = view;
    }

    void KisFilterSelectorWidget::setPaintDevice(bool showAll, KisPaintDeviceSP _paintDevice)
    {
        if (!_paintDevice) return;

        if (d->filtersModel) delete d->filtersModel;

        d->paintDevice = _paintDevice;
        d->thumb = d->paintDevice->createThumbnailDevice(100, 100);
        d->thumb->setDefaultBounds(new ThumbnailBounds());
        d->filtersModel = new KisFiltersModel(showAll, d->thumb);

        d->uiFilterSelector.filtersSelector->setFilterModel(d->filtersModel);
        d->uiFilterSelector.filtersSelector->header()->setVisible(false);

        KisConfig cfg(true);
        QModelIndex idx = d->filtersModel->indexForFilter(cfg.readEntry<QString>("FilterSelector/LastUsedFilter", "levels"));

        if (!idx.isValid()) {
            idx = d->filtersModel->indexForFilter("levels");
        }

        if (isFilterGalleryVisible()) {
            d->uiFilterSelector.filtersSelector->activateFilter(idx);
        }

    }

    void KisFilterSelectorWidget::showFilterGallery(bool visible)
    {
        if (d->showFilterGallery == visible) {
            return;
        }

        d->showFilterGallery = visible;
        update();
        emit sigFilterGalleryToggled(visible);
        emit sigSizeChanged();
    }

    void KisFilterSelectorWidget::showXMLdialog()
    {
        if (currentFilter()->showConfigurationWidget()) {
            QDialog *xmlDialog = new QDialog();
            xmlDialog->setMinimumWidth(500);
            xmlDialog->setWindowTitle(i18n("Filter configuration XML"));
            QVBoxLayout *xmllayout = new QVBoxLayout(xmlDialog);
            QPlainTextEdit *text = new QPlainTextEdit(xmlDialog);
            KisFilterConfigurationSP config = configuration();
            text->setPlainText(config->toXML());
            xmllayout->addWidget(text);
            QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, xmlDialog);
            connect(buttons, SIGNAL(accepted()), xmlDialog, SLOT(accept()));
            connect(buttons, SIGNAL(rejected()), xmlDialog, SLOT(reject()));
            xmllayout->addWidget(buttons);
            if (xmlDialog->exec()==QDialog::Accepted) {
                QDomDocument doc;
                doc.setContent(text->toPlainText());
                config->fromXML(doc.documentElement());
                if (config) {
                    d->currentFilterConfigurationWidget->setConfiguration(config);
                }
            }
        }
    }

    bool KisFilterSelectorWidget::isFilterGalleryVisible() const
    {
        return d->showFilterGallery;
    }

    KisFilterSP KisFilterSelectorWidget::currentFilter() const
    {
        return d->currentFilter;
    }

    void KisFilterSelectorWidget::setFilter(KisFilterSP f)
    {
        Q_ASSERT(f);
        Q_ASSERT(d->filtersModel);
        setWindowTitle(f->name());
        dbgKrita << "setFilter: " << f;
        d->currentFilter = f;
        delete d->currentCentralWidget;

        {
            bool v = d->uiFilterSelector.filtersSelector->blockSignals(true);
            d->uiFilterSelector.filtersSelector->setCurrentIndex(d->filtersModel->indexForFilter(f->id()));
            d->uiFilterSelector.filtersSelector->blockSignals(v);
        }

        KisConfigWidget* widget =
                d->currentFilter->createConfigurationWidget(d->uiFilterSelector.centralWidgetHolder, d->paintDevice);

        if (!widget) { // No widget, so display a label instead
            d->uiFilterSelector.comboBoxPresets->setEnabled(false);
            d->uiFilterSelector.pushButtonEditPressets->setEnabled(false);
            d->uiFilterSelector.btnXML->setEnabled(false);

            d->currentFilterConfigurationWidget = 0;
            d->currentCentralWidget = new QLabel(i18n("No configuration options"),
                                                 d->uiFilterSelector.centralWidgetHolder);
            d->uiFilterSelector.scrollArea->setMinimumSize(d->currentCentralWidget->sizeHint());
            qobject_cast<QLabel*>(d->currentCentralWidget)->setAlignment(Qt::AlignCenter);
        } else {
            d->uiFilterSelector.comboBoxPresets->setEnabled(true);
            d->uiFilterSelector.pushButtonEditPressets->setEnabled(true);
            d->uiFilterSelector.btnXML->setEnabled(true);

            d->currentFilterConfigurationWidget = widget;
            d->currentCentralWidget = widget;
            widget->layout()->setContentsMargins(0,0,0,0);
            d->currentFilterConfigurationWidget->setView(d->view);
            d->currentFilterConfigurationWidget->blockSignals(true);
            d->currentFilterConfigurationWidget->setConfiguration(d->currentFilter->defaultConfiguration());
            d->currentFilterConfigurationWidget->blockSignals(false);
            d->uiFilterSelector.scrollArea->setContentsMargins(0,0,0,0);
            d->uiFilterSelector.scrollArea->setMinimumWidth(widget->sizeHint().width() + 18);
            connect(d->currentFilterConfigurationWidget, SIGNAL(sigConfigurationUpdated()), this, SIGNAL(configurationChanged()));
        }

        // Change the list of presets
        delete d->currentBookmarkedFilterConfigurationsModel;
        d->currentBookmarkedFilterConfigurationsModel = new KisBookmarkedFilterConfigurationsModel(d->thumb, f);
        d->uiFilterSelector.comboBoxPresets->setModel(d->currentBookmarkedFilterConfigurationsModel);

        // Add the widget to the layout
        d->currentCentralWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        d->widgetLayout->addWidget(d->currentCentralWidget, 0 , 0);

        if (d->uiFilterSelector.chkRememberPreset->isChecked()) {
            int lastBookmarkedFilterConfiguration = KisConfig(true).readEntry<int>("lastBookmarkedFilterConfiguration/" + f->id(), 0);
            if (d->uiFilterSelector.comboBoxPresets->count() > lastBookmarkedFilterConfiguration) {
                d->uiFilterSelector.comboBoxPresets->setCurrentIndex(lastBookmarkedFilterConfiguration);
                slotBookmarkedFilterConfigurationSelected(lastBookmarkedFilterConfiguration);
            }
        }

        update();
    }

    void KisFilterSelectorWidget::setFilterIndex(const QModelIndex& idx)
    {
        if (!idx.isValid()) return;

        Q_ASSERT(d->filtersModel);
        KisFilter* filter = const_cast<KisFilter*>(d->filtersModel->indexToFilter(idx));
        if (filter) {
            setFilter(filter);
        }
        else {
            if (d->currentFilter) {
                bool v = d->uiFilterSelector.filtersSelector->blockSignals(true);
                QModelIndex idx = d->filtersModel->indexForFilter(d->currentFilter->id());
                d->uiFilterSelector.filtersSelector->setCurrentIndex(idx);
                d->uiFilterSelector.filtersSelector->scrollTo(idx);
                d->uiFilterSelector.filtersSelector->blockSignals(v);
            }
        }

        KisConfig cfg(false);
        cfg.writeEntry<QString>("FilterSelector/LastUsedFilter", d->currentFilter->id());
        emit(configurationChanged());
    }

    void KisFilterSelectorWidget::slotBookmarkedFilterConfigurationSelected(int index)
    {
        if (d->currentFilterConfigurationWidget) {
            QModelIndex modelIndex = d->currentBookmarkedFilterConfigurationsModel->index(index, 0);
            KisFilterConfigurationSP config  = d->currentBookmarkedFilterConfigurationsModel->configuration(modelIndex);
            d->currentFilterConfigurationWidget->setConfiguration(config);
            if (d->currentFilter && index != KisConfig(true).readEntry<int>("lastBookmarkedFilterConfiguration/" + d->currentFilter->id(), 0)) {
                KisConfig(false).writeEntry<int>("lastBookmarkedFilterConfiguration/" + d->currentFilter->id(), index);
            }
        }
    }

    void KisFilterSelectorWidget::editConfigurations()
    {
        KisSerializableConfigurationSP config =
                d->currentFilterConfigurationWidget ? d->currentFilterConfigurationWidget->configuration() : 0;
        KisBookmarkedConfigurationsEditor editor(this, d->currentBookmarkedFilterConfigurationsModel, config);
        editor.exec();
    }

    void KisFilterSelectorWidget::update()
    {
        d->uiFilterSelector.filtersSelector->setVisible(d->showFilterGallery);
        if (d->showFilterGallery) {
            setMinimumWidth(qMax(sizeHint().width(), 700));
            d->uiFilterSelector.scrollArea->setMinimumHeight(400);
            setMinimumHeight(d->uiFilterSelector.verticalLayout->sizeHint().height());
            if (d->currentFilter) {
                bool v = d->uiFilterSelector.filtersSelector->blockSignals(true);
                d->uiFilterSelector.filtersSelector->setCurrentIndex(d->filtersModel->indexForFilter(d->currentFilter->id()));
                d->uiFilterSelector.filtersSelector->blockSignals(v);
            }
        }
        else {
            if (d->currentCentralWidget) {
                d->uiFilterSelector.scrollArea->setMinimumHeight(qMin(400, d->currentCentralWidget->sizeHint().height()));
            }
            setMinimumSize(d->uiFilterSelector.verticalLayout->sizeHint());
        }
    }

    KisFilterConfigurationSP KisFilterSelectorWidget::configuration()
    {
        if (d->currentFilterConfigurationWidget) {
            KisFilterConfigurationSP config = dynamic_cast<KisFilterConfiguration*>(d->currentFilterConfigurationWidget->configuration().data());
            if (config) {
                return config;
            }
        } else if (d->currentFilter) {
            return d->currentFilter->defaultConfiguration();
        }
        return 0;

    }

    void KisFilterTree::setFilterModel(QAbstractItemModel *model)
    {
        m_model = model;

    }

    void KisFilterTree::activateFilter(QModelIndex idx)
    {
        setModel(m_model);
        selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
        expand(idx);
        scrollTo(idx);
        emit activated(idx);
    }

    void KisFilterSelectorWidget::setVisible(bool visible)
    {
        QWidget::setVisible(visible);
        if (visible) {
            update();
        }
    }

