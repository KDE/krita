/*
 *  kis_krayon.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __kis_krayon_h__
#define __kis_krayon_h__

#include <qsize.h>

#include <koIconChooser.h>

class QPoint;
class QPixmap;
class QImage;

class KisKrayon : public KoIconItem {
	typedef KoIconItem super;

public:
	KisKrayon();
	virtual ~KisKrayon();

	bool      isValidKrayon()	const { return m_validKrayon; }
	void      setValidKrayon(bool valid);  

	QPixmap&  pixmap()  const;
	QPixmap&  thumbPixmap() const;
	QImage*   image()   const;
	QSize     size()    const { return QSize(m_w, m_h); } 
	int       width()   const { return m_w; }
	int       height()  const { return m_h; }

protected:
	int       m_w, m_h;
	QImage   *m_pImage;
	QPixmap  *m_pPixmap;
	QPixmap  *m_pThumbPixmap;

private:
        bool      m_validKrayon;
};

#endif

