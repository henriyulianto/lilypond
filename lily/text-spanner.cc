/*
  text-spanner.cc -- implement Text_spanner

  source file of the GNU LilyPond music typesetter

  (c) 2000--2003 Jan Nieuwenhuizen <janneke@gnu.org>

  Revised over good by Han-Wen. 
*/

#include "molecule.hh"
#include "text-item.hh"
#include "text-spanner.hh"
#include "line-spanner.hh"
#include "spanner.hh"
#include "font-interface.hh"
#include "dimensions.hh"
#include "paper-def.hh"
#include "warn.hh"
#include "paper-column.hh"
#include "staff-symbol-referencer.hh"

/*
  TODO:
  - vertical start / vertical end (fixme-name) |
  - contination types (vert. star, vert. end)  |-> eat volta-bracket
  - more styles
  - more texts/positions
*/

MAKE_SCHEME_CALLBACK (Text_spanner, brew_molecule, 1);

/*
  TODO: this function is too long


  TODO: the string for ottava shoudl depend on the available space, ie.

  
  Long: 15ma        Short: 15ma    Empty: 15
         8va                8va            8
         8va bassa          8ba            8


*/
SCM
Text_spanner::brew_molecule (SCM smob) 
{
  Grob *me= unsmob_grob (smob);
  Spanner *spanner = dynamic_cast<Spanner*> (me);
  
  /* Ugh, must be same as Hairpin::brew_molecule.  */
  Real padding = 0.0;
  SCM itp= me->get_grob_property ("if-text-padding");
  if (gh_number_p (itp))
    padding = gh_scm2double (itp);

  Grob *common = spanner->get_bound (LEFT)->common_refpoint (spanner->get_bound (RIGHT), X_AXIS);
  Paper_def * paper = me->get_paper();

  SCM flare = me->get_grob_property ("bracket-flare");
  SCM shorten = me->get_grob_property ("shorten-pair");

  Interval span_points;
  Drul_array<bool> broken;
  Direction d = LEFT;
  do
    {
      Item *b = spanner->get_bound (d);
      broken[d] = b->break_status_dir () != CENTER;

      if (broken[d])
	{
	  if (d == LEFT)
	    span_points[d] = spanner->get_broken_left_end_align ();
	  else
	    span_points[d] = b->relative_coordinate (common, X_AXIS);
	}
      else
	  {
	    bool encl = to_boolean (me->get_grob_property ("enclose-bounds"));
	    span_points[d] = b->extent (common, X_AXIS)[encl ? d : -d];

	    if (is_number_pair (shorten))
	      span_points -= d * gh_scm2double (index_get_cell (shorten, d));
	  }
      
      if (is_number_pair (flare))
	span_points -= d * gh_scm2double (index_get_cell (flare, d));
    }
  while (flip (&d) != LEFT);


  SCM properties = Font_interface::font_alist_chain (me);
  SCM edge_text = me->get_grob_property ("edge-text");
  Drul_array<Molecule> edge;
  if (gh_pair_p (edge_text))
    {
      Direction d = LEFT;
      do
	{
	  if (!to_boolean (me->get_grob_property ("text-repeat-if-broken"))
	      && broken[d])
	    continue;
	  
	  SCM text = index_get_cell (edge_text, d);

	  if (Text_item::markup_p (text)) 
	    edge[d] = *unsmob_molecule (Text_item::interpret_markup (paper->self_scm (), properties, text));
	  
	  if (!edge[d].is_empty ())
	    edge[d].align_to (Y_AXIS, CENTER);
	}
      while (flip (&d) != LEFT);
    }

  Real thick = paper->get_realvar (ly_symbol2scm ("linethickness"));  
  SCM st = me->get_grob_property ("thickness");
  if (gh_number_p (st))
    {
      thick *=  gh_scm2double (st);
    }
  
  Drul_array<Molecule> edge_line;
  SCM edge_height = me->get_grob_property ("edge-height");
  if (is_number_pair (edge_height))
    {
      Direction d = LEFT;
      int dir = to_dir (me->get_grob_property ("direction"));
      do
	{
	  if (broken[d])
	    continue;
	  
	  Real dx = 0.0;
	  if (gh_pair_p (flare))
	    dx = gh_scm2double (index_get_cell (flare, d)) * d;

	  Real dy = gh_scm2double (index_get_cell (edge_height, d)) * - dir;
	  if (dy)
	    edge_line[d] = Line_spanner::line_molecule (me, thick, Offset(0,0),
							Offset (dx, dy));
	}
      while (flip (&d) != LEFT);
    }
  
  Molecule m;
  do
    {
      Interval ext = edge[d].extent (X_AXIS);
      if (!ext.is_empty ())
	{
	  edge[d].translate_axis (span_points[d], X_AXIS);
	  m.add_molecule (edge[d]);
	  span_points[d] += -d *  ext[-d];
	}
    }
  while (flip (&d) != LEFT);
  do
    {
      if (d* span_points[d] > d * edge[-d].extent(X_AXIS)[d])
	{
	  edge_line[d].translate_axis (span_points[d], X_AXIS);
	  m.add_molecule (edge_line[d]);
	}
    }
  while (flip (&d) != LEFT);

  if (!span_points.is_empty ())
    {
      Molecule l =Line_spanner::line_molecule (me, thick,
					       Offset (span_points[LEFT], 0),
					       Offset (span_points[RIGHT], 0));
      m.add_molecule (l);
    }
  m.translate_axis (- me->relative_coordinate (common, X_AXIS), X_AXIS);
  return m.smobbed_copy ();
}




ADD_INTERFACE (Text_spanner,"text-spanner-interface",
	       "generic text spanner",
	       "text-repeat-if-broken dash-period if-text-padding dash-fraction edge-height bracket-flare edge-text shorten-pair style thickness enclose-bounds width-correct");

