#!/usr/bin/python

import __main__
import getopt
import gettext
import os
import re
import string
import sys

# The directory to hold the translated and menuified tree.
outdir = '/var/www'
verbose = 0

LANGUAGES = (
	('site', 'English'),
	('nl', 'Nederlands'),
	)

localedir = 'out/locale'
try:
	import gettext
	gettext.bindtextdomain ('newweb', localedir)
	gettext.textdomain ('newweb')
	_ = gettext.gettext
except:
	def _ (s):
		return s
underscore = _


'''
NOTES

* Do not use absolute URL locations.  That breaks local installs
  of the website.  Rather, use '../' * depth to get to the root.

'''

PAGE_TEMPLATE = '''@MAIN@'''

LANGUAGES_TEMPLATE = '''\
<P class="location">%s</P>
'''

# ugh, move to .ithml
top_script = '''\
<SCRIPT>
  <!--
    function setfocus ()
      {
        document.f_lily_google.brute_query.focus ();
      }
    // !-->
  </SCRIPT>
'''

MENU_TEMPLATE = '''\
<TABLE cellpadding="0" cellspacing="0">
  <TR>%s</TR>
</TABLE>
'''

MENU_SEP = '''\
<TABLE width="100%" cellpadding="1" cellspacing="0">
  <TR>
    <TD class="menuactive"></TD>
  </TR>
</TABLE>
<TABLE width="100%" cellpadding="1" cellspacing="0">
  <TR>
    <TD></TD>
  </TR>
</TABLE>
'''

MENU_ITEM = '''\
<TD class="%%(css)sleftedge" width="1"></TD>
<TD class="%%(css)s">%s</TD>
<TD class="%%(css)srightedge" width="1"></TD>
'''

MENU_ITEM_INACTIVE = MENU_ITEM \
		     % '<A class=%(css)s" href="%(url)s">%(name)s</A>'
MENU_ITEM_ACTIVE = MENU_ITEM \
		   % '<A class="%(css)s" href="%(url)s">[%(name)s]</A>'

LOCATION_ITEM = '<a href="%(url)s">[%(name)s]</a>'
LOCATION_SEP = ' &gt; '

LOCATION_TITLE = 'LilyPond - %s'


def dir_lang (file, lang):
	return string.join ([lang] + string.split (file, '/')[1:], '/')


def file_lang (file, lang):
	(base, ext) = os.path.splitext (file)
	base = os.path.splitext (base)[0]
	if lang and lang != 'site':
		return base + '.' + lang + ext
	return base + ext


def rreverse (lst):
	list.reverse (lst)
	return lst


def format_page (html, file_name, lang):
	class dir_entry:
		def __init__ (self, dir, up):
			self.name = dir
			self.up = up

	def list_directories (dir, up):
		if not dir:
			return []
		return [dir_entry (dir, up)] \
		       + list_directories (os.path.split (dir)[0], up + 1)

	dir = os.path.dirname (file_name)
	directories = list_directories (dir, 0)

	base_name = os.path.basename (file_name)
	is_index = base_name == 'index.html'
	depth = len (directories)

	dir_split = rreverse (string.split (dir, '/'))
	def is_active (dir_entry, url):
		return (url == dir_split[dir_entry.up - 1] \
			or os.path.join (dir_entry.name, url) == file_name)

	def get_menu (d):
		f = os.path.join (dir_lang (d, 'site'), 'menu-entries.py')
		
		if os.path.isfile (f):
			menu = eval (open (f).read (), __main__.__dict__, {})
		else:
			menu = [('', os.path.splitext (d)[0]),]
		return menu

	class item:
		def __init__ (self, dir_entry, menu_entry):
			URL = 0
			NAME = 1
			self.is_active = is_active (dir_entry, menu_entry[URL])
			self.up = dir_entry.up
			self.url = menu_entry[URL]
			self.name = menu_entry[NAME]

	def directory_menu (dir_entry):
		return map (lambda x: item (dir_entry, x),
			    get_menu (dir_entry.name))
	
	listings = map (directory_menu, directories)

	def location_menu (dir_entry):
		active = filter (lambda x: is_active (dir_entry, x[0]),
				 get_menu (dir_entry.name))
		return map (lambda x: item (dir_entry, x), active)

	locations = filter (lambda x: x, map (location_menu, directories))
	locations = rreverse (map (lambda x: x[0], locations))

	is_root = is_index and depth == 1
	if is_root:
		locations = []
	else:
		home = item (dir_entry (lang, depth - 1), ('.', _ ("Home")))
		locations = [home] + locations

	def itemize (item, css, ACTIVE, INACTIVE):
		is_active = item.is_active
		name = item.name
		url = '../' * item.up + item.url
		if item.is_active:
			css += 'active'
			s = ACTIVE
		else:
			s = INACTIVE
		return s % vars ()

	def menuitemize (item):
		return itemize (item, 'menu',
				MENU_ITEM_ACTIVE, MENU_ITEM_INACTIVE)

	menus = map (lambda x: map (menuitemize, x), listings)
	menu_strings = map (lambda x: MENU_TEMPLATE % string.join (x),
			    menus)
	menu = string.join (rreverse (menu_strings), MENU_SEP)

	def locationize (item):
		return itemize (item, 'location',
				LOCATION_ITEM, LOCATION_ITEM)

	location = string.join (map (locationize, locations), LOCATION_SEP)
	location_title = LOCATION_TITLE % string.join (map (lambda x: x.name,
							    locations[1:]),
						       ' - ')

	## @AT@ substitutions.

	titles = [location_title]
	def grab_title (match):
		titles.append (match.group (1))
		return ''

	def grab_ihtml (match):
		s = match.group (1)
		for d in (dir, lang, 'site'):
			n = os.path.join (d, s)
			if os.path.exists (n):
				return open (n).read ()
		return match.group (0)

	def grab_gettext (match):
		return gettext (match.group (1))

	root_url = '../' * (depth - 1)
	main = re.sub ('<title>(.*?)</title>', grab_title, html)

	if is_root:
		script = top_script
	else:
		script = ''

	page_template = PAGE_TEMPLATE
	f = os.path.join (dir_lang (dir, 'site'), 'template.ihtml')
	if dir != lang and os.path.isfile (f):
		page_template = open (f).read ()

	# Ugh: factor 2 slowdown
	# page = re.sub ('@MAIN@', main, page_template)
	i = string.index (page_template, '@MAIN@')
	page = page_template[:i] + main + page_template[i+6:]

	page = re.sub ('@LOCATION@', location, page)
	page = re.sub ('@MENU@', menu, page)
	page = re.sub ('@SCRIPT@', script, page)
	page = re.sub ('@TITLE@', titles[-1], page)
	
	page = re.sub ('@DEPTH@', root_url, page)
	page = re.sub ('@DOC@', os.path.join (root_url, '../doc/'), page)
	page = re.sub ('@IMAGES@', os.path.join (root_url, 'images/'), page)
	page = re.sub ('@([-A-Za-z]*.ihtml)@', grab_ihtml, page)
	page = re.sub ('_@([^@]*)@', grab_gettext, page)
	page = re.sub ('\$\Date: (.*) \$', '\\1', page)

	rel_name = string.join (string.split (file_name, '/')[1:], '/')
	available = filter (lambda x: lang != x[0] \
			    and os.path.exists (os.path.join (x[0], rel_name)),
			    LANGUAGES)

	language_menu = ''
	for (prefix, name) in available:
		lang_file = file_lang (base_name, prefix)
		language_menu += '<a href="%(lang_file)s">%(name)s</a>' % vars ()
	# Disable language selection until we have something useful.
	if lang == 'site':
		language_menu = ''

	###page = PAGE_TEMPLATE % vars ()

	lang_dir = dir_lang (dir, lang)
	def langify_url (match):
		rel = match.group (1)
		file = os.path.join (lang_dir, rel)
		if os.path.isfile (file):
			rel = file_lang (rel, lang)
		elif os.path.isdir (file) \
		   and os.path.isfile (os.path.join (file, 'index.html')):
			rel = file_lang (os.path.join (rel, 'index.html'), lang)
		return '''href="%(rel)s"''' % vars ()
		
	if lang != 'site':
		page = re.sub ('''href=[\'"]([^/][^:\'"]*)[\'"]''',
			       langify_url, page)

	# Must add language menu after url langification
	languages = LANGUAGES_TEMPLATE % language_menu
	page = re.sub ('@LANGUAGES@', languages, page)

	return page


def do_file (file_name):
	if verbose:
		sys.stderr.write ('%s...\n' % file_name)
	lang = string.split (file_name, '/')[0]
	if lang == 'site':
		out_file_name = file_name
	else:
		out_file_name = file_lang (dir_lang (file_name, 'site'), lang)
	out_file_name = os.path.join (outdir, out_file_name)

	if file_name == out_file_name:
		raise 'cowardly resisting to overwrite source: ' + file_name

	html = open (file_name).read ()
	page = format_page (html, file_name, lang)
	open (out_file_name, 'w').write (page)
	
def do_options ():
	global outdir, verbose
	(options, files) = getopt.getopt (sys.argv[1:],
					  '',
					  ['outdir=', 'help', 'verbose']) 
	for (o, a) in options:
		if o == '--outdir':
			outdir = a
		elif o == '--verbose':
			verbose = 1
		elif o == '--help':
			sys.stdout.write (r'''
Usage:
format-page --outdir=DIRECTORY [--verbose]

This script is licensed under the GNU GPL
''')
		else:
			assert unimplemented
	return files


def main ():
	files = do_options ()

	global PAGE_TEMPLATE
	f = os.path.join (dir_lang ('', 'site'), 'template.ihtml')
	if os.path.isfile (f):
		PAGE_TEMPLATE = open (f).read ()

	for i in files:
		do_file (i)

if __name__ == '__main__':
	main ()

