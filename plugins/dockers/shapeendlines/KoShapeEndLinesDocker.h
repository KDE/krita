/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2002 Rob Buis <buis@kde.org>
   Copyright (C) 2004 Laurent Montel <montel@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPEENDLINESDOCKER_H
#define KOSHAPEENDLINESDOCKER_H

#include <KoUnit.h>
#include <KoXmlReader.h>
#include <KoCanvasObserver.h>
#include <QtGui/QDockWidget>
#include <QListView>
#include <QDockWidget>
#include <kdebug.h>
#include <QStringList>

class KoShapeBorderModel;
class KoCanvasController;
class KoLineEnd;
class QComboBox;

/// A docker for setting properties of end lines
class KoShapeEndLinesDocker : public QDockWidget, public KoCanvasObserver
{
    Q_OBJECT

public:
    /// Creates the end lines docker
    KoShapeEndLinesDocker();
    virtual ~KoShapeEndLinesDocker();
private slots:
    void applyChanges();
    /// End Line has changed
    void beginEndLineChanged(int index);
    void endEndLineChanged(int index);
    /// selection has changed
    void selectionChanged();
    /// reimplemented
    virtual void setCanvas( KoCanvasBase *canvas );

private:
    KoXmlDocument m_doc;
    QString m_beginEndLineCurrentName;
    QString m_endEndLineCurrentName;
    int m_iconW;
    int m_iconH;
    QSize m_iconSize;
    QMap<QString, QString> m_pathEndLineMap;
    QMap<QString, QString> m_viewEndLineMap;
    QComboBox * m_beginEndLineComboBox;
    QComboBox * m_endEndLineComboBox;
};

#endif // KOSHAPEENDLINESDOCKER_H

