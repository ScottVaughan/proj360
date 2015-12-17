#!/usr/bin/env python

import re
print "Content-type: text/html"
print
print "<h1>Grades</h1>"

fileIn = open('../grades.txt', "r")
while True:
	text = fileIn.readline()
	for line in text:
		if "#" in line:
			print "<h2>"
			print text
			print "</h2>"
		else:
			print "<br>"
			print text
			print "</br>"

