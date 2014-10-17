/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kmessagebox.h>

#include <QMetaType>
#include <klocalizedstring.h>

#include <kis_html_delegate.h>

#include <kis_gmic_filter_settings.h>
#include "kis_gmic_settings_widget.h"
#include <kis_gmic_input_output_widget.h>

#include <kis_gmic_filter_proxy_model.h>
#include "kis_gmic_updater.h"

static const QString maximizeStr = i18n("Maximize");

KisGmicWidget::KisGmicWidget(KisGmicFilterModel * filters, const QString &updateUrl): QWidget(),m_filterModel(filters),m_updateUrl(updateUrl)
{
    createMainLayout();
    setAttribute(Qt::WA_DeleteOnClose, true);
}

KisGmicWidget::~KisGmicWidget()
{
    dbgPlugins << "I'm dying...";
    delete m_filterModel;
}

void KisGmicWidget::createMainLayout()
{
    m_filterConfigLayout = new QGridLayout;

    int column = 0;
    int row = 0;
    m_inputOutputOptions = new KisGmicInputOutputWidget();
    m_filterConfigLayout->addWidget(m_inputOutputOptions, row, column);
    column++;

    m_filterTree = new QTreeView();

    KisGmicFilterProxyModel *proxyModel = new KisGmicFilterProxyModel(this);
    proxyModel->setSourceModel(m_filterModel);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_filterTree->setModel(proxyModel);
    m_filterTree->setItemDelegate(new HtmlDelegate());

    QItemSelectionModel *selectionModel= m_filterTree->selectionModel();
    connect(selectionModel, SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
            this, SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));

    m_filterConfigLayout->addWidget(m_filterTree, row, column);

    QPushButton * updateBtn = new QPushButton(this);
    updateBtn->setText("Update definitions");
    if (!m_updateUrl.isEmpty())
    {
        updateBtn->setToolTip("Fetching definitions from : " + m_updateUrl);
    }
    else
    {
        updateBtn->setEnabled(false);
    }
    connect(updateBtn, SIGNAL(clicked(bool)), this, SLOT(startUpdate()));
    m_filterConfigLayout->addWidget(updateBtn, row+1, column);

    QLineEdit * searchBox = new QLineEdit(this);
    connect(searchBox, SIGNAL(textChanged(QString)), proxyModel, SLOT(setFilterFixedString(QString)));
    m_filterConfigLayout->addWidget(searchBox, row+2, column);

    column++;

    m_filterOptions = new QWidget();
    m_filterConfigLayout->addWidget(m_filterOptions,row, column);
    m_filterConfigLayout->setColumnStretch(column, 1);
    m_filterOptionsRow = row;
    m_filterOptionsColumn = column;

    QDialogButtonBox * controlButtonBox = new QDialogButtonBox;
    QPushButton * maximize = new QPushButton(maximizeStr);
    connect(maximize, SIGNAL(clicked(bool)), this, SLOT(maximizeSlot()));

    controlButtonBox->addButton(maximize, QDialogButtonBox::AcceptRole);
    controlButtonBox->addButton(QDialogButtonBox::Ok);
    QAbstractButton *okButton = controlButtonBox->button(QDialogButtonBox::Ok);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(okFilterSlot()));

    controlButtonBox->addButton(QDialogButtonBox::Apply);
    QAbstractButton *applyButton = controlButtonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(clicked(bool)), this, SLOT(applyFilterSlot()));

    controlButtonBox->addButton(QDialogButtonBox::Cancel);
    QAbstractButton *cancelButton = controlButtonBox->button(QDialogButtonBox::Cancel);
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancelFilterSlot()));

    controlButtonBox->addButton(QDialogButtonBox::Reset);
    QAbstractButton *resetButton = controlButtonBox->button(QDialogButtonBox::Reset);
    connect(resetButton, SIGNAL(clicked(bool)), this, SLOT(resetFilterSlot()));

    m_filterConfigLayout->addWidget(controlButtonBox,row + 1, column, 1, 2);
    column++;

    setLayout(m_filterConfigLayout);
}


void KisGmicWidget::selectionChangedSlot(const QItemSelection & /*newSelection*/, const QItemSelection & /*oldSelection*/)
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

    m_filterConfigLayout->removeWidget(m_filterOptions);
    delete m_filterOptions;

    if (gmicCommand)
    {
        m_filterOptions = new KisGmicSettingsWidget(gmicCommand);
    }
    else
    {
        m_filterOptions = new QLabel("Select a filter...");
    }

    m_filterConfigLayout->addWidget(m_filterOptions,m_filterOptionsRow,m_filterOptionsColumn);
    m_filterConfigLayout->update();

     //find out the hierarchy level of the selected item
     int hierarchyLevel=1;
     QModelIndex seekRoot = index;
     while(seekRoot.parent() != QModelIndex())
     {
         seekRoot = seekRoot.parent();
         hierarchyLevel++;
     }
     QString showString = QString("%1, Level %2").arg(selectedText)
                          .arg(hierarchyLevel);
     setWindowTitle(showString);

     resize(sizeHint());
 }

void KisGmicWidget::applyFilterSlot()
{
    const QModelIndex index = m_filterTree->selectionModel()->currentIndex();
    QVariant settings = index.data(FilterSettingsRole);
    if (settings.isValid())
    {
        KisGmicFilterSetting * filterSettings = settings.value<KisGmicFilterSetting * >();
        filterSettings->setInputLayerMode(m_inputOutputOptions->inputMode());
        filterSettings->setOutputMode(m_inputOutputOptions->outputMode());


        dbgPlugins << "Valid settings!";
        dbgPlugins << "GMIC command : " << filterSettings->gmicCommand();

        emit sigApplyCommand(filterSettings);
    }
    else
    {
        dbgPlugins << "Filter is not selected!";
    }

}

void KisGmicWidget::cancelFilterSlot()
{
    emit sigClose();
}


void KisGmicWidget::okFilterSlot()
{
    applyFilterSlot();
    emit sigClose();
}

 void KisGmicWidget::closeEvent(QCloseEvent *event)
 {
     emit sigClose();
     event->accept();
 }

void KisGmicWidget::resetFilterSlot()
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
    resize(sizeHint());
}

void KisGmicWidget::maximizeSlot()
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
    KMessageBox::information(this, msg, i18nc("@title:window", "Updated"));
}
