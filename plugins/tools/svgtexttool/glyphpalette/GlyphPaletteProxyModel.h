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
/**
 * @brief The GlyphPaletteProxyModel class
 *
 * This class is a filter model for the KoFontGlyphModel,
 * which allows it to be used as a character map, allowing
 * filtering on blocks and strings.
 */
class GlyphPaletteProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList blockLabels READ blockLabels NOTIFY blockLabelsChanged)
    Q_PROPERTY(int blockFilter READ blockFilter WRITE setBlockFilter NOTIFY blockFilterChanged)
    Q_PROPERTY(QString searchText READ searchText() WRITE setSearchText NOTIFY searchTextChanged)
public:
    GlyphPaletteProxyModel(QObject *parent = nullptr);
    ~GlyphPaletteProxyModel() override;

    /**
     * @brief blockFilter
     * @return current blockfilter, value is value in blockLabels.
     */
    int blockFilter() const;

    /**
     * @brief searchText
     * @return current search text. This is a single unicode codepoint.
     */
    QString searchText() const;

    /**
     * @brief blockLabels
     * @return list of unicode block filter labels, with each entry representing
     * a QVariantMap with a "name" and a "value"
     */
    QVariantList blockLabels() const;

public Q_SLOTS:
    /**
     * @brief setSearchText
     * @param text -- if the text starts with "U+", the consecutive characters are interpreted as a hex value.
     * Otherwise, the first codepoint of text is searched upon.
     */
    void setSearchText(const QString &text);
    /**
     * @brief setBlockFilter
     * set the unicode block filter.
     * @param filter -- value from blockLabels();
     */
    void setBlockFilter(int filter);

    /**
     * @brief emitBlockLabelsChanged
     * This called "block labels changed" and is should be used when the source KoFontGlyphModel changes,
     * so that the unicode block filter list is updated to the new font.
     */
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
