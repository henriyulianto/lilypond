#include "key-item.hh"
#include "key.hh"
#include "debug.hh"
#include "molecule.hh"
#include "paper-def.hh"
#include "lookup.hh"
//#include "clef-reg.hh"
#include "key-reg.hh"

const int FLAT_TOP_PITCH=2; /* fes,ges,as and bes typeset in lower octave */
const int SHARP_TOP_PITCH=4; /*  ais and bis typeset in lower octave */



Key_item::Key_item(int c)
{
    set_c_position(c);
}

void
Key_item::read(Key_register const & key_reg_r)
{
    assert(!key_reg_r.key_.multi_octave_b_);
    const Array<int> &idx_arr =key_reg_r.accidental_idx_arr_; 
    for (int i = 0 ; i< idx_arr.size(); i++) {
	int note = idx_arr[i];
	int acc = ((Key &) key_reg_r.key_).oct(0).acc(note);

	add(note, acc);
    }
}

void 
Key_item::set_c_position(int c0)
{
    int octaves =(abs(c0) / 7) +1 ;
    c_position=(c0 + 7*octaves)%7;
}


void
Key_item::add(int p, int a)
{
    if ((a<0 && p>FLAT_TOP_PITCH) ||
        (a>0 && p>SHARP_TOP_PITCH)) {
      p -= 7; /* Typeset below c_position */
    }
    pitch.push(p);
    acc.push(a);
}


Molecule*
Key_item::brew_molecule_p()const
{
    Molecule*output = new Molecule;
    Real inter = paper()->internote();
    
    for (int i =0; i < pitch.size(); i++) {
	Symbol s= paper()->lookup_l()->accidental(acc[i]);
	Atom a(s);
	a.translate(Offset(0,(c_position + pitch[i]) * inter));
	Molecule m(a);
	output->add_right(m);	
    }
    Molecule m(paper()->lookup_l()->fill(Box(
	Interval(0, paper()->note_width()),
	Interval(0,0))));
    output->add_right(m);
    return output;
}
IMPLEMENT_STATIC_NAME(Key_item);
