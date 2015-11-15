all: proxy

proxy: proxy.c
	gcc -L/usr/local/Cellar/openssl/1.0.2d_1/lib -lssl -lcrypto proxy.c -o proxy

.PHONY: clean

clean:
	rm -f proxy

