/*   
  scm-option.cc --  implement option setting from Scheme
  
  source file of the GNU LilyPond music typesetter
  
  (c) 2001--2002  Han-Wen Nienhuys <hanwen@cs.uu.nl>
  
 */
#include <stdio.h>

#include "string.hh"
#include "lily-guile.hh"
#include "scm-option.hh"
#include "warn.hh"

/*
  This interface to option setting is meant for setting options are
  useful to a limited audience. The reason for this interface is that
  making command line options clutters up the command-line option name
  space.


  preferably, also dont use TESTING_LEVEL_GLOBAL, since it defeats
  another purpose of this very versatile interface, which is to
  support multiple debug/testing options concurrently.
  
 */


/* Write midi as formatted ascii stream? */
bool midi_debug_global_b;

/* General purpose testing flag */
int testing_level_global;

/*
  crash if internally the wrong type is used for a grob property.
 */
bool internal_type_checking_global_b;

LY_DEFINE (ly_option_usage, "ly:option-usage", 0, 0, 0, (SCM),
		  "Print ly-set-option usage")
{  
  printf ( _("lilypond -e EXPR means:").to_str0 ());
  puts ("");
  printf (_ ("  Evalute the Scheme EXPR before parsing any .ly files.").to_str0 ());
  puts ("");
  printf (_ ("  Multiple -e options may be given, they will be evaluated sequentially.").to_str0 ());
  puts ("");
  printf (_("  The function ly-set-option allows for access to some internal variables.").to_str0 ());
  puts ("\n");
  printf (_ ("Usage: lilypond -e \"(ly-set-option SYMBOL VAL)\"").to_str0 ());
  puts ("\n");
  printf (_ ("Where SYMBOL VAL pair is any of:").to_str0 ());
  puts ("");
  printf ( "  help ANY-SYMBOL\n"
	   "  internal-type-checking BOOLEAN\n"
	   "  midi-debug BOOLEAN\n"
	   "  testing-level INTEGER\n");
  
  exit (0);
  return SCM_UNSPECIFIED;
}

/* Add these as well:

@item -T,--no-timestamps
don't timestamp the output

@item -t,--test
Switch on any experimental features.  Not for general public use. */

LY_DEFINE (ly_set_option, "ly:set-option", 2, 0, 0, (SCM var, SCM val),
	    "Set a global option value.  Supported options include

@table @code
@item help
List all options.
@item midi-debug
If set to true, generate human readable MIDI
@item internal-type-checking
Set paranoia for property assignments 
@end table

This function is useful to call from the command line: @code{lilypond -e
\"(ly-set-option 'midi-debug #t)\"}.
")
{
  if (var == ly_symbol2scm ("help"))
    {
      /* lilypond -e "(ly-set-option 'help #t)" */
      ly_option_usage (SCM_EOL);
    }
  else if (var == ly_symbol2scm ("midi-debug"))
    {
      midi_debug_global_b = to_boolean (val);
    }
  else if (var == ly_symbol2scm ("testing-level"))
    {
     testing_level_global = gh_scm2int (val); 
    }
  else if (var == ly_symbol2scm ("internal-type-checking"))
    {
     internal_type_checking_global_b = to_boolean (val); 
    }
  else if (var == ly_symbol2scm ("find-old-relative"))
    {
      /*
	Seems to have been broken for some time!
	
	@item  -Q,--find-old-relative
	show all changes needed to convert a file to  relative octave syntax.


	
      */

      ;
    }
  else
    {
      warning (_("Unknown internal option!"));
    }
  

  return SCM_UNSPECIFIED;
}




