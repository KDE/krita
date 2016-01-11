/* This file is part of the KDE project
 * Copyright (C) 2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ABSTRACTSTYLESMODEL_H
#define ABSTRACTSTYLESMODEL_H

#include <QAbstractItemModel>
#include <QSize>

class KoStyleThumbnailer;

class KoCharacterStyle;

class QImage;

/** This class is used to provide widgets (like the @class StylesCombo) the styles available to the document being worked on.
 *
 * This is an abstract class supposed to be inherited only. DO NOT instentiate this class directly.
 *
 * On top of the standard QAbstractItemModel methods to re-implement, there are 3 specific methods which need to be re-implemented in order to be used with the styles widgets:
 * - setStyleThumbnailer: a @class KoStyleThumbnailer is used to layout/draw a preview of the style
 * - indexOf: for a given character or paragraph style, returns the corresponding QModelIndex
 * - stylePreview: returns a QImage, preview of the given style (given as row number in the list model)
 *
 * Following assumptions are made:
 * - the AbstractStylesModel derived model is a flat list of items (this also means that "parent" QModelIndexes are always invalid)
 * - the AbstractStylesModel derived model has only one column
 * - there is no header in the AbstractStylesModel derived model
*/

class AbstractStylesModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Type {
        CharacterStyle,
        ParagraphStyle
    };

    enum AdditionalRoles {
        CharacterStylePointer = Qt::UserRole + 1,
        ParagraphStylePointer,
        isModifiedStyle,
        isTitleRole,
        TitleString
    };

    explicit AbstractStylesModel(QObject *parent = 0);

    /** Re-implement from QAbstractItemModel.
     *
     * No methods are reimplemented there. Subclasses need to reimplement all of QAbstractItemModel's virtual methods.
     *
     */

    /** Specific methods of the AbstractStylesModel */

    /** Sets the @class KoStyleThumbnailer of the model. It is required that a @param thumbnailer is set before using the model. */
    virtual void setStyleThumbnailer(KoStyleThumbnailer *thumbnailer) = 0;

    /** Return a @class QModelIndex for the specified @param style.
      * @param style may be either a character or paragraph style.
    */
    virtual QModelIndex indexOf(const KoCharacterStyle &style) const = 0;

    /** Returns a QImage which is a preview of the style specified by @param row of the given @param size.
      * If size isn't specified, the default size of the given @class KoStyleThumbnailer is used.
    */
    virtual QImage stylePreview(int row, const QSize &size = QSize()) = 0;
//    virtual QImage stylePreview(QModelIndex &index, const QSize &size = QSize()) = 0;

    /** Returns the type of styles in the model */
    virtual AbstractStylesModel::Type stylesType() const = 0;

protected:
    KoStyleThumbnailer *m_styleThumbnailer;
    Type m_modelType;
};

#endif // ABSTRACTSTYLESMODEL_H
