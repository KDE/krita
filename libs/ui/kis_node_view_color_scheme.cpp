/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_view_color_scheme.h"

#include <QTreeView>
#include <QStyle>

#include <kis_painting_tweaks.h>
#include <QApplication>

#include <QGlobalStatic>
#include <kis_config.h>
Q_GLOBAL_STATIC(KisNodeViewColorScheme, s_instance)

struct KisNodeViewColorScheme::Private
{
    Private() {
        if (colorLabels.isEmpty()) {
            colorLabels << Qt::transparent;
            colorLabels << QColor(91,173,220);
            colorLabels << QColor(151,202,63);
            colorLabels << QColor(247,229,61);
            colorLabels << QColor(255,170,63);
            colorLabels << QColor(177,102,63);
            colorLabels << QColor(238,50,51);
            colorLabels << QColor(191,106,209);
            colorLabels << QColor(118,119,114);

            const QColor noLabelSetColor = qApp->palette().color(QPalette::Highlight);
            for (auto it = colorLabels.begin(); it != colorLabels.end(); ++it) {
                KisPaintingTweaks::dragColor(&(*it), noLabelSetColor, 0.35);
            }
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
    KisConfig cfg(true);
    return cfg.layerThumbnailSize(false);
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
        border();
}

QRect KisNodeViewColorScheme::relVisibilityRect() const
{
    return QRect(0, 0,
                 visibilitySize() + 2 * visibilityMargin() + 2 * border(),
                 visibilitySize() + 2 * visibilityMargin() + 1 * border());
}

QRect KisNodeViewColorScheme::relThumbnailRect() const
{
    return QRect(0, 0,
                 thumbnailSize() + 2 * thumbnailMargin() + 2 * border(),
                 thumbnailSize() + 2 * thumbnailMargin() + 1 * border());
}

QRect KisNodeViewColorScheme::relDecorationRect() const
{
    return QRect(0, 0,
                 decorationSize() + 2 * decorationMargin() + 2 * border(),
                 decorationSize() + 2 * decorationMargin() + 1 * border());
}

QRect KisNodeViewColorScheme::relExpandButtonRect() const
{
    const int newY = rowHeight() - decorationMargin() - decorationSize();
    QRect rc = relDecorationRect();
    rc.moveTop(newY);
    return rc;
}

QColor KisNodeViewColorScheme::colorFromLabelIndex(int index) const
{
    /**
     * We should ensure that the index of the overflowing range
     * will never be zero again.
     */
    if (index >= m_d->colorLabels.size()) {
        index = 1 + index % (m_d->colorLabels.size() - 1);
    } else {
        index = index % m_d->colorLabels.size();
    }

    return m_d->colorLabels[index];
}

QVector<QColor> KisNodeViewColorScheme::allColorLabels() const
{
    return m_d->colorLabels;
}
