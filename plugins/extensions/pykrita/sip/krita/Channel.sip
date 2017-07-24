class Channel : QObject
{
%TypeHeaderCode
#include "Channel.h"
%End
    Channel(const Channel & __0);
public:
    virtual ~Channel();
    bool operator==(const Channel &other) const;
    bool operator!=(const Channel &other) const;
    bool visible() const;
    void setVisible(bool value);
    QString name() const;
    int position() const;
    int channelSize() const;
    QRect bounds() const;
    QByteArray pixelData(const QRect &rect) const;
    void setPixelData(QByteArray value, const QRect &rect);
private:

};
