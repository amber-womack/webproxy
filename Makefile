all: webproxy

dfc: webproxy.c
	gcc webproxy.c -o webproxy

.PHONY: clean

clean:
	rm -f webproxy

