/*
Copyright 2008  Peter Simonsson <peter.simonsson@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KOZOOMINPUT_H
#define KOZOOMINPUT_H

#include <QStackedWidget>

class KoZoomInput : public QStackedWidget
{
    Q_OBJECT
    public:
        explicit KoZoomInput(QWidget* parent = 0);
        ~KoZoomInput();

        void setZoomLevels(const QStringList& levels);
        void setCurrentZoomLevel(const QString& level);

        virtual bool eventFilter(QObject* watched, QEvent* event);

    signals:
        void zoomLevelChanged(const QString& level);

    protected:
        void enterEvent(QEvent* event);
        void leaveEvent(QEvent* event);
        void keyPressEvent(QKeyEvent* event);

    private:
        class Private;
        Private* const d;
};

#endif //KOZOOMINPUT_H
