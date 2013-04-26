
lib_LIBRARIES = libOrion.a

ORION_SOURCES = \
   SIM_router.c SIM_arbiter.c SIM_crossbar.c \
   SIM_router_power.c \
   SIM_link.c \
   SIM_clock.c \
   SIM_router_area.c \
   SIM_router.c \
   SIM_array_l.c SIM_array_m.c SIM_cam.c \
   SIM_ALU.c \
   SIM_misc.c SIM_permu.c \
   SIM_static.c \
   SIM_util.c SIM_time.c \
   orion_router_power.c orion_router_area.c

libOrion_a_CXXFLAGS = $(CXX_FLAGS) -O0
libOrion_a_SOURCES = $(ORION_SOURCES) -O0

CXXLD = gcc -O0

##Headers
noinst_HEADERS = $(ALL_HDRS)

