/*
  slur-quanting.cc -- Score based slur formatting

  source file of the GNU LilyPond music typesetter

  (c) 1996--2004 Han-Wen Nienhuys <hanwen@cs.uu.nl>
  Jan Nieuwenhuizen <janneke@gnu.org>
*/

#include <math.h>

#include "accidental-interface.hh"
#include "beam.hh"
#include "directional-element-interface.hh"
#include "group-interface.hh"
#include "libc-extension.hh"
#include "lily-guile.hh"
#include "new-slur.hh"
#include "note-column.hh"
#include "output-def.hh"
#include "pitch.hh"
#include "slur-bezier-bow.hh"
#include "spanner.hh"
#include "staff-symbol-referencer.hh"
#include "staff-symbol.hh"
#include "stem.hh"
#include "warn.hh"
#include "paper-column.hh"


struct Slur_score
{
  Drul_array<Offset> attachment_;
  Real score_;
  Bezier curve_;

#if DEBUG_SLUR_QUANTING
  String score_card_;
#endif

  Slur_score()
  {
    score_ = 0.0;
  }
};

/*
  TODO: put in details property.,

  use lowercase.
*/
struct Slur_score_parameters
{
  int slur_region_size_;
  Real head_encompass_penalty_;
  Real stem_encompass_penalty_;
  Real closeness_factor_;
  Real edge_attraction_factor_;
  Real same_slope_penalty_;
  Real steeper_slope_factor_;
  Real non_horizontal_penalty_;
  Real head_strict_free_space_;
  Real max_slope_;
  Real max_slope_factor_;
  Real extra_object_collision_;
  Real accidental_collision_;
  Real free_head_distance_;
  Real extra_encompass_free_distance_;
  Slur_score_parameters (Grob*);
};

/*
  TODO:

  - curve around flag for y coordinate
  - better scoring.
  - short-cut: try a smaller region first.
  - collisions with accidentals
  - collisions with articulations (staccato, portato, sforzato, ...)
  - handle non-visible stems better.
*/
struct Encompass_info
{
  Real x_;
  Real stem_;
  Real head_;
  Encompass_info ()
  {
    x_ = 0.0;
    stem_ = 0.0;
    head_ = 0.0;
  }
};

struct Bound_info
{
  Box stem_extent_;
  Direction stem_dir_;
  Item *bound_;
  Grob *note_column_;
  Grob *slur_head_;
  Grob *staff_;
  Grob *stem_;
  Interval slur_head_extent_;
  Real neighbor_y_;
  Real staff_space_;

  Bound_info ()
  {
    stem_ = 0;
    neighbor_y_ = 0;
    staff_ = 0;
    slur_head_ = 0;
    stem_dir_ = CENTER;
    note_column_ = 0;
  }
};




static void
score_extra_encompass (Grob *me, Grob *common[],
		       Slur_score_parameters *score_param,
		       Drul_array<Bound_info> ,
		       Drul_array<Offset> ,
		       Array<Slur_score> * scores);
static void score_slopes (Grob *me, Grob *common[],
			  Slur_score_parameters *score_param,
			  Drul_array<Bound_info>,
			  Drul_array<Offset> base_attach,
			  Array<Slur_score> *scores);

static void score_edges (Grob *me, Grob *common[],
			 Slur_score_parameters *score_param,
			 Drul_array<Bound_info> extremes,
			 Drul_array<Offset> base_attach,
			 Array<Slur_score> *scores);
static void score_encompass (Grob *me, Grob *common[],
			     Slur_score_parameters*,
			     Drul_array<Bound_info>,
			     Drul_array<Offset>, Array<Slur_score> *scores);
static Bezier avoid_staff_line (Grob *me, Grob **common,
				Drul_array<Bound_info> extremes,
				Bezier bez);

static Encompass_info get_encompass_info (Grob *me,
					  Grob *col,
					  Grob **common);
static Bezier get_bezier (Grob *me, Drul_array<Offset>, Real, Real);
static Direction get_default_dir (Grob *me);

static void set_end_points (Grob *);
static Real broken_trend_y (Grob *me, Grob **, Direction dir);
static Drul_array<Bound_info> get_bound_info (Spanner *me, Grob **common);

static void generate_curves (Grob *me,
			     Grob *common[],
			     Drul_array<Bound_info> extremes,
			     Drul_array<Offset> base_attach,
			     Array<Slur_score> *scores);
static Array<Slur_score> enumerate_attachments
(Grob *me, Grob *common[], Slur_score_parameters*,
 Drul_array<Bound_info> extremes,
 Drul_array<Offset> base_attachment, Drul_array<Real> end_ys);
static Drul_array<Offset> get_base_attachments
(Spanner *sp, Grob **common, Drul_array<Bound_info> extremes);
static Drul_array<Real> get_y_attachment_range
(Spanner *sp, Grob **common, Drul_array<Bound_info> extremes,
 Drul_array<Offset> base_attachment);


Real
get_detail (SCM alist, SCM sym)
{
  SCM entry = scm_assq (sym, alist);
  return robust_scm2double (ly_c_pair_p (entry)
			    ? ly_cdr (entry) 
			    : SCM_EOL,
			    0.0);
}

void
init_score_param (Grob *me,
                  Slur_score_parameters *score_param)
{
  SCM details = me->get_property ("slur-details");
  
  score_param->slur_region_size_ 
    = (int) get_detail (details, ly_symbol2scm ("slur-region-size"));
  score_param->head_encompass_penalty_ 
    = get_detail (details, ly_symbol2scm ("head-encompass-penalty"));
  score_param->stem_encompass_penalty_ 
    = get_detail (details, ly_symbol2scm ("stem-encompass-penalty"));
  score_param->closeness_factor_ 
    = get_detail (details, ly_symbol2scm ("closeness-factor"));
  score_param->edge_attraction_factor_ 
    = get_detail (details, ly_symbol2scm ("edge-attraction-factor"));
  score_param->same_slope_penalty_ 
    = get_detail (details, ly_symbol2scm ("same-slope-penalty"));
  score_param->steeper_slope_factor_ 
    = get_detail (details, ly_symbol2scm ("steeper-slope-factor"));
  score_param->non_horizontal_penalty_ 
    = get_detail (details, ly_symbol2scm ("non-horizontal-penalty"));
  score_param->head_strict_free_space_ 
    = get_detail (details, ly_symbol2scm ("head-strict-free-space"));
  score_param->max_slope_ 
    = get_detail (details, ly_symbol2scm ("max-slope"));
  score_param->max_slope_factor_ 
    = get_detail (details, ly_symbol2scm ("max-slope-factor"));
  score_param->free_head_distance_ 
    = get_detail (details, ly_symbol2scm ("free-head-distance"));
  score_param->extra_object_collision_ 
    = get_detail (details, ly_symbol2scm ("extra-object-collision"));
  score_param->accidental_collision_ 
    = get_detail (details, ly_symbol2scm ("accidental-collision"));
  score_param->extra_encompass_free_distance_ 
    = get_detail (details, ly_symbol2scm ("extra-encompass-free-distance"));
}


Slur_score_parameters::Slur_score_parameters(Grob *me)
{
  init_score_param (me, this);
}

/* HDIR indicates the direction for the slur.  */
Real
broken_trend_y (Grob *me, Grob **common, Direction hdir)
{
  /* A broken slur should maintain the same vertical trend
     the unbroken slur would have had.  */
  Real by = 0.0;
  if (Spanner *mother = dynamic_cast<Spanner*> (me->original_))
    {
      int k = broken_spanner_index (dynamic_cast<Spanner*> (me));
      int j = k + hdir;
      if (j < 0 || j >= mother->broken_intos_.size ())
	return by;

      Grob *neighbor = mother->broken_intos_[j];
      if (hdir == RIGHT)
	neighbor->set_property ("direction",
				me->get_property ("direction"));

      Spanner *common_mother
	= dynamic_cast<Spanner*> (common[Y_AXIS]->original_);
      int common_k
	= broken_spanner_index (dynamic_cast<Spanner*> (common[Y_AXIS]));
      int common_j = common_k + hdir;

      if (common_j < 0 || common_j >= common_mother->broken_intos_.size())
	return by;

      Grob *common_next_system = common_mother->broken_intos_[common_j];
      Link_array<Grob> neighbor_cols
	= Pointer_group_interface__extract_grobs (neighbor, (Grob *)0,
						  "note-columns");

      Grob *neighbor_col
	= (hdir == RIGHT) ? neighbor_cols[0] : neighbor_cols.top ();
      Grob *neighbor_common
	= common_next_system->common_refpoint (neighbor_col, Y_AXIS);

      Direction vdir = get_grob_direction (me);
      Real neighbor_y
	= neighbor_col->extent (neighbor_common, Y_AXIS)
	.linear_combination (int(neighbor_cols.size()==1 ? CENTER : vdir))
	- common_next_system->relative_coordinate (neighbor_common, Y_AXIS);

      Link_array<Grob> my_cols
	= Pointer_group_interface__extract_grobs (me, (Grob *)0,
						  "note-columns");

      Grob *extreme_col = (hdir == RIGHT) ? my_cols.top() : my_cols[0];
      Real y = extreme_col->extent (common[Y_AXIS], Y_AXIS)
	.linear_combination (int ((my_cols.size() == 1) ? CENTER : vdir));
      by = (y*neighbor_cols.size() + neighbor_y*my_cols.size()) /
	(my_cols.size() + neighbor_cols.size());
    }
  return by;
}

Encompass_info
get_encompass_info (Grob *me,
		    Grob *col,
		    Grob **common)
{
  Grob *stem = unsmob_grob (col->get_property ("stem"));
  Encompass_info ei;
  Direction dir = get_grob_direction (me);

  if (!stem)
    {
      programming_error ("No stem for note column?");
      ei.x_ = col->relative_coordinate (common[X_AXIS], X_AXIS);
      ei.head_ = ei.stem_ = col->extent (common[Y_AXIS],
					 Y_AXIS)[get_grob_direction (me)];
      return ei;
    }
  Direction stem_dir = get_grob_direction (stem);

  if (Grob *head = Note_column::first_head (col))
    ei.x_ = head->extent (common[X_AXIS], X_AXIS).center ();
  else
    ei.x_ = col->extent (common[X_AXIS], X_AXIS).center ();

  Grob *h = Stem::extremal_heads (stem)[Direction (dir)];
  if (!h)
    {
      ei.head_ = ei.stem_ = col->extent (common[Y_AXIS], Y_AXIS)[dir];
      return ei;
    }

  ei.head_ = h->extent (common[Y_AXIS], Y_AXIS)[dir];

  if ((stem_dir == dir)
      && !stem->extent (stem, Y_AXIS).is_empty ())
    {
      ei.stem_ = stem->extent (common[Y_AXIS], Y_AXIS)[dir];
      if (Grob *b = Stem::get_beam (stem))
	ei.stem_ += stem_dir * 0.5 * Beam::get_thickness (b);

      Interval x = stem->extent (common[X_AXIS], X_AXIS);
      ei.x_ = x.is_empty ()
	? stem->relative_coordinate (common[X_AXIS], X_AXIS)
	: x.center ();
    }
  else
    ei.stem_ = ei.head_;

  return ei;
}


Direction
get_default_dir (Grob*me)
{
  Link_array<Grob> encompasses
    = Pointer_group_interface__extract_grobs (me, (Grob*) 0, "note-columns");

  Direction d = DOWN;
  for (int i= 0; i < encompasses.size (); i ++)
    {
      if (Note_column::dir (encompasses[i]) < 0)
	{
	  d = UP;
	  break;
	}
    }
  return d;
}

Direction
get_slur_dir (Grob *slur)
{
  Direction d = get_grob_direction (slur);
  if (d == CENTER)
    d = get_default_dir (slur);
  return d;
}

MAKE_SCHEME_CALLBACK (New_slur, after_line_breaking,1);
SCM
New_slur::after_line_breaking (SCM smob)
{
  Spanner *me = dynamic_cast<Spanner*> (unsmob_grob (smob));
  if (!scm_ilength (me->get_property ("note-columns")))
    {
      me->suicide ();
      return SCM_UNSPECIFIED;
    }

  if (!get_grob_direction (me))
    set_grob_direction (me, get_default_dir (me));

  set_end_points (me);

  return SCM_UNSPECIFIED;
}

Bezier
get_bezier (Grob *me, Drul_array<Offset> extremes, Real r_0, Real h_inf)
{
  Array<Offset> encompasses;
  encompasses.push (extremes[LEFT]);
  encompasses.push (extremes[RIGHT]);

  Slur_bezier_bow bb (encompasses,
		      get_grob_direction (me), h_inf, r_0);

  return bb.get_bezier ();
}

Drul_array<Bound_info>
get_bound_info (Spanner* me, Grob **common)
{
  Drul_array<Bound_info> extremes;
  Direction d = LEFT;
  Direction dir = get_grob_direction (me);

  do
    {
      extremes[d].bound_ = me->get_bound (d);
      if (Note_column::has_interface (extremes[d].bound_))
	{
	  extremes[d].note_column_ = extremes[d].bound_;
	  extremes[d].stem_ = Note_column::get_stem (extremes[d].note_column_);
	  extremes[d].stem_dir_ = get_grob_direction (extremes[d].stem_);
	  extremes[d].stem_extent_[X_AXIS]
	    = extremes[d].stem_->extent (common[X_AXIS], X_AXIS);
	  extremes[d].stem_extent_[Y_AXIS]
	    = extremes[d].stem_->extent (common[Y_AXIS], Y_AXIS);
	  extremes[d].slur_head_
	    = Stem::extremal_heads (extremes[d].stem_)[dir];
	  if (!extremes[d].slur_head_
	      && Note_column::has_rests (extremes[d].bound_))
	    {
	      extremes[d].slur_head_ = Note_column::get_rest (extremes[d].bound_);
	    }

	  if (extremes[d].slur_head_)
	    extremes[d].slur_head_extent_
	      = extremes[d].slur_head_->extent (common[X_AXIS], X_AXIS);

	  extremes[d].staff_ = Staff_symbol_referencer
	    ::get_staff_symbol (extremes[d].stem_);
	  extremes[d].staff_space_ = Staff_symbol_referencer
	    ::staff_space (extremes[d].stem_);
	}
      else
	extremes[d].neighbor_y_ = broken_trend_y (me, common, d);
    }
  while (flip (&d) != LEFT);
  return extremes;
}

void
set_end_points (Grob *me)
{
  Link_array<Grob> columns
    = Pointer_group_interface__extract_grobs (me, (Grob *) 0, "note-columns");

  if (columns.is_empty ())
    {
      me->suicide ();
      return;
    }

  SCM eltlist = me->get_property ("note-columns");
  SCM extra_list = me->get_property ("encompass-objects");
  Spanner *sp = dynamic_cast<Spanner*> (me);

  Grob *common[] = {0, 0};
  for (int i = X_AXIS; i < NO_AXES; i++)
    {
      Axis a = (Axis)i;
      common[a] = common_refpoint_of_list (eltlist, me, a);
      common[a] = common_refpoint_of_list (extra_list, common[a], a);
    }

  common[X_AXIS] = common[X_AXIS]->common_refpoint (sp->get_bound (RIGHT),
						    X_AXIS);
  common[X_AXIS] = common[X_AXIS]->common_refpoint (sp->get_bound (LEFT),
						    X_AXIS);

  Drul_array<Bound_info> extremes = get_bound_info (sp, common);
  Drul_array<Offset> base_attachment
    = get_base_attachments (sp, common, extremes);
  Drul_array<Real> end_ys
    = get_y_attachment_range (sp, common, extremes, base_attachment);
  Slur_score_parameters params (me);
  Array<Slur_score> scores = enumerate_attachments (me, common, &params,
						    extremes, base_attachment,
						    end_ys);

  generate_curves (me, common, extremes, base_attachment, &scores);
  score_edges (me, common, &params,extremes, base_attachment, &scores);
  score_slopes (me, common, &params,extremes, base_attachment, &scores);
  score_encompass (me, common, &params,extremes, base_attachment, &scores);
  score_extra_encompass (me, common, &params,extremes, base_attachment,
			 &scores);

  Real opt = 1e6;
  int opt_idx = 0;
  // why backwards?
  for (int i = scores.size (); i--;)
    {
      if (scores[i].score_ < opt)
	{
	  opt = scores[i].score_;
	  opt_idx = i;
	}
    }

#if DEBUG_SLUR_QUANTING
  SCM inspect_quants = me->get_property ("inspect-quants");
  if (to_boolean (me->get_paper ()
		  ->lookup_variable (ly_symbol2scm ("debug-slur-quanting")))
      && ly_c_pair_p (inspect_quants))
    {
      Drul_array<Real> ins = ly_scm2interval (inspect_quants);
      Real mindist = 1e6;
      for (int i = 0; i < scores.size (); i ++)
	{
	  Real d =fabs (scores[i].attachment_[LEFT][Y_AXIS] - ins[LEFT])
	    + fabs (scores[i].attachment_[RIGHT][Y_AXIS] - ins[RIGHT]);
	  if (d < mindist)
	    {
	      opt_idx = i;
	      mindist= d;
	    }
	}
      if (mindist > 1e5)
	programming_error ("Could not find quant.");
    }
  scores[opt_idx].score_card_ += to_string ("i%d", opt_idx);

  // debug quanting
  me->set_property ("quant-score",
		    scm_makfrom0str (scores[opt_idx].score_card_.to_str0 ()));
#endif

  Bezier b = scores[opt_idx].curve_;
  SCM controls = SCM_EOL;
  for (int i = 4; i--;)
    {
      Offset o = b.control_[i]
	- Offset (me->relative_coordinate (common[X_AXIS], X_AXIS),
		  me->relative_coordinate (common[Y_AXIS], Y_AXIS));
      controls = scm_cons (ly_offset2scm (o), controls);
    }
  me->set_property ("control-points", controls);
}


Drul_array<Real>
get_y_attachment_range (Spanner*me,
			Grob **common,
			Drul_array<Bound_info> extremes,
			Drul_array<Offset> base_attachment)
{
  Drul_array<Real> end_ys;
  Direction dir = get_grob_direction (me);
  Direction d = LEFT;
  do
    {
      if (extremes[d].note_column_)
	{
	  end_ys[d] = dir
	    * ((dir * (base_attachment[d][Y_AXIS] + 4.0 * dir))
	       >? (dir * (dir + extremes[d].note_column_->extent(common[Y_AXIS],
								 Y_AXIS)[dir]))
	       >? (dir * base_attachment[-d][Y_AXIS]));
	}
      else
	end_ys[d] = extremes[d].neighbor_y_ + 4.0 * dir;
    }
  while (flip (&d) != LEFT);

  return end_ys;
}

bool
spanner_less (Spanner *s1, Spanner* s2)
{
  Slice b1, b2;
  Direction d  = LEFT;
  do
    {
      b1[d] = s1->get_bound (d)->get_column ()->rank_;
      b2[d] = s2->get_bound (d)->get_column ()->rank_;
    } while (flip (&d) != LEFT);  

  return b2[LEFT] <= b1[LEFT] && b2[RIGHT] >= b1[RIGHT]
    && (b2[LEFT] != b1[LEFT] || b2[RIGHT] != b1[RIGHT]);
}


Drul_array<Offset>
get_base_attachments (Spanner *me,
		      Grob **common, Drul_array<Bound_info> extremes)
{
  Link_array<Grob> columns
    = Pointer_group_interface__extract_grobs (me, (Grob *)0, "note-columns");
  Drul_array<Offset> base_attachment;
  Real staff_space = Staff_symbol_referencer::staff_space ((Grob *) me);
  Direction dir = get_grob_direction (me);
  Direction d = LEFT;
  do
    {
      Grob *stem = extremes[d].stem_;
      Grob *head = extremes[d].slur_head_;

      Real x, y;
      if (!extremes[d].note_column_)
	{
	  y = extremes[d].neighbor_y_;
	  if (d== RIGHT)
	    x = extremes[d].bound_->extent (common[X_AXIS], X_AXIS)[d];
	  else
	    x = me->get_broken_left_end_align ();
	}
      else
	{
	  bool same_beam =
	    (extremes[d].stem_ && extremes[-d].stem_
	     && Stem::get_beam (extremes[d].stem_) == Stem::get_beam (extremes[-d].stem_));

	  /*
	    fixme: X coord should also be set in this case.
	   */
	  if (stem
	      && extremes[d].stem_dir_ == dir
	      && Stem::get_beaming (stem, -d)
	      && (!spanner_less (me, Stem::get_beam (stem))
		  || same_beam))
	    y = extremes[d].stem_extent_[Y_AXIS][dir];
	  else if (head)
	    y = head->extent (common[Y_AXIS], Y_AXIS)[dir];
	  y += dir * 0.5 * staff_space;

	  Real pos
	    = (y - extremes[d].staff_->relative_coordinate (common[Y_AXIS],
							    Y_AXIS))
	    * 2.0 / Staff_symbol::staff_space (extremes[d].staff_);

	  /* start off staffline. */
	  if (fabs (pos - my_round (pos)) < 0.2
	      && Staff_symbol_referencer::on_staffline (head, (int) rint (pos))
	      && Staff_symbol_referencer::line_count (head) - 1 >= rint (pos)
	      )
	    // TODO: calc from slur thick & line thick, parameter.	
	    y += 1.5 * staff_space * dir / 10;

	  Grob * fh = Note_column::first_head (extremes[d].note_column_);
	  x =
	    (fh ? fh->extent (common[X_AXIS], X_AXIS)
	     : extremes[d].bound_->extent (common[X_AXIS], X_AXIS))
	    .linear_combination (CENTER);
	}
      base_attachment[d] = Offset (x, y);

    } while (flip (&d) != LEFT);

  return base_attachment;
}

void
generate_curves (Grob *me, Grob **common,
		 Drul_array<Bound_info> extremes,
		 Drul_array<Offset>,
		 Array<Slur_score> *scores)
{
  (void) common;
  (void) extremes;
  Real staff_space = Staff_symbol_referencer::staff_space ((Grob *) me);
  Real r_0 = robust_scm2double (me->get_property ("ratio"), 0.33);
  Real h_inf = staff_space * ly_scm2double (me->get_property ("height-limit"));
  for (int i = 0; i < scores->size(); i++)
    {
      Bezier bez= get_bezier (me, (*scores)[i].attachment_, r_0, h_inf);
      bez = avoid_staff_line (me, common, extremes, bez);
      (*scores)[i].attachment_[LEFT] = bez.control_[0];
      (*scores)[i].attachment_[RIGHT] = bez.control_[3];
      (*scores)[i].curve_ = bez;
    }
}

Bezier
avoid_staff_line (Grob *me, Grob **common,
		  Drul_array<Bound_info> extremes,
		  Bezier bez)
{
  Offset horiz (1,0);
  Array<Real> ts = bez.solve_derivative (horiz);
  Real lt = me->get_paper ()->get_dimension (ly_symbol2scm ("linethickness"));
  Real thick = robust_scm2double (me->get_property ("thickness"), 1.0) *  lt;

  /* TODO: handle case of broken slur.  */
  if (!ts.is_empty ()
      && (extremes[LEFT].staff_ == extremes[RIGHT].staff_)
      && extremes[LEFT].staff_ && extremes[RIGHT].staff_)
    {
      Real y = bez.curve_point (ts[0])[Y_AXIS];

      Grob *staff = extremes[LEFT].staff_;

      Real staff_space = extremes[LEFT].staff_space_;
      Real p = 2 * (y - staff->relative_coordinate (common[Y_AXIS], Y_AXIS))
	/ staff_space;

      Real distance = fabs (my_round (p) - p);	//  in halfspaces
      if (distance < 4 * thick
	  && (int) fabs (my_round (p))
	  <= 2 * Staff_symbol_referencer::staff_radius (staff) + 0.1
	  && (int (fabs (my_round (p))) % 2
	      != Staff_symbol_referencer::line_count (staff) % 2))
	{
	  Direction resolution_dir =
	    (distance ?  get_grob_direction (me) : Direction (sign (p - my_round(p))));

	  // TODO: parameter
	  Real newp = my_round (p) + resolution_dir
	    * 5 * thick;
	
	  Real dy = (newp - p) * staff_space / 2.0;
#if 0
	  bez.translate (Offset (0, dy));
#else
	  bez.control_[1][Y_AXIS] += dy;
	  bez.control_[2][Y_AXIS] += dy;
	
#endif
	}
    }
  return bez;
}

Array<Slur_score>
enumerate_attachments (Grob *me, Grob *common[],
		       Slur_score_parameters *score_param,
		       Drul_array<Bound_info> extremes,
		       Drul_array<Offset> base_attachment,
		       Drul_array<Real> end_ys)
{
  (void) common;
  /*ugh.   */
  Array<Slur_score> scores;

  Direction dir = get_grob_direction (me);
  Real staff_space = Staff_symbol_referencer::staff_space ((Grob *) me);

  Drul_array<Offset> os;
  os[LEFT] = base_attachment[LEFT];
  Real minimum_length = staff_space
    * robust_scm2double (me->get_property ("minimum-length"), 2.0);

  for (int i = 0; dir * os[LEFT][Y_AXIS] <= dir * end_ys[LEFT]; i++)
    {
      os[RIGHT] = base_attachment[RIGHT];
      for (int j = 0; dir * os[RIGHT][Y_AXIS] <= dir * end_ys[RIGHT]; j++)
	{
	  Slur_score s;
	  Direction d = LEFT;
	  Drul_array<bool> attach_to_stem (false, false);
	  do
	    {
	      os[d][X_AXIS] = base_attachment[d][X_AXIS];
	      if (extremes[d].stem_
		  && !Stem::is_invisible (extremes[d].stem_)
		  && extremes[d].stem_dir_ == dir)
		{
		  if (dir == -d
		      && extremes[d].stem_extent_[Y_AXIS].contains (os[d][Y_AXIS]))
		    {
		      os[d][X_AXIS] =  extremes[d].slur_head_extent_[-d]
			- d * 0.3;
		      attach_to_stem[d] = true;
		    }
		  else if (dir *extremes[d].stem_extent_[Y_AXIS][dir]
			     < dir * os[d][Y_AXIS]
			   && !extremes[d].stem_extent_[X_AXIS].is_empty()
			   )
		    
		    os[d][X_AXIS] = extremes[d].stem_extent_[X_AXIS].center();
		}
	    }
	  while (flip (&d) != LEFT);

	  Offset dz;	
	  dz = os[RIGHT] - os[LEFT];
	  if (dz[X_AXIS] < minimum_length
	      || fabs (dz[Y_AXIS] / dz[X_AXIS]) > score_param->max_slope_
	      )
	    {
	      do
		{
		  if (extremes[d].slur_head_)
		    {
		      os[d][X_AXIS] = extremes[d].slur_head_extent_.center ();
		      attach_to_stem[d] = false;
		    }
		}
	      while (flip (&d) != LEFT);
	    }

	  dz = os[RIGHT] - os[LEFT];
	  do
	    {
	      if (extremes[d].slur_head_
		  && !attach_to_stem[d])
		{
		  /* Horizontally move tilted slurs a little.  Move
		     more for bigger tilts.
		    
		     TODO: parameter */
		  os[d][X_AXIS]
		    -= dir * extremes[d].slur_head_extent_.length ()
		    * sin (dz.arg  ()) / 3;
		}
	    }
	  while (flip (&d) != LEFT);
	  
	  s.attachment_ = os;
	  scores.push (s);
	  
	  os[RIGHT][Y_AXIS] += dir * staff_space / 2;
	}
      
      os[LEFT][Y_AXIS] += dir * staff_space / 2;
    }
  return scores;
}

void
score_encompass (Grob *me, Grob *common[],
		 Slur_score_parameters *score_param,
		 Drul_array<Bound_info> extremes,
		 Drul_array<Offset> base_attach,
		 Array<Slur_score> *scores)
{
  (void) extremes;
  (void) base_attach;

  Link_array<Grob> encompasses
    = Pointer_group_interface__extract_grobs (me, (Grob *)0, "note-columns");
  Direction dir = get_grob_direction (me);

  Array<Encompass_info> infos;

  for (int i = 0; i < encompasses.size(); i++)
    infos.push (get_encompass_info (me, encompasses[i], common));

  for (int i = 0; i < scores->size (); i++)
    {
      Bezier const &bez (scores->elem (i).curve_);
      Real demerit = 0.0;
      for (int j = 0; j < infos.size(); j++)
	{
	  Real x = infos[j].x_;

	  bool l_edge = j==0;
	  bool r_edge = j==infos.size()-1;
	  bool edge =  l_edge || r_edge;

	  if (!(x < scores->elem (i).attachment_[RIGHT][X_AXIS]
		&& x > scores->elem (i).attachment_[LEFT][X_AXIS]))
	    continue;
	
	  Real y = bez.get_other_coordinate (X_AXIS, x);
	  if (!edge)
	    {
	      Real head_dy = (y - infos[j].head_);
	      if (dir * head_dy < 0)
		demerit += score_param->head_encompass_penalty_;
	      else
		{
		  Real hd = (head_dy)
		    ? (1 / fabs (head_dy) - 1 / score_param->free_head_distance_)
		    : score_param->head_encompass_penalty_;
		  hd = (hd >? 0)<? score_param->head_encompass_penalty_;

		  demerit += hd;	
		}
	    }	

	  if (dir * (y - infos[j].stem_) < 0)
	    {
	      Real stem_dem =score_param->stem_encompass_penalty_ ;
	      if ((l_edge && dir == UP)
		  || (r_edge && dir == DOWN))
		stem_dem /= 5;

	      demerit +=  stem_dem;
	    }
	  else if (!edge)
	    {
	      Interval ext;
	      ext.add_point (infos[j].stem_);
	      ext.add_point (infos[j].head_);

	      // ?
	      demerit += -score_param->closeness_factor_
		* (dir
		   * (y - (ext[dir] + dir * score_param->free_head_distance_))
		   <? 0)
		/ infos.size ();
	    }
	}

#if DEBUG_SLUR_QUANTING
      (*scores)[i].score_card_ += to_string ("C%.2f", demerit);
#endif

      (*scores)[i].score_ += demerit;
    }
}

void
score_extra_encompass (Grob *me, Grob *common[],
		       Slur_score_parameters *score_param,
		       Drul_array<Bound_info> extremes,
		       Drul_array<Offset> base_attach,
		       Array<Slur_score> *scores)
{
  (void) base_attach;
  (void) extremes;

  Link_array<Grob> encompasses
    = Pointer_group_interface__extract_grobs (me, (Grob *)0,
					      "encompass-objects");
  Direction dir = get_grob_direction (me);
  Real lt =  me->get_paper ()->get_dimension (ly_symbol2scm ("linethickness"));
  Real thick = robust_scm2double (me->get_property ("thickness"), 1.0) * lt;

  Array<Real> xidxs;
  Array<Interval> yexts;
  Array<Interval> xexts;

  for (int i = encompasses.size (); i--; )
    {
      if (New_slur::has_interface (encompasses[i]))
	{
	  Grob * small_slur = encompasses[i];
	  Bezier b = New_slur::get_curve (small_slur);

	  Offset z = b.curve_point (0.5);
	  z += Offset (small_slur->relative_coordinate (common[X_AXIS], X_AXIS),
		       small_slur->relative_coordinate (common[Y_AXIS], Y_AXIS));

	  xexts.push (Interval (z[X_AXIS], z[X_AXIS]));
	  xidxs.push (0.0);
	  yexts.push (z[Y_AXIS] + thick * Interval (-0.5, 0.5));
	}
      else
	{
	  Grob *g = encompasses [i];
	  Interval xe = g->extent (common[X_AXIS], X_AXIS);
	  Interval ye = g->extent (common[Y_AXIS], Y_AXIS);

	  Real xp = 0.0;

	  if (Accidental_interface::has_interface (g))
	    {
	      /* Begin copy accidental.cc */
	      bool parens = false;
	      if (to_boolean (g->get_property ("cautionary")))
		{
		  SCM cstyle = g->get_property ("cautionary-style");
		  parens = ly_c_equal_p (cstyle, ly_symbol2scm ("parentheses"));
		}
	
	      SCM accs = g->get_property ("accidentals");
	      SCM scm_style = g->get_property ("style");
	      if (!ly_c_symbol_p (scm_style)
		  && !parens
		  && scm_ilength (accs) == 1)
		{
		  /* End copy accidental.cc */
		  switch (ly_scm2int (ly_car (accs)))
		    {
		    case FLAT:
		    case DOUBLE_FLAT:
		      xp = LEFT;
		      break ;
		    case SHARP:
		      xp = 0.5 * dir;
		      break ;
		    case NATURAL:
		      xp = -dir;
		      break; 
		    }
		}
	    }

	  xidxs.push (xp);
	  ye.widen (thick * 0.5);
	  yexts.push (ye);
	  xexts.push (xe);
	}
    }
  for (int i = 0; i < scores->size (); i++)
    {
      Real demerit = 0.0;
      for (int j = 0; j < xidxs.size(); j++)
	{
	  Drul_array<Offset> at = scores->elem (i).attachment_;
	  Interval slur_wid (at[LEFT][X_AXIS], at[RIGHT][X_AXIS]);

	  /*
	    to prevent numerical inaccuracies in
	    Bezier::get_other_coordinate().
	   */
	  slur_wid.widen (- 0.5 * thick);
	  Real x = xexts[j].linear_combination (xidxs[j]);
	  Real y = 0.0;
	  if (!slur_wid.contains (x))
	    {	
	      Direction contains_dir = CENTER;
	      Direction d = LEFT;
	      do
		{
		  if (xexts[j].contains (at[d][X_AXIS]))
		    contains_dir = d; 
		}
	      while (flip (&d) != LEFT);

	      if (!contains_dir)
		continue;
	      else
		y = at[contains_dir][Y_AXIS];
	    }
	  else
	    {
	      y = scores->elem (i).curve_.get_other_coordinate (X_AXIS, x);
	    }

	  Real collision_demerit = 
	       (Accidental_interface::has_interface (encompasses[j]))
	    ? score_param->accidental_collision_
	    : score_param->extra_object_collision_;
	  
	  Real dist = yexts[j].distance (y);
	  demerit +=
	    fabs (0 >? (score_param->extra_encompass_free_distance_ - dist)) /
	    score_param->extra_encompass_free_distance_  * collision_demerit;
	}
#if DEBUG_SLUR_QUANTING
      (*scores)[i].score_card_ += to_string ("X%.2f", demerit);
#endif
      (*scores)[i].score_ += demerit;
    }
}

void
score_edges (Grob *me, Grob *common[],
	     Slur_score_parameters * score_param,
	     Drul_array<Bound_info> extremes,
	     Drul_array<Offset> base_attach,
	     Array<Slur_score> *scores)
{
  (void) common;
  Direction dir = get_grob_direction (me);

  for (int i = 0; i < scores->size (); i++)
    {
      Direction d = LEFT;
      do
	{
	  Real y = scores->elem (i).attachment_[d][Y_AXIS];
	  Real dy = fabs (y - base_attach[d][Y_AXIS]);
	
	  Real factor = score_param->edge_attraction_factor_;
	  Real demerit = factor * dy;
	  if (extremes[d].stem_
	      && extremes[d].stem_dir_ == dir
	      && !Stem::get_beaming (extremes[d].stem_, -d)
	      )
	    demerit /= 5;
	
	  (*scores)[i].score_ += demerit;
#if DEBUG_SLUR_QUANTING
	  (*scores)[i].score_card_ += to_string ("E%.2f", demerit);
#endif
	}
      while (flip (&d) != LEFT);
    }
}

void
score_slopes (Grob *me, Grob *common[],
	      Slur_score_parameters *score_param,
	      Drul_array<Bound_info> extremes,
	      Drul_array<Offset> base_attach,
	      Array<Slur_score> * scores)
{
  (void) me;
  (void) base_attach;

  Drul_array<Real> ys;
  Direction d = LEFT;
  do
    {
      if (extremes[d].slur_head_)
	ys[d] = extremes[d].slur_head_->relative_coordinate (common[Y_AXIS],
							      Y_AXIS);
      else
	ys[d] = extremes[d].neighbor_y_;
    }
  while (flip (&d) != LEFT);

  bool has_beams
    = (extremes[LEFT].stem_ && Stem::get_beam (extremes[LEFT].stem_))
    || (extremes[RIGHT].stem_ && Stem::get_beam (extremes[RIGHT].stem_));

  Real dy = ys[RIGHT] - ys[LEFT];
  for (int i = 0; i < scores->size (); i++)
    {
      Offset slur_dz = (*scores)[i].attachment_[RIGHT]
	- (*scores)[i].attachment_[LEFT];
      Real slur_dy = slur_dz[Y_AXIS];
      Real demerit = 0.0;

      demerit += ((fabs (slur_dy / slur_dz[X_AXIS])
		   - score_param->max_slope_) >? 0)
	* score_param->max_slope_factor_;

      /* 0.2: account for staffline offset. */
      Real max_dy = (fabs (dy) + 0.2);
      if (has_beams)
	max_dy += 1.0;

      demerit += score_param->steeper_slope_factor_
	* ((fabs (slur_dy) -max_dy) >? 0);

      demerit += ((fabs (slur_dy/slur_dz[X_AXIS])
		   - score_param->max_slope_) >? 0)
	* score_param->max_slope_factor_;

      if (sign (dy) == 0
	  && sign (slur_dy) != 0)
	demerit += score_param->non_horizontal_penalty_;

      if (sign (dy)
	  && sign (slur_dy)
	  && sign (slur_dy) != sign (dy))
	demerit += has_beams
	  ? score_param->same_slope_penalty_ / 10
	  : score_param->same_slope_penalty_;

#if DEBUG_SLUR_QUANTING
      (*scores)[i].score_card_ += to_string ("S%.2f", d);
#endif
      (*scores)[i].score_ += demerit;
    }


  
}



