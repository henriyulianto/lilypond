/*   
  multi-measure-rest.hh -- declare Multi_measure_rest
  
  source file of the GNU LilyPond music typesetter
  
  (c) 1998--2001 Jan Nieuwenhuizen <janneke@gnu.org>
  
 */

#ifndef MULTI_MEASURE_REST_HH
#define MULTI_MEASURE_REST_HH

#include "lily-proto.hh"
#include "lily-guile.hh"
#include "rod.hh"

class Multi_measure_rest
{
public:
  static void set_interface (Grob*);
  static bool has_interface (Grob*);
  DECLARE_SCHEME_CALLBACK (brew_molecule, (SCM ));
  DECLARE_SCHEME_CALLBACK (percent, (SCM));
  static  void add_column (Grob*,Item*);
  DECLARE_SCHEME_CALLBACK (set_spacing_rods, (SCM ));
};

#endif /* MULTI_MEASURE_REST_HH */

