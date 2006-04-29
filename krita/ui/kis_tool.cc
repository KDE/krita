/*
 *  Copyright (c) 2002, 2003 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qwidget.h>
#include <qstring.h>
#include <kaction.h>
#include <qlabel.h>
#include <QActionGroup>

#include <klocale.h>
#include <kdebug.h>

#include "kis_tool.h"
#include "kis_tool.moc"


class KisTool::KisToolPrivate
{
public:
    QString uiname;
    QLabel * optionWidget;
};

QActionGroup *KisTool::toolActionGroup = 0;

KisTool::KisTool(const QString & name)
{
    m_action = 0;
    m_ownAction = false;
    d = new KisToolPrivate();
    d->uiname = name;
    d->optionWidget = 0;
}

KisTool::~KisTool()
{
    if (m_ownAction) {
        delete m_action;
        m_action = 0;
    }
    delete d;
}

QWidget* KisTool::createOptionWidget(QWidget* parent)
{

    d->optionWidget = new QLabel(i18n("No options for %1.").arg(d->uiname), parent);
    d->optionWidget->setWindowTitle(d->uiname);
    d->optionWidget->setAlignment(Qt::AlignCenter);
    return d->optionWidget;
}

QWidget* KisTool::optionWidget()
{
    return d->optionWidget;
}

QActionGroup *KisTool::actionGroup() const
{
    if (toolActionGroup == 0) {
        //XXX: Do we need a parent?
        toolActionGroup = new QActionGroup(0);
    }
    return toolActionGroup;
}

