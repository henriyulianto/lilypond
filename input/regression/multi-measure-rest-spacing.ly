\header {

    texidoc = "By setting texts starting with a multi-measure rest, an 
extra spacing column is created. This should not cause problems."
    }
\version "2.3.22"

\score {
    <<
	\set Score.skipBars = ##t
	\context Staff = flute  \new Voice { 
	    <<  { R1*40 }  { s1*0_"bla" }>> 
	}
    >>
    \layout {
	raggedright = ##t
    }
 }
