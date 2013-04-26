
lib_LIBRARIES = libIris.a

IRIS_SOURCES = \
		  ../kernel/link.cc \
		  ../kernel/clock.cc \
		  ../kernel/simulator.cc \
		  ../kernel/component.cc \
		  ../util/genericData.cc \
		  ../util/stats.cc \
		  data_types/impl/irisEvent.cc \
		  data_types/impl/flit.cc \
		  data_types/impl/highLevelPacket.cc \
		  data_types/impl/lowLevelPacket.cc \
		  components/interfaces/router.cc \
		  components/interfaces/processor.cc \
		  components/interfaces/interface.cc \
		  components/interfaces/irisLink.cc \
		  components/interfaces/networkComponent.cc \
		  components/impl/genericBuffer.cc \
		  components/impl/genericRC.cc \
		  components/impl/genericCrossbar.cc \
		  components/impl/genericInterfaceNB.cc \
		  components/impl/genericInterfaceNBb.cc \
		  components/impl/genericInterfaceNBm.cc \
		  components/impl/genericInterfaceNBVn.cc \
		  components/impl/genericInterfaceNBVs.cc \
		  components/impl/genericInterfaceNBpr.cc \
		  components/impl/genericArbiter.cc \
		  components/impl/genericLink.cc \
		  components/impl/genericVca.cc \
		  components/impl/ptopSwaVcs.cc \
		  components/impl/pvtopv_swa.cc \
		  components/impl/ptop_swa.cc \
		  components/impl/vcaMsgAware.cc \
		  components/impl/routerVcMP.cc \
		  components/impl/routerVcMPm.cc \
		  components/impl/rtVcMPmDBAR.cc \
		  components/impl/rtRCA1D.cc \
		  components/impl/rtVBDBAR.cc \
		  components/impl/rtSPDBAR.cc \
		  components/impl/rtPRDBAR.cc \
		  components/impl/rtPGCP.cc \
		  components/impl/rtPKTSP.cc \
		  components/impl/rtPKTSPa.cc \
		  components/impl/rtPKTSPb.cc \
		  components/impl/rtPKTSPtr.cc \
		  components/impl/rtPKTPR.cc \
		  components/impl/rtPKTPRtr.cc \
		  components/impl/routerVcMPft.cc \
		  components/impl/variantA.cc \
		  components/impl/variantB.cc \
		  components/impl/routerReqReply.cc \
		  components/impl/genericRouterPhy.cc \
		  components/impl/genericRouter4Stg.cc \
		  components/impl/genericSink.cc \
		  components/impl/customPktGenVn.cc \
		  components/impl/genericPktGen.cc \
		  components/impl/genericPktGenVn.cc \
		  components/impl/genericVcAllocator.cc \
		  components/impl/mcFrontEnd.cc \
		  components/impl/genericFlatMc.cc

#		  components/impl/genericRouter4Stg.cc \
##libIris
libIris_a_CXXFLAGS = $(CXX_FLAGS) -O0
libIris_a_SOURCES = $(IRIS_SOURCES)

CXXLD = g++ -O0

##Headers
noinst_HEADERS = $(ALL_HDRS)
