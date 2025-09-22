/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TEXTNGSHAPECONFIGWIDGET_H
#define TEXTNGSHAPECONFIGWIDGET_H

#include <QWidget>
#include <QTextEdit>

#include <kxmlguiwindow.h>
#include <KoColor.h>
#include <KoSvgText.h>//for the enums

#include <BasicXMLSyntaxHighlighter.h>

#include "ui_WdgSvgTextEditor.h"
#include "ui_WdgSvgTextSettings.h"

class KoSvgTextShape;
class KoDialog;

class SvgTextEditor : public KXmlGuiWindow
{
    Q_OBJECT
public:
    SvgTextEditor(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~SvgTextEditor();

    void setInitialShape(KoSvgTextShape *shape);

private Q_SLOTS:

    void slotCloseEditor();

    void save();

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

    void setSettings();

    void setModified(bool modified);
    void dialogButtonClicked(QAbstractButton *button);

Q_SIGNALS:
    void textUpdated(KoSvgTextShape *shape, const QString &svg, const QString &defs);
    void textEditorClosed();

protected:
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applySettings();

    QAction *createAction(const QString &name,
                          const char *member);
    void createActions();

    Ui_WdgSvgTextEditor m_textEditorWidget;
    QTextEdit *m_currentEditor {0};
    QWidget *m_page {0};
    QList<QAction*> m_svgTextActions;

    KoSvgTextShape *m_shape {0};
    BasicXMLSyntaxHighlighter *m_syntaxHighlighter;

    QString m_searchKey;

    class Private;
    QScopedPointer<Private> d;
};

#endif //TEXTNGSHAPECONFIGWIDGET_H
