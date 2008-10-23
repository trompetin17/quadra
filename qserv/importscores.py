
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
    <form action="submit" method="post">
      <p>Import from: <input name="url"></p>
    </form>
  </body>
</html>""")

class ImportHandler(_BaseHandler):
	def post(self):
		url = self.request.get('url', '')
		if not users.is_current_user_admin() or url == '':
			self.redirect('status?msg=notadmin')
			return

		logging.info('importing scores from ' + url)

		response = urlfetch.fetch(url, payload=urllib.urlencode({'data': 'gethighscores\nnum 100\n'}),
		                          headers={'Content-Type': 'application/x-www-form-urlencoded'},
		                          method=urlfetch.POST)

		if response.status_code != 200:
			logging.error('fetch error while importing (%d)' % response.status_code)
			self.redirect('status?msg=fetcherror')
			return

		lines = response.content.splitlines()
		if not len(lines) or lines.pop(0) != 'Ok':
			logging.error('invalid response while importing')
			self.redirect('status?msg=invalid')
			return

		params = {}
		for line in lines:
			(key, value) = line.split(None, 1)
			qserv.set_param(params, key, value)

		for score in params.itervalues():
			if 'score' in score and 'rec' in score:
				qserv.put_score(score)
				self.response.out.write('%s: %s\n' % (score['name'], score['score']))

		self.redirect('status?msg=success')

class StatusHandler(_BaseHandler):
	@login_required
	def get(self):
		msg = self.request.get('msg', 'unknown')
		if not users.is_current_user_admin():
			msg = 'notadmin'
		msgstrs = {'unknown': 'status unknown',
		           'notadmin': 'you do not have sufficient rights',
		           'fetcherror': 'fetch error while importing',
		           'invalid': 'invalid response from remote qserv',
		           'success': 'success'}
		self.response.out.write("""<html>
  <body>
    <p>%s</p>
		<p><a href="%s">logout</a></p>
  </body>
</html>""" % (msgstrs[msg], users.create_logout_url(self.root)))

