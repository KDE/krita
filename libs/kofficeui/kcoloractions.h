/* This file is part of the KDE libraries
    Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
    Copyright (C) 2002 Werner Trobin <trobin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef kcoloractions_h
#define kcoloractions_h

#include <kaction.h>

/**
 * An action whose pixmap is automatically generated from a color.
 * It knows three types of pixmaps: text color, frame color and background color
 */
class KColorAction : public KAction
{
    Q_OBJECT

public:
    enum Type {
	TextColor,
	FrameColor,
	BackgroundColor
    };

    // Create default (text) color action
    KColorAction( const QString& text, int accel = 0, QObject* parent = 0, const char* name = 0 );
    KColorAction( const QString& text, int accel,
		  QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
    KColorAction( QObject* parent = 0, const char* name = 0 );

    // Create a color action of a given type
    KColorAction( const QString& text, Type type, int accel = 0,
		  QObject* parent = 0, const char* name = 0 );
    KColorAction( const QString& text, Type type, int accel,
		  QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );

    virtual void setColor( const QColor &c );
    QColor color() const;

    virtual void setType( Type type );
    Type type() const;

private:
    void init();
    void createPixmap();

    QColor col;
    Type typ;
};


class KSelectColorAction : public KAction
{
    Q_OBJECT
public:
    enum Type {
        TextColor,
        LineColor,
        FillColor
    };

    KSelectColorAction( const QString& text, Type type,
                        const QObject* receiver, const char* slot,
                        KActionCollection* parent, const char* name );
    virtual ~KSelectColorAction();

    virtual int plug( QWidget* w, int index = -1 );

    QColor color() const;
    Type type() const;

public slots:
    virtual void setColor( const QColor &c );
    virtual void setType( Type t );

signals:
    void colorSelected( const QColor& color );

private:
    QString whatsThisWithIcon() const; // duplicated, as it's private in kaction

    Type m_type;
    QColor m_color;
};

#endif
