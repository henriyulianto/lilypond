/*   
  new-staff-margin-engraver.cc --  implement Instrument_name_engraver
  
  source file of the GNU LilyPond music typesetter
  
  (c) 2000 Han-Wen Nienhuys <hanwen@cs.uu.nl>
  
 */

#include "engraver.hh"
#include "item.hh"
#include "bar.hh"
#include "system-start-delimiter.hh"
#include "side-position-interface.hh"
#include "align-interface.hh"

class Instrument_name_engraver : public Engraver
{
  Item *text_;
  Score_element * delim_ ;

  void create_text (SCM s);
public:
  VIRTUAL_COPY_CONS(Translator);
  Instrument_name_engraver ();

  virtual void acknowledge_element (Score_element_info);
  virtual void do_pre_move_processing ();
};

ADD_THIS_TRANSLATOR(Instrument_name_engraver);

Instrument_name_engraver::Instrument_name_engraver ()
{
  text_ = 0;
  delim_ =0;
}


void
Instrument_name_engraver::do_pre_move_processing ()
{
  if (text_)
    {
      typeset_element (text_);
      text_ = 0;
    }
}

void
Instrument_name_engraver::create_text (SCM txt)
{
  if(!text_)
    {
      text_ = new Item (get_property ("InstrumentName"));
      text_->set_elt_property ("text", txt);

      if (delim_)
	text_->set_parent (delim_, Y_AXIS);

      announce_element (text_,0);
    }
}

void
Instrument_name_engraver::acknowledge_element (Score_element_info i)
{
  SCM s = get_property ("instrument");
  
  if (now_mom () > Moment (0))
    s = get_property ("instr");

  if (gh_string_p (s))
    {
      if (Bar::has_interface (i.elem_l_))
	{
	  create_text (s);
	}
    }

  if (Align_interface::has_interface (i.elem_l_)
      && Align_interface::axis  (i.elem_l_) == Y_AXIS      
      //System_start_delimiter::has_interface (i.elem_l_)
      && i.origin_trans_l_->daddy_trans_l_ == daddy_trans_l_)
    {
      delim_ = i.elem_l_;
    }
}




