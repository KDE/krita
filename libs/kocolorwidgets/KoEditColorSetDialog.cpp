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

#include <QScrollArea>
#include <QHBoxLayout>
#include <QFileInfo>

#include <KIcon>
#include <KLocale>
#include <KColorDialog>
#include <KInputDialog>
#include <KMessageBox>
#include <KFileDialog>

#include <KoColorSet.h>
#include <KoColorPatch.h>
#include <KoColorSpaceRegistry.h>

// debug
#include <KGlobal>
#include <KStandardDirs>
#include <KDebug>

KoEditColorSet::KoEditColorSet(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent)
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
        colorSet->load();
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

    m_scrollArea->setMinimumWidth(16*(12+2));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_scrollArea);
    widget.patchesFrame->setLayout(layout);

    widget.add->setIcon(KIcon("list-add"));
    widget.remove->setIcon(KIcon("list-remove"));
    widget.open->setIcon(KIcon("document-open"));
    widget.save->setIcon(KIcon("document-save"));

    setEnabled(m_activeColorSet != 0);
    setActiveColorSet(index);
    widget.remove->setEnabled(false); // initially no color selected

    connect(widget.add, SIGNAL(clicked()), this, SLOT(addColor()));
    connect(widget.remove, SIGNAL(clicked()), this, SLOT(removeColor()));
    connect(widget.open, SIGNAL(clicked()), this, SLOT(open()));
    connect(widget.save, SIGNAL(clicked()), this, SLOT(save()));
}

KoEditColorSet::~KoEditColorSet()
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

void KoEditColorSet::setActiveColorSet(int index)
{
    if (m_gridLayout) {
        delete m_gridLayout;
        m_activePatch = 0;
    }

    QWidget *wdg = new QWidget(m_scrollArea);
    m_gridLayout = new QGridLayout();
    m_gridLayout->setMargin(0);
    m_gridLayout->setSpacing(2);

    m_activeColorSet = m_colorSets.value(index);
    setEnabled(m_activeColorSet != 0);
    if (m_activeColorSet) {
        widget.remove->setEnabled(false);
        for (int i = 0; i < m_activeColorSet->nColors(); i++) {
            KoColorPatch *patch = new KoColorPatch(widget.patchesFrame);
            patch->setColor(m_activeColorSet->getColor(i).color);
            connect(patch, SIGNAL(triggered(KoColorPatch *)), this, SLOT(setTextLabel(KoColorPatch *)));
            m_gridLayout->addWidget(patch, i/16, i%16);
        }
    }

    wdg->setLayout(m_gridLayout);
    m_scrollArea->setWidget(wdg);
}

void KoEditColorSet::setTextLabel(KoColorPatch *patch)
{
    widget.colorName->setText(patch->color().toQColor().name());
    if (m_activePatch) {
        m_activePatch->setFrameShape(QFrame::NoFrame);
        m_activePatch->setFrameShadow(QFrame::Plain);
    }
    m_activePatch = patch;
    m_activePatch->setFrameShape(QFrame::Panel);
    m_activePatch->setFrameShadow(QFrame::Raised);
    widget.remove->setEnabled(true);
}

void KoEditColorSet::addColor()
{
    QColor color;
    int result = KColorDialog::getColor(color);
    if (result == KColorDialog::Accepted) {
        KoColorSetEntry newEntry;
        newEntry.color = KoColor(color, KoColorSpaceRegistry::instance()->rgb8());
        newEntry.name = KInputDialog::getText(i18n("Add Color To Palette"), i18n("Color name:"));
        KoColorPatch *patch = new KoColorPatch(widget.patchesFrame);
        patch->setColor(newEntry.color);
        connect(patch, SIGNAL(triggered(KoColorPatch *)), this, SLOT(setTextLabel(KoColorPatch *)));
        Q_ASSERT(m_gridLayout);
        Q_ASSERT(m_activeColorSet);
        m_gridLayout->addWidget(patch, m_activeColorSet->nColors()/16, m_activeColorSet->nColors()%16);
        m_activeColorSet->add(newEntry);
    }
}

void KoEditColorSet::removeColor()
{
    Q_ASSERT(m_activeColorSet);
    for (int i = 0; i < m_activeColorSet->nColors(); i++) {
        if (m_activePatch->color() == m_activeColorSet->getColor(i).color) {
            m_activeColorSet->remove(m_activeColorSet->getColor(i));
            setActiveColorSet(widget.selector->currentIndex());
            break;
        }
    }
}

void KoEditColorSet::open()
{
    Q_ASSERT(m_activeColorSet);
    QString fileName = KFileDialog::getOpenFileName(KUrl("file://"+m_activeColorSet->filename()), "*.gpl", this);
    KoColorSet *colorSet = new KoColorSet(fileName);
    colorSet->load();
    m_colorSets.append(colorSet);
    widget.selector->addItem(colorSet->name());
    widget.selector->setCurrentIndex(widget.selector->count() - 1);
}

void KoEditColorSet::save()
{
    Q_ASSERT(m_activeColorSet);
    if (!m_activeColorSet->save())
        KMessageBox::error(0, i18n("Cannot write to palette file %1. Maybe it is read-only. ", m_activeColorSet->filename()), i18n("Palette"));
}

KoColorSet *KoEditColorSet::activeColorSet()
{
    m_activeColorSetRequested = true;
    return m_activeColorSet;
}

KoEditColorSetDialog::KoEditColorSetDialog(const QList<KoColorSet *> &palettes, const QString &activePalette, QWidget *parent)
    : KDialog(parent)
{
    ui = new KoEditColorSet(palettes, activePalette, this);
    setMainWidget(ui);
    setCaption(i18n("Add/Remove Colors"));
    enableButton(KDialog::Ok, ui->isEnabled());
}

KoEditColorSetDialog::~KoEditColorSetDialog()
{
    delete ui;
}

KoColorSet *KoEditColorSetDialog::activeColorSet()
{
    return ui->activeColorSet();
}

#include <KoEditColorSetDialog.moc>
