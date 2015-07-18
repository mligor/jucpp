all: jucpp.a hello_http


jucpp.a:
	$(MAKE) -C jucpp

hello_http:
	$(MAKE) -C test1
         
clean:
	$(MAKE) -C jucpp clean
	$(MAKE) -C test1 clean

distclean:
	$(MAKE) -C jucpp distclean
	$(MAKE) -C test1 distclean

	
