/*
 *  tool_warp.cc -- part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon
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

#include "tool_warp.h"

#include <stdio.h>
#include <QRectF>
#include <QPainter>

//src format must be QImage::Format_ARGB32_Premultiplied
QImage bilinearInterpolation(QImage src, int pointsPerLine, int pointsPerColumn, QPointF **origPoint, QPointF **transfPoint)
{
	if (src.format() != QImage::Format_ARGB32_Premultiplied || pointsPerLine < 2 || pointsPerColumn < 2)
		return QImage();
	
    QRectF bRect(transfPoint[0][0], transfPoint[0][0]);
	for (int i = 0; i < pointsPerColumn; ++i) {
		for (int j = 0; j < pointsPerLine; ++j) {
			if ( transfPoint[i][j].x() < bRect.left() )
				bRect.setLeft(transfPoint[i][j].x());
			else if ( transfPoint[i][j].x() > bRect.right() )
				bRect.setRight(transfPoint[i][j].x());
			if ( transfPoint[i][j].y() < bRect.top() )
				bRect.setTop(transfPoint[i][j].y());
			else if ( transfPoint[i][j].y() > bRect.bottom() )
				bRect.setBottom(transfPoint[i][j].y());
		}
	}

	QImage dst(qRound(bRect.width()), qRound(bRect.height()), QImage::Format_ARGB32_Premultiplied);

	QPainter painter(&dst);
	painter.setBrush(Qt::SolidPattern);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, false);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(0, 0, dst.width(), dst.height(), QColor(0, 0, 0, 255));

	QTransform *transfQuad;
	transfQuad = (QTransform *)malloc((pointsPerColumn - 1) * (pointsPerLine - 1) * sizeof(QTransform));
	if (transfQuad == NULL)
		return QImage();

	unsigned int k = 0;
	for (int i = 0; i < pointsPerColumn - 1; ++i) {
		for (int j = 0; j < pointsPerLine - 1; ++j) {
			QPolygonF pSrc;
			QPolygonF pDst;
			pSrc << origPoint[i][j] - origPoint[0][0] << origPoint[i][j+1] - origPoint[0][0] << origPoint[i+1][j+1] - origPoint[0][0] << origPoint[i+1][j] - origPoint[0][0];
			pDst << transfPoint[i][j] - bRect.topLeft() << transfPoint[i][j+1] - bRect.topLeft()  << transfPoint[i+1][j+1] - bRect.topLeft()  << transfPoint[i+1][j] - bRect.topLeft() ;
			QTransform::quadToQuad(pDst, pSrc, transfQuad[k]);
			++k;

			painter.setBrush(QBrush(QColor::fromRgb(k))); //it is normal that the first color is 1 and not 0
			painter.drawConvexPolygon(pDst);
		}
	}

	for (int i = 0; i < dst.height(); ++i) {
		QRgb *dstLine = (QRgb *)dst.scanLine(i);
		QRgb *dstPix = dstLine;
		for (int j = 0; j < dst.width(); ++j) {
			QColor color = QColor::fromRgb(*dstPix);
			color.setAlpha(0);
			unsigned int idTransf = color.rgba();
			if (idTransf == 0) {
				*dstPix = qRgba(0, 0, 0, 0);
			} else {
				QTransform transf = transfQuad[idTransf - 1];
				QPoint srcPoint = transf.map(QPoint(j, i));
				*dstPix = src.pixel(srcPoint.x(), srcPoint.y());
			}
			++dstPix;
		}
	}

	return dst;
}
