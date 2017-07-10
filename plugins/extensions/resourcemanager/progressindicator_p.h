#ifndef PROGRESSINDICATOR_P_H
#define PROGRESSINDICATOR_P_H

#include <QFrame>
#include <kpixmapsequence.h>

class QVBoxLayout;
class QLabel;
class QString;
class KPixmapSequenceWidget;

class ProgressIndicator : public QFrame
{
    Q_OBJECT
public:
    explicit ProgressIndicator(QWidget *parent);

public Q_SLOTS:
    void busy(const QString &message);
    void error(const QString &message);
    void idle(const QString &message);

private:
    QLabel *m_statusLabel;
    KPixmapSequenceWidget *busyWidget;

    KPixmapSequence *m_busyPixmap;
    KPixmapSequence *m_errorPixmap;

};

#endif // PROGRESSINDICATOR_P_H
