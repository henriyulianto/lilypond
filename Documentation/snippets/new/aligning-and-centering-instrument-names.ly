\version "2.19.56"

\header {
  lsrtags = "paper-and-layout, text, titles"

  texidoc = "
The horizontal alignment of instrument names is tweaked by changing the
@code{Staff.InstrumentName #'self-alignment-X} property. The
@code{\\layout} variables @code{indent} and @code{short-indent} define
the space in which the instrument names are aligned before the first
and the following systems, respectively.

"
  doctitle = "Aligning and centering instrument names"
}

\paper { left-margin = 3\cm }

\score {
  \new StaffGroup <<

    \new Staff \with {
      \override InstrumentName.self-alignment-X = #LEFT
      instrumentName = \markup \left-column {
        "Left aligned"
        "instrument name"
        }
        shortInstrumentName = "Left"
      }

      {  c''1 \break c''1 }

    \new Staff \with {
      \override InstrumentName.self-alignment-X = #CENTER
      instrumentName = \markup \center-column {
        Centered
        "instrument name"
        }
      shortInstrumentName = "Centered"
    }

    { g'1 g'1}

    \new Staff \with {
      \override InstrumentName.self-alignment-X = #RIGHT
      instrumentName = \markup \right-column {
        "Right aligned"
        "instrument name"
      }
      shortInstrumentName = "Right"
    }

    { e'1 e'1 }

  >>

  \layout {
    ragged-right = ##t
    indent = 4\cm
    short-indent = 2\cm
  }
}
