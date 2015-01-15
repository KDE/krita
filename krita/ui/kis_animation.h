/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ANIMATION_H
#define KIS_ANIMATION_H

#include <QObject>

#include "krita_export.h"

#include <KoColorSpace.h>
#include <KoColor.h>

class QString;

class KRITAUI_EXPORT KisAnimation : public QObject
{
    Q_OBJECT
public:
    explicit KisAnimation(QObject *parent = 0);

    // Setters
    void setName(const QString &name);

    void setAuthor(const QString &author);

    void setDescription(const QString &description);

    void setFps(int fps);

    void setLocalPlaybackRange(int range);

    void setTime(int time);

    void setColorSpace(const KoColorSpace *colorSpace);

    void setHeight(qint32 h);

    void setWidth(qint32 w);

    void setResolution(double res);

    void setBgColor(KoColor bgColor);

    void setLocation(QString location);

    void enableFrameBreaking(bool enable);

    void enableOnionSkinning(bool enable);

    void enableLooping(bool enable);

    void setPrevOnionSkinOpacityValues(QList<int>* values);

    void setNextOnionSkinOpacityValues(QList<int>* values);

    void setNextOnionSkinColor(QColor color);

    void setPrevOnionSkinColor(QColor color);

    // Getters
    QString name() const;

    QString author() const;

    QString description() const;

    const KoColorSpace* colorSpace();

    int fps() const;

    int localPlaybackRange() const;

    int time() const;

    qint32 width() const;

    qint32 height() const;

    double resolution() const;

    KoColor bgColor() const;

    QString location();

    bool frameBreakingEnabled();

    bool onionSkinningEnabled();

    bool loopingEnabled();

    QList<int>* prevOnionSkinOpacityValues();

    QList<int>* nextOnionSkinOpacityValues();

    QColor prevOnionSkinColor();

    QColor nextOnionSkinColor();

private:
    QString m_name;
    QString m_author;
    QString m_description;
    int m_fps;
    int m_localPlaybackRange;
    int m_time;
    const KoColorSpace* m_colorSpace;
    qint32 m_width, m_height;
    double m_resolution;
    KoColor m_bgColor;
    QString m_location;
    bool m_frameBreakingEnabled;
    bool m_onionSkinningEnabled;
    bool m_loopingEnabled;
    QList<int>* m_prevOnionSkinOpacityValues;
    QList<int>* m_nextOnionSkinOpacityValues;
    QColor m_prevOnionSkinColor;
    QColor m_nextOnionSkinColor;
};

#endif // KIS_ANIMATION_H
