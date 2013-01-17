for i in [20, 50]:
	for case in ["singleMsg", "nSenders", "singleSender"]:
		f = open("in_%d_%s.cfg" % (i, case), "w")
		f.write(str(i) + "\n")
		for j in range(i):
			if case=="singleMsg" and j==0:
				f.write("1 0\n")
			elif case=="nSenders":
				f.write("1 0\n")
			elif case=="singleSender" and j==0:
				f.write(str(i) + " ")
				for k in range(i):
					f.write(str(k) + " ")
				f.write("\n")
			else:
				f.write("0\n")
		f.close()
