#ifndef __koffice_kchart_h__
#define __koffice_kchart_h__

//#include <ktable.h>
#include <koDocument.h>
#include "../kchart/kdchart/KDChartTable.h"
#include "../kchart/kdchart/KDChartData.h"

namespace KoChart
{
    class Part;

    // KDChartData is either a double (interpreted as a value), a
    // QString (interpreted as a label), a QDateTime (interpreted as a
    // date/time value) or empty.
    typedef KDChartData Value;
    typedef KDChartTableData Data;

    class WizardExtension : public QObject
    {
        Q_OBJECT
    public:
        WizardExtension( Part *part, const char *name = 0 );
        virtual ~WizardExtension();

        Part *part() const { return m_part; }

        virtual void show() = 0;
        // XXX add more?

    private:
        Part *m_part;
        class WizardExtensionPrivate;
        WizardExtensionPrivate *d;
    };

    class Part : public KoDocument
    {
        Q_OBJECT
    public:
        Part( QWidget *parentWidget, const char *widgetName,
              QObject *parent, const char *name,
              bool singleViewMode = false );

        virtual ~Part();

        virtual void setData( const Data &d ) = 0;

        virtual WizardExtension *wizardExtension();
    private:
        class PartPrivate;
        PartPrivate *d;
    };
};

#endif
