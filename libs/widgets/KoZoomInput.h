/* SPDX-FileCopyrightText: 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

        bool isFlat() const;
        void setFlat(bool flat);

        void setZoomLevels(const QStringList& levels);
        void setCurrentZoomLevel(const QString& level);
        void setCurrentZoomLevel(int index);

        bool eventFilter(QObject* watched, QEvent* event) override;

    Q_SIGNALS:
        void zoomLevelChanged(const QString& level);
        void zoomLevelChangedIndex(int index);
        void explicitZoomLevelRequested(const QString &level);

    protected:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *e) override;
#else
    void enterEvent(QEnterEvent *e) override;
#endif
        void leaveEvent(QEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;

    private:
        class Private;
        Private* const d;
};

#endif //KOZOOMINPUT_H
