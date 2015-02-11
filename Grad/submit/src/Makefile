# This make file has some specific targets args, time, and
# the following standard targets:
# all - build args, time
# clean - remove products of compilations e.g. executables, .o files ...
# test - run some executables. Note this is not really testing.
# submit - create the submission tarfile in the parent directory
all: grad

grad: grad.c
	gcc grad.c -o grad -lm

clean:
	-rm grad *.o grad.out

# Ignore the stuff below this line.
# We will discuss it in class Wednesday Jan 22.
submit: clean
	cd ../..;  tar cvfz Project00.tgz proj00
	echo "tarfile built in parent directory, now submit it to dropbox.cse.sc.edu"

