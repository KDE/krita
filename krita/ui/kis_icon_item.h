/*
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_ICON_ITEM_H_
#define KIS_ICON_ITEM_H_

#include <koIconChooser.h>
class KisResource;

class KisIconItem : public KoIconItem {
public:
	KisIconItem(KisResource *resource);
	virtual ~KisIconItem();

	virtual QPixmap& pixmap() const;
	virtual QPixmap& thumbPixmap() const;

	virtual int spacing() const;
	virtual void setSpacing(int spacing);

	virtual bool useColorAsMask() const;
	virtual void setUseColorAsMask(bool useColorAsMask);

	virtual bool hasColor() const;

	KisResource *resource() const;

protected:
	void updatePixmaps();
	QImage createColorMaskImage(QImage srcImage);

private:
	KisResource *m_resource;
	QPixmap m_pixmap;
	QPixmap m_thumb;
};

#endif // KIS_ICON_ITEM_H_

