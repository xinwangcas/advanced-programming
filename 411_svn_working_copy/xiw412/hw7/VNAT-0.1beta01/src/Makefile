CC		=	gcc
KCFLAGS		=	-DDEBUG -D__KERNEL__ -DMODULE -DMODVERSIONS \
			-DEXPORT_SYMTAB -Wall -I/usr/src/linux/include \
			-Wstrict-prototypes -O2 -fomit-frame-pointer -pipe \
			-fno-strength-reduce -malign-loops=2 -malign-jumps=2 \
			-malign-functions=2

KVNAT_OBJ	=	vnat.o vnatd.o vnatmap.o vnatproc.o vnatrule.o \
			vnatsock.o vnatvcmp.o misc.o umodecall.o

KVNATVIF_OBJ	=	vnatvif.o

RUNNING_KERNEL	:=	$(shell \
			cat /proc/cmdline | \
			awk '{ n=split($$0, fields); \
				for (i=1; i<=n; i++) print fields[i]; }' | \
			grep BOOT_FILE | \
			cut -d= -f2)

KERNEL_MAP	:=	$(shell \
			echo $(RUNNING_KERNEL) | \
			sed -e 's/vmlinu[zx]\|b*zImage/System\.map/')

KVNAT_SYMS	=	"sysctl_tcp_keepalive_time"

all:		kvnat.o kvnatvif.o

install:	kvnat.o kvnatvif.o
	@mkdir -p /lib/modules/`uname -r`/vnat
	@/bin/cp -f $? /lib/modules/`uname -r`/vnat
	@depmod -a -e

uninstall:
	@rm -f /lib/modules/`uname -r`/vnat/*.o
	@rmdir /lib/modules/`uname -r`/vnat
	@depmod -a -e

vnatsyms.h:
	@rm -f vnatsym.h
	@echo "Resolving symbols:"
	@for s in "$(KVNAT_SYMS)"; do \
		hit=`grep $$s /proc/ksyms`; \
		if [ x"$$hit" != "x" ]; then \
			offset=0x`echo $$hit | cut -d' ' -f1`; \
			echo "#define vnat_$$s $$offset /* /proc/ksyms */"; \
			echo "#define vnat_$$s $$offset /* /proc/ksyms */" >> vnatsyms.h; \
		else \
			hit=`grep $$s $(KERNEL_MAP)`; \
			if [ x"$$hit" != "x" ]; then \
				offset=0x`echo $$hit | cut -d' ' -f1`; \
				echo "#define vnat_$$s $$offset /* $(KERNEL_MAP) */"; \
				echo "#define vnat_$$s $$offset /* $(KERNEL_MAP) */" >> vnatsyms.h; \
			fi \
		fi \
	done

kvnat.o:	$(KVNAT_OBJ)
	ld -m elf_i386  -r -o $@ $^

kvnatvif.o:	$(KVNATVIF_OBJ)
	ld -m elf_i386  -r -o $@ $^

vnat.o:		vnat.c vnatd.h vnatmap.h vnatproc.h vnatsock.h vnatvif.h misc.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatd.o:	vnatd.c vnat.h vnatd.h vnatmap.h vnatrule.h vnatvcmp.h misc.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatmap.o:	vnatmap.c vnatmap.h vnat.h misc.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatproc.o:	vnatproc.c vnatproc.h vnat.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatrule.o:	vnatrule.c vnatrule.h vnat.h vnatmap.h misc.h umodecall.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatsock.o:	vnatsock.c vnatsock.h vnatmap.h vnatrule.h vnatvcmp.h \
		vnatvif.h misc.h umodecall.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatvcmp.o:	vnatvcmp.c vnatvcmp.h vnatmap.h vnatrule.h vnatsyms.h
	$(CC) $(KCFLAGS) -c -o $@ $<

vnatvif.o:	vnatvif.c vnatvif.h misc.h
	$(CC) $(KCFLAGS) -c -o $@ $<

misc.o:	misc.c misc.h
	$(CC) $(KCFLAGS) -c -o $@ $<

umodecall.o:	umodecall.c umodecall.h
	$(CC) $(KCFLAGS) -c -o $@ $<

clean:
	@rm -f a.out core vnatsyms.h *.o *.so *~ \
		/lib/modules/`uname -r`/vnat/*.o
