/* This file is part of the KDE project
 * Copyright (C) 2009 Jos van den Oever <jos@vandenoever.info>
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

#include "Panel.h"

#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeController.h>
#include <KoInlineTextObjectManager.h>
#include <KDebug>

Panel::Panel(QWidget *parent)
        : QDockWidget(i18n("Variables"), parent),
        m_canvas(0)
{
    QWidget *mainWidget = new QWidget(this);
    widget.setupUi(mainWidget);
    setWidget(mainWidget);
}

Panel::~Panel()
{
}

KoVariableManager* getVariableManager(KoCanvasBase *canvas) {
    if (!canvas) return NULL;
    KoInlineTextObjectManager* inlineManager
        = dynamic_cast<KoInlineTextObjectManager*>(
            canvas->shapeController()->dataCenter("InlineTextObjectManager"));
    if (!inlineManager) {
        return NULL;
    }

    return inlineManager->variableManager();
}

void
printVars(KoCanvasBase *canvas)
{
    KoVariableManager *manager = getVariableManager(canvas);
    if (!manager) return;

    foreach(const QString& name, manager->variables()) {
        qDebug() << "variable " << name;
    }
}

int countVars(KoCanvasBase *canvas)
{
    KoVariableManager *manager = getVariableManager(canvas);
    if (!manager) return 0;
    return manager->variables().size();
}

void Panel::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = canvas;
    Q_ASSERT(m_canvas);
    connect(m_canvas->toolProxy(), SIGNAL(toolChanged(const QString&)),
            this, SLOT(toolChangeDetected(const QString&)));
    connect(m_canvas->resourceProvider(),
            SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(resourceChanged(int, const QVariant&)));

    widget.variableCount->setText(QString::number(countVars(canvas)));
}

void Panel::toolChangeDetected(const QString &/*toolId*/)
{
    widget.variableCount->setText(QString::number(countVars(m_canvas)));
}

void Panel::resourceChanged(int /*key*/, const QVariant &/*value*/)
{
    // initial version: count all variables
    widget.variableCount->setText(QString::number(countVars(m_canvas)));
}

#include "Panel.moc"
