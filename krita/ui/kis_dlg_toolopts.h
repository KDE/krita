/*
 *  kis_dlg_toolopts.h - part of Krayon
 *
 *  Copyright (c) 2001 John Califf
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __tooloptionsdialog_h__
#define __tooloptionsdialog_h__

#include <qcheckbox.h>
#include <qradiobutton.h>

#include <kdialogbase.h>
#include <knuminput.h>

class QVBoxLayout;

enum tooltype
{
	tt_linetool,
	tt_rectangletool,
	tt_ellipsetool,
	tt_polylinetool,
	tt_brushtool,
	tt_airbrushtool,
	tt_pentool,
	tt_erasertool,
	tt_filltool,
	tt_colorchangertool,
	tt_colorpickertool,
	tt_movetool,
	tt_stamptool,
	tt_pastetool,
	tt_polygontool
};

struct ToolOptsStruct
{
	int lineThickness;
	int lineOpacity;

	int penThreshold;
	int penOpacity;

	int opacity;

	int polygonCorners;
	int polygonSharpness;

	bool fillShapes;
	bool usePattern;
	bool useGradient;
	bool convexPolygon;
	bool concavePolygon;
};


class KisToolTab : public QWidget
{
	Q_OBJECT

		public:

	KisToolTab(ToolOptsStruct ts, QWidget *parent = 0, const char *name = 0 );

	int  thickness()const    { return mpThickness->value(); };
	int  opacity()const      { return mpOpacity->value(); };
	int  penThreshold()const { return mpPenThreshold->value(); };
	int  density()const      { return mpDensity->value(); };
	int  granularuty()const  { return mpGranularity->value(); };
	int  corners()const      { return mpCorners->value(); };
	int  sharpness()const    { return mpSharpness->value(); };

	bool solid()const        { return mpSolid->isChecked(); };
	bool usePattern()const   { return mpUsePattern->isChecked(); };
	bool useGradient()const  { return mpUseGradient->isChecked(); };
	bool blendPattern()const { return mpBlendPattern->isChecked(); };
	bool blendGradient()const{ return mpBlendGradient->isChecked(); };
	bool convexPolygon()const  { return mpConvexPolygon->isChecked(); };
	bool concavePolygon()const { return mpConcavePolygon->isChecked(); };

 protected:

	KIntNumInput  *mpThickness;
	KIntNumInput  *mpOpacity;
	KIntNumInput  *mpPenThreshold;
	KIntNumInput  *mpDensity;
	KIntNumInput  *mpGranularity;
	KIntNumInput  *mpCorners;
	KIntNumInput  *mpSharpness;

	QCheckBox *mpSolid;
	QCheckBox *mpUsePattern;
	QCheckBox *mpUseGradient;
	QCheckBox *mpBlendPattern;
	QCheckBox *mpBlendGradient;
	QRadioButton *mpConvexPolygon;
	QRadioButton *mpConcavePolygon;
};


class NullToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	NullToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		     const char *name = 0 );
};


class LineToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	LineToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		     const char *name = 0 );
};


class PenToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	PenToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		    const char *name = 0 );
};


class BrushToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	BrushToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		      const char *name = 0 );
};


class EraserToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	EraserToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		       const char *name = 0 );
};


class AirBrushToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	AirBrushToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
			 const char *name = 0 );
};


class FillToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	FillToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		     const char *name = 0 );
};


class StampToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	StampToolTab( ToolOptsStruct ts,  QWidget *parent = 0,
		      const char *name = 0 );
};

class PolygonPreview;
class PolygonToolTab : public KisToolTab
{
	Q_OBJECT

		public:

	PolygonToolTab( ToolOptsStruct ts,  QWidget *parent = 0,

			const char *name = 0 );
 private:
	PolygonPreview *preview;

	private slots:
		void slotConvexPolygon();
	void slotConcavePolygon();
};


class ToolOptionsDialog : public KDialogBase
{
	Q_OBJECT

		public:

	ToolOptionsDialog( tooltype tt, ToolOptsStruct ts,
			   QWidget *parent = 0, const char *name = 0 );

	NullToolTab *nullToolTab()const
		{ return pNullToolTab; }

	LineToolTab *lineToolTab()const
		{ return pLineToolTab; }

	PenToolTab  *penToolTab()const
		{ return pPenToolTab; }

	BrushToolTab *brushToolTab()const
		{ return pBrushToolTab; }

	AirBrushToolTab *airBrushToolTab()const
		{ return pAirBrushToolTab; }

	EraserToolTab *eraserToolTab()const
		{ return pEraserToolTab; }

	FillToolTab *fillToolTab()const
		{ return pFillToolTab; }

	StampToolTab *stampToolTab()const
		{ return pStampToolTab; }

	PolygonToolTab *polygonToolTab()const
		{ return pPolygonToolTab; }

 private:

	NullToolTab     *pNullToolTab;
	LineToolTab     *pLineToolTab;
	PenToolTab      *pPenToolTab;
	BrushToolTab    *pBrushToolTab;
	AirBrushToolTab *pAirBrushToolTab;
	EraserToolTab   *pEraserToolTab;
	FillToolTab     *pFillToolTab;
	StampToolTab    *pStampToolTab;
	PolygonToolTab  *pPolygonToolTab;
};

#endif // __tooloptionsdialog.h__
