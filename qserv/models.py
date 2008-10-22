
from google.appengine.ext import db

class Game(db.Model):
	pass

class Score(db.Model):
	score = db.IntegerProperty(required=True)
	data = db.TextProperty(required=True)
	timestamp = db.DateTimeProperty(auto_now=True)
