#!/usr/bin/env python
import argparse
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description="Plot motion profile from robot log")
parser.add_argument("log_path", help="path to the robot log file")

log_file = open(parser.parse_args().log_path, "r")
log = log_file.read()
log_file.close()

separator = "********** Robot program starting **********\n"
log = log[log.find(separator) + len(separator):]
lines = log.split("\n")

data = []
for line in lines:
	if (len(line) > 0):
        	halves = line.split(" / ")
		time = float(halves[0])
		distances = [float(n) for n in halves[1].split(":")]
		outputs = [float(n) for n in halves[2].split(":")]
		error = float(halves[3])
        	data.append({"t": time, "dist_l": distances[0], "dist_r": distances[1], "out_l": outputs[0], "out_r": outputs[1], "error": error})

plt.figure(1)

plt.subplot(221)
plt.plot([d["t"] for d in data], [d["out_l"] for d in data])
plt.xlabel("time (s)")
plt.ylabel("left output")

plt.subplot(222)
plt.plot([d["t"] for d in data], [d["out_r"] for d in data])
plt.xlabel("time (s)")
plt.ylabel("right output")

plt.subplot(223)
plt.plot([d["t"] for d in data], [d["dist_l"] for d in data])
plt.xlabel("time (s)")
plt.ylabel("left distance (in)")

plt.subplot(224)
plt.plot([d["t"] for d in data], [d["dist_r"] for d in data])
plt.xlabel("time (s)")
plt.ylabel("right distance (in)")

plt.subplots_adjust(wspace=.5, hspace=.5)
plt.suptitle("Log Values")

plt.figure(2)

plt.subplot(111)
plt.plot([d["t"] for d in data], [d["dist_l"] - d["dist_r"] for d in data])
plt.xlabel("time (s)")
plt.ylabel("distance difference (in)")

plt.suptitle("Log Values")

plt.figure(3)

plt.subplot(111)
plt.plot([d["t"] for d in data], [d["error"] for d in data])
plt.xlabel("time (s)")
plt.ylabel("error")

plt.suptitle("Log Values")
plt.show()
