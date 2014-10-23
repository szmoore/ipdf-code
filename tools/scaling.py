#!/usr/bin/python -u

#much copy paste such python

import sys
import os
from matplotlib.pyplot import *
from numpy import *
import subprocess
import time

import gpubounds

def FixedScales(binname, x0=0, y0=0, w0=1, h0=1, s=0.5, steps=100, xz = 0.5, yz = 0.5, testsvg="svg-tests/grid.svg", renderer="gpu", fps=1):
	accuracy = []
	performance = []
	n = open("/dev/null", "w")
	#n = sys.stderr
	try:
		p = subprocess.Popen(binname + " -s stdin", bufsize=0, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=n, shell=True)
		p.stdin.write("%s\n" % renderer)
		p.stdin.write("setbounds %s %s %s %s\n" % (str(x0),str(y0),str(w0),str(h0)))
		p.stdin.write("loadsvg %s\n" % testsvg)
		p.stdin.write("querygpubounds original.dat\n")
		p.stdin.write("screenshot original.bmp\n")
	except Exception, e:
		print "%s - Couldn't start - %s" % (binname, str(e))
		return {"accuracy" : asarray(accuracy), \
			"performance" : asarray(performance)}
	
	for i in xrange(steps):
		try:
			start_time = time.time()
			p.stdin.write("clear\n")
			p.stdin.write("loop 1 zoom %s %s %s\n" % (str(xz), str(yz), str(s)))
			p.stdin.write("loadsvg %s\n" % testsvg)
			p.stdin.write("querygpubounds step%d.dat\n" % i)
		#while not os.path.isfile("step%d.dat" % i):
		#	pass
			p.stdin.write("loop %d printspf\n" % fps) # Print an FPS count to signal it is safe to read the file
			fpsout = p.stdout.readline().strip(" \r\n").split("\t")
		#print(str(fpsout))
			p.stdin.write("printbounds\n")
			bounds = p.stdout.readline().strip(" \r\n").split("\t")
			try:
				bounds = map(float, bounds)
			except:
				pass
		
			performance += [map(float, fpsout) + [time.time() - start_time]]
			accuracy += [bounds + [gpubounds.ComputeError("original.dat", "step%d.dat" % i), \
				 gpubounds.UniqueBounds("step%d.dat" % i)]]
				 
			os.unlink("step%d.dat" % i) # Don't need it any more
		#print accuracy[-1][-1]
			if accuracy[-1][-1] <= 1:
				print "%s - Quit early after %d steps - No precision left" % (binname, i)
				break
			if performance[-1][-1] > 60:
				print "%s - Quit early after %d steps - Took too long to render frames" % (binname, i)
				break
		except Exception, e:
			print "%s - Quit early after %d steps - Exception %s" % (binname, i, str(e))
			break
	
	try:
		p.stdin.write("screenshot final.bmp\n")
		p.stdin.write("quit\n")
		p.stdin.close()
	except Exception, e:
		print "%s - Couldn't exit - %s" % (binname, str(e))
		
	return {"accuracy" : asarray(accuracy),
			"performance" : asarray(performance)}


def TestInvariance(binname, x0=0, y0=0, w0=1, h0=1, s=-1, steps=1000, xz = 400, yz = 300, testsvg="svg-tests/grid.svg", renderer="gpu", fps=1):
	accuracy = []
	performance = []
	n = open("/dev/null", "w")
	#n = sys.stderr
	try:
		p = subprocess.Popen(binname + " -s stdin", bufsize=0, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=n, shell=True)
		p.stdin.write("%s\n" % renderer)
		p.stdin.write("setbounds %s %s %s %s\n" % (str(x0),str(y0),str(w0),str(h0)))
		p.stdin.write("loadsvg %s\n" % testsvg)
		p.stdin.write("querygpubounds original.dat\n")
		p.stdin.write("screenshot original.bmp\n")
		p.stdin.write("printbounds\n")
		bounds_orig = map(float, p.stdout.readline().strip(" \r\n").split("\t"))
	except Exception, e:
		print "%s - Couldn't start - %s" % (binname, str(e))
		return {"accuracy" : asarray(accuracy),
			"performance" : asarray(performance)}
	
	for i in xrange(1,steps,50):
		try:
			start_time = time.time()
			p.stdin.write("loop %d pxzoom %s %s %s\n" % (i, str(xz), str(yz), str(int(s))))
			p.stdin.write("printbounds\n")
			bounds = map(float, p.stdout.readline().strip(" \r\n").split("\t"))
			p.stdin.write("loop %d pxzoom %s %s %s\n" % (i, str(xz), str(yz), str(-int(s))))
			p.stdin.write("querygpubounds step%d.dat\n" % i)
			while not os.path.isfile("step%d.dat" % i):
				pass
			p.stdin.write("loop %d printspf\n" % fps) # Print an FPS count to signal it is safe to read the file
			fpsout = p.stdout.readline().strip(" \r\n").split("\t")
			#print(str(fpsout))
		
		
			bounds[0] = bounds[0]-bounds_orig[0]
			bounds[1] = bounds[1]-bounds_orig[1]
			bounds[2] = bounds[2]/bounds_orig[2]
			bounds[3] = bounds[3]/bounds_orig[3]
			performance += [map(float, fpsout) + [time.time() - start_time]]
			accuracy += [bounds + [gpubounds.ComputeError("original.dat", "step%d.dat" % i), \
				 gpubounds.UniqueBounds("step%d.dat" % i)]]
				 
			os.unlink("step%d.dat" % i) # Don't need it any more
			if accuracy[-1][-1] <= 10:
				print "%s - Quit early after %d steps - No precision left" % (binname, i)
				break
			if performance[-1][-1] > 60:
				print "%s - Quit early after %d steps - Took too long to render frames" % (binname, i)
				break
		except Exception, e:
			print "%s - Quit early after %d steps - %s" % (binname, i, str(e))
			
	try:
		p.stdin.write("screenshot final.bmp\n")
		p.stdin.write("quit\n")
		p.stdin.close()
	except Exception, e:
		print "%s - Couldn't exit - %s" % (binname, str(e))
	return {"accuracy" : asarray(accuracy),
			"performance" : asarray(performance)}

if __name__ == "__main__":
	results = Scaling("./single", xz=0, yz=0, fps=100)

	
