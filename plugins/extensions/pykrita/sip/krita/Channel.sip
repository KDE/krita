class Channel : QObject
{
%TypeHeaderCode
#include "Channel.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Channel passed to a Signal is provided as a QObject instead of
  // a Channel object
  // (not sure why, but explicit conversion here fix the problem)
  Channel* mc = qobject_cast<Channel*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Channel;
  }
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
