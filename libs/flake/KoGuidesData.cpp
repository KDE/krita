/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>

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

#include "KoGuidesData.h"
#include "KoViewConverter.h"
#include <KoUnit.h>
#include <KoOasisSettings.h>
#include <KoXmlWriter.h>

#include <QtGui/QPainter>

class KoGuidesData::Private
{
public:
    Private() : showGuideLines(true), guidesColor(Qt::lightGray) {}

    void parseHelpLine(const QString &text) {
        //<config:config-item config:name="SnapLinesDrawing" config:type="string">V7939H1139</config:config-item>
        QString str;
        int newPos = text.length() - 1; //start to element = 1
        for (int pos = text.length() - 1; pos >= 0;--pos) {
            if (text[pos] == 'P') {
                //point element
                str = text.mid(pos + 1, (newPos - pos));
                /*
                QStringList listVal = QStringList::split(",", str);
                int posX = (listVal[0].toInt()/100);
                int posY = (listVal[1].toInt()/100);
                point.setAttribute("posX", MM_TO_POINT(posX));
                point.setAttribute("posY", MM_TO_POINT(posY));
                */
                newPos = pos - 1;
            } else if (text[pos] == 'V') {
                //vertical element
                str = text.mid(pos + 1, (newPos - pos));
                //kDebug(30518)<<" vertical  :"<< str;
                qreal posX = str.toDouble() / 100.0;
                vertGuideLines.append(MM_TO_POINT(posX));

                newPos = (pos - 1);
            } else if (text[pos] == 'H') {
                //horizontal element
                str = text.mid(pos + 1, (newPos - pos));
                qreal posY = str.toDouble() / 100.0;
                horzGuideLines.append(MM_TO_POINT(posY));

                newPos = pos - 1;
            }
        }
    }

    /// list of positions of horizontal guide lines
    QList<qreal> horzGuideLines;
    /// list of positions of vertical guide lines
    QList<qreal> vertGuideLines;
    bool showGuideLines;
    QColor guidesColor;
};

KoGuidesData::KoGuidesData()
        : d(new Private())
{
}

KoGuidesData::~KoGuidesData()
{
    delete d;
}

void KoGuidesData::setHorizontalGuideLines(const QList<qreal> &lines)
{
    d->horzGuideLines = lines;
}

void KoGuidesData::setVerticalGuideLines(const QList<qreal> &lines)
{
    d->vertGuideLines = lines;
}

void KoGuidesData::addGuideLine(Qt::Orientation o, qreal pos)
{
    if (o == Qt::Horizontal) {
        d->horzGuideLines.append(pos);
    } else {
        d->vertGuideLines.append(pos);
    }
}

bool KoGuidesData::showGuideLines() const
{
    return d->showGuideLines;
}

void KoGuidesData::setShowGuideLines(bool show)
{
    d->showGuideLines = show;
}

QList<qreal> KoGuidesData::horizontalGuideLines() const
{
    return d->horzGuideLines;
}

QList<qreal> KoGuidesData::verticalGuideLines() const
{
    return d->vertGuideLines;
}

void KoGuidesData::paintGuides(QPainter &painter, const KoViewConverter &converter, const QRectF &area) const
{
    if (! showGuideLines())
        return;

    painter.setPen(d->guidesColor);
    foreach(qreal guide, d->horzGuideLines) {
        if (guide < area.top() || guide > area.bottom())
            continue;
        painter.drawLine(converter.documentToView(QPointF(area.left(), guide)),
                         converter.documentToView(QPointF(area.right(), guide)));
    }
    foreach(qreal guide, d->vertGuideLines) {
        if (guide < area.left() || guide > area.right())
            continue;
        painter.drawLine(converter.documentToView(QPointF(guide, area.top())),
                         converter.documentToView(QPointF(guide, area.bottom())));
    }
}

void KoGuidesData::setGuidesColor(const QColor &color)
{
    d->guidesColor = color;
}

QColor KoGuidesData::guidesColor() const
{
    return d->guidesColor;
}

bool KoGuidesData::loadOdfSettings(const KoXmlDocument & settingsDoc)
{
    d->vertGuideLines.clear();
    d->horzGuideLines.clear();

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

    QString str = firstView.parseConfigItemString("SnapLinesDrawing");
    if (!str.isEmpty())
        d->parseHelpLine(str);

    return true;
}

void KoGuidesData::saveOdfSettings(KoXmlWriter &settingsWriter)
{
    settingsWriter.startElement("config:config-item");
    settingsWriter.addAttribute("config:name", "SnapLinesDrawing");
    settingsWriter.addAttribute("config:type", "string");

    QString lineStr;

    foreach(qreal h, d->horzGuideLines) {
        int tmpY = static_cast<int>(POINT_TO_MM(h * 100.0));
        lineStr += 'H' + QString::number(tmpY);
    }
    foreach(qreal v, d->vertGuideLines) {
        int tmpX = static_cast<int>(POINT_TO_MM(v * 100.0));
        lineStr += 'V' + QString::number(tmpX);
    }

    settingsWriter.addTextNode(lineStr);
    settingsWriter.endElement(); // config:config-item
}
