#!/usr/bin/python

from common import *

import subprocess

def Build(binname):
	global options
	
	tokens = binname.split("-")
	transformations = "direct"
	mpfr_prec = 23
	if tokens[0] == "path":
		transformations = "path"
		tokens = tokens[1:]
	elif tokens[0] == "cumul":
		transformations = "cumulative"
		tokens = tokens[1:]

	realname = tokens[0]
	realtype = options["real_names"].index(realname)
	
	mainreal = 0
	pathreal = 0
	if transformations == "direct":
		mainreal = realtype
	else:
		pathreal = realtype # hackky.
		realtype = 1
	
	if realname == "mpfr":
		mpfr_prec = int(tokens[1])
	
	quadtree = "disabled"
	if "qtree" in tokens:
		quadtree = "enabled"
	
	
	if (os.system("make -C %s clean; make -C %s REALTYPE=%d MPFR_PRECISION=%d QUADTREE=%s CONTROLPANEL=disabled TRANSFORMATIONS=%s PATHREAL=%d" % (options["ipdf_src"], options["ipdf_src"], mainreal, mpfr_prec, quadtree, transformations, pathreal)) != 0):
		raise Exception("Make failed.")

	os.rename(options["ipdf_bin"], options["local_bin"]+binname)


def BuildAll():
	p = ProgressBar(len(options["tobuild"]))
	print("Building: %s" % str(options["tobuild"]))
	p.animate(0)
	for (i,b) in enumerate(options["tobuild"]): #options["real_names"]:
		if b in options["ignore"]:
			continue
		#try:
		Build(b)
		options["built"] += [b]
		#except:
		#	display("Failed to build %s" % b)
		p.animate(i+1)

if __name__ == "__main__":
	BuildAll()
