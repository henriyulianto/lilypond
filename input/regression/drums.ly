
\header {
   texidoc = "In drum notation, there is a special clef symbol, drums are
   placed to their own staff positions and have note heads according to the 
   drum, an extra symbol may be attached to the drum, and the number of lines 
   may be restricted."
}


\version "2.3.4"

drh = \drums { cymc4.^"crash" hhc16^"h.h." hh \repeat "unfold" 5 {hhc8 hho hhc8 hh16 hh} hhc4 r4 r2 }
drl = \drums {\repeat "unfold" 3 {bd4 sn8 bd bd4 << bd ss >> } bd8 tommh tommh bd toml toml bd tomfh16 tomfh }
timb = \drums { \repeat "unfold" 2 {timh4 ssh timl8 ssh r timh r4 ssh8 timl r4 cb8 cb} }

\score {
    \repeat "volta" 2 {
	<<
	\new DrumStaff \with {
	    drumStyleTable = #timbales-style
	    \override StaffSymbol #'line-count = #2
	    \override BarLine #'bar-size = #2
	} <<
	    \set Staff.instrument = "timbales"
	    \timb
	>>
	\new DrumStaff <<
	    \set Staff.instrument = "drums"
	    \new DrumVoice {\stemUp \drh }
	    \new DrumVoice {\stemDown \drl }
	>>
    >>
	}
    \paper {}

    %% broken:
    \midi{ \tempo 4=120 }
}


