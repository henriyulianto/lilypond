/*
  line-spanner.cc -- implement Line_spanner

  source file of the GNU LilyPond music typesetter

  (c) 2000--2003 Jan Nieuwenhuizen <janneke@gnu.org>
*/

#include "molecule.hh"
#include "item.hh"
#include "spanner.hh"
#include "line-spanner.hh"
#include "paper-def.hh"
#include "paper-column.hh"
#include "staff-symbol-referencer.hh"
#include "font-interface.hh"
#include "warn.hh"

#include <math.h>


/*
  slightishly clumsy interface?

  Make  a Scheme expression for a line going from (0,0) to (dx,dy). 
 */

static SCM
line_atom (Grob* me, Real thick, Real dx, Real dy)
{
  SCM type = me->get_grob_property ("type");
  Real staff_space = Staff_symbol_referencer::staff_space (me);
  
      // maybe these should be in line-thickness?
  Real length = staff_space;
  SCM s = me->get_grob_property ("dash-length");
  if (gh_number_p (s))
    length = gh_scm2double (s) * staff_space;

  Real period = 2 * length + thick;
  s = me->get_grob_property ("dash-period");
  if (gh_number_p (s))
    period = gh_scm2double (s) * staff_space;
      
  if (type == ly_symbol2scm ("dotted-line"))
    length = thick;
	
  if (type == ly_symbol2scm ("line"))
    length = period + thick;

  Real on = length - thick;
  Real off = period - on;

  SCM list = scm_list_n (ly_symbol2scm ("dashed-line"),
		      gh_double2scm (thick),
		      gh_double2scm (on),
		      gh_double2scm (off),
		      gh_double2scm (dx),
		      gh_double2scm (dy),
		      SCM_UNDEFINED);

  return list;
}

static SCM
zigzag_atom (Grob* me, Real thick, Real dx, Real dy)
{
  Real staff_space = Staff_symbol_referencer::staff_space (me);
  SCM ws = me->get_grob_property ("zigzag-width");
  SCM ls = me->get_grob_property ("zigzag-length");
  double w = (gh_number_p(ws) ? gh_scm2double(ws) : 1)*staff_space;
  double l = (gh_number_p(ls) ? gh_scm2double(ls) : 1)*w;
  double h = l>w/2 ? sqrt(l*l-w*w/4) : 0;
  
  SCM list = scm_list_n (ly_symbol2scm ("zigzag-line"),
		      gh_bool2scm (true),
		      gh_double2scm (w),
		      gh_double2scm (h),
		      gh_double2scm (thick),
		      gh_double2scm (dx),
		      gh_double2scm (dy),
		      SCM_UNDEFINED);

  return list;
}



Molecule
Line_spanner::line_molecule (Grob* me, Real thick, Real dx, Real dy)
{
  Molecule mol;
  SCM type = me->get_grob_property ("type");
  if (gh_symbol_p (type)
      && (type == ly_symbol2scm ("line")
	  || type == ly_symbol2scm ("dashed-line")
	  || type == ly_symbol2scm ("dotted-line")
	  || (type == ly_symbol2scm ("trill") && dy != 0)))
    {
      Box b (Interval (-0.5* thick +  (0 <? dx) ,0.5* thick+ (0 >? dx)),
	     Interval (- 0.5* thick + (0<? dy), 0.5*thick + (0 >? dy)));
      mol = Molecule (b, line_atom (me, thick, dx, dy));
    }
  else if (gh_symbol_p (type)
	   && type == ly_symbol2scm ("zigzag"))
    {
      // TODO:
      Box b (Interval (-0.5* thick +  (0 <? dx) ,0.5* thick+ (0 >? dx)),
	     Interval (- 0.5* thick + (0<? dy), 0.5*thick + (0 >? dy)));
      mol = Molecule (b, zigzag_atom (me, thick, dx, dy));

    }
  else if (gh_symbol_p (type)
	   && type == ly_symbol2scm ("trill"))
    {
      SCM alist_chain = Font_interface::font_alist_chain (me);
      SCM style_chain = scm_list_n (gh_cons (ly_symbol2scm ("font-family"),
					  ly_symbol2scm ("music")),
				 SCM_UNDEFINED);
      
      Font_metric *fm = Font_interface::get_font (me,
						  scm_list_n (style_chain,
							   alist_chain,
							   SCM_UNDEFINED));
      Molecule m = fm->find_by_name ("scripts-trill-element");
      do
	mol.add_at_edge (X_AXIS, RIGHT, m, 0,0);
      while (m.extent (X_AXIS).length ()
	     && mol.extent (X_AXIS).length ()
	     + m.extent (X_AXIS).length () < dx);

      /*
	FIXME: should center element on x/y
       */
      mol.translate_axis (m.extent (X_AXIS).length () / 2, X_AXIS);
      mol.translate_axis (-(mol.extent (Y_AXIS)[DOWN]
			    + mol.extent (Y_AXIS).length ())/2, Y_AXIS); 
    }
  return mol;
}

Offset
Line_spanner::get_broken_offset (Grob *me, Direction dir)
{
  Spanner *spanner = dynamic_cast<Spanner*> (me);
  Item* bound = spanner->get_bound (dir);
  
  if (!bound->break_status_dir ())
    {
      Grob *common[] = {
	bound->common_refpoint (Staff_symbol_referencer::get_staff_symbol (me),
				X_AXIS),
	bound->common_refpoint (Staff_symbol_referencer::get_staff_symbol (me),
				Y_AXIS)
      };
  
      return Offset (abs (bound->extent (common[X_AXIS], X_AXIS)[-dir]),
		      bound->extent (common[Y_AXIS], Y_AXIS).center ());
    }
  return Offset ();
}

/* A broken line-spaner should maintain the same vertical trend
   the unbroken line-spanner would have had.
   From slur */
Offset
Line_spanner::broken_trend_offset (Grob *me, Direction dir)
{
  Offset o;
  
  if (Spanner *mother =  dynamic_cast<Spanner*> (me->original_))
    {
      int k = broken_spanner_index (dynamic_cast<Spanner*> (me));
      Grob *neighbour = mother->broken_intos_[k + dir];      
      Offset neighbour_o = get_broken_offset (neighbour, dir);
      Offset me_o = get_broken_offset (me, -dir);

      // Hmm, why not return me_o[X], but recalc in brew_mol?
      o = Offset (0,
		  (neighbour_o[Y_AXIS]*me_o[X_AXIS]
		   - me_o[Y_AXIS]*neighbour_o[X_AXIS]) * dir /
		  (me_o[X_AXIS] + neighbour_o[X_AXIS]));
    }
  return o;
}


/*
  Warning: this thing is a cross-staff object, so it should have empty Y-dimensions.

 (If not, you risk that this is called from the staff-alignment
  routine, via molecule_extent. At this point, the staves aren't
  separated yet, so it doesn't work cross-staff.

*/

MAKE_SCHEME_CALLBACK (Line_spanner, brew_molecule, 1);
SCM
Line_spanner::brew_molecule (SCM smob) 
{
  Grob *me= unsmob_grob (smob);

  Spanner *spanner = dynamic_cast<Spanner*> (me);
  Drul_array<Item*>  bound (spanner->get_bound (LEFT),
			    spanner->get_bound (RIGHT));
  
  Grob *common[] = { me, me };
  for (int a = X_AXIS;  a < NO_AXES; a++)
    {
      common[a] = me->common_refpoint (bound[RIGHT], Axis (a));
      common[a] = common[a]->common_refpoint (bound[LEFT], Axis (a));
      
      if (!common[a])
	{
	  programming_error ("No common point!");
	  return SCM_EOL;
	}
    }
  
  Real gap = gh_scm2double (me->get_grob_property ("gap"));

  Offset ofxy (gap, 0); /*offset from start point to start of line*/
  Offset dxy ;
  Offset my_off;
  Offset his_off;


  
  if (bound[LEFT]->break_status_dir () || bound[RIGHT]->break_status_dir ())
    /* across line break */
    {
      Direction broken = bound[LEFT]->break_status_dir () ? LEFT : RIGHT;

      dxy[X_AXIS] = bound[RIGHT]->extent (common[X_AXIS], X_AXIS)[LEFT]
      	- bound[LEFT]->extent (common[X_AXIS], X_AXIS)[RIGHT];
      
      dxy += broken_trend_offset (me, broken);
      dxy[X_AXIS] -= 1 * gap;

      my_off = Offset (0,
		       me->relative_coordinate (common[Y_AXIS], Y_AXIS));

      his_off = Offset (0, 
			bound[-broken]->relative_coordinate (common[Y_AXIS],
							     Y_AXIS));

      if (broken == LEFT)
	{
	  my_off[Y_AXIS] += dxy[Y_AXIS];
	}
    }
  else
    {
      Real off = gap + ((bound[LEFT]->extent (bound[LEFT], X_AXIS).length ()*3)/4); // distance from center to start of line

      for (int a = X_AXIS; a < NO_AXES; a++)
	{
	  Axis ax = (Axis)a;
	  dxy[ax] =
	    + bound[RIGHT]->extent (common[X_AXIS], ax).center ()
	    - bound[LEFT]->extent (common[X_AXIS], ax).center ();

	  my_off[ax] =me->relative_coordinate (common[a], ax);
	  his_off[ax] = bound[LEFT]->relative_coordinate (common[a], ax);
	  
	}

      ofxy = dxy * (off/dxy.length ());
      dxy -= 2*ofxy;
    }

  Real thick = me->get_paper ()->get_var ("linethickness");  

  SCM s = me->get_grob_property ("thickness");
  if (gh_number_p (s))
    thick *= gh_scm2double (s);

  
  Molecule line = line_molecule (me, thick, dxy[X_AXIS], dxy[Y_AXIS]);
  line.translate_axis (bound[LEFT]->extent (bound[LEFT],
					    X_AXIS).length ()/2, X_AXIS); 
  line.translate (ofxy - my_off + his_off);
  return line.smobbed_copy ();
}



ADD_INTERFACE (Line_spanner, "line-spanner-interface",
  "Generic line drawn between two objects, eg. for use with glissandi.\n"
"gap is measured in staff-spaces.\n"
"The property 'type is one of: line, dashed-line, trill, dotted-line or zigzag.\n"
"\n",
  "gap dash-period dash-length zigzag-width zigzag-length thickness type");


