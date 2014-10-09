#!/usr/bin/python -u

import sys
import os
from pylab import *
import subprocess
import time

import gpubounds_error

def grid_scaling(binname, x0, y0, w0, h0, s, steps=100,testsvg="svg-tests/grid.svg"):
	data = []
	n = open("/dev/null", "w")
	p = subprocess.Popen(binname + " -s stdin", bufsize=0, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=n, shell=True)
	p.stdin.write("setbounds %s %s %s %s\n" % (str(x0),str(y0),str(w0),str(h0)))
	p.stdin.write("loadsvg %s\n" % testsvg)
	p.stdin.write("querygpubounds original.dat\n")
	p.stdin.write("screenshot original.bmp\n")
	for i in xrange(steps):
		p.stdin.write("clear\n")
		p.stdin.write("loop 1 zoom 0.5 0.5 %s\n" % str(s))
		p.stdin.write("loadsvg %s\n" % testsvg)
		p.stdin.write("querygpubounds step%d.dat\n" % i)
		while not os.path.isfile("step%d.dat" % i):
			pass
		p.stdin.write("clearperf\n")
		p.stdin.write("loop 10 wait\n")
		p.stdin.write("recordperf\n")
		p.stdin.write("printperf\n")
		perf = p.stdout.readline()
		time.sleep(0.5)
		data += [gpubounds_error.ComputeError("original.dat", "step%d.dat" % i)]
		#data += [gpubounds_error.UniqueBounds("step%d.dat" % i)]
	
	
	print "Quit"
	p.stdin.write("screenshot final.bmp\n")
	p.stdin.write("quit\n")
	p.stdin.close()
	
	data = asarray(data)
	plot(data)
	show(block=True)
	

if __name__ == "__main__":
	binname = "../src/ipdf"
	if len(sys.argv) > 1:
		binname = sys.argv[1]
	grid_scaling(binname, 0.5, 0.5, 1, 1, 0.5)
