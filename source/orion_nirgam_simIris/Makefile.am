
lib_LIBRARIES = libOrion.a

ORION_SOURCES = \
   SIM_router.cpp SIM_arbiter.cpp SIM_crossbar.cpp \
   SIM_router_power.cpp \
   SIM_link.cpp \
   SIM_clock.cpp \
   SIM_router_area.cpp \
   SIM_router.cpp \
   SIM_array_l.cpp SIM_array_m.cpp SIM_cam.cpp \
   SIM_ALU.cpp \
   SIM_misc.cpp SIM_permu.cpp \
   SIM_static.cpp \
   SIM_util.cpp SIM_time.cpp \
   orion_router_power.cpp orion_router_area.cpp

libOrion_a_CXXFLAGS = $(CXX_FLAGS) -O0
libOrion_a_SOURCES = $(ORION_SOURCES) -O0

CXXLD = g++ -O0

##Headers
noinst_HEADERS = $(ALL_HDRS)

