
import datetime
import logging
import pickle
import time

from google.appengine.ext import webapp

import models

def set_param(params, key, value):
	path = key.split('/')
	dir = params
	file = path.pop()

	while len(path):
		item = path.pop(0)
		if item not in dir:
			dir[item] = {}
		dir = dir[item]

	dir[file] = value

def format_params(root, params):
	output = ''

	for (key, value) in params.iteritems():
		if isinstance(value, dict):
			output += format_params(root + '/' + key, value)
		else:
			output += root + '/' + key + ' ' + str(value) + '\n'

	return output

def put_score(params):
	score = int(params['score'])

	item = models.Score.get_by_key_name('score:' + str(score))
	if item:
		try:
			data = pickle.loads(item.data)
			if data['rec'] != params['rec']:
				item = None
		except:
			logging.error('error unpickling score %d, will overwrite' % score)
			item = None

	if not item:
		# We scrub a few bits of informations before storing it.
		score_data = params.copy()
		score_data.pop('num', None)
		score_data.pop('port', None)
		if 'info' in score_data:
			score_data['info'] = params['info'].copy()
			score_data['info'].pop('remoteaddr', None)
		item = models.Score(key_name='score:' + str(score),
		                    score=score, data=pickle.dumps(score_data))
		item.put()

class QServHandler(webapp.RequestHandler):
	# Default number of scores/demos to send on gethighscores
	default_demos = 5
	# Number of scores kept in our store
	scores_to_keep = 100
	# Timeout before expiring games
	timeout = datetime.timedelta(0, 180)

	def get(self):
		return self.process()
		
	def post(self):
		return self.process()

	def postdemo(self):
		if 'score' in self.params and 'rec' in self.params:
			put_score(self.params)

		self.gethighscores()

	def get_scores(self):
		query = models.Score.all()

		scores_entities = {}
		for score in query:
			scores_entities[score.score] = score

		scores_list = scores_entities.keys()
		scores_list.sort(None, None, True)

		if len(scores_list) > self.scores_to_keep:
			old_scores = scores_list[self.scores_to_keep:]
			del scores_list[self.scores_to_keep:]

			for score in old_scores:
				logging.info('deleting score ' + str(score))
				scores_entities[score].delete()

		scores = {}
		for score in scores_list:
			scores[score] = pickle.loads(scores_entities[score].data)

		return scores

	def gethighscores(self):
		scores = self.get_scores()
		num = self.params.get('num', self.default_demos)
		scores_list = scores.keys()
		scores_list.sort(None, None, True)

		if len(scores_list) > num:
			del scores_list[num:]

		self.response.out.write('Ok\n')
		high = 0
		for score in scores_list:
			self.response.out.write(format_params('high' + str(high).zfill(3),
																						scores[score]))
			high += 1

	def postgame(self):
		key = self.params['info']['remoteaddr'] + ':' + str(self.params['port'])

		game = models.Game(key_name='game:' + key,
		                   addr=self.params['info']['remoteaddr'],
		                   port=int(self.params['port']),
		                   data=pickle.dumps(self.params))
		game.put()

		self.response.out.write('Game updated\n')

	def getgames(self):
		games = {}
		expiration = datetime.datetime.utcnow() - self.timeout

		for game in models.Game.all():
			if game.timestamp < expiration:
				game.delete()
				continue
			games[(game.addr, game.port)] = pickle.loads(game.data)
			set_param(games[(game.addr, game.port)], 'info/lastupdate',
			          int(time.mktime(game.timestamp.timetuple())))

		strip_players = False
		strip_port = False
		if 'info' in self.params:
			if 'quadra_version' in self.params['info']:
				if self.params['info']['quadra_version'] == '1.1.2':
					strip_players = True
			else:
				if 'qsnoop_version' not in self.params['info']:
					strip_port = True

		self.response.out.write('Current games\n')
		seen = {}
		for addr, info in games.iteritems():
			key = addr[0]
			if not strip_port:
				key += ':' + str(addr[1])
			if key in seen:
				continue
			seen[key] = True

			if strip_players:
				info.pop('players', None)

			self.response.out.write(format_params(key, info))

	def process(self):
		self.params = {}

		lines = self.request.get('data').splitlines()
		if not len(lines):
			lines = ['nothing']

		cmd = lines.pop(0)
		for line in lines:
			(key, value) = line.split(None, 1)
			set_param(self.params, key, value)

		try:
			if int(self.params['port']) > 65535:
				raise ValueError
		except:
			self.params['port'] = 3456

		set_param(self.params, 'info/remoteaddr', self.request.remote_addr)

		logging.info('command: ' + cmd)
		self.response.headers['Content-Type'] = 'text/plain'
		if cmd == 'postdemo':
			self.postdemo()
		elif cmd == 'gethighscores':
			self.gethighscores()
		elif cmd == 'postgame':
			self.postgame()
		elif cmd == 'getgames':
			self.getgames()
		else:
			self.response.set_status(500)
			self.response.out.write('Hi, I\'m the Python Quadra game server.\n'
															'You should use Quadra to talk to me :).\n')
