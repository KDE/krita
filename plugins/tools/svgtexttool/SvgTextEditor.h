/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TEXTNGSHAPECONFIGWIDGET_H
#define TEXTNGSHAPECONFIGWIDGET_H

#include <QWidget>

#include "ui_WdgSvgTextEditor.h"

#include <kxmlguiwindow.h>
#include <KoColor.h>
#include <KoSvgText.h>//for the enums

class KoSvgTextShape;
class KisFileNameRequester;

class SvgTextEditor : public KXmlGuiWindow
{
    Q_OBJECT
public:
    SvgTextEditor(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SvgTextEditor();

    //tiny enum to keep track of the tab on which editor something happens while keeping the code readable.
    enum Editor {
        Richtext, // 0
        SVGsource // 1
    };

    void setShape(KoSvgTextShape *shape);

private Q_SLOTS:

    /**
     * switch the text editor tab.
     */
    void switchTextEditorTab();
    /**
     * in rich text, check the current format, and toggle the given buttons.
     */
    void checkFormat();

    void openNew();
    void open();
    void save();
    void saveAs();
    void close();

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void deselect();
    void find();
    void findNext();
    void findPrev();
    void replace();

    void zoomOut();
    void zoomIn();
    void zoom();

    void insertSpecialCharacter();

    void setTextBold(QFont::Weight weight = QFont::Bold);
    void setTextWeightLight();
    void setTextWeightNormal();
    void setTextWeightDemi();
    void setTextWeightBlack();

    void setTextItalic(QFont::Style style = QFont::StyleOblique);
    void setTextDecoration(KoSvgText::TextDecoration decor = KoSvgText::DecorationUnderline);
    void setTextUnderline();
    void setTextOverline();
    void setTextStrikethrough();
    void setTextSubscript();
    void setTextSuperScript();
    void increaseTextSize();
    void decreaseTextSize();

    void alignLeft();
    void alignRight();
    void alignCenter();
    void alignJustified();

    void setTextFill();
    void setTextStroke();
    void setFont();
    void setSize();
    void setBaseline(KoSvgText::BaselineShiftMode baseline);

    void setShapeProperties();
    void slotConfigureToolbars();
    void slotToolbarToggled(bool);

Q_SIGNALS:

    void textUpdated(const QString &svg, const QString &defs);

protected:

    void wheelEvent(QWheelEvent *event) override;

private:

    void createAction(const QString &name,
                      const QString &text,
                      const QString &icon,
                      const char *member,
                      const QKeySequence &shortcut = QKeySequence());
    void createActions();

    Ui_WdgSvgTextEditor m_textEditorWidget;
    QWidget *m_page;
    QMap<QString, QAction> m_actions;

    KoSvgTextShape *m_shape;
};

#endif //TEXTNGSHAPECONFIGWIDGET_H
