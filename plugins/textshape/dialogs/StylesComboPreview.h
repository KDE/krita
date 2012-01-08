/* This file is part of the KDE project
 * Copyright (C) 2011 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef STYLESCOMBOPREVIEW_H
#define STYLESCOMBOPREVIEW_H

#include <QLineEdit>

class QModelIndex;
class QPixmap;
class QPushButton;
class QSize;
class QString;

class StylesComboPreview : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY( bool showAddButton READ isAddButtonShown WRITE setAddButtonShown )

public:
    explicit StylesComboPreview(QWidget *parent = 0);
    ~StylesComboPreview();

    QSize availableSize() const;
    void setAddButtonShown(bool show);
    bool isAddButtonShown() const;

    void setPreview(QPixmap pixmap);

signals:
    void resized();
    void newStyleRequested(QString name);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void paintEvent(QPaintEvent *event);

private slots:
    void addNewStyle();

private:
    void init();
    void updateAddButton();

    bool m_renamingNewStyle;
    bool m_shouldAddNewStyle;

    QPixmap m_stylePreview;

    QPushButton *m_addButton;
};

#endif // STYLESCOMBOPREVIEW_H
