/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ANIMATION_H
#define KIS_ANIMATION_H

#include <QStandardItemModel>
#include <krita_export.h>
#include <KoColorSpace.h>
#include <KoColor.h>

class QString;

class KRITAUI_EXPORT KisAnimation : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit KisAnimation(QObject *parent = 0);

    void setName(const QString &name);
    void setAuthor(const QString &author);
    void setDescription(const QString &description);
    void setFps(int fps);
    void setTime(int time);
    void setColorSpace(const KoColorSpace *colorSpace);
    void setHeight(qint32 h);
    void setWidth(qint32 w);
    void setResolution(double res);
    void setBgColor(KoColor bgColor);

    QString name() const;
    QString author() const;
    QString description() const;
    const KoColorSpace* colorSpace();
    int fps() const;
    int time() const;
    qint32 width() const;
    qint32 height() const;
    double resolution() const;
    KoColor bgColor() const;

    void load(const QString &url);
    void save(const QString &url);

private:
    QString m_name;
    QString m_author;
    QString m_description;
    int m_fps;
    int m_time;
    const KoColorSpace* m_colorSpace;
    qint32 m_width, m_height;
    double m_resolution;
    KoColor m_bgColor;
};

#endif // KIS_ANIMATION_H
