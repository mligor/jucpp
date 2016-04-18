all: libjucpp.a example angularExample mysql-example


libjucpp.a:
	$(MAKE) -C jucpp

example:
	$(MAKE) -C example

angularExample:
	$(MAKE) -C angularExample
	
mysql-example:
	$(MAKE) -C mysql-example

         
clean:
	$(MAKE) -C jucpp clean
	$(MAKE) -C example clean
	$(MAKE) -C angularExample clean
	$(MAKE) -C mysql-example clean

distclean:
	$(MAKE) -C jucpp distclean
	$(MAKE) -C example distclean
	$(MAKE) -C angularExample distclean
	$(MAKE) -C mysql-example distclean

	
