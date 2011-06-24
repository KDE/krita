/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef KUNDO2STACK_P_H
#define KUNDO2STACK_P_H

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtGui/qaction.h>

#include "kundo2stack.h"

class KUndo2Command;
class KUndo2Group;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

class KUndo2CommandPrivate
{
public:
    KUndo2CommandPrivate() : id(-1) {}
    QList<KUndo2Command*> child_list;
    QString actionText;
    QString text;
    int id;
};

#ifndef QT_NO_UNDOSTACK

#ifndef QT_NO_ACTION
class KUndo2Action : public QAction
{
    Q_OBJECT
public:
    KUndo2Action(const QString &textTemplate, const QString &defaultText, QObject *parent = 0);
public Q_SLOTS:
    void setPrefixedText(const QString &text);
private:
    QString m_textTemplate;
    QString m_defaultText;
};
#endif // QT_NO_ACTION

#endif // QT_NO_UNDOSTACK
#endif // KUNDO2STACK_P_H
