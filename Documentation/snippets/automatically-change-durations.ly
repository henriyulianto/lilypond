%% DO NOT EDIT this file manually; it is automatically
%% generated from LSR http://lsr.dsi.unimi.it
%% Make any changes in LSR itself, or in Documentation/snippets/new/ ,
%% and then run scripts/auxiliar/makelsr.py
%%
%% This file is in the public domain.
\version "2.14.2"

\header {
  lsrtags = "rhythms"

  texidoc = "
@code{shiftDurations} can be used to change the note lengths of a
piece of music.  It takes two arguments - the scaling factor as a power
of two, and the number of dots to be added as a positive integer.

"
  doctitle = "Automatically change durations"
} % begin verbatim


\paper { indent = 0 }

music = \relative c'' { a1 b2 c4 d8 r }

\score {
  \new Voice {
    \time 4/2
    \music
    \time 4/4
    \shiftDurations #1 #0 { \music }
    \time 2/4
    \shiftDurations #2 #0 { \music }
    \time 4/1
    \shiftDurations #-1 #0 { \music }
    \time 8/1
    \shiftDurations #-2 #0 { \music }
    \time 6/2
    \shiftDurations #0 #1 { \music }
    \time 7/2
    \shiftDurations #0 #2 { \music }
  }
}
