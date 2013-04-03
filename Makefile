default: clean lib rpm


send-test:
	gcc -Wall -pedantic -lcap -o icmp-send  caplib.c icmp-send-test.c

recv-test:
	gcc -Wall -pedantic -lcap -o icmp-receive  caplib.c icmp-receive-test.c

lib:
	gcc -Wall -pedantic -lcap -c -fPIC -o caplib.o caplib.c
	gcc -Wall -pedantic -lcap -c -fPIC -o icmplib.o icmplib.c
	# perhaps not the best way to named the shared library
	# but we don't use the shared library anyway, so...
	gcc -shared -Wl,-soname,libtinyicmp.so -o libtinyicmp.so icmplib.o caplib.o
	python setup.py build_ext --inplace

rpm:
	python setup-dist.py bdist --format=rpm

clean:
	python setup.py clean
	rm -f *.{o,so}
	rm -f RplIcmp.c

all: default recv-test send-test
