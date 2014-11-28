import sys
import os

def program_name(f):
	return os.path.splitext(os.path.basename(f.name))[0]

def transform_program(f):
	code = ""
	for line in f:
		code += line.replace("def main()", "def _" + program_name(f) + "_main()")
	return code

def make_os(f, programs):
	code = ""
	for line in f:
		if "@INSERT_PROGRAMS" in line:
			code += "/* PROGRAMS */\n"
			for p in programs:
				code += "vector_push(program_names, \"{0}\");\n".format(p)
				code += "vector_push(program_locations, :{0});\n".format("_" + p + "_main")
		else:
			code += line
	return code

def main():
	programs = []
	with open("stdlib.txt") as f:
		for line in f:
			sys.stdout.write(line)
	for i in range(1, len(sys.argv)):
		with open(sys.argv[i]) as f:
			code = transform_program(f)
			programs.append(program_name(f))
			print(code)
	with open("os.txt") as f:
		code = make_os(f, programs)
		print(code)

if __name__ == '__main__':
	main()
