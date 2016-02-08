/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_view_color_scheme.h"

#include <QTreeView>
#include <QStyle>


#include <QGlobalStatic>
Q_GLOBAL_STATIC(KisNodeViewColorScheme, s_instance)

struct KisNodeViewColorScheme::Private
{
    Private() {
        if (colorLabels.isEmpty()) {
            colorLabels << Qt::transparent;
            colorLabels << QColor(252, 235, 87);
            colorLabels << QColor(251, 183, 76);
            colorLabels << QColor(160, 127, 110);
            colorLabels << QColor(151, 202, 63);
            colorLabels << QColor(123, 166, 209);
            colorLabels << QColor(178, 138, 173);
            colorLabels << QColor(238, 50, 51);
            colorLabels << QColor(216, 218, 213);
            colorLabels << QColor(145, 147, 142);
        }
    }

    static QVector<QColor> colorLabels;
};

QVector<QColor> KisNodeViewColorScheme::Private::colorLabels;


KisNodeViewColorScheme::KisNodeViewColorScheme()
    : m_d(new Private)
{
}

KisNodeViewColorScheme::~KisNodeViewColorScheme()
{
}

KisNodeViewColorScheme* KisNodeViewColorScheme::instance()
{
    return s_instance;
}

QColor KisNodeViewColorScheme::gridColor(const QStyleOptionViewItem &option, QTreeView *view)
{
    const int gridHint = view->style()->styleHint(QStyle::SH_Table_GridLineColor, &option, view);
    const QColor gridColor = static_cast<QRgb>(gridHint);
    return gridColor;
}

int KisNodeViewColorScheme::visibilitySize() const
{
    return 16;
}

int KisNodeViewColorScheme::visibilityMargin() const
{
    return 2;
}


int KisNodeViewColorScheme::thumbnailSize() const
{
    return 20;
}

int KisNodeViewColorScheme::thumbnailMargin() const
{
    return 3;
}

int KisNodeViewColorScheme::decorationSize() const
{
    return 12;
}

int KisNodeViewColorScheme::decorationMargin() const
{
    return 1;
}


int KisNodeViewColorScheme::textMargin() const
{
    return 2;
}


int KisNodeViewColorScheme::iconSize() const
{
    return 16;
}

int KisNodeViewColorScheme::iconMargin() const
{
    return 1;
}

int KisNodeViewColorScheme::border() const
{
    return 1;
}

int KisNodeViewColorScheme::rowHeight() const
{
    return border() + 2 * thumbnailMargin() + thumbnailSize();
}

int KisNodeViewColorScheme::visibilityColumnWidth() const
{
    return border() +
        2 * visibilityMargin() + visibilitySize() +
        border();
}

int KisNodeViewColorScheme::indentation() const
{
    return
        2 * thumbnailMargin() + thumbnailSize() +
        border() +
        2 * decorationMargin() + decorationSize() +
        border();
}


QRect KisNodeViewColorScheme::relThumbnailRect() const
{
    return QRect(-indentation(),
                 border(),
                 thumbnailSize() + 2 * thumbnailMargin(),
                 thumbnailSize() + 2 * thumbnailMargin());
}

QRect KisNodeViewColorScheme::relDecorationRect() const
{
    return QRect(-border() - decorationMargin() - decorationSize(),
                 border() + decorationMargin(),
                 decorationSize(),
                 decorationSize());
}

QRect KisNodeViewColorScheme::relExpandButtonRect() const
{
    const int newY = rowHeight() - decorationMargin() - decorationSize();
    QRect rc = relDecorationRect();
    rc.moveTop(newY);
    return rc;
}

QColor KisNodeViewColorScheme::colorLabel(int index) const
{
    return m_d->colorLabels[index % m_d->colorLabels.size()];
}

QVector<QColor> KisNodeViewColorScheme::allColorLabels() const
{
    return m_d->colorLabels;
}
