#ifndef COMMAND_HH
#define COMMAND_HH

#include "glob.hh"
#include "vray.hh"
#include "scalar.hh"

enum Commandcode {
	NOP,
	INTERPRET,
	TYPESET,
	BREAK_PRE,BREAK_MIDDLE, BREAK_POST, BREAK_END
};

/// set a nonrythmical symbol
struct Command {
    Commandcode code;

    /// analogous to argv[]
    svec<Scalar> args;

    ///
    int priority;
    /** in what order relative to other TYPESET commands (eg, bar
       should precede meter). Highest priority first. */
    
    /****************/
    
    Command();
//    Command(Moment w);
    bool isbreak()const;
    void print() const;
};

/**
    A nonrhythmical "thing" in a  staff is called a "command".
    Commands have these properties:

    \begin{itemize}
    \item They are \bf{not} rhythmical, i.e. they do not have  a duration
    \item They have a staff-wide impact, i.e. a command cannot be targeted at
    only one voice in the staff: two voices sharing a staff can't have
    different clefs
    \item Commands are ordered, that is, when from musical point of view the
    commands happen simultaneously, the order in which Staff receives the
    commands can still make a difference in the output
    \item Some commands are actually score wide, so Score has to issue these
    commands to the Staff, eg. BREAK commands
    \end{itemize}

    At this moment we have three classes of commands:
    \begin{description}
    INTERPRET commands are not grouped.
    \item[TYPESET] These commands instruct the Staff to
    typeset symbols on the output, eg meter/clef/key changes
    \item[INTERPRET] These commands do not produce output, instead,
    they change the interpretation of other commands or requests. 
    example: shift output vertically, set the key.
    \item[BREAK_XXX] These commands group TYPESET commands in
    prebreak and postbreak commands. See Documentation/breaking    
    
    Staff can insert additional commands in a sequence of BREAK_XXX
    commands, eg. key change commands

    \end{description}
    
    These commands are generated by Score, since they have to be the
    same for the whole score.
    

    \begin{description}
    \item[BREAK_PRE]
    \item[BREAK_MIDDLE]
    \item[BREAK_POST]
    \item[BREAK_END]
    \item[TYPESET] METER,BAR
    \end{description}
 

    Commands can be freely copied, they do not have virtual methods.
    
    */


#endif
