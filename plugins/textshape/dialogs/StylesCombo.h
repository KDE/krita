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

class StylesModel;
class StylesComboPreview;

class StylesCombo : public QComboBox
{
    Q_OBJECT
public:
    StylesCombo(QWidget *parent);
    ~StylesCombo();

    void setStylesModel(StylesModel *model);
    void setLineEdit(QLineEdit *lineEdit);
    void setEditable(bool editable);

    void setStyleIsOriginal(bool original);

    bool eventFilter(QObject *, QEvent *);

public slots:
    void slotUpdatePreview();

signals:
    void selectionChanged(int index);
    void newStyleRequested(QString name);
    void showStyleManager(int index);
    void deleteStyle(int index);

private slots:
    void slotDeleteStyle(QModelIndex);
    void slotShowDia(QModelIndex);
    void slotSelectionChanged(int index);
    void slotItemClicked(QModelIndex);

private:
    StylesModel *m_stylesModel;
    StylesComboPreview *m_preview;
    QListView *m_view;
    int m_selectedItem;
    bool m_originalStyle;
};

#endif //STYLESCOMBO_H
