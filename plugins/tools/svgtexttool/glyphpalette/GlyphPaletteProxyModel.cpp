/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "GlyphPaletteProxyModel.h"
#include <KLocalizedString>
#include <QDebug>
#include <data/KoUnicodeBlockData.h>

GlyphPaletteProxyModel::GlyphPaletteProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

GlyphPaletteProxyModel::~GlyphPaletteProxyModel()
{
}

int GlyphPaletteProxyModel::filter() const
{
    return m_filter;
}

QMap<int, QString> GlyphPaletteProxyModel::filterLabels()
{
    QMap<int, QString> labels;
    KoFontGlyphModel *model = qobject_cast<KoFontGlyphModel*>(sourceModel());
    labels.insert(0, i18nc("@title", "All glyphs"));
    if (model) {
        for (int i=0; i < model->blocks().size(); i++) {
            labels.insert(i+1, model->blocks().at(i).name);
        }
    }
    return labels;
}

void GlyphPaletteProxyModel::setFilter(int filter)
{
    m_filter = filter;
    invalidateFilter();
}

bool GlyphPaletteProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceParent.isValid()) return true;
    KoFontGlyphModel *model = qobject_cast<KoFontGlyphModel*>(sourceModel());
    if (!model) {
        return true;
    }
    const KoUnicodeBlockData block = model->blocks().value(m_filter-1, KoUnicodeBlockDataFactory::noBlock());
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString main = sourceModel()->data(idx).toString();
    if (main.isEmpty()) return false;
    const QChar c = main.front();

    bool scriptMatch = m_filter == 0? true: block.match(c);
    return scriptMatch;
}
