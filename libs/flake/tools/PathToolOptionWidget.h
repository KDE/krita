/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef PATHTOOLOPTIONWIDGET_H
#define PATHTOOLOPTIONWIDGET_H

#include <QWidget>
#include <QFlags>

#include <ui_PathToolOptionWidgetBase.h>

class KoPathTool;
class KoPathShape;
class KoShapeConfigWidgetBase;
class KoCanvasBase;


class PathToolOptionWidget : public QWidget
{
    Q_OBJECT
public:
    enum Type {
        PlainPath = 1,
        ParametricShape = 2
    };
    Q_DECLARE_FLAGS(Types, Type)

    explicit PathToolOptionWidget(KoPathTool *tool, QWidget *parent = 0);
    ~PathToolOptionWidget() override;

public Q_SLOTS:
    void setSelectionType(int type);
    void setCurrentShape(KoPathShape *pathShape);

private Q_SLOTS:
    void slotShapePropertyChanged();

Q_SIGNALS:
    void sigRequestUpdateActions();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::PathToolOptionWidgetBase widget;

    KoPathShape *m_currentShape;
    QString m_currentShapeId;
    KoShapeConfigWidgetBase *m_currentPanel;
    KoCanvasBase *m_canvas;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PathToolOptionWidget::Types)

#endif
