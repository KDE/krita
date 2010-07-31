/*
 *  kis_warptransform_worker.cc -- part of Krita
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

#include "kis_warptransform_worker.h"
#include "kis_iterators_pixel.h"
#include "kis_random_sub_accessor.h"

#include <QTransform>
#include <QVector2D>
#include <QPainter>

#include <KoProgressUpdater.h>

#include <limits.h>
#include <math.h>

typedef struct KisWarpTransformWorker::s_Fraction {
	int dividend;
	int divisor;
	//dividend and divisor must be positive
	int sign;
} Fraction;

class KisWarpTransformWorker::Side {
public:
	int ymax, x_ymin;
	Fraction invm;
	int e;
	Side *next;

	inline bool operator<(Side& C2);
};

inline bool KisWarpTransformWorker::Side::operator<(Side& C2)
{
	Side C1 = *this;
	if (C1.x_ymin < C2.x_ymin)
		return true;
	else if (C1.x_ymin>C2.x_ymin)
		return false;
	else
		return (C1.invm.sign * C1.invm.dividend * C2.invm.divisor < C2.invm.sign * C2.invm.dividend * C1.invm.divisor);
}

struct KisWarpTransformWorker::s_ExtendedSide {
	QPoint *P, *S;
	Side *Side_;
	struct s_ExtendedSide *next;
} ExtendedSide;

inline KisWarpTransformWorker::Side KisWarpTransformWorker::CalcSide(QPoint P1, QPoint P2, Side *next)
{
	Side C;

	if (P1.y() <= P2.y()) {
		C.ymax = P2.y();
		C.x_ymin = P1.x();
		C.invm.dividend = (P2.x() - P1.x());
		C.invm.divisor = (P2.y() - P1.y());
	} else {
		C.ymax = P1.y();
		C.x_ymin = P2.x();
		C.invm.dividend = (P1.x() - P2.x());
		C.invm.divisor = (P1.y() - P2.y());
	}

	if (C.invm.dividend < 0)
		C.invm.sign = -1;
	else
		C.invm.sign = 1;

	C.invm.dividend = abs(C.invm.dividend);
	C.next = next;
	C.e = 0;

	return C;
}

inline KisWarpTransformWorker::Side KisWarpTransformWorker::CalcSide(QPoint *P1, QPoint *P2, Side *next)
{
	return CalcSide(*P1, *P2, next);
}

inline void KisWarpTransformWorker::AddExtSide(ExtendedSide **S, QPoint P0, QPoint P1)
{
	(*S)->next = new ExtendedSide;
	(*S) = (*S)->next;
	(*S)->P = new QPoint(P0);
	(*S)->S = new QPoint(P1);
	(*S)->Side_ = new Side;
	*((*S)->Side_) = CalcSide(P0, P1, NULL);
	(*S)->next = NULL;
}

void KisWarpTransformWorker::CreateExtSides(ExtendedSide **sides, QPolygon polygon)
{
	ExtendedSide *CurrExtSide;
	ExtendedSide *Senti;
	QPoint FirstPoint, CurrPoint, NextPoint;

	if (polygon.size() <= 1)
		return;

	Senti = new ExtendedSide;
	Senti->next = NULL;
	CurrExtSide = Senti;
	FirstPoint = polygon.at(0);
	CurrPoint = FirstPoint;

	for (int i = 1; i < polygon.size(); ++i) {
		NextPoint = polygon.at(i);
		AddExtSide(&CurrExtSide, CurrPoint, NextPoint);
		CurrPoint = NextPoint;
	}

	AddExtSide(&CurrExtSide, CurrPoint, FirstPoint);
	*sides = Senti->next;
	delete Senti;
}

inline void KisWarpTransformWorker::Insert(Side **L, Side *NewSide)
{
	NewSide->next = *L;
	*L = NewSide;
}

inline void KisWarpTransformWorker::InsertInSortedList(Side **L, Side *NewSide)
{
	Side *CurrSide = *L;

	if (*L == NULL || *NewSide < **L) {
		NewSide->next = *L;
		*L = NewSide;
		return;
	}

	while (CurrSide->next != NULL && *(CurrSide->next) < *NewSide)
		CurrSide = CurrSide->next;

	NewSide->next = CurrSide->next;
	CurrSide->next = NewSide;
}

void KisWarpTransformWorker::FreeSidesList(Side *L)
{
	Side *nextSide;

	while (L != NULL) {
		nextSide = L->next;
		delete L;
		L = nextSide;
	}
}

void KisWarpTransformWorker::FreeSidesTable(Side *TC[], int top, int bottom)
{
	for (int i = top; i <= bottom; ++i)
		FreeSidesList(TC[i]);
}

void KisWarpTransformWorker::FreeExtSide(ExtendedSide *S)
{
	if (S == NULL)
		return;

	delete S->P;
	delete S->S;
	delete S->Side_;
	delete S;
}

void KisWarpTransformWorker::FreeExtSides(ExtendedSide **S)
{
	ExtendedSide *NextExtSide;
	ExtendedSide *CurrentExtSide = *S;

	while (CurrentExtSide != NULL) {
		NextExtSide = CurrentExtSide->next;
		FreeExtSide(CurrentExtSide);
		CurrentExtSide = NextExtSide;
	}

	*S = NULL;
}

//requires dest allocated
//create a copie of the side represented by P, S, C and adds it to dest
inline void KisWarpTransformWorker::AddExtSide(ExtendedSide **dest, QPoint P, QPoint S, Side C)
{
	(*dest)->next = new ExtendedSide;
	*dest = (*dest)->next;
	(*dest)->P = new QPoint(P);
	(*dest)->S = new QPoint(S);
	(*dest)->Side_ = new Side;
	*((*dest)->Side_) = C;
	(*dest)->next = NULL;
}

inline void KisWarpTransformWorker::setRegion(bool reg[4], int x0, int y0, QRect clipRect)
{
	for (int i = 0; i < 4; ++i)
		reg[i] = false;

	if (x0 < clipRect.topLeft().x())
		reg[LEFTSIDE] = true;
	else if (x0 > clipRect.bottomRight().x())
		reg[RIGHTSIDE] = true;

	if (y0 < clipRect.topLeft().y())
		reg[UPSIDE] = true;
	else if (y0 > clipRect.bottomRight().y())
		reg[DOWNSIDE] = true;
}

void KisWarpTransformWorker::Sutherland_Hodgman(ExtendedSide **Dest, ExtendedSide *ExtSide, QRect clipRect, ClipperSide CS, bool &PreviousPointOut)
{
	bool reg[2][4];
	int dx, dy, xTmp, tmp, eTmp, sign;
	Side C;
	QPoint NewPoint;

	setRegion(reg[0], ExtSide->P->x(), ExtSide->P->y(), clipRect);
	setRegion(reg[1], ExtSide->S->x(), ExtSide->S->y(), clipRect);
	dx = ExtSide->Side_->invm.dividend;
	dy = ExtSide->Side_->invm.divisor;
	sign = ExtSide->Side_->invm.sign;
	switch (CS) {
	case LEFTSIDE:
		if ( (!reg[0][LEFTSIDE]) && (!reg[1][LEFTSIDE]) ) {
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else if ( (!reg[0][LEFTSIDE]) && reg[1][LEFTSIDE] ) {
			if (dy == 0) {
				tmp = ExtSide->Side_->ymax;
			} else {
				if (ExtSide->S->y() == ExtSide->Side_->ymax) {
					tmp = ExtSide->P->y();
					eTmp = ExtSide->Side_->e;
					xTmp = ExtSide->Side_->x_ymin;
					while (xTmp >= clipRect.topLeft().x()) {
						eTmp += dx;
						while (2 * eTmp > dy) {
							--xTmp;
							eTmp -= dy;
						}
						++tmp;
					}
					ExtSide->Side_->ymax = tmp;
				} else {
					tmp = ExtSide->S->y();
					while (ExtSide->Side_->x_ymin < clipRect.topLeft().x()) {
						ExtSide->Side_->e += dx;
						while (2 * ExtSide->Side_->e > dy) {
							++ExtSide->Side_->x_ymin;
							ExtSide->Side_->e -= dy;
						}
						++tmp;
					}
				}
			}

			NewPoint = QPoint(clipRect.topLeft().x(), tmp);
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), NewPoint, *(ExtSide->Side_));
			PreviousPointOut = true;
		} else if ( reg[0][LEFTSIDE] && (!reg[1][LEFTSIDE]) ) {
			if (dy == 0) {
				tmp = ExtSide->Side_->ymax;
			} else {
				if (ExtSide->P->y() == ExtSide->Side_->ymax) {
					tmp = ExtSide->S->y();
					eTmp = ExtSide->Side_->e;
					xTmp = ExtSide->Side_->x_ymin;
					while (xTmp >= clipRect.topLeft().x()) {
						eTmp += dx;
						while (2 * eTmp > dy) {
							--xTmp;
							eTmp -= dy;
						}
						++tmp;
					}
					ExtSide->Side_->ymax = tmp;
				} else {
					tmp = ExtSide->P->y();
					while (ExtSide->Side_->x_ymin < clipRect.topLeft().x()) {
						ExtSide->Side_->e += dx;
						while (2 * ExtSide->Side_->e > dy) {
							++ExtSide->Side_->x_ymin;
							ExtSide->Side_->e -= dy;
						}
						++tmp;
					}
				}
			}

			NewPoint = QPoint(clipRect.topLeft().x(), tmp);
			C = CalcSide(*((*Dest)->S), NewPoint, NULL);
			AddExtSide(Dest, *((*Dest)->S), NewPoint, C);
			AddExtSide(Dest, NewPoint, *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else {
			PreviousPointOut = true;
		}

		break;
	case RIGHTSIDE:
		if ( (!reg[0][RIGHTSIDE]) && (!reg[1][RIGHTSIDE]) ) {
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else if ( (!reg[0][RIGHTSIDE]) && reg[1][RIGHTSIDE] ) {
			if (dy == 0) {
				tmp = ExtSide->Side_->ymax;
			} else {
				if (ExtSide->S->y() == ExtSide->Side_->ymax) {
					tmp = ExtSide->P->y();
					eTmp = ExtSide->Side_->e;
					xTmp = ExtSide->Side_->x_ymin;
					while (xTmp <= clipRect.bottomRight().x()) {
						eTmp += dx;
						while (2 * eTmp > dy) {
							++xTmp;
							eTmp -= dy;
						}
						++tmp;
					}
					ExtSide->Side_->ymax = tmp;
				} else {
					tmp = ExtSide->S->y();
					while (ExtSide->Side_->x_ymin > clipRect.bottomRight().x()) {
						ExtSide->Side_->e += dx;
						while (2 * ExtSide->Side_->e > dy) {
							--ExtSide->Side_->x_ymin;
							ExtSide->Side_->e -= dy;
						}
						++tmp;
					}
				}
			}

			NewPoint = QPoint(clipRect.bottomRight().x(), tmp);
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), NewPoint, *(ExtSide->Side_));
			PreviousPointOut = true;
		} else if ( reg[0][RIGHTSIDE] && (!reg[1][RIGHTSIDE]) ) {
			if (dy == 0) {
				tmp = ExtSide->Side_->ymax;
			} else {
				if (ExtSide->P->y() == ExtSide->Side_->ymax) {
					tmp = ExtSide->S->y();
					eTmp = ExtSide->Side_->e;
					xTmp = ExtSide->Side_->x_ymin;
					while (xTmp <= clipRect.bottomRight().x()) {
						eTmp += dx;
						while (2 * eTmp > dy) {
							++xTmp;
							eTmp -= dy;
						}
						++tmp;
					}
					ExtSide->Side_->ymax = tmp;
				} else {
					tmp = ExtSide->P->y();
					while (ExtSide->Side_->x_ymin > clipRect.bottomRight().x()) {
						ExtSide->Side_->e += dx;
						while (2 * ExtSide->Side_->e > dy) {
							--ExtSide->Side_->x_ymin;
							ExtSide->Side_->e -= dy;
						}
						++tmp;
					}
				}
			}

			NewPoint = QPoint(clipRect.bottomRight().x(), tmp);
			C = CalcSide(*((*Dest)->S), NewPoint, NULL);
			AddExtSide(Dest, *((*Dest)->S), NewPoint, C);
			AddExtSide(Dest, NewPoint, *(ExtSide->S),*(ExtSide->Side_));
			PreviousPointOut = false;
		} else {
			PreviousPointOut = true;
		}

		break;
	case UPSIDE:
		if ( (!reg[0][UPSIDE]) && (!reg[1][UPSIDE]) ) {
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else if ( (!reg[0][UPSIDE]) && reg[1][UPSIDE] ) {
			ExtSide->Side_->e += dx * (clipRect.topLeft().y() - ExtSide->S->y());
			ExtSide->Side_->x_ymin += sign * ExtSide->Side_->e / dy;

			if (ExtSide->Side_->e > 0) {
				ExtSide->Side_->e = ExtSide->Side_->e % dy;
				while (2 * ExtSide->Side_->e > dy) {
					ExtSide->Side_->x_ymin += sign;
					ExtSide->Side_->e -= dy;
				}
			}

			NewPoint = QPoint(ExtSide->Side_->x_ymin, clipRect.topLeft().y());
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), NewPoint, *(ExtSide->Side_));
			PreviousPointOut = true;
		} else if ( reg[0][UPSIDE] && (!reg[1][UPSIDE]) ) {
			ExtSide->Side_->e += dx * (clipRect.topLeft().y() - ExtSide->P->y());
			ExtSide->Side_->x_ymin += sign *ExtSide->Side_->e / dy;

			if (ExtSide->Side_->e > 0) {
				ExtSide->Side_->e = ExtSide->Side_->e % dy;
				while (2 * ExtSide->Side_->e > dy) {
					ExtSide->Side_->x_ymin += sign;
					ExtSide->Side_->e -= dy;
				}
			}
			NewPoint = QPoint(ExtSide->Side_->x_ymin, clipRect.topLeft().y());
			C = CalcSide(*((*Dest)->S), NewPoint, NULL);
			AddExtSide(Dest, *((*Dest)->S), NewPoint, C);
			AddExtSide(Dest, NewPoint, *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else {
			PreviousPointOut = true;
		}
		break;
	case DOWNSIDE:
		if ( (!reg[0][DOWNSIDE]) && (!reg[1][DOWNSIDE]) ) {
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			AddExtSide(Dest, *(ExtSide->P), *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else if ( (!reg[0][DOWNSIDE]) && reg[1][DOWNSIDE] ) {
			tmp = (sign * dx * (clipRect.bottomRight().y() - ExtSide->P->y()) + ExtSide->P->x() * dy) / dy;
			NewPoint = QPoint(tmp, clipRect.bottomRight().y());
			if (PreviousPointOut) {
				C = CalcSide((*Dest)->S, ExtSide->P, NULL);
				AddExtSide(Dest, *((*Dest)->S), *(ExtSide->P), C);
			}
			ExtSide->Side_->ymax = clipRect.bottomRight().y();
			AddExtSide(Dest, *(ExtSide->P), NewPoint, *(ExtSide->Side_));
			PreviousPointOut = true;
		} else if ( reg[0][DOWNSIDE] && (!reg[1][DOWNSIDE]) ) {
			tmp = (sign * dx * (clipRect.bottomRight().y() - ExtSide->P->y()) + ExtSide->P->x() * dy) / dy;
			NewPoint = QPoint(tmp, clipRect.bottomRight().y());
			C = CalcSide(*((*Dest)->S), NewPoint, NULL);
			AddExtSide(Dest, *((*Dest)->S), NewPoint, C);
			ExtSide->Side_->ymax = clipRect.bottomRight().y();
			AddExtSide(Dest, NewPoint, *(ExtSide->S), *(ExtSide->Side_));
			PreviousPointOut = false;
		} else {
			PreviousPointOut = true;
		}
		break;
	}	
}

void KisWarpTransformWorker::ClipPolygone(ExtendedSide **ExtSides, QRect *clipper)
{
	ExtendedSide *Senti;
	ExtendedSide *NewExtSides;
	ExtendedSide *CurrExtSide;
	bool PreviousPointOut = false;
	QRect clipRect = clipper->adjusted(0, 0, 1, 1);

	Senti = new ExtendedSide;
	Senti->P = new QPoint(0, 0);
	Senti->S = new QPoint(0, 0);
	Senti->Side_ = new Side;
	Senti->Side_->next = NULL;
	Senti->next = NULL;

	for (int i = 0; i < 4; ++i) {
		if (*ExtSides == NULL) {
			return;
		} else if ((*ExtSides)->next == NULL) {
			//only one side : no interior => nothing to draw
			FreeExtSides(ExtSides);
			return;
		}

		CurrExtSide = *ExtSides;
		NewExtSides = Senti;
		Senti->next = NULL;
		PreviousPointOut = false;
		while (CurrExtSide != NULL) {
			Sutherland_Hodgman(&NewExtSides, CurrExtSide, clipRect, (ClipperSide)i, PreviousPointOut);
			CurrExtSide = CurrExtSide->next;
		}
		if (PreviousPointOut && Senti->next != NULL) {
			*(Senti->next->P) = *(NewExtSides->S);
			*(Senti->next->Side_) = CalcSide(Senti->next->P, Senti->next->S, NULL);
		}
		FreeExtSides(ExtSides);
		*ExtSides = Senti->next;
	}

	FreeExtSide(Senti);
}

void KisWarpTransformWorker::quadInterpolation(QImage *src, QImage *dst, QPolygon pSrc, QPolygon pDst)
{
	QRgb *pixels = NULL;
	Side *TC[dst->height() + 1];
	Side *TCA = NULL;
	int y;
	Side *PrevSide = NULL, *CurrSide = NULL, *NextSide = NULL;
	Side *Senti = NULL;
	bool InsidePolygone = false, ChangeSideForNextPixel = false;
	ExtendedSide *ExtSides = NULL, *CurrExtSide = NULL;
	QRect clipRect;
	QRgb transparent = qRgba(0, 0, 0, 0);

	QTransform mTransform;
	QTransform::quadToQuad(QPolygonF(pDst), QPolygonF(pSrc), mTransform);

	clipRect = QRect(QPoint(0, 0), QPoint(dst->width() - 1, dst->height() - 1));
	QRect boundRect = clipRect.intersected(pDst.boundingRect());

	for (int i = boundRect.top(); i <= boundRect.bottom() + 1; ++i)
		TC[i] = NULL;

	CreateExtSides(&ExtSides, pDst);
	ClipPolygone(&ExtSides, &clipRect);

	if (ExtSides == NULL || ExtSides->next == NULL) {
		FreeExtSides(&ExtSides);
		return;
	}

	Senti = new Side;

	//init TC
	CurrExtSide = ExtSides;
	while (CurrExtSide != NULL) {
		if (CurrExtSide->P->y() <= CurrExtSide->S->y())
			Insert(&TC[CurrExtSide->P->y()], CurrExtSide->Side_);
		else
			Insert(&TC[CurrExtSide->S->y()], CurrExtSide->Side_);

		CurrExtSide = CurrExtSide->next;
	}

	y = boundRect.top();

	while (TCA != NULL || y <= boundRect.bottom()) {
		pixels = (QRgb *)dst->scanLine(y);

		//insert elements of TC(y) in TCA
		CurrSide = TC[y];
		while (CurrSide != NULL) {
			NextSide = CurrSide->next;
			InsertInSortedList(&TCA, CurrSide);
			CurrSide = NextSide;
		}
		TC[y] = NULL;

		//delete elements of TCA for which ymax=y
		if (TCA != NULL) {
			Senti->next = TCA;
			PrevSide = Senti;
			CurrSide = Senti->next;
			while (CurrSide != NULL) {
				NextSide = CurrSide->next;
				if (CurrSide->ymax == y)
					PrevSide->next = NextSide;
				else
					PrevSide = CurrSide;

				CurrSide = NextSide;
			}
			PrevSide->next = NULL;
			TCA = Senti->next;
		}

		if (TCA != NULL) {
			Senti->x_ymin = INT_MIN;
			Senti->invm.divisor = 0;
			//senti is first in TCA : it must be the smallest element
			Senti->next = TCA;
			PrevSide = Senti;
		}

		//fill scanline
		CurrSide = TCA;
		InsidePolygone = false;
		ChangeSideForNextPixel = false;
		pixels += ptrdiff_t(boundRect.left());
		for (int j = boundRect.left(); j <= boundRect.right() + 1; ++j) {
			if (ChangeSideForNextPixel) {
				InsidePolygone = !InsidePolygone;
				ChangeSideForNextPixel = false;
			}

			while (CurrSide != NULL && CurrSide->x_ymin <= j) {
				if (CurrSide->invm.sign * CurrSide->e <= 0 || CurrSide->x_ymin == boundRect.right() + 1)
					InsidePolygone = !InsidePolygone;
				else
					ChangeSideForNextPixel = !ChangeSideForNextPixel;

				CurrSide->e += CurrSide->invm.dividend;
				while (2 * CurrSide->e > (CurrSide->invm.divisor)) {
					CurrSide->x_ymin += CurrSide->invm.sign;
					CurrSide->e -= CurrSide->invm.divisor;
				}

				if (*CurrSide < *PrevSide) {
					//we need to re-sort (partially) TCA
					NextSide = CurrSide->next;
					PrevSide->next = NextSide;
					InsertInSortedList(&TCA, CurrSide);
					CurrSide = NextSide;
				} else {
					PrevSide = CurrSide;
					CurrSide = CurrSide->next;
				}
			}

			if (InsidePolygone) {
				QPoint p = mTransform.map(QPointF(j, y)).toPoint();

				if (src->rect().contains(p))
					*pixels = src->pixel(p);
				else
					*pixels = transparent;
			}

			++pixels;
		}

		++y;
	}

	delete Senti;
	FreeSidesList(TCA);
	FreeSidesTable(TC, boundRect.top(), boundRect.bottom() + 1);
	FreeExtSides(&ExtSides);
}

/* Applies an affine transformation to the point v according the original
set of points p, the new set of points q, and the constant alpha.
The algorithm is based a paper "Image Deformation Using Moving Least Squares" */
inline QPointF KisWarpTransformWorker::calcAffineTransformation(QPointF v, int nbPoints, QPointF *p, QPointF *q, qreal alpha)
{
	qreal w[nbPoints];
	qreal sumWi = 0;
	QPointF pStar(0, 0), qStar(0, 0);
	QPointF pHat[nbPoints], qHat[nbPoints];

	for (int i = 0; i < nbPoints; ++i) {
		if (v == p[i])
			return q[i];

		QVector2D tmp(p[i] - v);
		w[i] = 1. / pow(tmp.lengthSquared(), alpha);
		pStar += w[i] * p[i];
		qStar += w[i] * q[i];
		sumWi += w[i];
	}
	pStar /= sumWi;
	qStar /= sumWi;

	qreal A_tmp[4] = {0, 0, 0, 0};
	for (int i = 0; i < nbPoints; ++i) {
		pHat[i] = p[i] - pStar;
		qHat[i] = q[i] - qStar;

		A_tmp[0] += w[i] * pow(pHat[i].x(), 2);
		A_tmp[3] += w[i] * pow(pHat[i].y(), 2);
		A_tmp[1] += w[i] * pHat[i].x() * pHat[i].y();
	}
	A_tmp[2] = A_tmp[1];
	qreal det_A_tmp = A_tmp[0] * A_tmp[3] - A_tmp[1] * A_tmp[2];

	qreal A_tmp_inv[4];

	if (det_A_tmp == 0)
		return v;

	A_tmp_inv[0] = A_tmp[3] / det_A_tmp;
	A_tmp_inv[1] = - A_tmp[1] / det_A_tmp;
	A_tmp_inv[2] = A_tmp_inv[1];
	A_tmp_inv[3] = A_tmp[0] / det_A_tmp;

	QPointF t = v - pStar;
	QPointF A_precalc(t.x() * A_tmp_inv[0] + t.y() * A_tmp_inv[1], t.x() * A_tmp_inv[2] + t.y() * A_tmp_inv[3]);
	qreal A_j;

	QPointF res = qStar;
	for (int j = 0; j < nbPoints; ++j) {
		A_j = A_precalc.x() * pHat[j].x() + A_precalc.y() * pHat[j].y();

		res += w[j] * A_j * qHat[j];
	}

	return res;
}

inline void KisWarpTransformWorker::switchVertices(QPoint **a, QPoint **b)
{
	QPoint *tmp = *a;
	*a = *b;
	*b = tmp;
}

QImage KisWarpTransformWorker::affineTransformation(QImage *src, qint32 pointsPerLine, qint32 pointsPerColumn, QPointF *origPoint, QPointF *transfPoint, qreal alpha, QPointF originalTopLeft, QPointF *newTopLeft)
{
	qint32 nbPoints = pointsPerLine * pointsPerColumn;

    QRectF origBRect(transfPoint[0], transfPoint[0]);
    QRectF transfBRect(transfPoint[0], transfPoint[0]);
	for (int i = 0; i < nbPoints; ++i) {
		if ( transfPoint[i].x() < transfBRect.left() )
			transfBRect.setLeft(transfPoint[i].x());
		else if ( transfPoint[i].x() > transfBRect.right() )
			transfBRect.setRight(transfPoint[i].x());
		if ( transfPoint[i].y() < transfBRect.top() )
			transfBRect.setTop(transfPoint[i].y());
		else if ( transfPoint[i].y() > transfBRect.bottom() )
			transfBRect.setBottom(transfPoint[i].y());
	}

	QImage dst(ceil(transfBRect.width()), ceil(transfBRect.height()), QImage::Format_ARGB32_Premultiplied);
	*newTopLeft = transfBRect.topLeft();

	QPainter painter(&dst);
	painter.setBrush(Qt::SolidPattern);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, false);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(0, 0, dst.width(), dst.height(), QColor(0, 0, 0, 0));

	if (src->height() == 0)
		return QImage();

	int pixelPrecision = 20;
	int verticesPerLine = src->width() / pixelPrecision + 2;

	QPoint *previousLineVertices = new QPoint[verticesPerLine]();
	QPoint *currentLineVertices = new QPoint[verticesPerLine]();
	Q_ASSERT(previousLineVertices != NULL && currentLineVertices != NULL);

	int i, prevI;
	int j, prevJ;
	int k, prevK;
	double x, y;
	bool lineDone;
	bool imageDone;

	//begin the treatment
	//first line needs a special treatment
	i = 0;
	y = originalTopLeft.y();
	j = 0;
	x = originalTopLeft.x();
	k = 0;
	QRgb *srcLine = (QRgb *)src->scanLine(i);
	QRgb *srcPix = srcLine;
	lineDone = false;
	while (!lineDone) {
		while (j < src->width()) {
			QPointF dstPosF = calcAffineTransformation(QPointF(x, y), nbPoints, origPoint, transfPoint, alpha);
			dstPosF -= *newTopLeft;
			QPoint dstPos = QPoint(qRound(dstPosF.x()), qRound(dstPosF.y()));

			previousLineVertices[k] = dstPos;

			srcPix += pixelPrecision;
			j += pixelPrecision;
			x += pixelPrecision;
			++k;
		}

		//maybe we skipped the last pixels of the column (if the image width is not a multiple of pixelPrecision)
		if (j - pixelPrecision < src->width() - 1) {
			j = src->width() - 1;
			x = j + originalTopLeft.x();
			srcPix = (QRgb *)src->scanLine(i) + src->width() - 1;
		} else
			lineDone = true;
	}

	//then the other lines
	imageDone = false;
	prevI = i;
	i += pixelPrecision;
	y += pixelPrecision;
	while (!imageDone) {
		while (i < src->height()) {
			srcLine = (QRgb *)src->scanLine(i);
			srcPix = srcLine;
			x = originalTopLeft.x();

			//first column needs a special treatment
			QPointF dstPosF = calcAffineTransformation(QPointF(x, y), nbPoints, origPoint, transfPoint, alpha);
			dstPosF -= *newTopLeft;
			QPoint dstPos = QPoint(qRound(dstPosF.x()), qRound(dstPosF.y()));

			currentLineVertices[0] = dstPos;

			srcPix += pixelPrecision;
			x += pixelPrecision;

			//the other columns
			j = pixelPrecision;
			prevJ = 0;
			k = 1;
			prevK = 0;
			lineDone = false;
			while (!lineDone) {
				while (j < src->width()) {
					dstPosF = calcAffineTransformation(QPointF(x, y), nbPoints, origPoint, transfPoint, alpha);
					dstPosF -= *newTopLeft;
					QPoint dstPos = QPoint(qRound(dstPosF.x()), qRound(dstPosF.y()));

					currentLineVertices[k] = dstPos;

					QPolygon pSrc, pDst;
					pSrc << QPoint(prevJ, prevI) << QPoint(j, prevI) << QPoint(j, i) << QPoint(prevJ, i);
					pDst << previousLineVertices[prevK] << previousLineVertices[k]  << currentLineVertices[k]  << currentLineVertices[prevK];

					quadInterpolation(src, &dst, pSrc, pDst);

					srcPix += pixelPrecision;
					prevJ = j;
					j += pixelPrecision;
					prevK = k;
					++k;
					x += pixelPrecision;
				}

				if (j - pixelPrecision < src->width() - 1) {
					j = src->width() - 1;
					x = j + originalTopLeft.x();
					srcPix = (QRgb *)src->scanLine(i) + src->width() - 1;
				} else
					lineDone = true;
			}

			switchVertices(&previousLineVertices, &currentLineVertices);
			
			prevI = i;
			i += pixelPrecision;
			y += pixelPrecision;
		}

		if (i - pixelPrecision < src->height() - 1) {
			i = src->height() - 1;
			y = i + originalTopLeft.y();
			srcPix = (QRgb *)src->scanLine(src->height() - 1);
		} else
			imageDone = true;
	}

	delete previousLineVertices;
	delete currentLineVertices;

	return dst;
}

KisWarpTransformWorker::KisWarpTransformWorker(KisPaintDeviceSP dev, qint32 pointsPerLine, qint32 pointsPerColumn, QPointF *origPoint, QPointF *transfPoint, qreal alpha, KoUpdater *progress)
        : m_dev(dev), m_progress(progress)
{
	m_pointsPerLine = pointsPerLine;
	m_pointsPerColumn = pointsPerColumn;
	m_origPoint = origPoint;
	m_transfPoint = transfPoint;
	m_alpha = alpha;
}

KisWarpTransformWorker::~KisWarpTransformWorker()
{
}

void KisWarpTransformWorker::quadInterpolation(KisPaintDeviceSP src, KisPaintDeviceSP dst, QPolygon pSrc, QPolygon pDst)
{
	Side *TCA = NULL;
	int y;
	Side *PrevSide = NULL, *CurrSide = NULL, *NextSide = NULL;
	Side *Senti = NULL;
	bool InsidePolygone = false, ChangeSideForNextPixel = false;
	ExtendedSide *ExtSides = NULL, *CurrExtSide = NULL;
	QRect clipRect;

	QTransform mTransform;
	QTransform::quadToQuad(QPolygonF(pDst), QPolygonF(pSrc), mTransform);

	QRect boundRect = pDst.boundingRect();
	if (boundRect.top() < 0)
		boundRect.setTop(0);
	if (boundRect.bottom() < 0)
		boundRect.setBottom(0);
	if (boundRect.left() < 0)
		boundRect.setLeft(0);
	if (boundRect.right() < 0)
		boundRect.setRight(0);
	Side *TC[boundRect.bottom() + 2];

	for (int i = 0; i <= boundRect.bottom() + 1; ++i)
		TC[i] = NULL;

	CreateExtSides(&ExtSides, pDst);
	ClipPolygone(&ExtSides, &boundRect);

	if (ExtSides == NULL || ExtSides->next == NULL) {
		FreeExtSides(&ExtSides);
		return;
	}

	Senti = new Side;

	//init TC
	CurrExtSide = ExtSides;
	while (CurrExtSide != NULL) {
		if (CurrExtSide->P->y() <= CurrExtSide->S->y())
			Insert(&TC[CurrExtSide->P->y()], CurrExtSide->Side_);
		else
			Insert(&TC[CurrExtSide->S->y()], CurrExtSide->Side_);

		CurrExtSide = CurrExtSide->next;
	}

	y = boundRect.top();

	KisRandomSubAccessorPixel srcAcc = src->createRandomSubAccessor();
	while (TCA != NULL || y <= boundRect.bottom()) {
		KisHLineIteratorPixel pixels = dst->createHLineIterator(0, y, boundRect.right() + 2);

		//insert elements of TC(y) in TCA
		CurrSide = TC[y];
		while (CurrSide != NULL) {
			NextSide = CurrSide->next;
			InsertInSortedList(&TCA, CurrSide);
			CurrSide = NextSide;
		}
		TC[y] = NULL;

		//delete elements of TCA for which ymax=y
		if (TCA != NULL) {
			Senti->next = TCA;
			PrevSide = Senti;
			CurrSide = Senti->next;
			while (CurrSide != NULL) {
				NextSide = CurrSide->next;
				if (CurrSide->ymax == y)
					PrevSide->next = NextSide;
				else
					PrevSide = CurrSide;

				CurrSide = NextSide;
			}
			PrevSide->next = NULL;
			TCA = Senti->next;
		}

		if (TCA != NULL) {
			Senti->x_ymin = INT_MIN;
			Senti->invm.divisor = 0;
			//senti is first in TCA : it must be the smallest element
			Senti->next = TCA;
			PrevSide = Senti;
		}

		//fill scanline
		CurrSide = TCA;
		InsidePolygone = false;
		ChangeSideForNextPixel = false;
		pixels += boundRect.left();
		for (int j = boundRect.left(); j <= boundRect.right() + 1; ++j) {
			if (ChangeSideForNextPixel) {
				InsidePolygone = !InsidePolygone;
				ChangeSideForNextPixel = false;
			}

			while (CurrSide != NULL && CurrSide->x_ymin <= j) {
				if (CurrSide->invm.sign * CurrSide->e <= 0 || CurrSide->x_ymin == boundRect.right() + 1)
					InsidePolygone = !InsidePolygone;
				else
					ChangeSideForNextPixel = !ChangeSideForNextPixel;

				CurrSide->e += CurrSide->invm.dividend;
				while (2 * CurrSide->e > (CurrSide->invm.divisor)) {
					CurrSide->x_ymin += CurrSide->invm.sign;
					CurrSide->e -= CurrSide->invm.divisor;
				}

				if (*CurrSide < *PrevSide) {
					//we need to re-sort (partially) TCA
					NextSide = CurrSide->next;
					PrevSide->next = NextSide;
					InsertInSortedList(&TCA, CurrSide);
					CurrSide = NextSide;
				} else {
					PrevSide = CurrSide;
					CurrSide = CurrSide->next;
				}
			}

			if (InsidePolygone) {
				QPointF p = mTransform.map(QPointF(j, y));

				srcAcc.moveTo(p);
                srcAcc.sampledOldRawData(pixels.rawData());
			}

			++pixels;
		}

		++y;
	}

	delete Senti;
	FreeSidesList(TCA);
	FreeSidesTable(TC, 0, boundRect.bottom() + 1);
	FreeExtSides(&ExtSides);
}

void KisWarpTransformWorker::run()
{
	KisPaintDeviceSP srcdev = new KisPaintDevice(*m_dev.data());
	QRect srcBounds = srcdev->exactBounds();

	qint32 nbPoints = m_pointsPerLine * m_pointsPerColumn;

    QRectF origBRect(m_transfPoint[0], m_transfPoint[0]);
    QRectF transfBRect(m_transfPoint[0], m_transfPoint[0]);
	for (int i = 0; i < nbPoints; ++i) {
		if ( m_transfPoint[i].x() < transfBRect.left() )
			transfBRect.setLeft(m_transfPoint[i].x());
		else if ( m_transfPoint[i].x() > transfBRect.right() )
			transfBRect.setRight(m_transfPoint[i].x());
		if ( m_transfPoint[i].y() < transfBRect.top() )
			transfBRect.setTop(m_transfPoint[i].y());
		else if ( m_transfPoint[i].y() > transfBRect.bottom() )
			transfBRect.setBottom(m_transfPoint[i].y());
	}

	m_dev->clear();

	if (srcBounds.height() == 0)
		return;

	int pixelPrecision = 10;
	int verticesPerLine = srcBounds.width() / pixelPrecision + 2;

	QPoint *previousLineVertices = new QPoint[verticesPerLine]();
	QPoint *currentLineVertices = new QPoint[verticesPerLine]();
	Q_ASSERT(previousLineVertices != NULL && currentLineVertices != NULL);

	int i, prevX;
	int j, prevY;
	int k, prevK;
	double x, y;
	bool lineDone;
	bool imageDone;

	m_lastProgressReport = 0;
	m_progressStep = 0;
	m_progressTotalSteps = verticesPerLine * verticesPerLine;
	m_progress->setProgress(0);

	//begin the treatment
	//first line needs a special treatment
	i = 0;
	y = srcBounds.top();
	j = 0;
	x = srcBounds.left();
	k = 0;
	KisHLineConstIteratorPixel srcPix = srcdev->createHLineConstIterator(x, y, srcBounds.width());
	KisRandomSubAccessorPixel dstAcc = m_dev->createRandomSubAccessor();
	lineDone = false;
	while (!lineDone) {
		while (j < srcBounds.width()) {
			QPointF dstPosF = calcAffineTransformation(QPointF(x, y), nbPoints, m_origPoint, m_transfPoint, m_alpha);
			QPoint dstPos = QPoint(qRound(dstPosF.x()), qRound(dstPosF.y()));

			previousLineVertices[k] = dstPos;

			srcPix += pixelPrecision;
			j += pixelPrecision;
			x += pixelPrecision;
			++k;
		}

		//maybe we skipped the last pixels of the column (if the image width is not a multiple of pixelPrecision)
		if (j - pixelPrecision < srcBounds.width() - 1) {
			j = srcBounds.width() - 1;
			x = j + srcBounds.left();
			srcPix = srcdev->createHLineConstIterator(x, y, 1);
		} else
			lineDone = true;
	}

	//then the other lines
	imageDone = false;
	prevY = y;
	i += pixelPrecision;
	y += pixelPrecision;
	while (!imageDone) {
		while (i < srcBounds.height()) {
			srcPix = srcdev->createHLineConstIterator(x, y, srcBounds.width());
			x = srcBounds.left();

			//first column needs a special treatment
			QPointF dstPosF = calcAffineTransformation(QPointF(x, y), nbPoints, m_origPoint, m_transfPoint, m_alpha);
			QPoint dstPos = QPoint(qRound(dstPosF.x()), qRound(dstPosF.y()));

			currentLineVertices[0] = dstPos;

			srcPix += pixelPrecision;
			prevX = x;
			x += pixelPrecision;

			//the other columns
			j = pixelPrecision;
			k = 1;
			prevK = 0;
			lineDone = false;
			while (!lineDone) {
				while (j < srcBounds.width()) {
					dstPosF = calcAffineTransformation(QPointF(x, y), nbPoints, m_origPoint, m_transfPoint, m_alpha);
					QPoint dstPos = QPoint(qRound(dstPosF.x()), qRound(dstPosF.y()));

					currentLineVertices[k] = dstPos;

					QPolygon pSrc, pDst;
					pSrc << QPoint(prevX, prevY) << QPoint(x, prevY) << QPoint(x, y) << QPoint(prevX, y);
					pDst << previousLineVertices[prevK] << previousLineVertices[k]  << currentLineVertices[k]  << currentLineVertices[prevK];

					quadInterpolation(srcdev, m_dev, pSrc, pDst);

					srcPix += pixelPrecision;
					j += pixelPrecision;
					prevK = k;
					++k;
					prevX = x;
					x += pixelPrecision;

					m_progressStep += 1.;
					if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
						m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
						m_progress->setProgress(m_lastProgressReport);
					}
					if (m_progress->interrupted()) {
						break;
					}
				}

				if (j - pixelPrecision < srcBounds.width() - 1) {
					j = srcBounds.width() - 1;
					x = j + srcBounds.left();
					srcPix = srcdev->createHLineConstIterator(x, y, 1);
				} else
					lineDone = true;
			}

			switchVertices(&previousLineVertices, &currentLineVertices);
			
			prevY = y;
			i += pixelPrecision;
			y += pixelPrecision;
		}

		if (i - pixelPrecision < srcBounds.height() - 1) {
			i = srcBounds.height() - 1;
			y = i + srcBounds.top();
			srcPix = srcdev->createHLineConstIterator(srcBounds.left(), y, 1);
		} else
			imageDone = true;
	}

	delete previousLineVertices;
	delete currentLineVertices;
}
