/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GLYPHPALETTEPROXYMODEL_H
#define GLYPHPALETTEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <KoFontGlyphModel.h>
#include <QChar>

class GlyphPaletteProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList blockLabels READ blockLabels NOTIFY blockLabelsChanged)
    Q_PROPERTY(int blockFilter READ blockFilter WRITE setBlockFilter NOTIFY blockFilterChanged)
    Q_PROPERTY(QString searchText READ searchText() WRITE setSearchText NOTIFY searchTextChanged)
public:
    GlyphPaletteProxyModel(QObject *parent = nullptr);
    ~GlyphPaletteProxyModel() override;

    int blockFilter() const;
    QString searchText() const;

    QVariantList blockLabels() const;

public Q_SLOTS:
    void setSearchText(const QString &text);
    void setBlockFilter(int filter);

    void emitBlockLabelsChanged();

Q_SIGNALS:
    void blockLabelsChanged();
    void blockFilterChanged();
    void searchTextChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
private:
    struct Private;
    QScopedPointer<Private> d;

};

#endif // GLYPHPALETTEPROXYMODEL_H
