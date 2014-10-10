#!/usr/bin/python

from common import *

import subprocess

def Build(real_type, quadtree=False, controlpanel=False):
    global options
    real_name = ""
    if (type(real_type) == str):
        quadtree = "enabled" if (real_type.split("-")[-1] == "qtree") else quadtree
        real_type = real_type.split("-")[0]
        real_name = real_type
        real_type = options["real_names"].index(real_type)
    else:
        real_name = options["real_names"][real_type]
        
    quadtree = "enabled" if quadtree else "disabled"
    controlpanel = "enabled" if controlpanel else "disabled"
    if (os.system("make -C %s clean" % options["ipdf_src"]) != 0):
        raise Exception("Make clean failed.")
    if (os.system("make -C %s REALTYPE=%d QUADTREE=%s CONTROLPANEL=%s" % (options["ipdf_src"], real_type, quadtree, controlpanel)) != 0):
        raise Exception("Make failed.")
        
    q = "-qtree" if quadtree == "enabled" else ""
    os.rename(options["ipdf_bin"], options["local_bin"]+real_name+q)


def BuildAll():
	p = ProgressBar(len(options["tobuild"]))
	print("Building: %s" % str(options["tobuild"]))
	p.animate(0)
	for (i,b) in enumerate(options["tobuild"]): #options["real_names"]:
		if b in options["ignore"]:
			continue
		try:
			Build(b, False, False)
			options["built"] += [b]
		except:
			display("Failed to build %s" % b)
		p.animate(i+1)

if __name__ == "__main__":
	BuildAll()
