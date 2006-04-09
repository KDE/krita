/***************************************************************************
 * testwindow.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "testwindow.h"
#include "testplugin.h"

#include <qlabel.h>
#include <q3vbox.h>
#include <qmenu.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qdir.h>

#include <ktextedit.h>
#include <kpushbutton.h>
#include <kmenubar.h>
#include <kstandarddirs.h>

TestWindow::TestWindow(const QString& interpretername, const QString& scriptcode)
    : KMainWindow()
    , m_interpretername(interpretername)
    , m_scriptcode(scriptcode)
{
    Kross::Api::Manager* manager = Kross::Api::Manager::scriptManager();
    manager->addModule( Kross::Api::Module::Ptr(new TestPluginModule("krosstestpluginmodule")) );
    m_scriptcontainer = manager->getScriptContainer("test");

    QMenu *menuFile = new QMenu( this );
    menuBar()->insertItem( "&File", menuFile );

    m_scriptextension = new Kross::Api::ScriptGUIClient(this, this);

    QString file = KGlobal::dirs()->findResource("appdata", "testscripting.rc");
    if(file.isNull())
        file = QDir(QDir::currentDirPath()).filePath("testscripting.rc");
    else Kross::krossdebug("-------------------------222222");

    Kross::krossdebug(QString("XML-file: %1").arg(file));
    m_scriptextension->setXMLFile(file);

    //menuFile->insertSeparator();

    KAction* execaction = m_scriptextension->action("executescriptfile");
    if(execaction) execaction->plug(menuFile);

    KAction* configaction = m_scriptextension->action("configurescripts");
    if(configaction) configaction->plug(menuFile);

    KAction* scriptsaction = m_scriptextension->action("installedscripts");
    if(scriptsaction) scriptsaction->plug(menuFile);

    Q3VBox* mainbox = new Q3VBox(this);

    QGroupBox* interpretergrpbox = new QGroupBox("Interpreter", mainbox);
    interpretergrpbox->setAlignment(Qt::AlignTop);
    QStringList interpreters = Kross::Api::Manager::scriptManager()->getInterpreters();
    m_interpretercombo = new QComboBox(interpretergrpbox);
    m_interpretercombo->insertStringList(interpreters);
    m_interpretercombo->setCurrentText(interpretername);

    QGroupBox* scriptgrpbox = new QGroupBox("Scripting code", mainbox);
    scriptgrpbox->setAlignment(Qt::AlignTop);
    m_codeedit = new KTextEdit(scriptgrpbox);
    m_codeedit->setText(m_scriptcode);
    m_codeedit->setWordWrapMode(QTextOption::NoWrap);
    m_codeedit->setTextFormat(Qt::PlainText);

    Q3HBox* btnbox = new Q3HBox(mainbox);
    KPushButton* execbtn = new KPushButton("Execute", btnbox);
    connect(execbtn, SIGNAL(clicked()), this, SLOT(execute()));

    setCentralWidget(mainbox);
    setMinimumSize(600,420);
}

TestWindow::~TestWindow()
{
}

void TestWindow::execute()
{
    m_scriptcontainer->setInterpreterName( m_interpretercombo->currentText() );
    m_scriptcontainer->setCode(m_codeedit->text());
    Kross::Api::Object::Ptr result = m_scriptcontainer->execute();
    if(m_scriptcontainer->hadException()) {
        Kross::krossdebug( QString("EXCEPTION => %1").arg(m_scriptcontainer->getException()->toString()) );
    }
    else {
        QString s = result ? result->toString() : QString::null;
        Kross::krossdebug( QString("DONE => %1").arg(s) );
    }
}

#include "testwindow.moc"
