#!@PYTHON@

# mf-to-table.py -- convert spacing info in MF logs .afm and .tex
#
# source file of the GNU LilyPond music typesetter
#
# (c) 1997--2004 Han-Wen Nienhuys <hanwen@cs.uu.nl>

import os
import sys
import getopt
import string
import re
import time


postfixes = ['log', 'dvi', '2602gf', 'tfm']

def read_log_file (fn):
	str = open (fn).read ()
	str = re.sub ('\n', '', str)
	str = re.sub ('[\t ]+', ' ', str)

	deps = []
	autolines = []
	def include_func (match, d = deps):
		d.append (match.group (1))
		return ''

	def auto_func (match, a = autolines):
		a.append (match.group (1))
		return ''

	str = re.sub ('\(([a-zA-Z_0-9-]+\.mf)', include_func, str)
	str = re.sub ('@{(.*?)@}', auto_func, str)

	return (autolines, deps)


class Char_metric:
	def __init__ (self):
		pass


def tfm_checksum (fn):
	sys.stderr.write ("Reading checksum from `%s'\n" % fn)
	s = open (fn).read ()
	s = s[ 12 * 2 : ]
	cs_bytes = s[:4]

	shift = 24
	cs = 0
	for b in cs_bytes:
		cs = cs  + (long (ord (b)) << shift)
		shift = shift - 8

	return cs


## ugh.  What's font_family supposed to be?  It's not an afm thing.
font_family = 'feta'

def parse_logfile (fn):
	(autolines, deps) = read_log_file (fn)
	charmetrics = []
	global_info = {}
	group = ''

	for l in autolines:
		tags = string.split (l, '@:')
		if tags[0] == 'group':
			group = tags[1]
		elif tags[0] == 'puorg':
			group = ''
		elif tags[0] == 'char':
			name = tags[9]

			name = re.sub ('-', 'M', name)
			if group:
				name = group + '.' + name
			m = {
				'description': tags[1],
				'name': name,
				'tex': tags[10],
				'code': string.atoi (tags[2]),
				'breapth': string.atof (tags[3]),
				'width': string.atof (tags[4]),
				'depth': string.atof (tags[5]),
				'height': string.atof (tags[6]),
				'wx': string.atof (tags[7]),
				'wy': string.atof (tags[8]),
			}
			charmetrics.append (m)
		elif tags[0] == 'font':
			global font_family
			font_family = (tags[3])
			# To omit 'GNU' (foundry) from font name proper:
			# name = tags[2:]
			#urg
			if 0: # testing
				tags.append ('Regular')

			encoding = re.sub (' ','-', tags[5])
			tags = tags[:-1]
			name = tags[1:]
			global_info['DesignSize'] = string.atof (tags[4])
			global_info['FontName'] = string.join (name,'-')
			global_info['FullName'] = string.join (name,' ')
			global_info['FamilyName'] = string.join (name[1:-1],
								 '-')
			if 1:
				global_info['Weight'] = tags[4]
			else: # testing
				global_info['Weight'] = tags[-1]

			global_info['FontBBox'] = '0 0 1000 1000'
			global_info['Ascender'] = '0'
			global_info['Descender'] = '0'
			global_info['EncodingScheme'] = encoding

		elif tags[0] == 'parameter':
			global_info[tags[1]] = tags[2];
			
	return (global_info, charmetrics, deps)


def write_afm_char_metric (file, charmetric):
	f = 1000;
	tup = (charmetric['code'],
	       charmetric['name'],
	       -charmetric['breapth'] * f,
	       -charmetric['depth'] * f,
	       charmetric['width'] * f,
	       charmetric['height'] * f,
	       charmetric['wx'] * f,
	       charmetric['wy'] * f)

	file.write ('C %d ; N %s ; B %d %d %d %d ; W %d %d ;\n' % tup)


def write_afm_header (file):
	file.write ("StartFontMetrics 2.0\n")
	file.write ("Comment Automatically generated by mf-to-table.py\n")


def write_afm_metric (file, global_info, charmetrics):
	for (k, v) in global_info.items():
		file.write ("%s %s\n" % (k, v))
	file.write ('StartCharMetrics %d\n' % len(charmetrics ))
	for m in charmetrics:
		write_afm_char_metric (file, m)
	file.write ('EndCharMetrics\n')
	file.write ('EndFontMetrics\n')


def write_tex_defs (file, global_info, charmetrics):
	## nm = global_info['FontFamily']
	nm = font_family
	for m in charmetrics:
		file.write (r'''\gdef\%s%s{\char%d}%%%s''' % \
			    (nm, m['tex'], m['code'],'\n'))
	file.write ('\\endinput\n')


def write_character_lisp_table (file, global_info, charmetrics):

	def conv_char_metric (charmetric):
		f = 1.0
		s = """(%s .
((bbox . (%f %f %f %f))
 (attachment . (%f . %f))))
""" %(charmetric['name'],
		 -charmetric['breapth'] * f,
		 -charmetric['depth'] * f,
		 charmetric['width'] * f,
		 charmetric['height'] * f,
		 charmetric['wx'],
		 charmetric['wy'])

		return s

	for c in charmetrics:
		file.write (conv_char_metric (c))


def write_global_lisp_table (file, global_info):
	str = ''

	keys = ['staffsize', 'stafflinethickness', 'staff_space',
		'linethickness', 'black_notehead_width', 'ledgerlinethickness',
		'blot_diameter'
		]
	for k in keys:
		if global_info.has_key (k):
			str = str + "(%s . %s)\n" % (k,global_info[k])

	file.write (str)

	
def write_ps_encoding (name, file, global_info, charmetrics):
	encs = ['.notdef'] * 256
	for m in charmetrics:
		encs[m['code']] = m['name']

	file.write ('/%s [\n' % name)
	for m in range (0, 256):
		file.write ('  /%s %% %d\n' % (encs[m], m))
	file.write ('] def\n')


def write_fontlist (file, global_info, charmetrics):
	## nm = global_info['FontFamily']
	nm = font_family
	per_line = 2
	file.write (
r"""%% LilyPond file to list all font symbols and the corresponding names
%% Automatically generated by mf-to-table.py

\score {
  \lyrics { \time %d/8
""" % (2 * per_line + 1))

	count = 0
	for m in charmetrics:
		count += 1

		## \musicglyph and \markup require "_" to be escaped
		## differently
		scm_string = re.sub ('_', r'_', m['name'])
		tex_string = re.sub ('_', r'\\_' , m['name'])

		## prevent TeX from interpreting "--" as long dash
		tex_string = re.sub ('--','-{}-', tex_string)

		file.write ('''    \\markup { \\raise #0.75 \\vcenter
	      \\musicglyph #"%s"
	      \\typewriter " %s" } 4\n''' % (scm_string, tex_string))

		if (count % per_line) == 0:
			file.write ('    \\skip 8 \\break\n')
	file.write (r"""  }

  \layout {
    interscoreline = 1.0
    indent = 0.0 \cm
    \context {
      \Lyrics
      \override SeparationItem #'padding = #2
      minimumVerticalExtent = ##f
    }
    \context {
      \Score
      \remove "Bar_number_engraver"
    }
  }
}
""")


def write_deps (file, deps, targets):
	for t in targets:
		t = re.sub ( '^\\./', '', t)
		file.write ('%s '% t)
	file.write (": ")
	for d in deps:
		file.write ('%s ' % d)
	file.write ('\n')


def help ():
	sys.stdout.write(r"""Usage: mf-to-table [OPTIONS] LOGFILEs

Generate feta metrics table from preparated feta log.

Options:
  -a, --afm=FILE         specify .afm file
  -d, --dep=FILE         print dependency info to FILE
  -h, --help             print this help
  -l, --ly=FILE          name output table
  -o, --outdir=DIR       prefix for dependency info
  -p, --package=DIR      specify package
  -t, --tex=FILE         name output tex chardefs

  """)
	sys.exit (0)


(options, files) = \
  getopt.getopt (sys.argv[1:],
		 'a:d:hl:o:p:t:',
		 ['enc=', 'afm=', 'outdir=', 'dep=', 'lisp=',
		  'global-lisp=',
		  'tex=', 'ly=', 'debug', 'help', 'package='])

global_lisp_nm = ''
char_lisp_nm = ''
enc_nm = ''
texfile_nm = ''
depfile_nm = ''
afmfile_nm = ''
lyfile_nm = ''
outdir_prefix = '.'

for opt in options:
	o = opt[0]
	a = opt[1]
	if o == '--dep' or o == '-d':
		depfile_nm = a
	elif o == '--outdir' or o == '-o':
		outdir_prefix = a
	elif o == '--tex' or o == '-t':
		texfile_nm = a
	elif o == '--lisp': 
		char_lisp_nm = a
	elif o == '--global-lisp': 
		global_lisp_nm = a
	elif o == '--enc':
		enc_nm = a
	elif o == '--ly' or o == '-l':
		lyfile_nm = a
	elif o== '--help' or o == '-h':
		help()
	elif o=='--afm' or o == '-a':
		afmfile_nm = a
	elif o == '--debug':
		debug_b = 1
	else:
		print o
		raise getopt.error

base = re.sub ('.tex$', '', texfile_nm)

for filenm in files:
	(g, m, deps) = parse_logfile (filenm)
	cs = tfm_checksum (re.sub ('.log$', '.tfm', filenm))
	afm = open (afmfile_nm, 'w')

	write_afm_header (afm)
	afm.write ("Comment TfmCheckSum %d\n" % cs)
	afm.write ("Comment DesignSize %.2f\n" % g['DesignSize'])

	del g['DesignSize']

	write_afm_metric (afm, g, m)

	write_tex_defs (open (texfile_nm, 'w'), g, m)
	enc_name = 'FetaEncoding'
	if re.search ('parmesan', filenm):
		enc_name = 'ParmesanEncoding'
	elif re.search ('feta-brace', filenm):
		enc_name = 'FetaBraceEncoding'

	write_ps_encoding (enc_name, open (enc_nm, 'w'), g, m)
	write_character_lisp_table (open (char_lisp_nm, 'w'), g, m)  
	write_global_lisp_table (open (global_lisp_nm, 'w'), g)  
	if depfile_nm:
		write_deps (open (depfile_nm, 'wb'), deps,
			    [base + '.dvi', base + '.pfa', base + '.pfb',
			     texfile_nm, afmfile_nm])
	if lyfile_nm:
		write_fontlist (open (lyfile_nm, 'w'), g, m)
