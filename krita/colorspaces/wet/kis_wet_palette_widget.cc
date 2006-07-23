/*
 * This file is part of Krita
 *
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <qdrawutil.h>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QSpinBox>
#include <QStyle>
#include <QToolTip>
#include <QVBoxLayout>
#include <QGridLayout>

#include <klocale.h>
#include <knuminput.h>
#include <khbox.h>

#include <kis_meta_registry.h>
#include <kis_factory.h>
#include <kis_canvas_subject.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <kis_color_cup.h>

#include "kis_wet_colorspace.h"
#include "kis_wet_palette_widget.h"

KisWetPaletteWidget::KisWetPaletteWidget(QWidget *parent, const char *name) : super(parent)
{
    setObjectName(name);

    m_subject = 0;

    QVBoxLayout * vl = new QVBoxLayout(this);
    vl->setMargin(0);
    vl->setSpacing(0);
    vl->setObjectName("main layout");

    QGridLayout * l = new QGridLayout();
    l->setSpacing(2);
    l->setObjectName("color wells grid");
    vl->addLayout(l);

    KoColorCup * b;
    int WIDTH = 24;
    int HEIGHT = 24;

    b = new KoColorCup(this);
    b->setColor( QColor(240, 32, 160) );
    l->addWidget(b, 0, 0);
    b->setToolTip( i18n("Quinacridone Rose"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(159, 88, 43));
    l->addWidget(b, 0, 1);
    b->setToolTip(i18n("Indian Red"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor( QColor(254, 220, 64) );
    l->addWidget(b, 0, 2);
    b->setToolTip(i18n("Cadmium Yellow"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(36, 180, 32));
    l->addWidget(b, 0, 3);
    b->setToolTip(i18n("Hookers Green"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(16, 185, 215));
    l->addWidget(b, 0, 4);
    b->setToolTip(i18n("Cerulean Blue"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(96, 32, 8));
    l->addWidget(b, 0, 5);
    b->setToolTip(i18n("Burnt Umber"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(254, 96, 8));
    l->addWidget(b, 0, 6);
    b->setToolTip(i18n("Cadmium Red"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(255, 136, 8));
    l->addWidget(b, 0, 7);
    b->setToolTip(i18n("Brilliant Orange"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(240, 199, 8));
    l->addWidget(b, 1, 0);
    b->setToolTip(i18n("Hansa Yellow"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(96, 170, 130));
    l->addWidget(b, 1, 1);
    b->setToolTip(i18n("Phthalo Green"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(48, 32, 170));
    l->addWidget(b, 1, 2);
    b->setToolTip(i18n("French Ultramarine"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(118, 16, 135));
    l->addWidget(b, 1, 3);
    b->setToolTip(i18n("Interference Lilac"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(254, 254, 254));
    l->addWidget(b, 1, 4);
    b->setToolTip(i18n("Titanium White"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(64, 64, 74));
    l->addWidget(b, 1, 5);
    b->setToolTip(i18n("Ivory Black"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    b = new KoColorCup(this);
    b->setColor(QColor(255, 255, 255));
    l->addWidget(b, 1, 6);
    b->setToolTip(i18n("Pure Water"));
    b->setFixedSize(WIDTH, HEIGHT);
    connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

    QGridLayout * g2 = new QGridLayout();
    vl->addLayout(g2);

    QLabel * label = new QLabel(i18n("Paint strength:"), this);
    g2->addWidget(label, 0, 0);
    m_strength = new KDoubleNumInput(0.0, 2.0, 1.0, this, 0.1, 1);
    m_strength->setRange(0.0, 2.0, 0.1, true);
    connect(m_strength, SIGNAL(valueChanged(double)), this,  SLOT(slotStrengthChanged(double)));
    g2->addWidget(m_strength, 0, 1);

    label = new QLabel(i18n("Wetness:"), this);
    g2->addWidget(label, 1, 0);
    m_wetness = new KIntNumInput(16, this);
    connect(m_wetness, SIGNAL(valueChanged(int)), this, SLOT(slotWetnessChanged(int)));
    m_wetness->setRange(0, 16, true);
    g2->addWidget(m_wetness, 1, 1);

    g2->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 2, 0);
}

void KisWetPaletteWidget::update(KisCanvasSubject *subject)
{
    m_subject = subject;

}

void KisWetPaletteWidget::slotFGColorSelected(const QColor& c)
{
    KisWetColorSpace* cs = dynamic_cast<KisWetColorSpace*>(KisMetaRegistry::instance()->csRegistry()->colorSpace(KoID("WET", ""), ""));
    Q_ASSERT(cs);

    WetPack pack;
    quint8* data = reinterpret_cast<quint8*>(&pack);
    cs->fromQColor(c, data);
    pack.paint.w = 15 * m_wetness->value();
    // upscale from double to uint16:
    pack.paint.h = static_cast<quint16>(m_strength->value() * (double)(0xffff/2));
    KoColor color(data, cs);

    if(m_subject)
        m_subject->setFGColor(color);
}

void KisWetPaletteWidget::slotWetnessChanged(int n)
{
    if (!m_subject)
        return;

    KisWetColorSpace* cs = dynamic_cast<KisWetColorSpace*>(KisMetaRegistry::instance()->csRegistry()->colorSpace(KoID("WET", ""), ""));
    Q_ASSERT(cs);

    KoColor color = m_subject->fgColor();
    color.convertTo(cs);
    WetPack pack = *(reinterpret_cast<WetPack*>(color.data()));
    pack.paint.w = 15 * n;

    color.setColor(reinterpret_cast<quint8*>(&pack), cs);
    m_subject->setFGColor(color);
}

void KisWetPaletteWidget::slotStrengthChanged(double n)
{
    if (!m_subject)
        return;

    KisWetColorSpace* cs = dynamic_cast<KisWetColorSpace*>(
            KisMetaRegistry::instance()->csRegistry()->colorSpace(KoID("WET", ""), ""));
    Q_ASSERT(cs);

    KoColor color = m_subject->fgColor();
    color.convertTo(cs);
    WetPack pack = *(reinterpret_cast<WetPack*>(color.data()));
    pack.paint.h = static_cast<quint16>(n * (double)(0xffff/2)); // upscale from double to uint16

    color.setColor(reinterpret_cast<quint8*>(&pack), cs);
    m_subject->setFGColor(color);
}


#include "kis_wet_palette_widget.moc"
