DOXYGEN	=	doxygen
SRCPATH	=	../src
SRCDOC	=	$(SRCPATH)/vnat.c $(SRCPATH)/vnatd.c

man:	vnat.dox $(SRCDOC)
	$(DOXYGEN) $<

clean:
	@rm -rf man8