\version "1.5.68"
%
% setup for Request->Element conversion. Guru-only
%
StaffContext = \translator {
	\type "Staff_performer"
	\name Staff
	\accepts Voice

	\consists "Key_performer"
	\consists "Tempo_performer"
	\consists "Time_signature_performer"

}

VoiceContext = \translator {
	\type "Performer_group_performer"
	\name Voice
	\consists "Dynamic_performer"
	\consists "Span_dynamic_performer"
	\consists "Piano_pedal_performer"
	\accepts "Thread"
}

ThreadContext = \translator {
	\type "Performer_group_performer"
	\name Thread
	\consists "Note_performer"
	\consists "Tie_performer"
}

FiguredBassContext = \translator {
	\type "Performer_group_performer"
	\name FiguredBass 
	\consists "Swallow_performer"
}

GrandStaffContext = \translator {
	\type "Performer_group_performer"
	\name GrandStaff
	\accepts RhythmicStaff
	\accepts Staff
}

PianoStaffContext = \translator {
        \type "Performer_group_performer"
	\name "PianoStaff"
	\accepts Staff
}

ScoreContext = \translator {
	\type "Score_performer"

	\name Score
	\alias Timing
	instrument = #"bright acoustic"
	\accepts Staff
	\accepts GrandStaff
	\accepts PianoStaff
	\accepts Lyrics 
	\accepts StaffGroup
	\accepts ChoirStaff
	\accepts RhythmicStaff
	\accepts ChordNames
	\accepts FiguredBass

	\alias "Timing"
	\consists "Timing_translator"
	\consists "Swallow_performer"
	
	dynamicAbsoluteVolumeFunction = #default-dynamic-absolute-volume
	instrumentEqualizer = #default-instrument-equalizer
}


\translator {
	\type "Performer_group_performer"
	\consists "Lyric_performer"
	\name LyricsVoice
}

\translator{
	\type "Performer_group_performer"
	\name ChoirStaff
	\accepts Staff
}
\translator { 
	\type "Staff_performer"
	\accepts LyricsVoice
	\name Lyrics
	\consists "Time_signature_performer"
	\consists "Tempo_performer"
}

\translator {
	\type "Staff_performer"
	\accepts ChordNameVoice
	\name ChordNames
}

\translator {
	\type Performer_group_performer
	\consists Note_performer
	\name ChordNameVoice	
}

\translator {
	\type Performer_group_performer

	\name StaffGroup
	\accepts Staff
}



\translator { \ScoreContext }
\translator { \StaffContext }
\translator { \StaffContext \name RhythmicStaff }
\translator { \VoiceContext }
\translator { \ThreadContext }
\translator { \PianoStaffContext }
\translator { \GrandStaffContext }
\translator { \FiguredBassContext }
