% paper20-init.ly


\version "1.3.146"

paperTwenty = \paper {
	staffheight = 20.0\pt
	\stylesheet #(make-style-sheet 'paper20)
	
	\include "params-init.ly"
}

\paper { \paperTwenty }
