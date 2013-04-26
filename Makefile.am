bin_PROGRAMS = manifold_simIris

OBJDIR = $(TOPDIR)/build
BUILDDIR = $(TOPDIR)/build
#CXX_FLAGS = -D_FILE_OFFSET_BITS=64 -I source -m32 -ansi -g -gstabs+ -O0 -static-libgcc
CXX_FLAGS = -D_FILE_OFFSET_BITS=64 -I source -ansi -g -gstabs+ -O0 -static-libgcc
ZESTO_FLAGS = -O0 -g -m32 -DMIN_SYSCALL_MODE -DUSE_SSE_MOVE -Wall -DDEBUG -msse4a -mfpmath=sse
MLIBS  = -lm
ZESTOCXXLD = g++ -lc -lm
CXXLD = g++ -lc -lm -ldl -lpthread -O0
CXXLD_PROFILE = g++ -lgcov -lc -O0
#DEBUG_FLAGS = -D_DEBUG -D_DEBUG_ROUTER
DEBUG_FLAGS = -g -pg -D_STLP_DEBUG -D_DEBUG -DDEBUG -D_DEBUG_ROUTER -D_DEBUG_INTERFACE -DDEBUG
PROFILE_FLAGS = -fprofile-arcs -ftest-coverage -lgcov -g -pg
FE_SOURCES = \
	     source/frontend/impl/mesh.cc \
	     source/frontend/impl/torus.cc \
	     source/frontend/impl/ring.cc \
	     source/frontend/impl/visual.cc

##source/util/mc_constants.h \

#SUBDIRS = source/zesto source/memctrl source/simIris .
#SUBDIRS = source/simIris source/orion source/orion3 .
SUBDIRS = source/simIris source/orion_macsim source/orion source/orion3 .

ALL_HDRS = \
	   source/kernel/simulator.h \
	   source/kernel/link.h \
	   source/kernel/component.h \
	   source/util/genericData.h \
	   source/util/simIrisComponentHeader.h \
	   source/simIris/data_types/impl/irisEvent.h \
	   source/simIris/data_types/impl/flit.h \
	   source/simIris/data_types/impl/highLevelPacket.h \
	   source/simIris/data_types/impl/lowLevelPacket.h \
	   source/util/stats.h \
	   source/simIris/components/interfaces/crossbar.h \
	   source/simIris/components/interfaces/addressDecoder.h \
	   source/simIris/components/interfaces/interface.h \
	   source/simIris/components/interfaces/buffer.h \
	   source/simIris/components/interfaces/outputBuffer.h \
	   source/simIris/components/interfaces/virtualChannelArbiter.h \
	   source/simIris/components/interfaces/router.h \
	   source/simIris/components/interfaces/portArbiter.h \
	   source/simIris/components/interfaces/processor.h \
	   source/simIris/components/interfaces/irisLink.h \
	   source/simIris/components/interfaces/networkComponent.h \
	   source/simIris/components/impl/genericRC.h \
	   source/simIris/components/impl/genericCrossbar.h \
	   source/simIris/components/impl/genericBuffer.h \
	   source/simIris/components/impl/genericInterfaceNB.h \
	   source/simIris/components/impl/genericInterfaceNBb.h \
	   source/simIris/components/impl/genericInterfaceNBm.h \
	   source/simIris/components/impl/genericInterfaceNBVn.h \
	   source/simIris/components/impl/genericInterfaceNBVs.h \
	   source/simIris/components/impl/genericInterfaceNBpr.h \
	   source/simIris/components/impl/genericArbiter.h \
	   source/simIris/components/impl/genericLink.h \
	   source/simIris/components/impl/pvtopv_swa.h \
	   source/simIris/components/impl/ptop_swa.h \
	   source/simIris/components/impl/ptopSwaVcs.h \
	   source/simIris/components/impl/vcaMsgAware.h \
	   source/simIris/components/impl/genericRouter4Stg.h \
	   source/simIris/components/impl/routerVcMP.h \
	   source/simIris/components/impl/routerVcMPm.h \
	   source/simIris/components/impl/rtVcMPmDBAR.h \
	   source/simIris/components/impl/rtRCA1D.h \
	   source/simIris/components/impl/rtVBDBAR.h \
	   source/simIris/components/impl/rtSPDBAR.h \
	   source/simIris/components/impl/rtPRDBAR.h \
	   source/simIris/components/impl/rtPGCP.h \
	   source/simIris/components/impl/rtPKTSP.h \
	   source/simIris/components/impl/rtPKTSPa.h \
	   source/simIris/components/impl/rtPKTSPb.h \
	   source/simIris/components/impl/rtPKTSPtr.h \
	   source/simIris/components/impl/rtPKTPR.h \
	   source/simIris/components/impl/rtPKTPRtr.h \
	   source/simIris/components/impl/routerVcMPft.h \
	   source/simIris/components/impl/variantA.h \
	   source/simIris/components/impl/variantB.h \
	   source/simIris/components/impl/routerReqReply.h \
	   source/simIris/components/impl/genericRouterPhy.h \
	   source/simIris/components/impl/customPktGenVn.h \
	   source/simIris/components/impl/genericPktGen.h \
	   source/simIris/components/impl/genericPktGenVn.h \
	   source/simIris/components/impl/genericSink.h \
	   source/simIris/components/impl/genericVcAllocator.h \
	   source/simIris/components/impl/genericVca.h \
	   source/frontend/impl/mesh.h \
	   source/memctrl/addr_map.h \
	   source/memctrl/request.h \
	   source/memctrl/request_handler.h \
	   source/memctrl/channel_handler.h \
	   source/memctrl/rank_handler.h \
	   source/memctrl/bank_handler.h \
	   source/memctrl/bus_handler.h \
	   source/memctrl/cmd_issuer.h \
	   source/memctrl/bus.h \
	   source/memctrl/data_bus_handler.h \
	   source/memctrl/cmd_bus_handler.h \
	   source/memctrl/dram.h \
	   source/memctrl/refresh_manager.h \
	   source/memctrl/response_handler.h \
	   source/memctrl/stats.h \
	   source/simIris/components/impl/genericFlatMc.h \
	   source/memctrl/mshr.h \
	   source/memctrl/mshr_standalone.h \
    source/orion/orion_router_power.h \
    source/orion/SIM_ALU.h \
    source/orion/SIM_ALU_model.h \
    source/orion/SIM_arbiter.h \
    source/orion/SIM_array.h \
    source/orion/SIM_array_internal.h \
    source/orion/SIM_array_model.h \
    source/orion/SIM_cam.h \
    source/orion/SIM_clock.h \
    source/orion/SIM_crossbar.h \
    source/orion/SIM_link.h \
    source/orion/SIM_link_model.h \
    source/orion/SIM_misc.h \
    source/orion/SIM_misc_model.h \
    source/orion/SIM_parameter.h \
    source/orion/SIM_permu.h \
    source/orion/SIM_permu_model.h \
    source/orion/SIM_port.h \
    source/orion/SIM_reg.h \
    source/orion/SIM_router_area.h \
    source/orion/SIM_router.h \
    source/orion/SIM_router_model.h \
    source/orion/SIM_router_power.h \
    source/orion/SIM_static.h \
    source/orion/SIM_technology.h \
    source/orion/SIM_technology_v1.h \
    source/orion/SIM_technology_v2.h \
    source/orion/SIM_time.h \
    source/orion/SIM_util.h \
    source/orion3/CLKCTRL.h \
    source/orion3/config.h \
    source/orion3/INBUF.h \
    source/orion3/mainpage.h \
    source/orion3/NONPR.h \
    source/orion3/OUTBUF.h \
    source/orion3/router_area.h \
    source/orion3/router.h \
    source/orion3/router_power.h \
    source/orion3/SIM_ALU.h \
    source/orion3/SIM_ALU_model.h \
    source/orion3/SIM_arbiter.h \
    source/orion3/SIM_array.h \
    source/orion3/SIM_array_internal.h \
    source/orion3/SIM_array_model.h \
    source/orion3/SIM_cam.h \
    source/orion3/SIM_clock.h \
    source/orion3/SIM_crossbar.h \
    source/orion3/SIM_link.h \
    source/orion3/SIM_link_model.h \
    source/orion3/SIM_misc.h \
    source/orion3/SIM_misc_model.h \
    source/orion3/SIM_parameter.h \
    source/orion3/SIM_permu.h \
    source/orion3/SIM_permu_model.h \
    source/orion3/SIM_port_alpha.h \
    source/orion3/SIM_port.h \
    source/orion3/SIM_port_ibm.h \
    source/orion3/SIM_port_NEC.h \
    source/orion3/SIM_port_raw.h \
    source/orion3/SIM_port_TRIPS.h \
    source/orion3/SIM_reg.h \
    source/orion3/SIM_router_area.h \
    source/orion3/SIM_router.h \
    source/orion3/SIM_router_model.h \
    source/orion3/SIM_router_power.h \
    source/orion3/SIM_static.h \
    source/orion3/SIM_technology.h \
    source/orion3/SIM_technology_v1.h \
    source/orion3/SIM_technology_v2.h \
    source/orion3/SIM_technology_v2_NEW.h \
    source/orion3/SIM_time.h \
    source/orion3/SIM_util.h \
    source/orion3/SWVC.h \
    source/orion3/technology_area_power.h \
    source/orion3/XBAR.h

ZESTO_HDRS = \
	     source/zesto/bbtracker.h \
	     source/zesto/thread.h    \
	     source/zesto/eio.h \
	     source/zesto/endian.h \
	     source/zesto/eval.h \
	     source/zesto/host.h \
	     source/zesto/loader.h \
	     source/zesto/machine.h \
	     source/zesto/memory.h  \
	     source/zesto/misc.h \
	     source/zesto/options.h \
	     source/zesto/range.h \
	     source/zesto/regs.h \
	     source/zesto/sim.h  \
	     source/zesto/stats.h \
	     source/zesto/symbol.h \
	     source/zesto/syscall.h \
	     source/zesto/version.h  \
	     source/zesto/machine.def \
	     source/zesto/elf.h \
	     source/zesto/x86flow.def \
	     source/zesto/zesto-structs.h \
	     source/zesto/zesto-core.h \
	     source/zesto/zesto-opts.h \
	     source/zesto/zesto-oracle.h \
	     source/zesto/zesto-fetch.h \
	     source/zesto/zesto-decode.h \
	     source/zesto/zesto-alloc.h \
	     source/zesto/zesto-exec.h \
	     source/zesto/zesto-commit.h \
	     source/zesto/zesto-cache.h \
	     source/zesto/zesto-bpred.h \
	     source/zesto/zesto-memdep.h \
	     source/zesto/zesto-prefetch.h \
	     source/zesto/zesto-uncore.h

MCPAT_OBJS= \
	    source/mcpat/obj_opt/Ucache.o source/mcpat/obj_opt/XML_Parse.o \
	    source/mcpat/obj_opt/arbiter.o source/mcpat/obj_opt/area.o \
	    source/mcpat/obj_opt/array.o source/mcpat/obj_opt/bank.o \
	    source/mcpat/obj_opt/basic_circuit.o source/mcpat/obj_opt/basic_components.o \
	    source/mcpat/obj_opt/cacti_interface.o \
	    source/mcpat/obj_opt/component.o source/mcpat/obj_opt/core.o \
	    source/mcpat/obj_opt/crossbar.o source/mcpat/obj_opt/decoder.o \
	    source/mcpat/obj_opt/htree2.o source/mcpat/obj_opt/interconnect.o \
	    source/mcpat/obj_opt/io.o source/mcpat/obj_opt/logic.o \
	    source/mcpat/obj_opt/iris_interface.o source/mcpat/obj_opt/mat.o \
	    source/mcpat/obj_opt/memoryctrl.o source/mcpat/obj_opt/noc.o \
	    source/mcpat/obj_opt/nuca.o source/mcpat/obj_opt/parameter.o source/mcpat/obj_opt/processor.o \
	    source/mcpat/obj_opt/router.o \
	    source/mcpat/obj_opt/sharedcache.o source/mcpat/obj_opt/subarray.o \
	    source/mcpat/obj_opt/technology.o source/mcpat/obj_opt/uca.o \
	    source/mcpat/obj_opt/wire.o source/mcpat/obj_opt/xmlParser.o

### manifold_fullSim
#manifold_fullSim_CXXFLAGS = $(CXX_FLAGS)
#manifold_fullSim_CXXLD= $(ZESTOCXXLD)

#manifold_fullSim_SOURCES = $(FE_SOURCES)\
    #			   source/frontend/impl/manifold_fullsim.cc

#manifold_fullSim_LDADD = source/simIris/libIris.a \
    #			 source/memctrl/libsimMC.a \
    #			 source/zesto/libZesto.a

## manifold_simIris
manifold_simIris_CXXFLAGS = -O0 $(CXX_FLAGS) -lpthread -lgsl -lgslcblas
manifold_simIris_CXXLD= $(CXXLD) -O0

manifold_simIris_SOURCES = $(FE_SOURCES)\
			   source/frontend/impl/manifold_simiris.cc

#manifold_simIris_LDADD = source/simIris/libIris.a source/orion/libOrion.a  source/orion3/libOrion3.a
manifold_simIris_LDADD = source/simIris/libIris.a source/orion_macsim/liborion.a  source/orion/libOrion.a source/orion3/libOrion3.a

## manifold_simMC
#manifold_simMC_CXXFLAGS = $(CXX_FLAGS)
#manifold_simMC_CXXLD= $(CXXLD)
#manifold_simMC_SOURCES = $(FE_SOURCES)\
#			 source/frontend/impl/manifold_simmc.cc

#manifold_simMC_LDADD = source/simIris/libIris.a \
#		       source/memctrl/libsimMCSA.a

##Headers
noinst_HEADERS = \
		 source/frontend/impl/mesh.h \
		 source/util/config_params.h \
		 $(ALL_HDRS)

###Headers
#noinst_HEADERS = \
    #		 source/frontend/impl/mesh.h \
    #		 source/util/config_params.h \
    #		 $(ALL_HDRS) $(ZESTO_HDRS)
