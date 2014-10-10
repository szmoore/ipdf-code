#!/usr/bin/python
import sys
import os
import math

# Calculates the total error in coordinates of GPU bounds rectangles of objects

def ComputeError(reference, other):
	if type(reference) == str:
		reference = open(reference, "r")
	if type(other) == str:
		other = open(other, "r")
		
	total = 0.0
	while True:
		a = reference.readline()
		b = other.readline()
		if a == "" and b == "":
			reference.close()
			other.close()
			return total
		if a[0] == '#' and b[0] == '#':
			continue
		a = map(float, a.strip(" \r\n").split("\t"))
		b = map(float, b.strip(" \r\n").split("\t"))
		deltaArea = abs(b[3]*b[4] - a[3]*a[4])
		
		deltaCoord = b[1] - a[1] + b[2] - a[2]
		total += math.sqrt(deltaArea) + abs(deltaCoord)

# Counts the number of unique bounds
def UniqueBounds(other):
	other = open(other, "r")
	store = {}
	for l in other.readlines():
		if l[0] == "#":
			continue
		#print "L is " + str(l)
		l = map(float, l.strip(" \r\n").split("\t"))
		l = tuple(l[1:])
		if not l in store.keys():
			store[l] = 1
		else:
			store[l] += 1
	other.close()
	return len(store.keys())
	
def main(argv):
	print str(ComputeError(argv[1], argv[2])) + "\t" + str(UniqueBounds(argv[1])) + "\t" + str(UniqueBounds(argv[2]))
	return 0
	
	
if __name__ == "__main__":
	sys.exit(main(sys.argv))
