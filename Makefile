all: proxy

dfc: proxy.c
	gcc proxy.c -o proxy

.PHONY: clean

clean:
	rm -f proxy

