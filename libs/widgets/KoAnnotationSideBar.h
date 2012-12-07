/* This file is part of the KDE project
 * Copyright (C) 2011 Steven Kakoczky <steven.kakoczky@gmail.com>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#ifndef KOANNOTATIONSIDEBAR_H
#define KOANNOTATIONSIDEBAR_H

#include <QWidget>
#include "kowidgets_export.h"
#include "KoBalloon.h"
#include "KoAnnotationBalloon.h"

#include <QTextInlineObject>

class KoAnnotationManager;

/**
 * This keeps track of Annotations; adding, removing, repositioning, displaying.
 */
class KOWIDGETS_EXPORT KoAnnotationSideBar : public QWidget
{
    Q_OBJECT
public:
    explicit KoAnnotationSideBar(const KoAnnotationManager *manager, QWidget *parent = 0);

public slots:
    // add a new annotation to the list
	//void addAnnotation(int position);
	// remove the annotation with this id
	void removeAnnotation(int id);

protected:
	// reimplemented
	//virtual void paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
					   //const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

private:
    // set the positions of the balloons relative to each other and the boundaries
    //void setPositions();
    void repositionInsert(int index);
    // reposition balloons around index, item removed before this call
    void repositionRemove(int index);

private:
	QList<KoAnnotationBalloon*> *annotations;

};

#endif // KOANNOTATIONSIDEBAR_H
