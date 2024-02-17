/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "GlyphPaletteProxyModel.h"
#include <KLocalizedString>
#include <QDebug>

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
    labels.insert(QChar::Script_Unknown, i18n("Any Script"));
    labels.insert(QChar::Script_Latin, i18n("Latin"));
    labels.insert(QChar::Script_Greek, i18n("Greek"));
    labels.insert(QChar::Script_Cyrillic, i18n("Cyrillic"));
    labels.insert(QChar::Script_Armenian, i18n("Armenian"));
    labels.insert(QChar::Script_Hebrew, i18n("Hebrew"));
    labels.insert(QChar::Script_Arabic, i18n("Arabic"));
    labels.insert(QChar::Script_Syriac, i18n("Syriac"));
    return labels;
}

void GlyphPaletteProxyModel::setFilter(int filter)
{
    m_filter = QChar::Script(filter);
    invalidateFilter();
}

bool GlyphPaletteProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    QString main = sourceModel()->data(idx).toString();
    QChar::Script script = main.isEmpty()? QChar::Script_Unknown :main.front().script();

    bool scriptMatch = m_filter == 0 || m_filter == 1? true: script == m_filter;
    return scriptMatch;
}
