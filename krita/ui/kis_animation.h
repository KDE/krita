#ifndef KIS_ANIMATION_H
#define KIS_ANIMATION_H

#include <QStandardItemModel>
#include <krita_export.h>

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
    QString name() const;
    QString author() const;
    QString description() const;
    int fps() const;
    int time() const;
    void load(const QString &url);
    void save(const QString &url);

private:
    QString m_name;
    QString m_author;
    QString m_description;
    int m_fps;
    int m_time;
};

#endif // KIS_ANIMATION_H
