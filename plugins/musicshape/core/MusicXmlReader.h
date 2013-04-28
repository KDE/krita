/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef MUSICXMLREADER_H
#define MUSICXMLREADER_H

#include <KoXmlReaderForward.h>
#include <QString>

namespace MusicCore {
    class Sheet;
    class Clef;
    class Staff;
    class TimeSignature;
    class Part;

class MusicXmlReader {
public:
    explicit MusicXmlReader(const char* musicNamespace = "http://www.calligra.org/music");
    
    Sheet* loadSheet(const KoXmlElement& scoreElement);
private:
    const char* m_namespace;
    
    QString getProperty(const KoXmlElement& elem, const char *propName);
    Clef* loadClef(const KoXmlElement& element, Staff* staff);
    TimeSignature* loadTimeSignature(const KoXmlElement& element, Staff* staff);
    void loadPart(const KoXmlElement& partElement, Part* part);
    
    KoXmlElement namedItem( const KoXmlNode& node, const char* localName );
    bool checkNamespace(const KoXmlNode& node);
};

} // namespace MusicCore

#endif // MUSICXMLREADER_H
