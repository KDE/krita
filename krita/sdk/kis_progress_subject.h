/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *                2004 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_PROGRESS_SUBJECT_H_
#define KIS_PROGRESS_SUBJECT_H_

#include <qobject.h>
#include <krita_export.h>

class KRITAUI_EXPORT KisProgressSubject : public QObject {
    Q_OBJECT

protected:
    KisProgressSubject() {};
    KisProgressSubject(QObject * parent, const char * name) : QObject(parent) { setObjectName(name); }
    virtual ~KisProgressSubject();

public:
    virtual void cancel() = 0;

signals:
    void notifyProgress(int percent);
    void notifyProgressStage(const QString& stage, int percent);
    void notifyProgressDone();
    void notifyProgressError();
};

#endif // KIS_PROGRESS_SUBJECT_H_

