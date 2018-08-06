/* This file is part of the KDE project
 * Copyright (C) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "KisColorsetChooser.h"

#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>

#include <klocalizedstring.h>

#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerProvider.h>
#include <resources/KoResource.h>
#include <resources/KoColorSet.h>

#include <resources/KoPattern.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "kis_int_parse_spin_box.h"

class ColorSetDelegate : public QAbstractItemDelegate
{
public:
    ColorSetDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    ~ColorSetDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }
};

void ColorSetDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    if (! index.isValid())
        return;

    KoResource* resource = static_cast<KoResource*>(index.internalPointer());
    KoColorSet* colorSet = static_cast<KoColorSet*>(resource);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }
    else {
        painter->setBrush(option.palette.text().color());
    }
    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, colorSet->name());

    int size = 7;
    for (quint32 i = 0; i < colorSet->nColors() && i*size < (quint32)option.rect.width(); i++) {
        QRect rect(option.rect.x() + i*size, option.rect.y() + option.rect.height() - size, size, size);
        painter->fillRect(rect, colorSet->getColorGlobal(i).color().toQColor());
    }

    painter->restore();
}

KisColorsetChooser::KisColorsetChooser(QWidget* parent): QWidget(parent)
{
    KoResourceServer<KoColorSet> * rserver = KoResourceServerProvider::instance()->paletteServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter(new KoResourceServerAdapter<KoColorSet>(rserver));
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setItemDelegate(new ColorSetDelegate(this));
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->setRowHeight(30);
    m_itemChooser->setColumnCount(1);
    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(resourceSelected(KoResource*)));

    KConfigGroup cfg = KSharedConfig::openConfig()->group("");
    m_itemChooser->configureKineticScrolling(cfg.readEntry("KineticScrollingGesture", 0),
                                             cfg.readEntry("KineticScrollingSensitivity", 75),
                                             cfg.readEntry("KineticScrollingScrollbar", true));

    QPushButton* saveButton = new QPushButton(i18n("Save"));
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSave()));

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(i18n("Insert name"));
    m_nameEdit->setClearButtonEnabled(true);

    m_columnEdit = new KisIntParseSpinBox(this);
    m_columnEdit->setRange(1, 30);
    m_columnEdit->setValue(10);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_itemChooser, 0, 0, 1, 3);
    layout->setColumnStretch(1, 1);
    layout->addWidget(saveButton, 2, 2, 1, 1);
    layout->addWidget(m_nameEdit, 1, 1, 1, 2);
    layout->addWidget(new QLabel(i18n("Name:"), this), 1, 0, 1, 1);
    layout->addWidget(m_columnEdit, 2, 1, 1, 1);
    layout->addWidget(new QLabel(i18n("Columns:"), this), 2, 0, 1, 1);
}

KisColorsetChooser::~KisColorsetChooser()
{
}

void KisColorsetChooser::resourceSelected(KoResource* resource)
{
    emit paletteSelected(static_cast<KoColorSet*>(resource));
}

void KisColorsetChooser::slotSave()
{
    KoResourceServer<KoColorSet> * rserver = KoResourceServerProvider::instance()->paletteServer();

    KoColorSet* colorset = new KoColorSet();
    colorset->setValid(true);

    QString saveLocation = rserver->saveLocation();
    QString name = m_nameEdit->text();
    int columns = m_columnEdit->value();

    bool newName = false;
    if(name.isEmpty()) {
        newName = true;
        name = i18n("Palette");
    }
    QFileInfo fileInfo(saveLocation + name + colorset->defaultFileExtension());

    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(saveLocation + name + QString("%1").arg(i) + colorset->defaultFileExtension());
        i++;
    }
    colorset->setFilename(fileInfo.filePath());
    if(newName) {
        name = i18n("Palette %1", i);
    }
    colorset->setName(name);
    colorset->setColumnCount(columns);
    rserver->addResource(colorset);
}
