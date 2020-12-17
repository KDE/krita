/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef STARSHAPECONFIGWIDGET_H
#define STARSHAPECONFIGWIDGET_H

#include <ui_StarShapeConfigWidget.h>

#include <KoShapeConfigWidgetBase.h>

class StarShape;

class StarShapeConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT
public:
    StarShapeConfigWidget();
    /// reimplemented
    void open(KoShape *shape) override;
    /// reimplemented
    void save() override;
    /// reimplemented
    void setUnit(const KoUnit &unit) override;
    /// reimplemented
    bool showOnShapeCreate() override
    {
        return false;
    }
    /// reimplemented
    KUndo2Command *createCommand() override;

private Q_SLOTS:
    void typeChanged();
private:
    Ui::StarShapeConfigWidget widget;
    StarShape *m_star;
};

#endif // STARSHAPECONFIGWIDGET_H
