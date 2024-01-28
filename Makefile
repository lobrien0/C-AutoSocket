

# Default target
all:
	cd examples/client
	make 
	cd ../server
	make
	cd ../..
	cp examples/client/clientAS ./
	cp examples/server/serverAS ./


# Clean rule
clean:
	rm -f clientAD serverAS
