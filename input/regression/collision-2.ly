\version "2.3.22"
\header {
texidoc = "Single head notes may collide. "
}
    \layout { raggedright= ##t }


\score {
    

  \context Staff  \transpose c c' <<  
	{  c4 d e f g2 g4 a | }  \\
	{ g4 f e g  g2 g2 } 
  >>
}


