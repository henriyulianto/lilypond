
\version "2.3.22"
\header {
    texidoc = "Tieing a grace to the to a following grace or main note works."
}

    \layout { raggedright= ##t }

\score {  \context Voice \relative c'' {
    c4 \grace { c8 ~ c16 ~ } c4 
  }
}

