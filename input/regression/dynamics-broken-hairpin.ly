
\version "2.6.0"
\header{
texidoc = "Broken crescendi should be open on one side."
}

\score {  \relative c'' { 
    c1 \< \break c1\!  \> \break c1\!
  }
  \layout {
    linewidth = 4.\cm
  }
}
  

