
SUBDIRS = recorder

messages:
	for subdir in $(SUBDIRS); do \
	  echo "Making messages in $$subdir"; \
	  (cd $$subdir && $(MAKE) -f Makefile.am messages) \
	done
