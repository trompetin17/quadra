
from google.appengine.ext import db

class Game(db.Model):
	addr = db.StringProperty(required=True)
	port = db.IntegerProperty()
	data = db.BlobProperty(required=True)
	timestamp = db.DateTimeProperty(auto_now=True,required=True)

class Score(db.Model):
	score = db.IntegerProperty(required=True)
	data = db.BlobProperty(required=True)
	timestamp = db.DateTimeProperty(auto_now=True)
