#ifndef KIS_STARTUP_DIALOG_H
#define KIS_STARTUP_DIALOG_H

#include <QDialog>

/**
 * @brief The KisStartupDialog class shows the file selectioed, custom document
 * widgets and template lists. A bit like it was in KOffice 1.4...
 */
class KisStartupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KisStartupDialog(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // KOSTARTUPDIALOG_H
