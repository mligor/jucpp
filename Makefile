all: jucpp.a hello_http


jucpp.a:
	$(MAKE) -C jucpp

example:
	$(MAKE) -C example
         
clean:
	$(MAKE) -C jucpp clean
	$(MAKE) -C example clean

distclean:
	$(MAKE) -C jucpp distclean
	$(MAKE) -C example distclean

	
