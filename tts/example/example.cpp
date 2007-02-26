#include "example.h"
#include <qpushbutton.h>

// include for sending qcop messages
#include <qpe/qcopenvelope_qws.h>
#include <qcopchannel_qws.h>

/* 
 *  Constructs a Example which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
Example::Example( QWidget* parent,  const char* name, WFlags fl )
    : ExampleBase( parent, name, fl )
{
    listOfMsg << "something"
	      << "Windows isn't a virus, viruses do something."
              << "Frankly, I think it's a piece of crap. It contains all the design mistakes you can make, and manages to even make up a few of its own. -Linus Torvalds on Mac OS X"
	      << "...the number of UNIX installations has grown to 10, with more expected... -Dennis Ritchie and Ken Thompson, June 1972"
	      << "An algorithm must be seen to be believed. -D. E. Knuth"
	      << "ACK and you shall receive."
	      << "Hiroshima 45, Chernobyl 86, Windows 95"
	      << "I haven't lost my mind; it's backed up on tape somewhere."
	      << "A hacker does for love what others would not do for money."
	      << "You had mail, but the super-user read it, and deleted it!"
	      << "This web site best viewed using tcpdump."
	      << "You have zero privacy anyway. Get over it. -Scott McNealy, CEO, Sun Microsystems, Inc."
	      << "Got Root?"
	      << "Linux: Because rebooting is for adding new hardware."
	      << "Finish your mail packet! Children are offline in India."
	      << "I came, I saw, I deleted all your files."
	      << "Do you like me for my brain or my baud?"
	      << "Hacking is not magic."
	      << "All computers run at the same speed...with the power off."
	      << "Do the users of AOL realize AOL runs on a UNIX system? Probably not."
	      << "It said, Insert disk #3 but only two will fit!"
	      << "Software is like sex. It's better when it's free. -Linus Torvalds"
	      << "Gimme root, gimme fire, gimme that which I desire."
	      << "Those who can't write, write help files."
	      << "Ever notice how fast Windows runs? Neither did I..."
	      << "No program done by a hacker will work unless he is on the system."
	      << "This is Linux country. If you listen carefully, you can hear Windows reboot..."
	      << "Unix is not necessarily evil, like OS/2. -Peter Norton"
	      << "Unix is user friendly - it's just picky about its friends."
	      << "Asking whether machines can think is like asking whether submarines can swim."
	      << "Linux...because I'm better than you."
	      << "Hackers have kernel knowledge."
	      << "Microsoft is not the answer. Microsoft is the question. No  is the answer!"
	      << "Regarding security, WindowsNT is an OS with a  Kick me  sign stuck on it's back."
	      << "My computer's sick. I think my modem is a carrier."
	      << "The software said Windows95 or better, so I got a Mac..."
	      << "You never finish a program, you just stop working on it."
	      << "Unix is simple, but it takes a genius to understand the simplicity. -Dennis Ritchie"
	      << "Do files get embarrassed when they get unzipped?"
	      << "VMS is a text-only adventure game. If you win you can use Unix.  -W. Davidson"
	      << "I write all my critical routines in assembler, and my comedy routines in FORTRAN."
	      << "When you open Windows...BUGS GET IN!!!"
	      << "Programmers are tools for converting caffeine into code."
	      << "USER ERROR: replace user and press any key to continue."
	      << "Programming is like sex: one mistake and you're providing support for a lifetime. -Michael Sinz";
    pos = 0;
    connect(say, SIGNAL(clicked()), this, SLOT(saySomething()));
    if(!QCopChannel::isRegistered("QPE/Tts"))
       qDebug("QCopChannel QPE/Tts is not available");
}

/*  
 *  Destroys the object and frees any allocated resources
 */
Example::~Example()
{
    // no need to delete child widgets, Qt does it all for us
}


void Example::saySomething()
{
    QString anyString;

    anyString = listOfMsg[pos];
    if(pos < listOfMsg.count()-1)
        pos++;
    else
        pos = 0;

    qDebug(anyString);

    // this 2 lines do all interesting....
    QCopEnvelope e("QPE/Tts","sayText(QString)");
      e << anyString;
}
