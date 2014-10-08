#!/usr/bin/python

w = 800
h = 600
step = 1
print '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n \
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN"\n \
"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">\n \
<svg xmlns="http://www.w3.org/2000/svg"\n \
     xmlns:xlink="http://www.w3.org/1999/xlink"\n \
     width="%d" height="%d">\n' % (w,h)
     
print "\t<path"
print "\t\td=\""
for y in xrange(0,h+step,step):
	print "\t\t\tM 0,%d L %d,%d" % (y,w,y)

print "\t\t\" style=\"stroke-width:1px; stroke:black;\"/>"

print "\t<path"
print "\t\td=\""
for x in xrange(0,w+step,step):
	print "\t\t\tM %d,0 L %d,%d" % (x,x,h)

print "\t\t\" style=\"stroke:#aaaaff;stroke-width:1px; fill:none;\"/>"

print "</svg>"
