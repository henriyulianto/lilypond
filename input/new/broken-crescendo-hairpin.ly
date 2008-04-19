\version "2.11.38"

\header {
  lsrtags = "expressive-marks"
  texidoc = "
In order to make parts of a crescendo hairpin invisible, the following
method is used: A white rectangle is drawn on top of the respective
part of the crescendo hairpin, making it invisible. The rectangle is
defined as postscript code within a text markup.

To fine-tune the position and size of the markup, the number
preceding @code{setgray} in the postscript definition can be set to a
value <1 making it grey. The two numbers before @code{scale} in the
postscript code are responsible for the width and height of the
rectangle, the two numbers before @code{translate} change the x- and
y-origin of the rectangle. 

Make sure to put the hairpin in a lower layer than the text markup to
actually draw the rectangle above the hairpin. 
"
  doctitle = "Broken crescendo hairpin"
}

\score {
  \relative c' {
    << {
      \dynamicUp
      \override DynamicLineSpanner #'staff-padding = #4
      r2 r16 c'8.\pp r4
    } \\ {
      \override DynamicLineSpanner #'layer = #0
      des,2\mf\< ~
      \override TextScript #'layer = #2
      des16_\markup {
        \postscript #"1.9 -8 translate 5 4 scale 1 setgray 0 0 moveto 0 1
        lineto 1 1 lineto 1 0 lineto 0 0 lineto fill"
      }
      r8. des4 ~ des16->\sff
    } >>
  }
  \layout { ragged-right = ##t }
}
