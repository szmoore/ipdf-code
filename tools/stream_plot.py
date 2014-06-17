#!/usr/bin/python -u

# Script to plot data in real(ish) time
# Naive approach is to just read from stdin and plot each point as it arrives
# This isn't so great if you have a fast data rate, the plot window flashes and gets out of sync
# Improvement is to use a thread to read from stdin, and a thread (main thread) that periodically plots the data (set the time interval to be sufficiently slow)

import os
import sys
import select
import threading
import time
import Gnuplot, Gnuplot.funcutils

plt = Gnuplot.Gnuplot(persist=1)
data = []
semaphore = threading.Semaphore()
running = True

def ProcessData(points):
	#TODO: Alter to change data as it is read
	global data
	if len(data) > 0:
		# Store finite differences
		points += [points[i]-data[-1][i] for i in xrange(len(points))]
	else:
		points += [0 for _ in xrange(len(points))]
	return points

def PlotData():
	#TODO: Alter to change plotting of data
	global data
	data = data[-100:]

	plt.plot(Gnuplot.Data(data, with_="lp",using="2:($8/$9)", title="[Real] clock_gettime()"))
	plt.replot(Gnuplot.Data(data, with_="lp",using="2:($8/$10)", title="[CPU] SDL_GetPerformanceCounter()"))
	plt.replot(Gnuplot.Data(data, with_="lp",using="2:($8/$11)", title="[GPU] OpenGL Query (GL_TIME_ELAPSED)"))

# Produces data points by reading from stdin
def Producer():
	global data
	global running
	while running:
		line = sys.stdin.readline()
		line = line.strip(" \t\r\n")
		if line == "":
			continue
		semaphore.acquire()
		try:
			data.append(ProcessData(map(float, line.split())))
		except:
			pass
		semaphore.release()

# "Consumes" data points by plotting them. Not really a consumer because it doesn't actually clear any but whatever
def Consumer():
	global data
	global running
	data_size = len(data)
	while running:
		prev_size = data_size
		data_size = len(data)
		if data_size != prev_size and data_size >= 2:
			semaphore.acquire()
			PlotData()
			data_size = len(data)
			semaphore.release()
			
		time.sleep(1)
	
# Setup plot and start threads
def main(argv):
	global running
	plt.title("Performance Graph")
	plt.xlabel("Real Time (s)")
	plt.ylabel("Average FPS (since last data point)")
	producer = threading.Thread(target=Producer)
	producer.start()
	try:
		Consumer() # Can run plot in main thread, easier that way
	except KeyboardInterrupt:
		sys.stderr.write("Press enter to exit.\n")
		pass
	running = False
	producer.join()

if __name__ == "__main__":
	main(sys.argv)
	
