import requests

print "Content-type: text/html"
print
print "<h1>News</h1>"
r = requests.get('http://news.google.com')
#creates an image from binary data returned by a request
from PIL import Image
from StringIO import StringIO
i = Image.open(StringIO(r.content))

print "<h1>Headlines</h1>"
css_soup.find_all("a", {"class":"titletext"})
