/* This file is part of the KDE project
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
#ifndef STYLESCOMBO_H
#define STYLESCOMBO_H

#include <QComboBox>

class QListView;

class AbstractStylesModel;
class StylesComboPreview;

/** This combo is specifically designed to allow choosing a text style, be it a character style or a paragraph style.
  * The combo itself does not know what type of style it is dealing with. In that respect it follows pretty much the normal QComboBox paradigm.
  * This is achieved by setting a @class StylesModel to the combo.
  * The combo also creates and uses a @class StylesDelegate in order to paint the items as preview in the dropdown menu. This delegate also provide a button to call the style manager dialog directly.
  * Additionally the combo display the style as a preview in its main area.
  * The combo allows its user to specify if the current selected style should be considered as original or not. If the style has been modified, a + button appears in the main area. Pressing it will allow to change the name of the style. Focusing out, or pressing enter will send a signal for creating a new style. Escaping will prevent this signal to be sent and return to the preview.
*/

class StylesCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit StylesCombo(QWidget *parent);
    ~StylesCombo();

    /** Use this method to set the @param model of the combo. */
    void setStylesModel(AbstractStylesModel *model);

    /** This method is an override of QComboBox setLineEdit. We need to make it public since its Qt counterpart is public. However, this method is not supposed to be used (unless you know what you are doing). The StylesCombo relies on its own internal QLineEdit subclass for quite a lot of its functionnality. There is no guarantee that the whole thing will work in case the line edit is replaced */
    void setLineEdit(QLineEdit *lineEdit);
    /** Same as above */
    void setEditable(bool editable);

    /** This method is used to specify if the currently selected style is in its original state or is considered modified. In the later case, the + button will appear (see the class description) */
    void setStyleIsOriginal(bool original);

    bool eventFilter(QObject *, QEvent *);

    /** When we don't want edit icon for our items in combo */
    void showEditIcon(bool show);

public Q_SLOTS:
    /** This slot needs to be called if the preview in the main area needs to be updated for some reason */
    void slotUpdatePreview();

Q_SIGNALS:
    /** This is emitted when a selection is made (programatically or by user interaction). It is
      * to be noted that this signal is also emitted when an item is selected again.
      * @param index: the index of the selected item. */
    void selected(int index);
    void selected(const QModelIndex &index);

    /** This is emitted when a selection is changed (programatically or by user interaction). It is
      * to be noted that this signal is _not_ emitted when an item is selected again. Not even if it
      * had been modified.
      * @param index: the index of the selected item. */
    void selectionChanged(int index);

    /** This signal is emitted on validation of the name of a modified style (after pressing the + button). This validation happens on focus out or pressed enter key.
      * @param name: the name by which the new style should be called */
    void newStyleRequested(const QString &name);

    /** This signal is emitted when the "show style manager" button is pressed in the dropdown list.
      * @param index: the index of the item on which the button was pressed */
    void showStyleManager(int index);

    /** This signal is emitted when the "delete style" button is pressed in the dropdown list.
      * @param index: the index of the item on which the button was pressed
      * This is currently disabled */
    void deleteStyle(int index);

private Q_SLOTS:
    void slotDeleteStyle(const QModelIndex &);
    void slotShowDia(const QModelIndex &);
    void slotSelectionChanged(int index);
    void slotItemClicked(const QModelIndex &);
    void slotPreviewClicked();
    void slotModelReset();

private:
    AbstractStylesModel *m_stylesModel;
    StylesComboPreview *m_preview;
    QListView *m_view;
    int m_selectedItem;
    bool m_originalStyle;
    QModelIndex m_currentIndex;
};

#endif //STYLESCOMBO_H
