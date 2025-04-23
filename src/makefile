create: detect_dups.h
	gcc detect_dups.c -lcrypto -o detect_dups; ./detect_dups test1

test5: 
	gcc detect_dups.c -lcrypto -o detect_dups; ./detect_dups test5

test: 
	gcc nftw_test.c -o nftw_test; ./nftw_test test1

clean: 
	rm detect_dups

.PHONY: test5