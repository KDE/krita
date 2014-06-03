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
#ifndef MUSIC_CORE_VOICEELEMENT_H
#define MUSIC_CORE_VOICEELEMENT_H

#include <QObject>

namespace MusicCore {

class Staff;
class VoiceBar;

/**
 * This is the base class for all musical elements that can be added to a voice.
 */
class VoiceElement : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new VoiceElement.
     */
    explicit VoiceElement(int length = 0);

    /**
     * Destructor.
     */
    virtual ~VoiceElement();

    /**
     * Returns the staff this music element should be displayed on. It can also be NULL, for example if the element
     * should not be visible.
     */
    Staff* staff() const;

    /**
     * Sets the staff this element should be displayed on.
     *
     * @param staff the new staff this element should be displayed on
     */
    void setStaff(Staff* staff);

    VoiceBar* voiceBar() const;
    void setVoiceBar(VoiceBar* voiceBar);

    /**
     * Returns the x position of this musical element. The x position of an element is measured relative to the left
     * barline of the bar the element is in.
     */
    virtual qreal x() const;

    /**
     * Returns the y position of this musical element. The y position of an element is measure relative to the top
     * of the staff it is in.
     */
    virtual qreal y() const;

    /**
     * Returns the width of this musical element.
     */
    virtual qreal width() const;

    /**
     * Returns the height of this musical element.
     */
    virtual qreal height() const;

    /**
     * Returns the duration of this musical elements in ticks.
     */
    int length() const;
    
    /**
     * Returns the beatline of this element. This is an x position relative to the start of the element.
     */
    virtual qreal beatline() const;
public slots:
    /**
     * Sets the x position of this musical element.
     */
    void setX(qreal x);

    /**
     * Sets the y position of this musical element.
     */
    void setY(qreal y);
protected slots:
    /**
     * Changes the duration of this musical element.
     *
     * @param length the new duration of this musical element
     */
    void setLength(int length);

    /**
     * Sets the width of this musical element.
     *
     * @param width the new width of this musical element
     */
    void setWidth(qreal width);

    /**
     * Sets the height of this musical element.
     *
     * @param height the new height of this musical element
     */
    void setHeight(qreal height);
    
    void setBeatline(qreal beatline);
signals:
    void xChanged(qreal x);
    void yChanged(qreal y);
    void lengthChanged(int length);
    void widthChanged(qreal width);
    void heightChanged(qreal height);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_VOICEELEMENT_H
