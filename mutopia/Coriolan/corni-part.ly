\header{
filename =	 "corni-part.ly";
% %title =	 "Ouvert\\"ure zu Collins Trauerspiel \\"Coriolan\\" Opus 62";
description =	 "";
composer =	 "Ludwig van Beethoven (1770-1792)";
enteredby =	 "JCN";
copyright =	 "public domain";


}

\version "0.1.10";

\include "global.ly"
\include "corni.ly"

\score{
%	\property Score.part = 1
	\$corni_staff
	\paper{}
}

