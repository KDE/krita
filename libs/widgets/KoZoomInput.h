/* Copyright 2008  Peter Simonsson <peter.simonsson@gmail.com>
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

#ifndef KOZOOMINPUT_H
#define KOZOOMINPUT_H

#include <QStackedWidget>

class KoZoomInput : public QStackedWidget
{
    Q_OBJECT
    public:
        explicit KoZoomInput(QWidget* parent = 0);
        ~KoZoomInput() override;

        void setZoomLevels(const QStringList& levels);
        void setCurrentZoomLevel(const QString& level);

        bool eventFilter(QObject* watched, QEvent* event) override;

    Q_SIGNALS:
        void zoomLevelChanged(const QString& level);

    protected:
        void enterEvent(QEvent* event) override;
        void leaveEvent(QEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;

    private:
        class Private;
        Private* const d;
};

#endif //KOZOOMINPUT_H
