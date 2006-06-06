/*
 *  Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#include "KoToolManager.h"
//#include "KoToolBox.h"
#include "KoToolRegistry.h"

#include <KoToolFactory.h>

#include "kactioncollection.h"
#include "kdebug.h"

#include <QToolButton>
#include <QButtonGroup>
#include <QVBoxLayout>

KoToolManager::KoToolManager()
: QObject()
, m_toolBox(0)
{
}

KoToolManager::~KoToolManager()
{
}

void KoToolManager::setup() {
    if(m_tools.size() > 0)
        return;
    KoToolRegistry *registry = KoToolRegistry::instance();
    foreach(KoID id, registry->listKeys()) {
        ToolHelper *t = new ToolHelper(registry->get(id));
kDebug(30004) << "   th" << t->id().name() << endl;
        connect(t, SIGNAL(toolActivated(ToolHelper*)), this, SLOT(toolActivated(ToolHelper*)));
        m_tools.append(t);
    }

    // do some magic for the sorting here :)  TODO
}

QWidget *KoToolManager::createToolBox() {
    setup();
    QWidget *widget = new QWidget();
    widget->setMinimumSize(20, 100);
    QButtonGroup *group = new QButtonGroup(widget);
    QVBoxLayout *lay = new QVBoxLayout(widget);
    foreach(ToolHelper *tool, m_tools) {
        QAbstractButton *but = tool->createButton(widget);
        group->addButton(but);
        lay->addWidget(but);
    }
    return widget;
}

void KoToolManager::registerTools(KActionCollection *ac) {
    // TODO
}

void KoToolManager::addCanvasView(const KoCanvasView *view) {
    // TODO
}

void KoToolManager::toolActivated(ToolHelper *tool) {
    kDebug(30004) << "ToolActivated " << tool->id().name();
}

//   ************ ToolHelper **********
QAbstractButton* ToolHelper::createButton(QWidget *parent) {
    QToolButton *but = new QToolButton(parent);
    but->setText(m_toolFactory->id().name());
    connect(but, SIGNAL(clicked()), this, SLOT(buttonPressed()));
    return but;
}

void ToolHelper::buttonPressed() {
    emit toolActivated(this);
}

//static
KoToolManager* KoToolManager::s_instance = 0;
KoToolManager* KoToolManager::instance() {
    if(s_instance == 0)
        s_instance = new KoToolManager();
    return s_instance;
}

KoID ToolHelper::id() const {
    return m_toolFactory->id();
}

#include "KoToolManager.moc"
