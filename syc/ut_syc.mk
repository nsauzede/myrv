SYCR:=.ut/.3p/systemc-3.0.1
SYC:=$(SYCR)/the_install
SYCI:=$(SYC)/include
SYCL:=$(SYC)/`cat $(SYCR)/config.log|grep "Libraries"|cut -d">" -f2`

CXXFLAGS:=-I$(SYCI)
LDLIBS:=-Wl,--defsym=sc_main=0 -L$(SYCL) -lsystemc
LD_LIBRARY_PATH:=$(SYCL)
VGO:=--suppressions=ut_systemc.supp --gen-suppressions=all
