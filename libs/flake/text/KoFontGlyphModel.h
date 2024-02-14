/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTGLYPHMODEL_H
#define KOFONTGLYPHMODEL_H

#include <QAbstractItemModel>
#include <QScopedPointer>
#include "KoFontLibraryResourceUtils.h"
#include "kritaflake_export.h"

/**
 * @brief The KoFontGlyphModel class
 * Creates a tree model of all the glyphs in a given face.
 *
 * The primary parents are the basic codepoints, the children of those parents (if any),
 * are glyph variations.
 */
class KRITAFLAKE_EXPORT KoFontGlyphModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    enum GlyphType {
        Base,
        UnicodeVaritionSelector,
        OpenType
    };

    KoFontGlyphModel();
    ~KoFontGlyphModel();

    enum Roles {
        OpenTypeFeatures = Qt::UserRole + 1
    };

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex indexForString(QString grapheme);
    void setFace(FT_FaceSP face);

    QHash<int, QByteArray> roleNames() const override;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KOFONTGLYPHMODEL_H
