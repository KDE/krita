/*
 * Copyright (c) 2013-2014 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include <kis_gmic_widget.h>
#include <QGridLayout>
#include <QPushButton>
#include <qdialogbuttonbox.h>
#include <QLabel>
#include <QCloseEvent>
#include <QLineEdit>
#include <QApplication>
#include <kis_debug.h>
#include <QMessageBox>

#include <QMetaType>
#include <klocalizedstring.h>

#include <kis_html_delegate.h>

#include <kis_gmic_filter_settings.h>
#include "kis_gmic_settings_widget.h"
#include <kis_gmic_input_output_widget.h>

#include <kis_gmic_filter_proxy_model.h>
#include "kis_gmic_updater.h"


static const QString maximizeStr = i18n("Maximize");
static const QString selectFilterStr = i18n("Select a filter...");

KisGmicWidget::KisGmicWidget(KisGmicFilterModel * filters, const QString &updateUrl): m_filterModel(filters),m_updateUrl(updateUrl)
{
    dbgPlugins << "Constructor:" << this;

    setupUi(this);
    createMainLayout();
    setAttribute(Qt::WA_DeleteOnClose, true);

    m_filterApplied = false;
    m_onCanvasPreviewActivated = false;
    m_onCanvasPreviewRequested = false;
}

KisGmicWidget::~KisGmicWidget()
{
    dbgPlugins << "Destructor:" << this;
    delete m_filterModel;
}

void KisGmicWidget::createMainLayout()
{


    connect(m_inputOutputOptions->previewCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotPreviewChanged(bool)));
    connect(m_inputOutputOptions->previewSizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPreviewSizeChanged()));
    connect(m_inputOutputOptions->previewSizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotConfigurationChanged()));

    connect(m_inputOutputOptions->zoomInButton, SIGNAL(clicked(bool)), this, SLOT(slotNotImplemented()));
    connect(m_inputOutputOptions->zoomOutButton, SIGNAL(clicked(bool)), this, SLOT(slotNotImplemented()));

    KisGmicFilterProxyModel *proxyModel = new KisGmicFilterProxyModel(this);
    proxyModel->setSourceModel(m_filterModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterTree->setModel(proxyModel);
    m_filterTree->setItemDelegate(new HtmlDelegate());

    connect(m_filterTree->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
            this, SLOT(slotSelectedFilterChanged(const QItemSelection &, const QItemSelection &)));

    if (!m_updateUrl.isEmpty())
    {
        updateBtn->setToolTip("Fetching definitions from : " + m_updateUrl);
    }
    else
    {
        updateBtn->setEnabled(false);
    }

    connect(updateBtn, SIGNAL(clicked(bool)), this, SLOT(startUpdate()));
    connect(searchBox, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));

    QPushButton * maximize = new QPushButton(maximizeStr);
    controlButtonBox->addButton(maximize, QDialogButtonBox::ActionRole);
    connect(maximize, SIGNAL(clicked(bool)), this, SLOT(slotMaximizeClicked()));
    connect(controlButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(slotOkClicked()));
    connect(controlButtonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)), this, SLOT(slotApplyClicked()));
    connect(controlButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), this, SLOT(slotCancelClicked()));
    connect(controlButtonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)), this, SLOT(slotResetClicked()));

    int indexOfFilterOptions = m_filterConfigLayout->indexOf(m_filterOptions);
    Q_ASSERT(indexOfFilterOptions != -1);
    int rowSpan = 0, colSpan = 0;
    m_filterConfigLayout->getItemPosition(indexOfFilterOptions, &m_filterOptionsRow, &m_filterOptionsColumn, &rowSpan, &colSpan);

    switchOptionsWidgetFor(new QLabel(selectFilterStr));
}


void KisGmicWidget::slotSelectedFilterChanged(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/)
 {
     //get the text of the selected item
    const QModelIndex index = m_filterTree->selectionModel()->currentIndex();
    QString selectedText = index.data(Qt::DisplayRole).toString();


    QVariant var = index.data(CommandRole);

    Command * gmicCommand(0);
    if (!var.isValid())
    {
        gmicCommand = 0;
        dbgPlugins << "Invalid QVariant, invalid command? : ';' ";
    }
    else
    {
        gmicCommand = var.value<Command *>();
    }


    if (gmicCommand)
    {
        KisGmicSettingsWidget * filterOptions = new KisGmicSettingsWidget(gmicCommand);
        QObject::connect(filterOptions, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotConfigurationChanged()));
        switchOptionsWidgetFor(filterOptions);
    }
    else
    {
        switchOptionsWidgetFor(new QLabel(selectFilterStr));
        emit sigPreviewActiveLayer();
    }



#ifdef DEBUG_MODEL
     //find out the hierarchy level of the selected item
     int hierarchyLevel = 1;
     QModelIndex seekRoot = index;
     while(seekRoot.parent() != QModelIndex())
     {
         seekRoot = seekRoot.parent();
         hierarchyLevel++;
     }

     QString showString = QString("%1, Level %2").arg(selectedText)
                          .arg(hierarchyLevel);
     setWindowTitle(showString);
#endif
 }


void KisGmicWidget::slotCancelClicked()
{
    if (m_onCanvasPreviewRequested)
    {
        emit sigCancelOnCanvasPreview();
    }
    close();
}


void KisGmicWidget::slotOkClicked()
{
    if (m_inputOutputOptions->previewSize() == ON_CANVAS)
    {
        emit sigAcceptOnCanvasPreview();
    }
    else
    {
        if (!m_filterApplied)
        {
            KisGmicFilterSetting * filterSettings = currentFilterSettings();
            if (filterSettings)
            {
                emit sigFilterCurrentImage(filterSettings);
            }
            m_filterApplied = true;
        }
    }


    emit sigRequestFinishAndClose();
    hide();
}

 void KisGmicWidget::closeEvent(QCloseEvent *event)
 {
     event->accept();
     emit sigClose();
 }

void KisGmicWidget::slotResetClicked()
{
    const QModelIndex index = m_filterTree->selectionModel()->currentIndex();
    QVariant var = index.data(CommandRole);
    Command * gmicCommand(0);
    if (!var.isValid())
    {
        gmicCommand = 0;
        dbgPlugins << "Filter not selected!";
        return;
    }
    else
    {
        gmicCommand = var.value<Command *>();
    }

    gmicCommand->reset();
    KisGmicSettingsWidget * currentSettingsWidget = qobject_cast<KisGmicSettingsWidget *>(m_filterOptions);
    if (currentSettingsWidget)
    {
        currentSettingsWidget->reload();
    }

}

void KisGmicWidget::slotMaximizeClicked()
{
    QPushButton * maximizeButton = qobject_cast<QPushButton *>(sender());
    if (!maximizeButton)
    {
        return;
    }
    if (isMaximized())
    {
        // restore clicked
        showNormal();
        maximizeButton->setText(maximizeStr);
    }
    else
    {
        showMaximized();
        maximizeButton->setText(i18n("Restore"));
    }
}

void KisGmicWidget::startUpdate()
{
    m_updater = new KisGmicUpdater(m_updateUrl);
    connect(m_updater, SIGNAL(updated()), this, SLOT(finishUpdate()));
    m_updater->start();
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void KisGmicWidget::finishUpdate()
{
    QApplication::restoreOverrideCursor();
    m_updater->deleteLater();
    QString msg = i18nc("@info",
                        "Update filters done. "
                        "Restart G'MIC dialog to finish updating! ");
    QMessageBox::information(this, i18nc("@title:window", "Updated"), msg);
}

void KisGmicWidget::slotPreviewChanged(bool enabling)
{
    if (enabling)
    {
        requestComputePreview();
    }
    else
    {
        if (m_inputOutputOptions->previewSize() == ON_CANVAS)
        {
            emit sigCancelOnCanvasPreview();
            m_onCanvasPreviewRequested = false; // cancelled
        }
        else
        {
            emit sigPreviewActiveLayer();
        }
    }

}

void KisGmicWidget::slotPreviewSizeChanged()
{
    if (m_inputOutputOptions->previewSize() == ON_CANVAS)
    {
        m_onCanvasPreviewActivated = true;
    }
    else
    {
        if (m_onCanvasPreviewActivated)
        {
            emit sigCancelOnCanvasPreview();
            m_onCanvasPreviewActivated = false;
            m_onCanvasPreviewRequested = false;
        }
    }
}


void KisGmicWidget::slotConfigurationChanged()
{
    if (m_inputOutputOptions->previewCheckBox->isChecked())
    {
        requestComputePreview();
    }
    else
    {
        emit sigPreviewActiveLayer();
    }

}

void KisGmicWidget::slotApplyClicked()
{
    if (m_inputOutputOptions->previewSize() == ON_CANVAS)
    {
        KisGmicFilterSetting * filterSettings = currentFilterSettings();
        if (!filterSettings)
        {
            return;
        }

        if (m_inputOutputOptions->previewCheckBox->isChecked())
        {
            emit sigAcceptOnCanvasPreview();
            emit sigPreviewFilterCommand(filterSettings);
        }
        else
        {
            emit sigFilterCurrentImage(filterSettings);
            m_filterApplied = true;
        }


    }
    else // Tiny, Small, Medium, Large preview
    {

            KisGmicFilterSetting * filterSettings = currentFilterSettings();
            if (filterSettings)
            {
                emit sigFilterCurrentImage(filterSettings);
                m_filterApplied = true;
                requestComputePreview();
            }
    }
}

KisGmicFilterSetting* KisGmicWidget::currentFilterSettings()
{
    KisGmicFilterSetting * filterSettings = 0;
    QVariant settings = m_filterTree->selectionModel()->currentIndex().data(FilterSettingsRole);
    if (settings.isValid())
    {
        dbgPlugins << "Valid settings!";
        filterSettings = settings.value<KisGmicFilterSetting * >();
        filterSettings->setInputLayerMode(m_inputOutputOptions->inputMode());
        filterSettings->setOutputMode(m_inputOutputOptions->outputMode());
        filterSettings->setPreviewMode(m_inputOutputOptions->previewMode());
        filterSettings->setPreviewSize(m_inputOutputOptions->previewSize());
        dbgPlugins << "GMIC command : " << filterSettings->gmicCommand();
        dbgPlugins << "GMIC preview command : " << filterSettings->previewGmicCommand();
    }
    else
    {
        dbgPlugins << "Filter is not selected!";
    }

    return filterSettings;
}

void KisGmicWidget::requestComputePreview()
{
    KisGmicFilterSetting * filterSettings = currentFilterSettings();
    if (filterSettings)
    {
        emit sigPreviewFilterCommand(filterSettings);
        if (m_onCanvasPreviewActivated)
        {
            m_onCanvasPreviewRequested = true;
        }
    }
    else
    {
        emit sigPreviewActiveLayer();
    }
}

void KisGmicWidget::switchOptionsWidgetFor(QWidget* widget)
{
    m_filterConfigLayout->removeWidget(m_filterOptions);
    delete m_filterOptions;

    m_filterOptions = widget;

    m_filterConfigLayout->addWidget(m_filterOptions, m_filterOptionsRow, m_filterOptionsColumn);
    m_filterConfigLayout->update();

}

KisFilterPreviewWidget * KisGmicWidget::previewWidget()
{
    return m_inputOutputOptions->previewWidget();
}

void KisGmicWidget::slotNotImplemented()
{
    QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Sorry, support not implemented yet."));
}
