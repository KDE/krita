/*
 *  kis_resourceserver.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kis_resourceserver_h__
#define __kis_resourceserver_h__

#include <qptrlist.h>
#include <qstring.h>

#include "kis_brush.h"
#include "kis_pattern.h"

class KisResourceServer
{

public:

  KisResourceServer();
  virtual ~KisResourceServer();

  int brushCount() { return m_brushes.count(); }
  int patternCount() { return m_patterns.count(); }

  QPtrList<KoIconItem> brushes() { return m_brushes; }
  QPtrList<KoIconItem> patterns() { return m_patterns; }

 protected:
  const KisBrush* loadBrush( const QString& filename );
  const KisPattern* loadPattern( const QString& filename );
 
 private:
  QPtrList<KoIconItem>  m_brushes;
  QPtrList<KoIconItem>  m_patterns;
};

#endif // __kis_resourceserver_h__
