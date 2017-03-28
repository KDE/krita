/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoEditColorSetDialog.h"

#include <KoIcon.h>

#include <QScrollArea>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QColorDialog>
#include <QInputDialog>

#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <resources/KoColorSet.h>
#include <KoColorPatch.h>
#include <KoColorSpaceRegistry.h>
#include <KoFileDialog.h>

// debug
#include <WidgetsDebug.h>

KoEditColorSetWidget::KoEditColorSetWidget(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent)
    : QWidget(parent),
    m_colorSets(palettes),
    m_gridLayout(0),
    m_activeColorSet(0),
    m_activePatch(0),
    m_initialColorSetCount(palettes.count()),
    m_activeColorSetRequested(false)
{
    widget.setupUi(this);
    foreach (KoColorSet *colorSet, m_colorSets) {
        //colorSet->load(); resources are loaded at startup...
        widget.selector->addItem(colorSet->name());
    }
    connect(widget.selector, SIGNAL(currentIndexChanged(int)), this, SLOT(setActiveColorSet(int)));

    // A widget that shows all colors from active palette
    // FIXME no need to handcode the QScrollArea if designer can add QScrollArea (Qt 4.4?)
    m_scrollArea = new QScrollArea(widget.patchesFrame);

    int index = 0;
    foreach (KoColorSet *set, m_colorSets) {
        if (set->name() == activePalette) {
            m_activeColorSet = set;
            index = widget.selector->findText(set->name());
            widget.selector->setCurrentIndex(index);
        }
    }
    if (!m_activeColorSet && !palettes.isEmpty()) {
        m_activeColorSet = palettes.first();
        index = widget.selector->findText(m_activeColorSet->name());
    }

    int columns = 16;
    if(m_activeColorSet) {
        columns = m_activeColorSet->columnCount();
        if (columns==0){
            columns = 16;
        }
    }
    m_scrollArea->setMinimumWidth(columns*(12+2));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_scrollArea);
    widget.patchesFrame->setLayout(layout);

    widget.add->setIcon(koIcon("list-add"));
    widget.remove->setIcon(koIcon("list-remove"));
    widget.open->setIcon(koIcon("document-open"));
    widget.save->setIcon(koIcon("document-save"));

    setEnabled(m_activeColorSet != 0);
    setActiveColorSet(index);
    widget.remove->setEnabled(false); // initially no color selected

    connect(widget.add, SIGNAL(clicked()), this, SLOT(addColor()));
    connect(widget.remove, SIGNAL(clicked()), this, SLOT(removeColor()));
    connect(widget.open, SIGNAL(clicked()), this, SLOT(open()));
    connect(widget.save, SIGNAL(clicked()), this, SLOT(save()));
}

KoEditColorSetWidget::~KoEditColorSetWidget()
{
    // only delete new color sets
    uint colorSetCount = m_colorSets.count();
    for( uint i = m_initialColorSetCount; i < colorSetCount; ++i ) {
        KoColorSet * cs = m_colorSets[i];
        // if the active color set was requested by activeColorSet()
        // the caller takes ownership and then we do not delete it here
        if( cs == m_activeColorSet && m_activeColorSetRequested )
            continue;
        delete cs;
    }
}

void KoEditColorSetWidget::setActiveColorSet(int index)
{
    if (m_gridLayout) {
        qDeleteAll(m_gridLayout->children());
        delete m_gridLayout;
        m_activePatch = 0;
    }

    QWidget *wdg = new QWidget(m_scrollArea);
    m_gridLayout = new QGridLayout();
    m_gridLayout->setMargin(0);
    m_gridLayout->setSpacing(2);

    m_activeColorSet = m_colorSets.value(index);
    setEnabled(m_activeColorSet != 0);
    int columns = 16;

    if (m_activeColorSet) {
        columns = m_activeColorSet->columnCount();
        if (columns==0){columns=16;}
        widget.remove->setEnabled(false);
        for (quint32 i = 0; i < m_activeColorSet->nColors(); i++) {
            KoColorPatch *patch = new KoColorPatch(widget.patchesFrame);
            KoColorSetEntry c = m_activeColorSet->getColorGlobal(i);
            patch->setColor(c.color);
            patch->setToolTip(c.name);
            connect(patch, SIGNAL(triggered(KoColorPatch *)), this, SLOT(setTextLabel(KoColorPatch *)));
            m_gridLayout->addWidget(patch, i/columns, i%columns);
        }
    }
    m_scrollArea->setMinimumWidth(columns*(12+2));

    wdg->setLayout(m_gridLayout);
    m_scrollArea->setWidget(wdg);
}

void KoEditColorSetWidget::setTextLabel(KoColorPatch *patch)
{
    widget.colorName->setText(patch->toolTip());
    if (m_activePatch) {
        m_activePatch->setFrameShape(QFrame::NoFrame);
        m_activePatch->setFrameShadow(QFrame::Plain);
    }
    m_activePatch = patch;
    m_activePatch->setFrameShape(QFrame::Panel);
    m_activePatch->setFrameShadow(QFrame::Raised);
    widget.remove->setEnabled(true);
}

void KoEditColorSetWidget::addColor()
{
    QColor color;

    color = QColorDialog::getColor(color);
    if (color.isValid()) {
        KoColorSetEntry newEntry;
        newEntry.color = KoColor(color, KoColorSpaceRegistry::instance()->rgb8());
        newEntry.name = QInputDialog::getText(this, i18n("Add Color To Palette"), i18n("Color name:"));
        KoColorPatch *patch = new KoColorPatch(widget.patchesFrame);
        patch->setColor(newEntry.color);
        patch->setToolTip(newEntry.name);
        connect(patch, SIGNAL(triggered(KoColorPatch *)), this, SLOT(setTextLabel(KoColorPatch *)));
        Q_ASSERT(m_gridLayout);
        Q_ASSERT(m_activeColorSet);
        m_gridLayout->addWidget(patch, m_activeColorSet->nColors()/m_activeColorSet->columnCount(), m_activeColorSet->nColors()%m_activeColorSet->columnCount());
        m_activeColorSet->add(newEntry);
    }
}

void KoEditColorSetWidget::removeColor()
{
    Q_ASSERT(m_activeColorSet);
    for (quint32 i = 0; i < m_activeColorSet->nColors(); i++) {
        KoColorSetEntry c = m_activeColorSet->getColorGlobal(i);
        if (m_activePatch->color() == c.color) {
            m_activeColorSet->removeAt(i);
            setActiveColorSet(widget.selector->currentIndex());
            break;
        }
    }
}

void KoEditColorSetWidget::open()
{
    Q_ASSERT(m_activeColorSet);
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "OpenColorSet");
    dialog.setDefaultDir(m_activeColorSet->filename());
    dialog.setMimeTypeFilters(QStringList() << "application/x-gimp-color-palette");
    QString fileName = dialog.filename();
    KoColorSet *colorSet = new KoColorSet(fileName);
    colorSet->load();
    m_colorSets.append(colorSet);
    widget.selector->addItem(colorSet->name());
    widget.selector->setCurrentIndex(widget.selector->count() - 1);
}

void KoEditColorSetWidget::save()
{
    Q_ASSERT(m_activeColorSet);
    if (!m_activeColorSet->save())
        KMessageBox::error(0, i18n("Cannot write to palette file %1. Maybe it is read-only. ", m_activeColorSet->filename()), i18n("Palette"));
}

KoColorSet *KoEditColorSetWidget::activeColorSet()
{
    m_activeColorSetRequested = true;
    return m_activeColorSet;
}

KoEditColorSetDialog::KoEditColorSetDialog(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent)
    : KoDialog(parent)
{
    ui = new KoEditColorSetWidget(palettes, activePalette, this);
    setMainWidget(ui);
    setCaption(i18n("Add/Remove Colors"));
    enableButton(KoDialog::Ok, ui->isEnabled());
}

KoEditColorSetDialog::~KoEditColorSetDialog()
{
    delete ui;
}

KoColorSet *KoEditColorSetDialog::activeColorSet()
{
    return ui->activeColorSet();
}
