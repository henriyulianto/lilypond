\version "2.3.4"
\header  {
  texidoc = "Grace notes in different voices/staves are synchronized."
}

\score  {\relative c'' << \context Staff  { c2
	 \grace  c8
  c4 c4 }
		\new Staff { c2 \clef bass
 \grace {  dis8[ ( d8] \key es\major  }

    c4) c4 }
		\new Staff { c2 c4 c4 \bar "|." }
		>>
		\paper { raggedright = ##t}
 } 

