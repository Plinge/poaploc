#ifndef QCOMMANDLINEARGUMENTS_H
#define QCOMMANDLINEARGUMENTS_H

#include <qstring.h>

#include <qstringlist.h>
#include <qmap.h>

#include <qlist.h>
/**
 * @par history 
 * - 2010-03-09 axel  added support for multiple unnnamed args
 * - 2009-09-09 axel  adapted to Qt4. added support for intenger and double parameters
 * - 2003, 2004  by froglogic Porten & Stadlbauer GbR
 * 
 *  @todo check for double -char assingment 
 */
class QCommandLineArguments {
public:
    QCommandLineArguments( int argc, char *argv[] );
    QCommandLineArguments( const QStringList &a );

	QString help();
    QString list();
    QString appName() const { return aname; }

    // switch (no arguments)
    void addSwitch( char s, const QString &lname, bool *b );

    // options (with arguments, sometimes optional)
    void addOption( char s, const QString &l, QString *v );
	void addOption( char s, const QString &l, int *v );
	void addOption( char s, const QString &l, double *v );
    void addVarLengthOption( const QString &l, QStringList *v );
    void addRepeatableOption( char s, QStringList *v );
    void addRepeatableOption( const QString &l, QStringList *v );
    void addOptionalOption( const QString &l, QString *v,
                                const QString &def );
    void addOptionalOption( char s, const QString &l,
				QString *v, const QString &def );

    // bare arguments
    void addArgument( const QString &name, QString *v );
    void addOptionalArgument( const QString &name, QString *v );

    bool parse( bool untilFirstSwitchOnly );
    bool parse() { return parse( false ); }

    bool isSet( const QString &name ) const;

    int currentArgument() const { return currArg; }

private:
    enum OptionType { OUnknown, OEnd, OSwitch, OInt, ODouble, OArg1, OOpt, ORepeat, OVarLen };

    struct Option;
    friend struct Option;

    struct Option {
        Option( OptionType t = OUnknown,
                char s = 0, const QString &l = QString::null )
            : type( t ),
              sname( s ),
              lname( l ),
              boolValue( 0 ) { }

        OptionType type;
        char sname;		// short option name (0 if none)
        QString lname;	// long option name  (null if none)
        union {
            bool *boolValue;
            QString *stringValue;
            QStringList *listValue;
			int *intValue;
			double *doubleValue;
        };
        QString def;
    };

    QList<Option> options;
    typedef QList<Option>::const_iterator OptionConstIterator;
    QMap<QString, int> setOptions;

    void init( int argc, char *argv[], int offset = 1 );
    void addOption( Option o );
    void setSwitch( const Option &o );

    QStringList args;
    QString aname;

    int numReqArgs;
    int numOptArgs;
    QList<Option> reqArgs;
    QList<Option> optArgs;
    //Option reqArg;
    //Option optArg;

    int currArg;
};

#endif

