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

#include "KoAnnotationSideBar.h"
#include <kuser.h>
#include "KoAnnotation.h"
#include "KoAnnotationManager.h"

KoAnnotationSideBar::KoAnnotationSideBar(const KoAnnotationManager *manager, QWidget *parent) :
    QWidget(parent)
{
	annotations = new QList<KoAnnotationBalloon*>();
	QVBoxLayout *layout = new QVBoxLayout(this);
	KUser user(KUser::UseRealUserID);
    KoAnnotation *note = manager->annotation(manager->annotationNameList().first());
	KoAnnotationBalloon *balloon = new KoAnnotationBalloon(0, note, this);
	layout->addWidget(balloon, Qt::AlignTop);
	setLayout(layout);
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::darkGray);
	setPalette(pal);
	setAutoFillBackground(true);
}

/*void KoAnnotationSideBar::addAnnotation(int position)
{
	KoAnnotationBalloon *curr, *newBalloon;
    int i;
    for(i = 0; i < annotations->size(); ++i)
    {
        curr = annotations->at(i);
        if(curr->y() < position) // TODO: check for relative position in line for same y values
        {
            break; // insert here. if never reached, insert at end
        }
    }
	newBalloon = new KoAnnotationBalloon(position, this);
    newBalloon->resize(this->width(), newBalloon->sizeHint().height());
    newBalloon->move(0, position);
    annotations->insert(i, newBalloon);
   repositionInsert(i);
    newBalloon->setVisible(true);
	//newBalloon->setFocus();
    this->repaint();
}*/

void KoAnnotationSideBar::removeAnnotation(int /*id*/)
{

}

/*void KoAnnotationSideBar::paint(QPainter& painter, QPaintDevice* device,
								const QTextDocument* document, const QRectF& rect,
								QTextInlineObject obj, int position, const QTextCharFormat& format)
{
    int i;
	KoBalloon *curr;
    painter.setBackgroundMode(Qt::OpaqueMode);
    painter.setBackground(QBrush(Qt::gray));

    //setPositions();

    for(i = 0; i < annotations->size(); ++i)
    {
        curr = annotations->at(i);
		//curr->paint(event);
        QPoint anchorConnection((curr->pos()).x() - 30, curr->y());// 30 pixels to the left of current balloon
// TODO: change distance based on magnification from viewer
        painter.drawLine(anchorConnection, curr->pos());
    }
}*/

/*
 * fix collisions with adjacent balloons
 */
void KoAnnotationSideBar::repositionInsert(int index)
{
    if(index < 0 || index >= annotations->size()) return; //just to be safe
    if(annotations->size() <= 1) return; // no point in checking for collisions

    KoBalloon *curr;
    int tempTop, tempBottom, distance, currIndex;
    currIndex = index;
    // check if it collides with a lower balloon, if it does, move it up and fix any upper collisions
    if(index < annotations->size() - 1)
    {
        currIndex++;
    }
    // minimum of 2 items and currIndex > 0
    curr = annotations->at(currIndex--);
    tempTop = curr->pos().y();
    curr = annotations->at(currIndex--);
    tempBottom = curr->pos().y() + curr->height();

    while(tempTop < tempBottom)
    {
        distance = tempBottom - tempTop;
        tempTop = curr->pos().y() - distance;
        curr->move(curr->pos().x(), tempTop);
        if(currIndex < 0) break;
        curr = annotations->at(currIndex--);
        tempBottom = curr->pos().y() + curr->height();
    }
    // fix if balloons were moved past 0
    tempBottom = 0;
    currIndex++;
    while(tempTop < tempBottom)
    {
        curr->move(curr->pos().x(), tempBottom);
        tempBottom += curr->height();
        if(currIndex >= annotations->size()) break;
        curr = annotations->at(++currIndex);
        tempTop = curr->pos().y();
    }

}// END repositionInsert

void KoAnnotationSideBar::repositionRemove(int index)
{
    if(index < 0 || index > annotations->size()) return;
    if(annotations->empty()) return;


}// END repositionRemove

/* void KoAnnotationSideBar::setPositions()
{
    int i, newY;
    KoBalloon *curr;
    for(i = 0; i < annotations->size(); ++i)
    {
        curr = annotations->at(i);
       newY = curr->pos().y();
         if(lower collision)
               newY = collision->pos().y() - curr->height();
           if(newY < 0) newY = 0;
           if(upper collision)
               newY = collision->pos().y() + collision->height();


    }
} */
