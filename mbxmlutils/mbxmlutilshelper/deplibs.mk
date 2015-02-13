# Note: with automake 1.14 we can use %reldir% instead of $(prefix)/share/mbxmlutils/python!
deplibs.target:
	echo "Create dependency file(s) (only if python is found) for $(lib_LTLIBRARIES)"
	if which python &> /dev/null; then \
	  for lib_la in $(lib_LTLIBRARIES); do \
	    . ./$$lib_la; \
	    if test -n "$$dlname"; then \
	      if test $$libdir/$$dlname -nt $$libdir/$$dlname.deplibs; then \
	        echo "Create dependency files for $$libdir/$$dlname"; \
	        python $(prefix)/share/mbxmlutils/python/deplibs.py $$libdir/$$dlname > $$libdir/$$dlname.deplibs || exit; \
	      fi; \
	    fi; \
	  done; \
	fi