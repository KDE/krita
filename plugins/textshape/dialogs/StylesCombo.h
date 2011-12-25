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
#ifndef STYLESCOMBO_H
#define STYLESCOMBO_H

#include <QComboBox>
#include <QTextBlockFormat>
#include <QTextCharFormat>

class QListView;

class KoParagraphStyle;
class KoStyleManager;
class StylesModel;
class StylesComboView;
class StylesComboPreview;

class StylesCombo : public QComboBox
{
    Q_OBJECT
public:
    StylesCombo(QWidget *parent);
    ~StylesCombo();

    void setStylesModel(StylesModel *model);
//    void setStyleManager(KoStyleManager *styleManager);
    void setLineEdit(QLineEdit *lineEdit);
    void setEditable(bool editable);

    void setStyleIsOriginal(bool original);

    bool eventFilter(QObject *, QEvent *);

public slots:
//    void setCurrentFormat(const QTextBlockFormat &format);
//    void setCurrentFormat(const QTextCharFormat &format);

signals:
//    void selectionChanged(QModelIndex index);
    void selectionChanged(int index);
//    void paragraphStyleSelected(KoParagraphStyle *style);
    void newStyleRequested(QString name);

protected:
//    virtual void paintEvent(QPaintEvent *e);

private slots:
    void showDia();
    void deleteStyle(QModelIndex);
    void slotSelectionChanged(int index);
    void previewResized();

private:
    StylesModel *m_stylesModel;
//    StylesComboView *m_view;
    StylesComboPreview *m_preview;
    QListView *m_view;

//    QTextBlockFormat m_currentBlockFormat;
//    QTextCharFormat m_currentCharFormat;

//    bool skipNextHide;
};

#endif //STYLESCOMBO_H
