#/*
  template7.cc -- instantiate Request_column

  source file of the GNU LilyPond music typesetter

  (c) 1997 Han-Wen Nienhuys <hanwen@stack.nl>
*/

#include "proto.hh"
#include "plist.hh"
#include "plist.tcc"
#include "pcursor.tcc"

#include "music-list.hh"
#include "music-iterator.hh"

template POINTERLIST_INSTANTIATE(Music);
template POINTERLIST_INSTANTIATE(Music_iterator);
