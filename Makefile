all: libjucpp.a example angularExample


libjucpp.a:
	$(MAKE) -C jucpp

example:
	$(MAKE) -C example

angularExample:
	$(MAKE) -C angularExample

         
clean:
	$(MAKE) -C jucpp clean
	$(MAKE) -C example clean
	$(MAKE) -C angularExample clean

distclean:
	$(MAKE) -C jucpp distclean
	$(MAKE) -C example distclean
	$(MAKE) -C angularExample distclean

	
