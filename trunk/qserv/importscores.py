
import logging
import urllib

from google.appengine.api import urlfetch
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext.webapp.util import login_required

import qserv

def setup(root):
	"""Returns the handlers for the score importation. root must have a
	trailing slash."""
	_BaseHandler.root = root
	return [(root, FormHandler),
	        (root + 'submit', ImportHandler),
	        (root + 'status', StatusHandler)]

class _BaseHandler(webapp.RequestHandler):
	pass

class FormHandler(_BaseHandler):
	@login_required
	def get(self):
		if not users.is_current_user_admin():
			self.redirect('status?msg=notadmin')
			return
		self.response.out.write("""<html>
  <body>
    <form action="submit" method="post" enctype="multipart/form-data">
      <p>Import from: <input name="highscores" type="file"></p>
      <p><input value="Upload highscores" type="submit"></p>
    </form>
  </body>
</html>""")

class ImportHandler(_BaseHandler):
	def post(self):
		if not users.is_current_user_admin():
			self.redirect('status?msg=notadmin')
			return

		highscores_data = self.request.get('highscores', '')
		if highscores_data == '':
			self.redirect('status?msg=missingdata')
			return

		lines = highscores_data.splitlines()
		if lines and lines[0] == 'Ok':
			lines.pop(0)

		params = {}
		for line in lines:
			(key, value) = line.split(None, 1)
			qserv.set_param(params, key, value)

		for score in params.itervalues():
			if 'score' in score and 'rec' in score:
				qserv.put_score(score)

		self.redirect('status?msg=success')

class StatusHandler(_BaseHandler):
	@login_required
	def get(self):
		msg = self.request.get('msg', 'unknown')
		if not users.is_current_user_admin():
			msg = 'notadmin'
		msgstrs = {'unknown': 'status unknown',
		           'notadmin': 'you do not have sufficient rights',
		           'missingdata': 'highscore data is missing',
		           'success': 'success'}
		self.response.out.write("""<html>
  <body>
    <p>%s</p>
		<p><a href="%s">logout</a></p>
  </body>
</html>""" % (msgstrs[msg], users.create_logout_url(self.root)))

