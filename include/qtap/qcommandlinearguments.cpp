//#include "stdafx.h"
#include "qcommandlinearguments.h"

//#include <qapplication.h>
#include <qfileinfo.h>
#include <qstack.h>
#include <stdlib.h>
#include <stdexcept>

/**
   \class GetOpt

   \brief A command line option parser.

   This class helps to overcome the repetitive, tedious and
   error-prone task of parsing the command line options passed to your
   application by the user. Specify the acceptable syntax with a
   minimum of statements in a readable way, check it against the
   actual arguments passed and find the retrieved values in variables
   of your program. The name \em GetOpt is based on similar utilities
   build into the Unix shell and other languages.

   A command line that a user might have entered is:

   \code
   app -v --config=my.cnf -Wall input.dat
   \endcode

   The typical usage has three stages:

   -# Construct a parser specifying what arguments to parse
   -# Set up the list of allowed and required options
   -# Run the parser

   For the first step there are three different constructors that
   either take arguments directly from \c main(), \c QApplication or a
   user specified list. Setting up the accepted syntax is done by a
   set of \c add functions like addSwitch(). The final step of running
   the parser is simply done by calling parse().

   A short example implementing a \c --verbose switch:

   \code
   int main(int argc, char **argv)
   {
       GetOpt opts(argc, argv);
       bool verbose;
       opts.addSwitch("verbose", &verbose);
       if (!opts.parse())
           return 1;
       if (verbose)
           cout << "VERBOSE mode on" << endl;
       ...
   \endcode

   For a better understanding of the function names we'll better
   define some terms used in the API and its documentation:

   - \em Argument An argument is a plain text token like e.g. a file
   name one typically passes to an editor when invoking it.
   - \em Switch A switch is an on/off kind of argument without the need
     of additional information. Example: \c --debug.
   - \em Option An option is a normally optional argument with a key-value
   syntax like \c --output=out.txt or \c -I/usr/include.
   - \em Short \em Option A short option is a one letter option with a
   preceding dash. Like \c -v.
   - \em Long \em Option A long option has a more verbose,
   multi-letter name like \c --debug.
   .

   \author froglogic GbR <contact@froglogic.com>
*/


/**
   Constructs a command line parser from the arguments stored in a
   previously created QApplication instance.

   Example usage:
   \code
   QApplication a(argc, argv);

   GetOpt opt;
   \endcode

   This constructor is probably the most convenient one to use in a
   regular Qt application. Note that QApplication may already have
   removed Qt (or X11) specific arguments. Also see
   QApplication::argv() and QApplication::argc().
 */

/**
   Construct a command line parser from the array \a argv of string
   pointers with the size \a argc. Those parameters have the form
   typically found in the \c main() function. That means that you can
   simply pass on the arguments specified by the user of your
   application.

   Example usage:

   \code
   int main(int argc, char **argv) {
       GetOpt opt(argc, argv);
       ...
   }
   \endcode
 */
QCommandLineArguments::QCommandLineArguments( int argc, char *argv[] )
{
    init( argc, argv );
}

/**
   Construct a command line parser from the arguments specified in the
   list of arguments \a a. This constructor is convenient in those
   cases where you want to parse a command line assembled on-the-fly
   instead of relying on the \c argc and \c arg parameters passed to
   the \c main() function.
 */
QCommandLineArguments::QCommandLineArguments( const QStringList &a )
    : args( a )
{
    init( 0, 0 );
}

/**
   \internal
*/
void QCommandLineArguments::init( int argc, char *argv[], int offset )
{
    numReqArgs = numOptArgs = 0;
    currArg = 1; // appname is not part of the arguments
    if ( argc ) {
	// application name
	aname = QFileInfo( QString::fromUtf8( argv[0] ) ).fileName();
	// arguments
	for ( int i = offset; i < argc; ++i )
	    args.append( QString::fromUtf8( argv[i] ) );
    }
}

/**
   \fn bool GetOpt::parse()

   Parse the command line arguments specified in the constructor under
   the conditions set by the various \c add*() functions. On success,
   the given variable reference will be initialized with their
   respective values and true will be returned. Returns false
   otherwise.

   In the future there'll be a way to retrieve an error message. In
   the current version the message will be printed to \c stderr.
*/

/**
   \internal
*/
bool QCommandLineArguments::parse( bool untilFirstSwitchOnly )
{
    //    qDebug( "parse(%s)", args.join( QString( "," ) ).ascii() );
    // push all arguments as we got them on a stack
    // more pushes might following when parsing condensed arguments
    // like --key=value.
    QStack<QString> stack;
    //{
	//QStringList::const_iterator it = args. -.fromLast();
	//const QStringList::const_iterator end = args.end();
	//while ( it != end ) {
	//    stack.push( *it );
	//    --it;
	//}
    //}
	for (int i=args.count()-1;i>=0;i--) {
		stack.push(args.at(i));
	}
	
    int reqArgFound=0;
    int optArgFound=0;
    
    const OptionConstIterator obegin = options.begin();
    const OptionConstIterator oend = options.end();
    enum { StartState, ExpectingState, OptionalState } state = StartState;
    Option currOpt;
    enum TokenType { LongOpt, ShortOpt, Arg, End } t, currType = End;
    bool extraLoop = true; // we'll do an extra round. fake an End argument
    while ( !stack.isEmpty() || extraLoop ) {
	QString a;
	QString origA;
	// identify argument type
	if ( !stack.isEmpty() ) {
	    a = stack.pop();
        ////qWarning("%s", (const char*)a.toLocal8Bit());
	    currArg++;
	    origA = a;
	    //	    qDebug( "popped %s", a.ascii() );
	    if ( a.startsWith( QString::fromLatin1( "--" ) ) ) {
            // recognized long option
            a = a.mid( 2 );
            if ( a.isEmpty() ) {
                qWarning( "'--' feature not supported, yet" );
                exit( 2 );
            }
            t = LongOpt;
            // split key=value style arguments
            int equal = a.indexOf( '=' );
            if ( equal > 0 ) {
                /////qWarning("got special format, %s", (const char*)a.toLocal8Bit());
                stack.push( a.mid( equal + 1 ) );
                currArg--;
                a = a.left( equal );

            }
	    } else if ( a.length() == 1 ) {
            t = Arg;
        } else if ( a[0] == '-' && state !=  ExpectingState ) {
#if 0 // compat mode for -long style options
            if ( a.length() == 2 ) {
                t = ShortOpt;
                a = a[1];
            } else {
                a = a.mid( 1 );
                t = LongOpt;
                // split key=value style arguments
                int equal = a.find( '=' );
                if ( equal >= 0 ) {
                    stack.push( a.mid( equal + 1 ) );
                    currArg--;
                    a = a.left( equal );
                }
            }
#else
            // short option
            t = ShortOpt;
            // followed by an argument ? push it for later processing.
            if ( a.length() > 2 ) {
                stack.push( a.mid( 2 ) );
                currArg--;
            }
            a = a[1];
#endif
	    } else {
            t = Arg;
	    }
	} else {
	    // faked closing argument
	    t = End;
	}
	// look up among known list of options
	Option opt;
	if ( t != End ) {
	    OptionConstIterator oit = obegin;
	    while ( oit != oend ) {
            const Option &o = *oit;
            if ( ( t == LongOpt && a == o.lname ) || // ### check state
                 ( t == ShortOpt && a[0].unicode() == o.sname ) ) {
                opt = o;
                break;
            }
            ++oit;
	    }
	    if ( t == LongOpt && opt.type == OUnknown ) {
            if ( currOpt.type != OVarLen ) {
                qWarning( "Unknown option --%s", (const char*) a.toLocal8Bit() );
                return false;
            } else {
                // VarLength options support arguments starting with '-'
                t = Arg;
            }
	    } else if ( t == ShortOpt && opt.type == OUnknown ) {
            if ( currOpt.type != OVarLen ) {
                qWarning( "Unknown option -%c", a[0].unicode() );
                return false;
            } else {
                // VarLength options support arguments starting with '-'
                t = Arg;
            }
	    }

	} else {
	    opt = Option( OEnd );
	}

	// interpret result
	switch ( state ) {
	case StartState:
	    if ( opt.type == OSwitch ) {
            setSwitch( opt );
            setOptions.insert( opt.lname, 1 );
            setOptions.insert( QString( QChar( opt.sname ) ), 1 );
	    } else if ( opt.type == OArg1 || opt.type == ODouble ||  opt.type == OInt ||  opt.type == ORepeat ) {
            state = ExpectingState;
            currOpt = opt;
            currType = t;
            setOptions.insert( opt.lname, 1 );
            setOptions.insert( QString( QChar( opt.sname ) ), 1 );
	    } else if ( opt.type == OOpt || opt.type == OVarLen ) {
            state = OptionalState;
            currOpt = opt;
            currType = t;
            setOptions.insert( opt.lname, 1 );
            setOptions.insert( QString( QChar( opt.sname ) ), 1 );
	    } else if ( opt.type == OEnd ) {
            // we're done
	    } else if ( opt.type == OUnknown && t == Arg ) {
            if ( reqArgFound < numReqArgs  ) {
                Option reqArg = reqArgs.at(reqArgFound++);
                if ( reqArg.stringValue->isNull() ) { // ###
                    *reqArg.stringValue = a;
                } else {
                    qWarning( "Too many arguments" );
                    return false;
                }
            } else if ( optArgFound < numOptArgs  ) {
                Option optArg = optArgs.at(reqArgFound++);
                if ( optArg.stringValue->isNull() ) { // ###
                    *optArg.stringValue = a;
                } else {
                    qWarning( "Too many arguments" );
                    return false;
                }
            }
	    } else {
            qFatal( "unhandled StartState case %d",  opt.type );
	    }
	    break;
	case ExpectingState:
	    if ( t == Arg ) {
            if ( currOpt.type == OArg1 ) {
                *currOpt.stringValue = a;
                state = StartState;
            } else if ( currOpt.type == OInt ) { // AP
                *currOpt.intValue = a.toInt();
                state = StartState;
            } else if ( currOpt.type == ODouble ) {// AP
                *currOpt.doubleValue = a.toDouble();
                state = StartState;
            } else
                if ( currOpt.type == ORepeat ) {
                    currOpt.listValue->append( a );
                    state = StartState;
                } else {
                    abort();
                }
	    } else {
            QString n = currType == LongOpt ?
                        currOpt.lname : QString( QChar( currOpt.sname ) );
            qWarning( "Expected an argument after '%s' option", (const char*)n.toLocal8Bit() );
            return false;
	    }
	    break;
	case OptionalState:
	    if ( t == Arg ) {
            if ( currOpt.type == OOpt ) {
                *currOpt.stringValue = a;
                state = StartState;
            } else if ( currOpt.type == OVarLen ) {
                currOpt.listValue->append( origA );
                // remain in this state
            } else {
                abort();
            }
	    } else {
            // optional argument not specified
            if ( currOpt.type == OOpt )
                *currOpt.stringValue = currOpt.def;
            if ( t != End ) {
                // re-evaluate current argument
                stack.push( origA );
                currArg--;
            }
            state = StartState;
	    }
	    break;
	}

	if ( untilFirstSwitchOnly && opt.type == OSwitch )
	    return true;

	// are we in the extra loop ? if so, flag the final end
	if ( t == End )
	    extraLoop = false;
    }

    if ( numReqArgs > reqArgFound ) {
    	qWarning( "Lacking required argument(s)" );
    	return false;
    }

    return true;
}

/**
   \internal
*/
void QCommandLineArguments::addOption( Option o )
{
    // ### check for conflicts
    foreach(Option o2, options) {    
        if (o.sname == o2.sname && o2.sname != 0) {
            qWarning( "Duplicate option sname '%c'!" , o.sname );
            throw std::runtime_error("Duplicate option sname!");
        }
    }		
    options.append( o );
}

/**
   Adds a switch with the long name \a lname. If the switch is found
   during parsing the bool \a *b will bet set to true. Otherwise the
   bool will be initialized to false.

   Example:

   \code
   GetOpt opt;
   bool verbose;
   opt.addSwitch("verbose", &verbose);
   \endcode

   The boolean flag \c verbose will be set to true if \c --verbose has
   been specified in the command line; false otherwise.
*/
void QCommandLineArguments::addSwitch( char s, const QString &lname, bool *b )
{
    Option opt( OSwitch, s, lname );
    opt.boolValue = b;
    addOption( opt );
    // ### could do all inits at the beginning of parse()
    *b = false;
}

/**
   \internal
*/
void QCommandLineArguments::setSwitch( const Option &o )
{
//    assert( o.type == OSwitch );
    *o.boolValue = true;
}

/**
   Registers an option with the short name \a s and long name \a l to
   the parser. If this option is found during parsing the value will
   be stored in the string pointed to by \a v. By default \a *v will
   be initialized to \c QString::null.
*/
void QCommandLineArguments::addOption( char s, const QString &l, QString *v )
{
    Option opt( OArg1, s, l );
    opt.stringValue = v;
    addOption( opt );
  //  *v = QString::null;
}

void QCommandLineArguments::addOption( char s, const QString &l, int *v )
{
    Option opt( OInt, s, l );
    opt.intValue = v;
    addOption( opt );
//    *v = 0;
}

void QCommandLineArguments::addOption( char s, const QString &l, double *v )
{
    Option opt( ODouble, s, l );
    opt.doubleValue = v;
    addOption( opt );
 //   *v = 0.0;
}

/**
   Registers a long option \a l that can have a variable number of
   corresponding value parameters. As there currently is no way to
   tell the end of the value list the only sensible use of this option
   is at the end of the command line.

   Example:

   \code
   QStringList args;
   opt.addVarLengthOption("exec", &args);
   \endcode

   Above code will lead to "-f" and "test.txt" being stored in \a args
   upon

   \code
   myapp --exec otherapp -f test.txt
   \endcode
 */
void QCommandLineArguments::addVarLengthOption( const QString &l, QStringList *v )
{
    Option opt( OVarLen, 0, l );
    opt.listValue = v;
    addOption( opt );
    *v = QStringList();
}

/**
   Registers an option with the short name \a s that can be specified
   repeatedly in the command line. The option values will be stored in
   the list pointed to by \a v. If no \a s option is found \a *v will
   remain at its default value of an empty QStringList instance.

   Example:

   To parse the \c -I options in a command line like
   \code
   myapp -I/usr/include -I/usr/local/include
   \endcode

   you can use code like this:

   \code
   GetOpt opt;
   QStringList includes;
   opt.addRepeatableOption('I', &includes);
   opt.parse();
   \endcode
 */
void QCommandLineArguments::addRepeatableOption( char s, QStringList *v )
{
    Option opt( ORepeat, s, QString::null );
    opt.listValue = v;
    addOption( opt );
    *v = QStringList();
}

/**
   Registers an option with the long name \a l that can be specified
   repeatedly in the command line.

   \sa addRepeatableOption( char, QStringList* )
 */
void QCommandLineArguments::addRepeatableOption( const QString &l, QStringList *v )
{
    Option opt( ORepeat, 0, l );
    opt.listValue = v;
    addOption( opt );
    *v = QStringList();
}

/**
   Adds a long option \a l that has an optional value parameter. If
   the value is not specified by the user it will be set to \a def.

   Example:

   \code
   GetOpt opt;
   QString file;
   opt.addOptionalOption("dump", &file, "<stdout>");
   \endcode

   \sa addOption
 */
void QCommandLineArguments::addOptionalOption( const QString &l, QString *v,
                                const QString &def )
{
    addOptionalOption( 0, l, v, def );
}

/**
   Adds a short option \a s that has an optional value parameter. If
   the value is not specified by the user it will be set to \a def.
 */
void QCommandLineArguments::addOptionalOption( char s, const QString &l,
				QString *v, const QString &def )
{
    Option opt( OOpt, s, l );
    opt.stringValue = v;
    opt.def = def;
    addOption( opt );
    *v = QString::null;
}

/**
   Registers a required command line argument \a name. If the argument
   is missing parse() will return false to indicate an error and \a *v
   will remain with its default QString::null value. Otherwise \a *v
   will be set to the value of the argument.

   Example:

   To accept simple arguments like

   \code
   myeditor letter.txt
   \endcode

   use a call like:

   \code
   QString &file;
   opt.addArgument("file", &file);
   \endcode

   Note: the \a name parameter has a rather descriptive meaning for
   now. It might be used for generating a usage or error message in
   the future. Right now, the only current use is in relation with the
   isSet() function.
 */
void QCommandLineArguments::addArgument( const QString &name, QString *v )
{
    Option opt( OUnknown, 0, name );
    opt.stringValue = v;
    //reqArg = opt;
    
    ++numReqArgs;
    *v = QString::null;
    
    reqArgs.append( opt );
}

/**
   Registers an optional command line argument \a name. For a more
   detailed description see the addArgument() documentation.

 */
void QCommandLineArguments::addOptionalArgument( const QString &name, QString *v )
{
    Option opt( OUnknown, 0, name );
    opt.stringValue = v;
    //optArg = opt;
    
    ++numOptArgs;
    *v = QString::null;
    
    optArgs.append( opt );
}

/**
   Returns true if the (long) option or switch \a name has been found
   in the command line; returns false otherwise. Leading hyphens are
   not part of the name.

   As the set/not set decision can also be made depending on the value
   of the variable reference used in the respective \c add*() call
   there's generally little use for this function.
*/

bool QCommandLineArguments::isSet( const QString &name ) const
{
    return setOptions.find( name ) != setOptions.end();
}


QString QCommandLineArguments::help()
{
	QString ret = "USAGE:";
	Option o;

	bool optinals=false;
    foreach(o, options) {
        if (o.type == OSwitch) {
            if (!optinals) {
                ret+=" [";
                optinals=true;
            } else {
                ret.append(" | ");
            }
            QString def;
            if (o.sname!=0) {
                def.append('-');
                def.append(o.sname);
            }
            if (!o.lname.isEmpty()) {
                if (!def.isEmpty()) def.append(", ");
                def.append("--");
                def.append(o.lname);
            }
            def.append(' ');
            ret += def;
        }
    }

    foreach(o, options) {
		if (o.type == OArg1 || o.type == OInt  || o.type == ODouble) {
			if (!optinals) {
				ret+=" [";
				optinals=true;
			} else {
				ret.append(" | ");
			}
			QString def;
			if (o.sname!=0) {
				def.append('-');
				def.append(o.sname);
			}
			if (!o.lname.isEmpty()) {
				if (!def.isEmpty()) def.append(", ");
				def.append("--");
				def.append(o.lname);
			}
			def.append(' ');
			if (o.type==OArg1) {
				def.append('<');
				def.append(o.lname);
				def.append('>');
			}
			if (o.type==OInt) {
				def.append('#');				
			}
			if (o.type==ODouble) {
				def.append("#.#");				
			}
			//def.append("\t");
			//def.append(o.description);
			

			ret += def;
		} 
	}

	if (optinals) ret+="]";


	if (numReqArgs) {		
        foreach(o, reqArgs) {    
            QString def;
            def.append(" ");
            def.append('<');
            def.append(o.lname);
            def.append('>');
            ret += def;
        }		
	}
	
	if (numOptArgs) {		
        foreach(o, optArgs) {    
            QString def;
            def.append(" ");
            def.append('[');
            def.append(o.lname);
            def.append(']');		
            ret += def;
        }
	}

	return ret;
}

	

QString QCommandLineArguments::list()
{
    QString ret = "";
    Option o;
    QStringList list;

    foreach(o, options) {


        QString def;

        if (!o.lname.isEmpty()) {


            def.append(o.lname);
            def.append(": ");
            switch (o.type) {
            case OInt:
                def.append(QString("%1").arg(*o.intValue));
                break;
            case ODouble:
                def.append(QString("%1").arg(*o.doubleValue));
                break;
            case OSwitch:
                def.append(*o.boolValue ? "true" : "false");
                break;
            case OArg1:
                def.append(QString("%1").arg(*o.stringValue));
                break;
            default:
                def.append("...");
                break;
            }


        }

        list.append(def);

    }

    qSort(list);
    ret  = list.join("\n");

    return ret;
}



