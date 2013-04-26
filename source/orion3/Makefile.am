
lib_LIBRARIES = libOrion3.a

ORION3_SOURCES = \
   CLKCTRL.c \
   INBUF.c \
   NONPR.c \
   orion_router.c \
   OUTBUF.c \
   router_area.c \
   router.c \
   router_power.c \
   SIM_ALU.c \
   SIM_arbiter.c \
   SIM_array_l.c \
   SIM_array_m.c \
   SIM_cam.c \
   SIM_clock.c \
   SIM_crossbar.c \
   SIM_link.c \
   SIM_misc.c \
   SIM_permu.c \
##   SIM_reg.c \
   SIM_router_area.c \
   SIM_router.c \
   SIM_router_power.c \
   SIM_static.c \
   SIM_time.c \
   SIM_util.c \
   SWVC.c \
##   test_alu.c \
   test.c \
##   test_permu.c \
   XBAR.c

libOrion3_a_CXXFLAGS = $(CXX_FLAGS) -O0
libOrion3_a_SOURCES = $(ORION3_SOURCES) -O0

CXXLD = g++ -O0

##Headers
noinst_HEADERS = $(ALL_HDRS)

