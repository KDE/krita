class View : QObject
{
%TypeHeaderCode
#include "View.h"
%End
    View(const View & __0);
public:
    Window * window() const;
    void setWindow(Window*  value);
    Document * document() const;
    void setDocument(Document*  value);
    bool visible() const;
    void setVisible(bool value);
    Canvas * canvas() const;
    void setCanvas(Canvas*  value);
public Q_SLOTS:
    void close(bool confirm);
private:
};
