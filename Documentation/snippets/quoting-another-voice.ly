%% DO NOT EDIT this file manually; it is automatically
%% generated from LSR http://lsr.dsi.unimi.it
%% Make any changes in LSR itself, or in Documentation/snippets/new/ ,
%% and then run scripts/auxiliar/makelsr.py
%%
%% This file is in the public domain.
\version "2.14.2"

\header {
  lsrtags = "staff-notation"

%% Translation of GIT committish: b482c3e5b56c3841a88d957e0ca12964bd3e64fa
  texidoces = "

La propiedad @code{quotedEventTypes} determina los tipos de eventos
musicales que resultan citados.  El valor predeterminado es
@code{(note-event rest-event tie-event beam-event tuplet-span-event)},
que significa que sólo aparecen en la expresión @code{\\quoteDuring}
las notas, silencios, ligaduras, barras y grupos especiales.  En el
ejemplo siguiente, el silencio de semicorchea no aparece en el
fragmento citado porque @code{rest-event} no está dentro de los
@code{quotedEventTypes}.

Para ver una lista de los tipos de evento, consulte la sección
@qq{Music classes} de la Referencia de funcionamiento interno.


"
  doctitlees = "Citar otra voz"


%% Translation of GIT committish: 0a868be38a775ecb1ef935b079000cebbc64de40
  texidocde = "
Die @code{quotedEventTypes}-Eigenschaft bestimmt die
Typen an Musikereignissen, die zitiert werden.  Die
Standardeinstellung ist @code{(note-event rest-event)}, womit
nur Noten und Pausen der zitierten Stimme für den
@code{\\quoteDuring}-Ausdruck übernommen werden.  Im
Beispiel hier wird die 16-Pause nicht übernommen, weil
sich @code{rest-event} nicht in @code{quotedEventTypes} befindet.

"

  doctitlede = "Eine andere Stimme zitieren"

%% Translation of GIT committish: 3b125956b08d27ef39cd48bfa3a2f1e1bb2ae8b4
  texidocfr = "
Les types d'événement pris en charge pour la citation peuvent se régler
avec la propriété @code{quotedEventTypes}.  Par défaut, sa valeur est
fixée à @code{(note-event rest-event tie-event beam-event
tuplet-span-event)}, ce qui signifie que seuls les notes, silences,
liaisons, ligatures et nolets seront mentionnés par @code{\\quoteDuring}.
Dans l'exemple suivant, le quart de soupir n'est pas reproduit puisqu'il
n'est pas mentionné parmi les @code{quotedEventTypes}.

Pour connaître la liste des types d'événements, reportez-vous au
chapitre @emph{Music classes} de la référence des propriétés internes.

"
  doctitlefr = "Citation d'une autre voix"


  texidoc = "
The @code{quotedEventTypes} property determines the music event types
which should be quoted.  The default value is @code{(note-event
rest-event tie-event beam-event tuplet-span-event)}, which means that
only the notes, rests, ties, beams and tuplets of the quoted voice will
appear in the @code{\\quoteDuring} expression. In the following
example, a 16th rest is not quoted since @code{rest-event} is not in
@code{quotedEventTypes}.

For a list of event types, consult the @qq{Music classes} section of
the Internals Reference.

"
  doctitle = "Quoting another voice"
} % begin verbatim


quoteMe = \relative c' {
  fis4 r16 a8.-> b4\ff c
}
\addQuote quoteMe \quoteMe

original = \relative c'' {
  c8 d s2
  \once \override NoteColumn #'ignore-collision = ##t
  es8 gis8
}

<<
  \new Staff {
    \set Staff.instrumentName = #"quoteMe"
    \quoteMe
  }
  \new Staff {
    \set Staff.instrumentName = #"orig"
    \original
  }
  \new Staff \relative c'' <<
    \set Staff.instrumentName = #"orig+quote"
    \set Staff.quotedEventTypes =
      #'(note-event articulation-event)
    \original
    \new Voice {
      s4
      \set fontSize = #-4
      \override Stem #'length-fraction = #(magstep -4)
      \quoteDuring #"quoteMe" { \skip 2. }
    }
  >>
>>
