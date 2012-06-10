%% DO NOT EDIT this file manually; it is automatically
%% generated from LSR http://lsr.dsi.unimi.it
%% Make any changes in LSR itself, or in Documentation/snippets/new/ ,
%% and then run scripts/auxiliar/makelsr.py
%%
%% This file is in the public domain.
\version "2.14.2"

\header {
  lsrtags = "contexts-and-engravers, staff-notation"

%% Translation of GIT committish: b482c3e5b56c3841a88d957e0ca12964bd3e64fa
  texidoces = "
Se puede usar el delimitador de comienzo de un sistema
@code{SystemStartSquare} estableciéndolo explícitamente dentro de
un contexto @code{StaffGroup} o @code{ChoirStaffGroup}.

"
  doctitlees = "Uso del corchete recto al comienzo de un grupo de pentagramas"


%% Translation of GIT committish: 0a868be38a775ecb1ef935b079000cebbc64de40
  texidocde = "
Die Klammer zu Beginn von Systemgruppen kann auch in eine eckige Klammer
(@code{SystemStartSquare}) umgewandelt werden, wenn man sie explizit
im @code{StaffGroup}- oder @code{ChoirStaffGroup}-Kontext setzt.

"
  doctitlede = "Eine eckige Klammer zu Beginn von Systemgruppen benutzen"

%% Translation of GIT committish: 4ab2514496ac3d88a9f3121a76f890c97cedcf4e
  texidocfr = "
Un regroupement de portées sera indiqué par un simple rectangle
-- @code{SystemStartSquare} -- en début de ligne dès lors que vous le
mentionnerez explicitement au sein d'un contexte @code{StaffGroup} ou
@code{ChoirStaffGroup}.

"
  doctitlefr = "Indication de regroupement de portées par un rectangle"


  texidoc = "
The system start delimiter @code{SystemStartSquare} can be used by
setting it explicitly in a @code{StaffGroup} or @code{ChoirStaff}
context.

"
  doctitle = "Use square bracket at the start of a staff group"
} % begin verbatim


\score {
  \new StaffGroup { <<
  \set StaffGroup.systemStartDelimiter = #'SystemStartSquare
    \new Staff { c'4 d' e' f' }
    \new Staff { c'4 d' e' f' }
  >> }
}

