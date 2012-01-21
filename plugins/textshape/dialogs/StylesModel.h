/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
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
#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <KIcon>

class KoStyleThumbnailer;

class KoStyleManager;
class KoParagraphStyle;
class KoCharacterStyle;

class QImage;
class QSignalMapper;
class QSize;

/** This class is used to provide widgets (like the @class StylesCombo) the styles available to the document being worked on. The @class StylesModel can be of two types: character styles or paragraph styles type. This allows the widget to ignore the type of style it is handling.
  * Character styles in ODF can be specified in two ways. First, a named character style, specifying character formatting properties. It is meant to be used on a couple of individual characters. Secondely, a paragraph style also specifies character formatting properties, which are to be considered the default for that particular paragraph.
  * For this reason, the @class Stylesmodel, when of the type @value characterStyle, do not list the paragraph style names. Only the specific named chracter styles are listed. Additionnaly, as the first item, a virtual style "As paragraph" is provided. Selecting this "style" will set the character properties as specified by the paragraph style currently applied to the selection.
  * This class requires that a @class KoStyleManager and a @class KoStyleThumbnailer be set. See below methods.
*/

class StylesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Type {
        CharacterStyle,
        ParagraphStyle
    };

    explicit StylesModel(KoStyleManager *styleManager, Type modelType, QObject *parent = 0);
    ~StylesModel();

    /** Re-implemented from QAbstractItemModel. */

    virtual QModelIndex index(int row, int column=0, const QModelIndex &parent = QModelIndex()) const;

    virtual int rowCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    /** Specific methods of the StylesModel */

    /** Returns the @class KoParagraphStyle corresponding to the specified @param index. */
    KoParagraphStyle *paragraphStyleForIndex(const QModelIndex &index) const;
    /** Return a @class QModelIndex for the specified @param style. */
    QModelIndex indexForParagraphStyle(const KoParagraphStyle &style) const;

    /** Returns the @class KoCharacterhStyle corresponding to the specified @param index. */
    KoCharacterStyle *characterStyleForIndex(const QModelIndex &index) const;
    /** Return a @class QModelIndex for the specified @param style. */
    QModelIndex indexForCharacterStyle(const KoCharacterStyle &style) const;

    /** Returns a QImage which is a preview of the style specified by @param row of the given @param size.
      * If size isn't specified, the default size of the given @class KoStyleThumbnailer is used.
    */
    QImage stylePreview(int row, QSize size = QSize());

    /** Sets the @class KoStyleManager of the model. Setting this will populate the styles. It is required that a @param manager is set before using the model.
      * CAUTION: Populating the style will select the first inserted item. If this model is already set on a view, this might cause the view to emit an item selection changed signal.
    */
    void setStyleManager(KoStyleManager *manager);
    /** Sets the @class KoStyleThumbnailer of the model. It is required that a @param thumbnailer is set before using the model. */
    void setStyleThumbnailer(KoStyleThumbnailer *thumbnailer);

    /** Specifies which paragraph style is currently the active one (on the current paragraph). This is used in order to properly preview the "As paragraph" virtual character style. */
    void setCurrentParagraphStyle(int styleId);

private slots:
    void addParagraphStyle(KoParagraphStyle*);
    void addCharacterStyle(KoCharacterStyle*);
    void removeParagraphStyle(KoParagraphStyle*);
    void removeCharacterStyle(KoCharacterStyle*);
    void updateName(int styleId);

private:
    void updateParagraphStyles();
    void updateCharacterStyles();

protected:
    QList<int> m_styleList; // list of style IDs

private:
    KoStyleManager *m_styleManager;
    KoStyleThumbnailer *m_styleThumbnailer;

    KoParagraphStyle *m_currentParagraphStyle;
    KoCharacterStyle *m_defaultCharacterStyle;
    Type m_modelType;

    KIcon m_paragIcon, m_charIcon;

    QSignalMapper *m_styleMapper;
};

#endif
