// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/* This file is part of the KDE project
Copyright 2011 Aurélien Gâteau <agateau@kde.org>
Copyright 2011 Paul Mendez <paulestebanms@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
#ifndef KOCONTEXTBARBUTTON_H
#define KOCONTEXTBARBUTTON_H

// KDE

// Qt
#include <QToolButton>
#include <QTimeLine>

class QTimeLine;
/**
 * A button with a special look, appears when hovering over thumbnails
 */
class KoContextBarButton : public QToolButton {
    Q_OBJECT
public:
    KoContextBarButton(const QString &iconName, QWidget* parent=0);
    ~KoContextBarButton();

public slots:
    void setFadingValue(int value);

protected:
    void paintEvent(QPaintEvent*);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);


private:
    /** Starts button fading animation */
    void startFading();

    /** Stops button fading animation */
    void stopFading();
    bool m_isHovered;
    bool m_leftMouseButtonPressed;
    int m_fadingValue;
    QTimeLine *m_fadingTimeLine;
};

#endif /* KOCONTEXTBARBUTTON_H */
