/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoGridData.h"
#include "KoViewConverter.h"

#include <KoUnit.h>
#include <KoOasisSettings.h>
#include <KoXmlWriter.h>

#include <KoIcon.h>

#include <KToggleAction>
#include <KLocale>
#include <QPainter>
#include <QRectF>
#include <KDebug>

#define DEFAULT_GRID_SIZE_MM 5.0

class KoGridData::Private
{
public:
    Private()
        : snapToGrid(false),
        showGrid(false),
        paintGridInBackground(false),
        gridX(MM_TO_POINT(DEFAULT_GRID_SIZE_MM)),
        gridY(MM_TO_POINT(DEFAULT_GRID_SIZE_MM)),
        gridColor(Qt::lightGray),
        toggleGridAction(0)
    {
    }

    ~Private()
    {
        delete toggleGridAction;
    }

    bool snapToGrid;
    bool showGrid;
    bool paintGridInBackground;
    qreal gridX, gridY;
    QColor gridColor;
    KToggleAction *toggleGridAction;
};

KoGridData::KoGridData()
        : d(new Private())
{
}

KoGridData::~KoGridData()
{
    delete d;
}

qreal KoGridData::gridX() const
{
    return d->gridX;
}

qreal KoGridData::gridY() const
{
    return d->gridY;
}

void KoGridData::setGrid(qreal x, qreal y)
{
    d->gridX = x;
    d->gridY = y;
}

bool KoGridData::snapToGrid() const
{
    return d->snapToGrid;
}

void KoGridData::setSnapToGrid(bool on)
{
    d->snapToGrid = on;
}

QColor KoGridData::gridColor() const
{
    return d->gridColor;
}

void KoGridData::setGridColor(const QColor & color)
{
    d->gridColor = color;
}

bool KoGridData::showGrid() const
{
    if (d->toggleGridAction)
        return d->toggleGridAction->isChecked();
    return d->showGrid;
}

void KoGridData::setShowGrid(bool showGrid)
{
    if (d->toggleGridAction)
        d->toggleGridAction->setChecked(showGrid);
    d->showGrid = showGrid;
}

bool KoGridData::paintGridInBackground() const
{
    return d->paintGridInBackground;
}

void KoGridData::setPaintGridInBackground(bool inBackground)
{
    d->paintGridInBackground = inBackground;
}

void KoGridData::paintGrid(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const
{
    if (! showGrid())
        return;

    painter.setPen(gridColor());

    qreal x = 0.0;
    do {
        painter.drawLine(converter.documentToView(QPointF(x, area.top())),
                         converter.documentToView(QPointF(x, area.bottom())));
        x += gridX();
    } while (x <= area.right());

    x = - gridX();
    while (x >= area.left()) {
        painter.drawLine(converter.documentToView(QPointF(x, area.top())),
                         converter.documentToView(QPointF(x, area.bottom())));
        x -= gridX();
    };

    qreal y = 0.0;
    do {
        painter.drawLine(converter.documentToView(QPointF(area.left(), y)),
                         converter.documentToView(QPointF(area.right(), y)));
        y += gridY();
    } while (y <= area.bottom());

    y = - gridY();
    while (y >= area.top()) {
        painter.drawLine(converter.documentToView(QPointF(area.left(), y)),
                         converter.documentToView(QPointF(area.right(), y)));
        y -= gridY();
    };
}

bool KoGridData::loadOdfSettings(const KoXmlDocument & settingsDoc)
{
    KoOasisSettings settings(settingsDoc);
    KoOasisSettings::Items viewSettings = settings.itemSet("ooo:view-settings");
    if (viewSettings.isNull())
        return false;

    KoOasisSettings::IndexedMap viewMap = viewSettings.indexedMap("Views");
    if (viewMap.isNull())
        return false;

    KoOasisSettings::Items firstView = viewMap.entry(0);
    if (firstView.isNull())
        return false;

    qreal gridX = firstView.parseConfigItemInt("GridFineWidth", DEFAULT_GRID_SIZE_MM);
    qreal gridY = firstView.parseConfigItemInt("GridFineHeight", DEFAULT_GRID_SIZE_MM);
    d->gridX = MM_TO_POINT(gridX / 100.0);
    d->gridY = MM_TO_POINT(gridY / 100.0);
    d->snapToGrid = firstView.parseConfigItemBool("IsSnapToGrid");

    return true;
}

void KoGridData::saveOdfSettings(KoXmlWriter &settingsWriter)
{
    settingsWriter.startElement("config:config-item");
    settingsWriter.addAttribute("config:name", "IsSnapToGrid");
    settingsWriter.addAttribute("config:type", "boolean");
    settingsWriter.addTextNode(snapToGrid() ? "true" : "false");
    settingsWriter.endElement();

    if (d->gridX >= 0.0) {
        settingsWriter.startElement("config:config-item");
        settingsWriter.addAttribute("config:name", "GridFineWidth");
        settingsWriter.addAttribute("config:type", "int");
        settingsWriter.addTextNode(QString::number(static_cast<int>(POINT_TO_MM(d->gridX * 100.0))));
        settingsWriter.endElement();
    }

    if (d->gridY >= 0.0) {
        settingsWriter.startElement("config:config-item");
        settingsWriter.addAttribute("config:name", "GridFineHeight");
        settingsWriter.addAttribute("config:type", "int");
        settingsWriter.addTextNode(QString::number(static_cast<int>(POINT_TO_MM(d->gridY * 100.0))));
        settingsWriter.endElement();
    }
}

KToggleAction *KoGridData::gridToggleAction(QWidget* canvas)
{
    if (! d->toggleGridAction) {
        d->toggleGridAction = new KToggleAction(koIcon("view-grid"), i18n("Show Grid"), 0);
        d->toggleGridAction->setToolTip(i18n("Shows or hides grid"));
        d->toggleGridAction->setChecked(d->showGrid);
    }
    if (canvas)
        QObject::connect(d->toggleGridAction, SIGNAL(toggled(bool)), canvas, SLOT(update()));
    return d->toggleGridAction;
}
