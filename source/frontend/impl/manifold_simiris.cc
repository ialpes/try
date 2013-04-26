/*
 * =====================================================================================
 *
 *       Filename:  simMc2Mesh.cc
 *
 *    Description:  This is a 2x2 mesh network with 3 trace packet generators
 *    and a single memory controller
 *
 *        Version:  1.0
 *        Created:  04/20/2010 02:59:08 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _simmc2mesh_cc_INC
#define  _simmc2mesh_cc_INC

#include	"topology.h"
#include	"mesh.h"
#include	"torus.h"
#include	"ring.h"
#include	"visual.h"
#include	"../../simIris/data_types/impl/flit.h"
#include	"../../util/stats.h"
#include	"../../simIris/data_types/impl/highLevelPacket.h"
#include	"../../util/genericData.h"
#include	"../../util/config_params.h"
//#include    "../../orion/orion_router_power.h"
#include	<string.h>
#include	<sys/time.h>
#include	<algorithm>
#include    <cstdlib>
#include        <gsl/gsl_rng.h>
#include        <gsl/gsl_randist.h>
#include    <iostream>
#include    <fstream>
#include    <sstream>

extern "C" {
#include "../../orion_macsim/SIM_router.h"
#include "../../orion_macsim/SIM_router_power.h"
#include "../../orion_macsim/SIM_parameter.h"
#include "../../orion_macsim/SIM_array.h"
#include "../../orion_macsim/SIM_misc.h"
#include "../../orion_macsim/SIM_static.h"
#include "../../orion_macsim/SIM_clock.h"
#include "../../orion_macsim/SIM_util.h"
#include "../../orion_macsim/SIM_link.h"
/*
#include "SIM_router.h"
#include "SIM_router_power.h"
#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_misc.h"
#include "SIM_static.h"
#include "SIM_clock.h"
#include "SIM_util.h"
#include "SIM_link.h"
*/
}

#define GSL_RNG_SEED 12345

#define _PRINTOUT_CONFIG
#define _PRINTOUT
//#define _PRINTOUT_DETAILS
#define _DBG_METRIC
//#define _DBG_FILTRE_FAULT
//#define _ORION_NIRGAM
#ifdef _ORION_NIRGAM
#include    "../../orion/orion_router_power.h"
#endif

unsigned long int net_pack_info[8][8];

extern void interface_simiris(ullint);

Topology * topology_ptr = NULL;
//extern Topology * topology_ptr;// = NULL;


vector <void *> ptr_router;


void
dump_configuration ( void )
{
	/* Creating mesh topology with the following config */
	cerr << " type:\t" << network_type << endl;
	cerr << " vcs:\t" << vcs << endl;
	cerr << " ports:\t" << ports << endl;
	cerr << " buffer_size:\t" << buffer_size << endl;
	cerr << " credits:\t" << credits << endl;
	cerr << " no_nodes:\t" << no_nodes << endl;
	cerr << " grid_size:\t" << grid_size << endl;
	cerr << " unidirectional_links:  \t" << 2*links << endl;
	cerr << " no_of_cores:\t" << no_of_cores << endl;
	cerr << " concentration:\t" << concentration << endl;
	cerr << " no_of_traces:\t" << traces.size() << endl;
	cerr << " interfaces:\t" << concentration << endl;
	cerr << " cores:\t" << no_of_cores << endl;
	cerr << " max_sim_time:\t" << max_sim_time << endl;
	cerr << " max_phy_link_bits:\t" << max_phy_link_bits << endl;
	cerr << " THREAD_BITS_POSITION:\t" << THREAD_BITS_POSITION<< endl;
	cerr << " MC_ADDR_BITS:\t" << MC_ADDR_BITS<< endl;
	cerr << " TWO_STAGE_ROUTER:\t" << do_two_stage_router << endl;
	cerr << " ROUTING_SCHEME:\t" << routing_scheme << " " << rc_method << endl;
	cerr << " PKT_TYPE (TRAFFIC PATTERN):  " << pattern_type_string << endl;
	cerr << " SW_ARBITRATION:\t" << sw_arbitration_scheme<< " " << sw_arbitration<< endl;
	cerr << " Msg_class with arbitration priority:\t" << msg_type_string << endl;
	cerr << " BANK_BITS:\t" << BANK_BITS << endl;
	cerr << " NETWORK_BITS:\t" << NETWORK_ADDRESS_BITS << endl;
	cerr << " COMMAND_BITS:\t" << NETWORK_THREADID_BITS << endl;
	cerr << " COREID_BITS:\t" << NETWORK_COMMAND_BITS << endl;
	cerr << " NO_OF_THREADS:\t" << NO_OF_THREADS << endl;
	cerr << " NO_OF_CHANNELS:\t" << NO_OF_RANKS << endl;
	cerr << " NO_OF_RANKS:\t" << NO_OF_RANKS << endl;
	cerr << " NO_OF_BANKS:\t" << NO_OF_BANKS << endl;
	cerr << " NO_OF_ROWS:\t" << NO_OF_ROWS << endl;
	cerr << " NO_OF_COLUMNS:\t" << NO_OF_COLUMNS << endl;
	cerr << " COLUMN_SIZE:\t" << COLUMN_SIZE << endl;
	cerr << " router_model:\t" << router_model_string<< endl;
	cerr << " mc_model:\t" << mc_model_string<< endl;
	cerr << " do_request_reply_network:\t" << do_request_reply_network << endl;
	cerr << " mc_response_pkt_payload_length:\t" << mc_response_pkt_payload_length<< endl;
	cerr << " no_of_mcs:\t" << no_mcs << endl;
	cerr << " mc_positions:\t";
	for( uint i=0; i<mc_positions.size(); i++)
		cerr << mc_positions[i] << " ";
	cerr << endl;
	cerr << " no_of_pktgens:\t" << no_pktgens << endl;
	cerr << " pktgen_positions:\t";
	for( uint i=0; i<pktgen_positions.size(); i++)
		cerr << pktgen_positions[i] << " ";
	cerr << endl;
	cerr << " fault_injection_rate:\t" << FAULT_INJECTION_RATE <<"%"<< endl;
	cerr << " no_of_fault:\t" << nb_faults << endl;

	cerr << " faulty_opnodes:\t";
	for( uint i=0; i<faulty_opnodes.size(); i++)
		cerr << faulty_opnodes[i] << " ";
	cerr << endl;

	cerr << " faulty_outports:\t";
	for( uint i=0; i<faulty_outports.size(); i++)
		cerr << faulty_outports[i] << " ";
	cerr << endl;

	cerr << " faulty_ipnodes:\t";
	for( uint i=0; i<faulty_ipnodes.size(); i++)
		cerr << faulty_ipnodes[i] << " ";
	cerr << endl;

	cerr << " faulty_inports:\t";
	for( uint i=0; i<faulty_inports.size(); i++)
		cerr << faulty_inports[i] << " ";
	cerr << endl;

	cerr << " faulty_type:\t";
	for( uint i=0; i<fault_type.size(); i++)
		cerr << fault_type[i] << " ";
	cerr << endl;

	cerr << " time_inject:\t";
	for( uint i=0; i<temps_inject.size(); i++)
		cerr << temps_inject[i] << " ";
	cerr << endl;

	cerr << " fault_duration:\t";
	for( uint i=0; i<fault_dure.size(); i++)
		cerr << fault_dure[i] << " ";
	cerr << endl;

	cerr << " terminal_model:\t" << terminal_model_string<< endl;
	cerr << " mean_irt:\t" << mean_irt<< endl;
	cerr << " terminal_msg_class:\t" << terminal_msg_class_string << endl;
	cerr << " no_msg_classes:\t" << no_msg_classes<< endl;
	cerr << " pkt_payload_length:\t" << pkt_payload_length<< endl;
	cerr << " stat_print_level:\t" << stat_print_level<< endl;

	if( terminal_model_string == "TPG" && traces.size() < (no_nodes - no_mcs) )
	{
		cout << " Not enough trace files for simulation " << endl;
		exit(1);
	}
	return ;
}

void
print_access_counts()
{
	cerr << "\nIB_cycles: " << istat->get_total_ib_cycles() << endl;
	cerr << "RC_cycles: " << istat->get_total_rc_cycles() << endl;
	cerr << "VCA_cycles: " << istat->get_total_vca_cycles() << endl;
	cerr << "SA_cycles: " << istat->get_total_sa_cycles() << endl;
	cerr << "ST_cycles: " << istat->get_total_st_cycles() << endl;
	cerr << "Total_flits: " << istat->get_total_flits_passed() << endl;
	cerr << "Total_credits: " << istat->get_total_credits_passed() << endl;
}

/* to fill the mc_position, pktgen_position if not given
 * parameter should pass is no_mc/no_pktgen, vectors, grid size
 */
void
generate_nodes(const uint no, const uint gs, std::vector<uint> *v)
{
	const gsl_rng_type * T=new gsl_rng_type();
	gsl_rng * dest_gen=new gsl_rng();
	gsl_rng_env_setup();
	gsl_rng_default_seed = 2^25;
	T = gsl_rng_default;
	dest_gen = gsl_rng_alloc (T);

	srand ( time(NULL) );

	while ((*v).size()<no)
	{
		uint random_position=rand()%(gs*gs);
		std::vector<uint>::iterator itr=std::find((*v).begin(),(*v).end(),random_position);
		if (itr==(*v).end())
			(*v).push_back(random_position);
	}
}

void
generate_nodes(const uint no, const uint gs, std::vector<uint> *v, std::vector<uint> *v1)
{
	const gsl_rng_type * T=new gsl_rng_type();
	gsl_rng * dest_gen=new gsl_rng();
	gsl_rng_env_setup();
	gsl_rng_default_seed = 2^25;
	T = gsl_rng_default;
	dest_gen = gsl_rng_alloc (T);

	srand ( time(NULL) );

	while ((*v1).size()<no)
	{
		uint random_position=rand()%(gs*gs);
		std::vector<uint>::iterator itr=std::find((*v).begin(),(*v).end(),random_position);
		if (itr==(*v).end())
		{
			std::vector<uint>::iterator itr1=std::find((*v1).begin(),(*v1).end(),random_position);
			if (itr1==(*v1).end())
				(*v1).push_back(random_position);
		}

	}
}

int
main ( int argc, char *argv[] )
{
	/* initialize random seed: */
	ullint sim_start_time = time(NULL);
	if(argc<2)
	{
		cout << "Error: Requires config file for input parameters\n";
		return 1;
	}

	for(int ii=0;ii<8;ii++)
		for(int jj=0;jj<8;jj++)
			net_pack_info[ii][jj]=0;

	ifstream fd(argv[1]);
	if (!fd.is_open())
	{
		cerr << argv[1]<< " : no such file ! \n";
		return 0;
	}
	string data, word;
	string rtr_model;

	while(!fd.eof())
	{
		getline(fd,data);
		string::size_type position = -1;
		position = data.find("#");
		istringstream iss( data, istringstream::in);
		//cerr <<" position = "<< position <<  endl;
		//cerr <<" data.size() = "<< data.size() <<  endl;

		while ( position > data.size() && iss >> word )
		{
			//cerr << " position > data.size() && iss >> word  is true " << endl;
			if ( word.compare("TYPE") == 0)
				iss >> network_type;
			if ( word.compare("PRINT_SETUP") == 0)
				iss >> print_setup;
			if ( word.compare("VCS") == 0)
				iss >> vcs;
			if ( word.compare("PORTS") == 0)
				iss >> ports;
			if ( word.compare("BUFFER_SIZE") == 0)
				iss >> buffer_size;
			if ( word.compare("CREDITS") == 0)
				iss >> credits;
			if ( word.compare("GRID_SIZE") == 0)
				iss >> grid_size;
			if ( word.compare("NO_OF_NODES") == 0)
				iss >> no_nodes;
			if ( word.compare("MCS") == 0)
				iss >> no_mcs;
			if ( word.compare("PKTGENS") == 0)
				iss >> no_pktgens;
			if ( word.compare("CORES_PER_NODE") == 0)
				iss >> no_of_cores;
			if ( word.compare("CONCENTRATION") == 0)
				iss >> concentration;
			if ( word.compare("MAX_SIM_TIME") == 0)
				iss >> max_sim_time;
			if ( word.compare("SIM_WARMUP_TIME") == 0)
				iss >> sim_warmup_time;
			if ( word.compare("OUTPUT_PATH") == 0)
				iss >> output_path;
			if ( word.compare("PHY_LINK_WIDTH") == 0)
				iss >> max_phy_link_bits;
			if ( word.compare("ROUTER_MODEL") == 0)
				iss >> router_model_string;
			NO_OF_THREADS = no_nodes;
			/* Init parameters of mc_constants*/
			if ( word.compare("MC_MODEL") == 0)
				iss >> mc_model_string;
			if ( word.compare("TERMINAL_MODEL") == 0)
				iss >> terminal_model_string;
			if ( word.compare("MEAN_IRT") == 0)
				iss >> mean_irt;
			if ( word.compare("TERMINAL_MSG_CLASS") == 0)
				iss >> terminal_msg_class_string;
			if ( word.compare("NO_MSG_CLASS") == 0)
				iss >> no_msg_classes;
			if ( word.compare("PKT_PAYLOAD_LEN") == 0)
				iss >> pkt_payload_length;
			if ( word.compare("PKT_SIZE") == 0)
				iss >> pkt_size;
			if ( word.compare("MC_RESP_PAYLOAD_LEN") == 0)
				iss >> mc_response_pkt_payload_length;
			if ( word.compare("THREAD_BITS_POSITION") == 0)
				iss >> THREAD_BITS_POSITION;
			if ( word.compare("MC_ADDR_BITS") == 0)
				iss >> MC_ADDR_BITS;
			if ( word.compare("BANK_BITS") == 0)
				iss >> BANK_BITS;
			if ( word.compare("NO_OF_CHANNELS") == 0)
				iss >> NO_OF_CHANNELS;

			if ( word.compare("NO_OF_RANKS") == 0)
				iss >> NO_OF_RANKS;
			if ( word.compare("NO_OF_BANKS") == 0)
				iss >> NO_OF_BANKS;

			if ( word.compare("NO_OF_ROWS") == 0)
				iss >> NO_OF_ROWS;
			if ( word.compare("NO_OF_COLUMNS") == 0)
				iss >> NO_OF_COLUMNS;
			if ( word.compare("COLUMN_SIZE") == 0)
				iss >> COLUMN_SIZE;
			if ( word.compare("MSHR_SIZE") == 0)
				iss >> MSHR_SIZE;

			if ( word.compare("MAX_BUFFER_SIZE") == 0)
				iss >> MAX_BUFFER_SIZE;
			if ( word.compare("MAX_CMD_BUFFER_SIZE") == 0)
				iss >> MAX_CMD_BUFFER_SIZE;
			if ( word.compare("RESPONSE_BUFFER_SIZE") == 0)
				iss >> RESPONSE_BUFFER_SIZE;

			if ( word.compare("NETWORK_ADDRESS_BITS") == 0)
				iss >> NETWORK_ADDRESS_BITS;
			if ( word.compare("NETWORK_THREADID_BITS") == 0)
				iss >> NETWORK_THREADID_BITS ;
			if ( word.compare("NETWORK_COMMAND_BITS") == 0)
				iss >> NETWORK_COMMAND_BITS;

			if ( word.compare("SRCONE") == 0)
				iss >> srcone;
			if ( word.compare("DSTONE") == 0)
				iss >> dstone;

			/*  ******************************************* */
			if ( word.compare("PKT_TYPE") == 0)
				iss >> pattern_type_string;
			if ( word.compare("TWO_STAGE_ROUTER") == 0)
				iss >> do_two_stage_router;
			if ( word.compare("ROUTING_SCHEME") == 0)
				iss >> routing_scheme;
			if ( word.compare("CONGEST_METRIC") == 0)
				iss >> cg_metric_string;
			if ( word.compare("SW_ARBITRATION") == 0)
				iss >> sw_arbitration_scheme;
			if ( word.compare("PRIORITY_MSG_TYPE") == 0)
				iss >> msg_type_string;
			if ( word.compare("DRAM_PAGE_POLICY") == 0)
				iss >> dram_page_policy_string;
			if ( word.compare("ADDRESS_MAP_SCHEME") == 0)
				iss >> addr_map_scheme_string;
			if ( word.compare("MC_SCHEDULING_ALGORITHM") == 0)
				iss >> mc_scheduling_algorithm_string;
			if ( word.compare("REQ_REPLY") == 0)
				iss >> do_request_reply_network;
			if ( word.compare("TRACE") == 0)
			{
				iss >> trace_name;
				traces.push_back(trace_name);
			}
			if ( word.compare("MC_POS") == 0)
			{
				uint mc_pos;
				for (uint i=0; i< no_mcs; i++)
				{
					iss >> mc_pos;
					mc_positions.push_back(mc_pos);
				}
			}
			if ( word.compare("PKT_POS") == 0)
			{
				uint pktgen_pos;
				for (uint i=0; i< no_pktgens; i++)
				{
					iss >> pktgen_pos;
					pktgen_positions.push_back(pktgen_pos);
				}
			}
			if ( word.compare("FAULT_INJECTION_RATE") == 0)
			{
				iss >> FAULT_INJECTION_RATE;
			}
			if ( word.compare("NB_FAULTS") == 0)
			{
				iss >> nb_faults;
			}
			if ( word.compare("FAULTY_OPNODES") == 0)
			{
				uint faultnode;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> faultnode;
					faulty_opnodes.push_back(faultnode);
				}
			}
			if ( word.compare("FAULTY_OUTPORTS") == 0)
			{
				uint faultport;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> faultport;
					faulty_outports.push_back(faultport);

				}
			}
			if ( word.compare("FAULTY_IPNODES") == 0)
			{
				uint faultipnode;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> faultipnode;
					faulty_ipnodes.push_back(faultipnode);
				}
			}
			if ( word.compare("FAULTY_INPORTS") == 0)
			{
				uint faultinport;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> faultinport;
					faulty_inports.push_back(faultinport);

				}
			}
			if ( word.compare("TEMPS_INJECT") == 0)
			{
				uint tempsinject;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> tempsinject;
					temps_inject.push_back(tempsinject);

				}
			}
			if ( word.compare("FAULTY_TYPE") == 0)
			{
				uint faulttype;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> faulttype;
					fault_type.push_back(faulttype);

				}
			}
			if ( word.compare("FAULTY_DURE") == 0)
			{
				uint faultdure;
				for(uint i=0;i<nb_faults;i++)
				{
					iss >> faultdure;
					fault_dure.push_back(faultdure);

				}
			}

		}
	}
	//   assert(mc_positions.size() == no_mcs);
	//   assert(pktgen_positions.size() == no_pktgens);

	for ( uint i=0; i<argc; i++)
	{
		if( strcmp(argv[i],"--thread_id_bits")==0)
			THREAD_BITS_POSITION = atoi(argv[i+1]);
		if( strcmp(argv[i],"--mc_bits")==0)
			MC_ADDR_BITS = atoi(argv[i+1]);
		if( strcmp(argv[i],"--router_two_stg")==0)
			do_two_stage_router = atoi(argv[i+1]);
		if( strcmp(argv[i],"--routing_scheme")==0)
			routing_scheme = argv[i+1];
		if( strcmp(argv[i],"--sw_arbitration")==0)
			sw_arbitration_scheme = argv[i+1];
		if( strcmp(argv[i],"--msg_type")==0)
			msg_type_string = argv[i+1];
		if( strcmp(argv[i],"--bank_bits")==0)
			BANK_BITS = atoi(argv[i+1]);
		if( strcmp(argv[i],"--mirt")==0)
			mean_irt= atoi(argv[i+1]);
	}

	if( routing_scheme.compare("") != 0){
		if( routing_scheme.compare("virtualnetwork") == 0)
			rc_method = VIRTUALNETWORK;
		else if( routing_scheme.compare("hierachy") == 0)
			rc_method = HIERACHY;
		else if( routing_scheme.compare("oddeven") == 0)
			rc_method = ODD_EVEN;
		else if( strcmp(routing_scheme.c_str(),"negativefirst") == 0)
			rc_method = NEGATIVE_FIRST;
		else if( strcmp(routing_scheme.c_str(),"westfirst") == 0)
			rc_method = WEST_FIRST;
		else if( strcmp(routing_scheme.c_str(),"northlast") == 0)
			rc_method = NORTH_LAST;
		else if( strcmp(routing_scheme.c_str(),"northlastnm") == 0)
			rc_method = NORTH_LAST_NON_MINIMAL;
		else if( routing_scheme.compare("xy") == 0)
			rc_method = XY;
		else if( routing_scheme.compare("dyxy") == 0)
			rc_method = DY_XY;
		else if( routing_scheme.compare("torus_routing") == 0)
			rc_method = TORUS_ROUTING;
		else if( routing_scheme.compare("ring_routing") == 0)
			rc_method = RING_ROUTING;
		else
		{
			cerr << "Routing Scheme : " << routing_scheme << " is not found !"<<endl;
			exit(0);
		}
	}

	if( cg_metric_string.compare("") != 0){
		if( cg_metric_string.compare("nometric") == 0)
			cg_metric = NOMETRIC;
		else if( cg_metric_string.compare("buffersize") == 0)
			cg_metric = BUFFER_SIZE;
		else if( cg_metric_string.compare("virtualchannel") == 0)
			cg_metric = VIRTUAL_CHANNEL;
		else if( cg_metric_string.compare("crossbardemande") == 0)
			cg_metric = CROSSBAR_DEMANDE;
		else if( cg_metric_string.compare("flitremain") == 0)
			cg_metric = FLIT_REMAIN;
		else if( cg_metric_string.compare("xbvc") == 0)
			cg_metric = XB_VC;
		else if( cg_metric_string.compare("vcbf") == 0)
			cg_metric = VC_BF;
		else if( cg_metric_string.compare("xbbf") == 0)
			cg_metric = XB_BF;
		else if( cg_metric_string.compare("mix1") == 0)
			cg_metric = MIX1;
		else if( cg_metric_string.compare("mix2") == 0)
			cg_metric = MIX2;
		else if( cg_metric_string.compare("mix3") == 0)
			cg_metric = MIX3;
		else if( cg_metric_string.compare("mix4") == 0)
			cg_metric = MIX4;
		else if( cg_metric_string.compare("mix5") == 0)
			cg_metric = MIX5;
		else {
			cerr << "Congestion Metric : " << cg_metric_string << " is not found !"<<endl;
			exit(0);
		}
	}


	if( strcmp(sw_arbitration_scheme.c_str(),"round_robin") == 0)
		sw_arbitration = ROUND_ROBIN;
	if( strcmp(sw_arbitration_scheme.c_str(),"round_robin_p") == 0)
		sw_arbitration = ROUND_ROBIN_PRIORITY;
	if( strcmp(sw_arbitration_scheme.c_str(),"fcfs") == 0)
		sw_arbitration = FCFS;

	if( strcmp(msg_type_string.c_str(),"PRIORITY_REQ") == 0)
		priority_msg_type = PRIORITY_REQ;
	if( strcmp(msg_type_string.c_str(),"ONE_FLIT_REQ") == 0)
		priority_msg_type = ONE_FLIT_REQ;
	if( strcmp(msg_type_string.c_str(),"RESPONSE_PKT") == 0)
		priority_msg_type = RESPONSE_PKT;

	if( dram_page_policy_string.compare("OPEN_PAGE_POLICY") == 0)
		dram_page_policy = OPEN_PAGE_POLICY;
	if( dram_page_policy_string.compare("CLOSE_PAGE_POLICY") == 0)
		dram_page_policy = CLOSE_PAGE_POLICY;

	if( mc_scheduling_algorithm_string.compare("FR_FCFS") == 0)
		mc_scheduling_algorithm = FR_FCFS;
	if( mc_scheduling_algorithm_string.compare("FC_FS") == 0)
		mc_scheduling_algorithm = FC_FS;
	if( mc_scheduling_algorithm_string.compare("PAR_BS") == 0)
		mc_scheduling_algorithm = PAR_BS;

	if( addr_map_scheme_string.compare("PAGE_INTERLEAVING") == 0)
		addr_map_scheme = PAGE_INTERLEAVING;
	if( addr_map_scheme_string.compare("PERMUTATION") == 0)
		addr_map_scheme = PERMUTATION;
	if( addr_map_scheme_string.compare("CACHELINE_INTERLEAVING") == 0)
		addr_map_scheme = CACHELINE_INTERLEAVING;
	if( addr_map_scheme_string.compare("GENERIC") == 0)
		addr_map_scheme = GENERIC;
	if( addr_map_scheme_string.compare("NO_SCHEME") == 0)
		addr_map_scheme = NO_SCHEME;
	if( addr_map_scheme_string.compare("LOCAL_ADDR_MAP") == 0)
		addr_map_scheme = LOCAL_ADDR_MAP;

	if( router_model_string.compare("") != 0){
		if( router_model_string.compare("VIRTUAL") == 0)
			router_model = VIRTUAL;
		else if( router_model_string.compare("VIRTUALft") == 0)
			router_model = VIRTUALft;
		else if( router_model_string.compare("VIRTUALm") == 0)
			router_model = VIRTUALm;
		else if( router_model_string.compare("PHYSICAL") == 0)
			router_model = PHYSICAL;
		else if( router_model_string.compare("VIRTUAL4Stg") == 0)
			router_model = VIRTUAL4Stg;
		else if( router_model_string.compare("VARIANTA") == 0)
			router_model = VARIANTA;
		else if( router_model_string.compare("VARIANTB") == 0)
			router_model = VARIANTB;
		else if( router_model_string.compare("VBDBAR") == 0)
			router_model = VBDBAR;
		else if( router_model_string.compare("SPDBAR") == 0)
			router_model = SPDBAR;
		else if( router_model_string.compare("PRDBAR") == 0)
			router_model = PRDBAR;
		else if( router_model_string.compare("RTDBAR") == 0)
			router_model = RTDBAR;
		else if( router_model_string.compare("RTPGCP") == 0)
			router_model = RTPGCP;
		else if( router_model_string.compare("RTPKTSPa") == 0)
					router_model = RTPKTSPa;
		else if( router_model_string.compare("RTPKTSPb") == 0)
					router_model = RTPKTSPb;
		else if( router_model_string.compare("RTPKTSP") == 0)
			router_model = RTPKTSP;
//		else if( router_model_string.compare("RTPKTSPtr") == 0)
//			router_model = RTPKTSPtr;
		else if( router_model_string.compare("RTPKTPR") == 0)
			router_model = RTPKTPR;
//		else if( router_model_string.compare("RTPKTPRtr") == 0)
//			router_model = RTPKTPRtr;

		else if( router_model_string.compare("RTRCA1D") == 0)
					router_model = RTRCA1D;
		else {
			cerr << "Router model : " << router_model_string << " is not found !"<<endl;
			exit(0);
		}
	}

	if( mc_model_string.compare("") != 0){
		if( mc_model_string.compare("GENERIC_MC") == 0)
			mc_model = GENERIC_MC;
		else
			if( mc_model_string.compare("FLAT_MC") == 0)
				mc_model = FLAT_MC;
			else
				if( mc_model_string.compare("SINK") == 0)
					mc_model = SINK;
				else {
					cerr << "MC model : " << mc_model_string << " is not found !"<<endl;
					exit(0);
				}
	}

	if( terminal_model_string.compare("") != 0)
	{
		if( terminal_model_string.compare("GENERIC") == 0)
			terminal_model = GENERIC_PKTGEN;
		else
			if( terminal_model_string.compare("TPG") == 0)
				terminal_model = TPG;
			else
				if( terminal_model_string.compare("PKTGENVN") == 0)
					terminal_model = PKTGENVN;
				else
					if( terminal_model_string.compare("CUSTOMPG") == 0)
						terminal_model = CUSTOMPG;
					else {
						cerr << "Terminal model : " << terminal_model_string << " is not found !"<<endl;
						exit(0);
					}
	}

	if( pattern_type_string.compare("") != 0)
	{
		if( pattern_type_string.compare("randomuni") == 0)
			pattern_type = RANDOM_UNI;
		else
			if( pattern_type_string.compare("transpose") == 0)
				pattern_type = TRANSPOSE;
			else
				if( pattern_type_string.compare("hotspot") == 0)
					pattern_type = HOTSPOT;
				else {
					cerr << "Traffic Pattern Type : " << pattern_type_string << " is not found !"<<endl;
					exit(0);
				}
	}

	if( strcmp(terminal_msg_class_string.c_str(),"PRIORITY_REQ") == 0)
		terminal_msg_class = PRIORITY_REQ;
	if( strcmp(terminal_msg_class_string.c_str(),"ONE_FLIT_REQ") == 0)
		terminal_msg_class = ONE_FLIT_REQ;
	if( strcmp(terminal_msg_class_string.c_str(),"RESPONSE_PKT") == 0)
		terminal_msg_class = RESPONSE_PKT;

	//   if (mc_positions.size() < no_mcs )
	//   	generate_nodes(no_mcs,grid_size,&mc_positions);
	//   sort(mc_positions.begin(),mc_positions.end());
	//
	//   if (pktgen_positions.size() < no_pktgens )
	//   	generate_nodes(no_pktgens,grid_size,&mc_positions,&pktgen_positions);
	//   sort(pktgen_positions.begin(),pktgen_positions.end());


	/* Compute additional parameters */
	uint edge_links = 0;
	if ( network_type == "MESH" || network_type == "Mesh" || network_type == "mesh")
	{
		/* For a 2D mesh [Gs*(GS-1)*k] internal links + edge links [4*GS] +
           terminal connections [GS*GS]. Note this is the unidirectional measure.
           Links are denoted as
           follows east going for router_n, west going for router_n, north going for router_n
           and south going for router_n (0<n<no_nodes). Going by this convention one of the edge
           links will be east going for rightmost router+1 which is not part of the mesh. Hence
           there are grid_size such unidirectional nodes for every k dimension that does not get connected.
           FIX that */
		links = (grid_size * (grid_size+1) *2 /*k=2 for mesh*/ ) + (grid_size*grid_size /* for the interfaces */ );
		edge_links = grid_size*4*2; /* for a mesh there are 4 sides each with GS nodes, 2 bidirectional */
		cout << "Links of Mesh= " << links << endl;
	}
	else if (network_type == "TORUS" || network_type == "Torus" || network_type == "torus" )
	{
		links = (grid_size * grid_size * 2/* k */) +(grid_size*grid_size)/*interfaces*/;
		/*no edge links for the torus */
		cout << "Links of Torus = " << links << endl;
	}
	else if (network_type == "RING" || network_type == "Ring" || network_type == "ring" )
	{
		//links = grid_size * grid_size * ports ;
		links = grid_size * 2;
		cout << "Links of Ring = " << links << endl;
	}
	fd.close();

	cerr << "\n-----------------------------------------------------------------------------------\n";
	cerr << "** CAPSTONE - Cycle Accurate Parallel Simulator Technologgy for On-Chip Networks **\n";
	cerr << " This is simIris. A second version of Capstone." << endl;
	cerr << "-- Computer Architecture and Systems Lab                                         --\n"
			<< "-- Georgia Institute of Technology                                               --\n"
			<< "-----------------------------------------------------------------------------------\n";
	cerr << "Cmd line: ";
	for( int i=0; i<argc; i++)
		cerr << argv[i] << " ";
	cerr << endl;

#ifdef _PRINTOUT_CONFIG
	dump_configuration();
#endif
	init_dram_timing_parameters();
	if ( network_type == "MESH" || network_type == "Mesh" || network_type == "mesh" )
		topology_ptr = new Mesh();
	else if ( network_type == "TORUS" || network_type == "Torus" || network_type == "torus" )
		topology_ptr = new Torus();
	else if ( network_type == "RING" || network_type == "Ring" || network_type == "ring" )
		topology_ptr = new Ring();
	else if (network_type == "NONE" )
	{
		cout << "Topology not specified...exiting \n" ;
		exit(1);
	}
	topology_ptr->init( ports, vcs, credits, buffer_size, no_nodes, grid_size, links);
	topology_ptr->max_sim_time = max_sim_time;


	/* Create the routers and interfaces */
	for( uint i=0; i<no_nodes; i++)
	{
		switch ( router_model )
		{
		case PHYSICAL:
			//topology_ptr->routers.push_back( new GenericRouterPhy());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNB());
			rtr_model = "GenericRouterPhy";
			break;
		case VIRTUAL:
			topology_ptr->routers.push_back( new RouterVcMP());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNB());
			rtr_model = "RouterVcMP";
			break;
		case VIRTUALm:
			topology_ptr->routers.push_back( new RouterVcMPm());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBm());
			//topology_ptr->interfaces.push_back ( new GenericInterfaceNB());
			rtr_model = "RouterVcMPm";
			break;
		case VIRTUALft:
			topology_ptr->routers.push_back( new RouterVcMPft());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBm());
			rtr_model = "RouterVcMPft";
			break;
		case VARIANTA:
			topology_ptr->routers.push_back( new VariantA());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBVn());
			rtr_model = "VariantA";
			break;
		case VARIANTB:
			topology_ptr->routers.push_back( new VariantB());
			//topology_ptr->interfaces.push_back ( new GenericInterfaceNBVs());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBb());
			rtr_model = "VariantB";
			break;
		case VIRTUAL4Stg:
			//topology_ptr->routers.push_back( new GenericRouter4Stg());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBm());
			rtr_model = "VIRTUAL4Stg";
			break;
		case RTRCA1D:
			topology_ptr->routers.push_back( new RtRCA1D());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBm());
			rtr_model = "RtRCA1D";
			break;
		case RTDBAR:
			topology_ptr->routers.push_back( new RtVcMPmDBAR());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBm());
			rtr_model = "RtVcMPmDBAR";
			break;
		case VBDBAR:
			topology_ptr->routers.push_back( new RtVBDBAR());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBb());
			rtr_model = "RtVBDBAR";
			break;
		case SPDBAR:
			topology_ptr->routers.push_back( new RtSPDBAR());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBVs());
			rtr_model = "RtSPDBAR";
			break;
		case PRDBAR:
			topology_ptr->routers.push_back( new RtPRDBAR());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBpr());
			rtr_model = "RtPRDBAR";
			break;
		case RTPGCP:
			topology_ptr->routers.push_back( new RtPGCP());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBm());
			rtr_model = "RtPGCP";
			break;
		case RTPKTSP:
			topology_ptr->routers.push_back( new RtPKTSP());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBVs());
			rtr_model = "RTPKTSP";
			break;
		case RTPKTSPa:
			topology_ptr->routers.push_back( new RtPKTSPa());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBVs());
			rtr_model = "RTPKTSPa";
			break;
		case RTPKTSPb:
			topology_ptr->routers.push_back( new RtPKTSPb());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBVs());
			rtr_model = "RTPKTSPb";
			break;
//		case RTPKTSPtr:
//			topology_ptr->routers.push_back( new RtPKTSPtr());
//			topology_ptr->interfaces.push_back ( new GenericInterfaceNBVs());
//			rtr_model = "RTPKTSPtr";
//			break;
		case RTPKTPR:
			topology_ptr->routers.push_back( new RtPKTPR());
			topology_ptr->interfaces.push_back ( new GenericInterfaceNBpr());
			rtr_model = "RTPKTPR";
			break;
//		case RTPKTPRtr:
//			topology_ptr->routers.push_back( new RtPKTPRtr());
//			topology_ptr->interfaces.push_back ( new GenericInterfaceNBpr());
//			rtr_model = "RTPKTPRtr";
//			break;
		default:
			cout << " Incorrect router model " << endl;
			exit(1);
			break;
		}
	}

	/*  Create the TPG and mc modules */
	vector<uint>::iterator itr;
	for( uint i=0; i<no_nodes; i++)
	{
		itr = find(mc_positions.begin(), mc_positions.end(), i);
		if( itr != mc_positions.end())
		{
			switch ( mc_model )
			{
			case GENERIC_MC:
				/*
                    topology_ptr->processors.push_back( new McFrontEnd() );
				 * */
				cout << " MC not integrated this rev " << endl;
				break;
			case FLAT_MC:
				topology_ptr->processors.push_back( new GenericFlatMc());
				break;
			case SINK:
				/*if (pattern_type==RANDOM_UNI || pattern_type==HOTSPOT){
                		topology_ptr->processors.push_back( new GenericSink());
                	}
                    else if (pattern_type==TRANSPOSE){
                    	if (terminal_model == GENERIC_PKTGEN)
                    		topology_ptr->processors.push_back( new GenericPktGen());
                    	else if (terminal_model == PKTGENVN)
                    		topology_ptr->processors.push_back( new GenericPktGenVn());
                    }*/
				if (terminal_model == GENERIC_PKTGEN)
					topology_ptr->processors.push_back( new GenericPktGen());
				else if (terminal_model == PKTGENVN)
					topology_ptr->processors.push_back( new GenericPktGenVn());
				else if (terminal_model == CUSTOMPG)
					topology_ptr->processors.push_back( new CustomPktGenVn());
				break;
			default:
				cout << " Unknown MC model " << endl;
				exit(1);
				break;
			}
		}
		else
		{
			switch ( terminal_model )
			{
			case GENERIC_PKTGEN:
				topology_ptr->processors.push_back( new GenericPktGen() );
				//if (pattern_type==RANDOM_UNI || pattern_type==HOTSPOT){
				for ( uint j=0; j<mc_positions.size(); j++)
					static_cast<GenericPktGen*>(topology_ptr->processors[i])->mc_node_ip.push_back(mc_positions[j]);
				//}
				//else if (pattern_type==TRANSPOSE){}
				break;
			case PKTGENVN:
				topology_ptr->processors.push_back( new GenericPktGenVn() );
				//if (pattern_type==RANDOM_UNI || pattern_type==HOTSPOT){
				for ( uint j=0; j<mc_positions.size(); j++)
					static_cast<GenericPktGenVn*>(topology_ptr->processors[i])->mc_node_ip.push_back(mc_positions[j]);
				//}
				//else if (pattern_type==TRANSPOSE){}
				break;
			case CUSTOMPG:
				topology_ptr->processors.push_back( new CustomPktGenVn() );
				//if (pattern_type==RANDOM_UNI || pattern_type==HOTSPOT){
				for ( uint j=0; j<mc_positions.size(); j++)
					static_cast<CustomPktGenVn*>(topology_ptr->processors[i])->mc_node_ip.push_back(mc_positions[j]);
				//}
				//else if (pattern_type==TRANSPOSE){}
				break;
			case TPG:
				/*
                    topology_ptr->processors.push_back( new GenericTracePktGen() );
                    static_cast<GenericTracePktGen*>(topology_ptr->processors[i])->set_trace_filename(traces[i]);
				 * */
				cout << " not supported this revision " << endl;
				exit(1);
				/*
				 * Need to pass the mc positions to the injecting node so it can pick a destination node_ip
				 * from the mc_positions vector. This may not be needed if
				 * the node_ip is available as part of the trace. In
				 * either case make sure to assert that the destination
				 * node_ip is one of the sink nodes.
                    for ( uint j=0; j<mc_positions.size(); j++)
                        static_cast<GenericTracePktGen*>(topology_ptr->processors[i])->mc_node_ip.push_back(mc_positions[j]);;
				 */

				break;
			default:
				cout << " Unknown Terminal model " << endl;
				exit(1);
				break;

			}
		}
	}


	/* Create the links */
	for ( uint i=0; i<links; i++)
	{
		topology_ptr->link_a.push_back(new GenericLink());
		topology_ptr->link_b.push_back(new GenericLink());
	}

	topology_ptr->connect_interface_processor();
	cout << " ok  " << endl;
	/* Set all the component ids */
	uint comp_id = 0, alink_comp_id = 1000, blink_comp_id = 5000;
	for ( uint i=0 ; i<no_nodes; i++ )
	{
		topology_ptr->processors[i]->setComponentId(comp_id++);
		//        topology_ptr->processors[i]->my_mesh = (void*)topology_ptr;
		topology_ptr->interfaces[i]->setComponentId(comp_id++);
		topology_ptr->routers[i]->setComponentId(comp_id++);
	}

	for ( uint i=0 ; i<links; i++ )
	{
		topology_ptr->link_a[i]->setComponentId(alink_comp_id++);
		topology_ptr->link_b[i]->setComponentId(blink_comp_id++);
		topology_ptr->link_a[i]->link_id=i;
		topology_ptr->link_b[i]->link_id=links+i;
		topology_ptr->link_a[i]->setup();
		topology_ptr->link_b[i]->setup();
	}

	/* Set warmup time */  //sim_warmup_time
	for ( uint i=0 ; i<no_nodes; i++ )
	{
		IrisEvent* event_proc = new IrisEvent();
		event_proc->type = RESET_STAT_EVENT;
		Simulator::Schedule(sim_warmup_time, &NetworkComponent::process_event, topology_ptr->processors[i], event_proc);
		IrisEvent* event_if = new IrisEvent();
		event_if->type = RESET_STAT_EVENT;
		Simulator::Schedule(sim_warmup_time, &NetworkComponent::process_event, topology_ptr->interfaces[i], event_if);
		IrisEvent* event_rt = new IrisEvent();
		event_rt->type = RESET_STAT_EVENT;
		Simulator::Schedule(sim_warmup_time, &NetworkComponent::process_event, topology_ptr->routers[i], event_rt);
	}
	for ( uint i=0 ; i<links; i++ )
	{
		IrisEvent* event_la = new IrisEvent();
		event_la->type = RESET_STAT_EVENT;
		Simulator::Schedule(sim_warmup_time, &NetworkComponent::process_event, topology_ptr->link_a[i], event_la);
		IrisEvent* event_lb = new IrisEvent();
		event_lb->type = RESET_STAT_EVENT;
		Simulator::Schedule(sim_warmup_time, &NetworkComponent::process_event, topology_ptr->link_b[i], event_lb);
	}

	IrisEvent* event_one = new IrisEvent();
	event_one->type = ONE_PULL_EVENT;
	event_one->dst_id = dstone;
	//Simulator::Schedule(sim_warmup_time, &NetworkComponent::process_event, topology_ptr->processors[srcone], event_one);

	cerr << " ******************** SETUP COMPLETE *****************\n" << endl;
	/*  Set up the node ips for components */
	for ( uint i=0 ; i<no_nodes ; i++ )
	{
		topology_ptr->interfaces[i]->node_ip = i;
		topology_ptr->routers[i]->node_ip = i;
		topology_ptr->processors[i]->node_ip = i;
	}

	/* Set the number of ports, vcs, credits and buffer sizes for the
	 * components */
	for ( uint i=0 ; i<no_nodes ; i++ )
	{
		topology_ptr->interfaces[i]->set_no_vcs(vcs);
		topology_ptr->interfaces[i]->set_no_credits(credits);
		//        topology_ptr->interfaces[i]->set_buffer_size(credits);
		uint temp_max =ceil((max(pkt_payload_length,mc_response_pkt_payload_length)+0.0)/max_phy_link_bits)+2;
		topology_ptr->interfaces[i]->set_buffer_size(vcs, max(temp_max, credits));
		//          topology_ptr->interfaces[i]->set_buffer_size(vcs, ceil((max(pkt_payload_length,mc_response_pkt_payload_length)+0.0)/max_phy_link_bits)+2);
		//        topology_ptr->processors[i]->set_output_path(output_path);
	}

	pkt_size=  ceil((pkt_payload_length+0.0)/max_phy_link_bits)+2;
	cerr << " pkt_size = " << pkt_size << endl;

	topology_ptr->setup();
	for ( uint i=0 ; i<no_pktgens; i++ )//pktgen_positions
	{
		if (terminal_model == GENERIC_PKTGEN)
			static_cast<GenericPktGen*>(topology_ptr->processors[pktgen_positions[i]])->enable( vcs, true);
		else if (terminal_model == PKTGENVN)
			static_cast<GenericPktGenVn*>(topology_ptr->processors[pktgen_positions[i]])->enable( vcs, true);
		else if (terminal_model == CUSTOMPG)
			static_cast<CustomPktGenVn*>(topology_ptr->processors[pktgen_positions[i]])->enable( vcs, true);
	}

	for ( uint i=0 ; i<no_nodes ; i++ )
	{
		if (terminal_model == GENERIC_PKTGEN)
			static_cast<GenericPktGen*>(topology_ptr->processors[i])->log_enable();
		else if (terminal_model == PKTGENVN)
			static_cast<GenericPktGenVn*>(topology_ptr->processors[i])->log_enable();
		else if (terminal_model == CUSTOMPG)
			static_cast<CustomPktGenVn*>(topology_ptr->processors[i])->log_enable();
		topology_ptr->processors[i]->set_output_path(output_path);
	}

	/*    for ( uint i=0 ; i<no_mcs; i++ )//pktgen_positions
    {
    	if (mc_model == SINK)
    		static_cast<GenericSink*>(topology_ptr->processors[mc_positions[i]])->set_output_path(output_path);

    }
	 */

	istat->init();

	if (network_type == "RING" || network_type == "Ring" || network_type == "ring" )
	{
		vector< uint > grid_x;
		vector< uint > grid_y;
		grid_x.resize(no_nodes);
		grid_y.resize(1);

		/* Limitation of only modelling squares */
		for ( uint i=0 ; i<no_nodes; i++ )
			grid_x[i] = i;

		grid_y[0] = 0;

		for ( uint i=0 ; i<no_nodes ; i++ )
			topology_ptr->routers[i]->set_no_nodes(no_nodes);

		for ( uint i=0 ; i<no_nodes ; i++ )
			for( uint j=0; j < ports ; j++)
				for( uint k=0; k < no_nodes ; k++) // Assuming is a square topology_ptr.
				{
					static_cast<Router*>(topology_ptr->routers[i])->set_grid_x_location(j,k, grid_x[k]);
					static_cast<Router*>(topology_ptr->routers[i])->set_grid_y_location(j,0, grid_y[0]);
				}
	}
	else
	{
		/*  Set no of ports and positions for routing */
		vector< uint > grid_x;
		vector< uint > grid_y;
		grid_x.resize(no_nodes);
		grid_y.resize(no_nodes);

		/* Limitation of only modelling squares */
		for ( uint i=0 ; i<grid_size ; i++ )
			for ( uint j=0 ; j<grid_size ; j++ )
			{
				grid_x[(i*grid_size)+j] = j;
				grid_y[(i*grid_size)+j] = i;
			}

		/* set grid x, y for GenericRC in Router */
		for ( uint i=0 ; i<no_nodes ; i++ )
			topology_ptr->routers[i]->set_no_nodes(no_nodes);

		for ( uint i=0 ; i<no_nodes ; i++ )
			for( uint j=0; j < ports ; j++)
				for( uint k=0; k < no_nodes ; k++) // Assuming is a square topology_ptr.
				{
					static_cast<Router*>(topology_ptr->routers[i])->set_grid_x_location(j,k, grid_x[k]);
					static_cast<Router*>(topology_ptr->routers[i])->set_grid_y_location(j,k, grid_y[k]);
				}
	}

	topology_ptr->connect_interface_routers();
	topology_ptr->connect_routers();

	/* find neighbors, after all connection are set up */
	for ( uint i=0 ; i<no_nodes ; i++ )
	{
		static_cast<Router*>(topology_ptr->routers[i])->find_neighbors(ports, vcs, credits, buffer_size);
	}

	/*  Printing out all component after setup */
	if( print_setup )
	{
		for ( uint i=0 ; i<no_nodes; i++ )
		{
			cout << "\nTPG: " << i << " " << topology_ptr->processors[i]->toString();
			cout << "\ninterface: " << i << " " << topology_ptr->interfaces[i]->toString();
			cout << "\nrouter: " << i << " " << topology_ptr->routers[i]->toString();
		}

		for ( uint i=0 ; i<links ; i++ )
		{
			cout << "\nlinka_" << i << " " << topology_ptr->link_a[i]->toString();
			cout << "\nlinkb_" << i << " " << topology_ptr->link_b[i]->toString();
		}
	}

	/********************* Fault Injection ******************/
#ifdef _DBG_FILTRE_FAULT
	uint nb_pf=0;
	for(uint i=0;i<nb_faults;i++)
	{
		if(fault_type[i]==0 || fault_type[i]==3)
		{
			topology_ptr->routers[faulty_opnodes[i]]->set_fault(faulty_outports[i]);
			nb_pf++;
		}

	}
	cout << " nb of permantent fault injected "<<nb_pf;
#else
	for(uint i=0;i<nb_faults;i++)
	{

		if(!fault_type[i])
			topology_ptr->routers[faulty_opnodes[i]]->set_fault(faulty_outports[i]);
		else
		{
			topology_ptr->routers[faulty_opnodes[i]]->set_temp_opfault(faulty_outports[i],temps_inject[i]-0.1,fault_dure[i]);
			topology_ptr->routers[faulty_ipnodes[i]]->set_temp_ipfault(faulty_inports[i],temps_inject[i]-0.1,fault_dure[i]);
		}
	}
#endif
	/* print out router status */
	for ( uint i=0 ; i<no_nodes; i++ )
	{
		IrisEvent* event_rt = new IrisEvent();
		event_rt->type = TICK_EVENT;
		Simulator::Schedule(max_sim_time-1, &NetworkComponent::process_event, topology_ptr->routers[i], event_rt);
	}

	for ( uint i=0 ; i<no_nodes; i++ )
	{
		ptr_router.push_back(topology_ptr->routers[i]);
	}

	Simulator::StopAt(max_sim_time);
	Simulator::Run();

#ifdef _PRINTOUT_DETAILS
	cerr << topology_ptr->print_stats();
	/* Init McPat for Energy model and use the counters to compute energy */
	//interface_simiris(max_sim_time);
	print_access_counts();
#endif

	ullint total_link_utilization = 0;
	ullint total_link_cr_utilization = 0;
	for ( uint i=0 ; i<links ; i++ )
	{
		total_link_utilization += topology_ptr->link_a[i]->get_flits_utilization();
		total_link_cr_utilization += topology_ptr->link_a[i]->get_credits_utilization();
	}

	for ( uint i=0 ; i<links ; i++ )
	{
		total_link_utilization += topology_ptr->link_b[i]->get_flits_utilization();
		total_link_cr_utilization += topology_ptr->link_b[i]->get_credits_utilization();
	}

	double bytes_delivered = (total_link_utilization + 0.0 )*max_phy_link_bits/(network_frequency*1e6); /* Assuming 1flit=1phit */
	double available_bw = (links - edge_links+0.0)*max_phy_link_bits*max_sim_time/(network_frequency*1e6);

	/* avg_packet_latency  */
	ullint average_packet_latency = 0;
	uint counter=0;
	cerr << endl;
	for ( uint i=0; i< mc_positions.size(); i ++ )
	{
		//    	ullint tmp=static_cast<GenericPktGen*>(topology_ptr->processors[i])->get_average_packet_latency();
		//ullint tmp=static_cast<GenericSink*>(topology_ptr->processors[mc_positions[i]])->get_average_packet_latency();
		//ullint tmp=topology_ptr->processors[i]->get_average_packet_latency();
		ullint tmp=topology_ptr->processors[mc_positions[i]]->get_average_packet_latency();
		if(tmp!=0) {
			average_packet_latency +=tmp;
			counter++;
			cerr << counter <<": node = "<< i << " average_packet_latency = "<< tmp <<endl;

		}
	}
	average_packet_latency = (average_packet_latency+0.0)/counter;
	cerr << "count = "<< counter << " average_packet_latency = "<< average_packet_latency <<endl;

#ifdef _PRINTOUT
	cerr << "\n\n************** Link Stats ***************\n";
	cerr << " Total flits passed on the links: " << total_link_utilization << endl;
	cerr << " Total credits passed on the links: " << total_link_cr_utilization << endl;
	cerr << " Links not used: " << 2*edge_links << " of " << 2*links << " links." << endl;
	cerr << " Avg flits per link used " << (total_link_utilization+0.0)/(2*(links - edge_links)) << endl;
	cerr << " Utilized BW (data only) :(A) " << bytes_delivered *1e-6<< " Mbps. " << endl;
	cerr << " Max BW available in the network:(B) " << available_bw *1e-6<< " Mbps. " << endl;
	cerr << " \% A/B " << bytes_delivered/available_bw << endl;
	cerr << " Average_packet_latency: " << average_packet_latency << " (measured at Destination)"<< endl;
	//    cerr << " SimTime: "<< Simulator::Now() << endl;
	cerr << " Last Pkt Arrival Time: "<< Simulator::LastTimer() << endl;
#endif
	ullint  tot_pkts = 0, tot_pkts_in = 0, tot_pkts_out = 0, tot_flits_in =0, tot_flits_out = 0;
	ullint  tot_pkts_drop=0, pkts_drop_NI=0 , pkts_drop_RT=0;
	ullint  tot_pshf_created=0, tot_pstf_created=0;
	ullint  tot_vs_packet_out =0;

	for ( uint i=0 ; i<no_nodes ; i++ )
	{
		tot_pkts_out += topology_ptr->interfaces[i]->get_packets_out();
		tot_pkts_in += topology_ptr->interfaces[i]->get_packets_in();
		tot_pkts += topology_ptr->interfaces[i]->get_packets();  // in + out
		tot_flits_out += topology_ptr->interfaces[i]->get_flits_out();
		tot_flits_in += topology_ptr->interfaces[i]->get_flits_in();
		pkts_drop_RT += topology_ptr->routers[i]->get_packets_drop();
		pkts_drop_NI += topology_ptr->interfaces[i]->get_packets_drop();
		tot_pshf_created += topology_ptr->routers[i]->get_pshf_created();
		tot_pstf_created += topology_ptr->routers[i]->get_pstf_created();
		tot_vs_packet_out += topology_ptr->processors[i]->get_vs_packet_out();
	}
	cerr << " No of pkts sent out  " << tot_pkts_out << "  " << endl;
	cerr << " No of virtual pkts sent out " << tot_vs_packet_out << "  " << endl;
	cerr << " No of pkts arrived  " << tot_pkts_in << "  " << endl;
	tot_pkts_out=0;
	tot_pkts_in=0;
	for ( uint i=0; i< mc_positions.size(); i ++ )
	{
		tot_pkts_out += (topology_ptr->processors[mc_positions[i]]->get_packets_in());
	}
	for ( uint i=0; i< pktgen_positions.size(); i ++ )
	{
		tot_pkts_in += (topology_ptr->processors[pktgen_positions[i]]->get_packets_out()-topology_ptr->processors[pktgen_positions[i]]->get_vs_packet_out());
	}
	cerr << "************** compare ***************\n";
	cerr << " No of pkts sent out  " << tot_pkts_out << "  " << endl;
	cerr << " No of pkts arrived  " << tot_pkts_in << "  " << endl;
	tot_pkts_drop = pkts_drop_NI + pkts_drop_RT;

	ullint sim_time_ms = (time(NULL) - sim_start_time);
	ullint 	sim_time = max_sim_time - sim_warmup_time ;


#ifdef _ORION_NIRGAM
	double power_router=0, power_link=0;
	power_router = power_main((istat->get_total_ib_cycles()+istat->get_total_rc_cycles())/sim_time,istat->get_total_ib_cycles()/sim_time
			,istat->get_total_st_cycles()/sim_time,istat->get_total_sa_cycles()/sim_time,
			istat->get_total_vca_cycles()/sim_time,istat->get_total_vca_cycles()/sim_time);
	//power_main(double buf_rl,double buf_wl,double c_tl,double n_arb,double nvc_read,double nvc_write)
	power_link = link_power(istat->get_total_flits_passed()/sim_time);
	//double link_power(double link_tl)
#ifdef _PRINTOUT
	cerr << "\n\n************** Power Stats ***************\n";
	cerr << " Power Router: " << power_router << " \tW"<<endl;
	cerr << " Power Link  : " << power_link   << " \tW"<<endl;
#endif

#else

	// orion 2
	//cerr << "\n\n************** I am here ***************\n";

    SIM_router_info_t router_info;
    SIM_router_info_t *info = &router_info;
    double Pdynamic, Pleakage, Plink;
    char path[80] = "router_power";
    double freq = 1.3e9;
    double link_len = .005;     //unit meter
    int total_cycles = sim_time;//manifold::kernel::Manifold::NowTicks();
    //double activity_factor = 0.1;//(double)stat_packets_in / total_cycles;

    info->n_total_in = ports;
    info->n_total_out = ports;
    info->n_switch_out = ports;

    //link power
    //Pdynamic = 0.5 * activity_factor * LinkDynamicEnergyPerBitPerMeter(link_len, Vdd) * freq * link_len * (double)max_phy_link_bits;
	//Pleakage = LinkLeakagePowerPerMeter(link_len, Vdd) * link_len * max_phy_link_bits;
	//Plink = (Pdynamic + Pleakage) * PARM(in_port);

    double total_router_power=0;
    double activity_factor=0, Eavg=0;

    if(0)
    {
    	for ( uint i=0 ; i<no_nodes ; i++ )
    	{

    		//activity_factor = (double)(topology_ptr->routers[i]->get_stat_flits()+0.0)/sim_time;//(double)stat_packets_in / total_cycles;
    		activity_factor = topology_ptr->routers[i]->get_stat_flits()/sim_time;//(double)stat_packets_in / total_cycles;

    		//cerr << "i="<<i<<endl;
    		SIM_router_power_t router_power;
    		SIM_router_power_t *router = &router_power;
    		double ret = SIM_router_init(info, router, NULL);
    		Eavg = SIM_router_stat_energy(info, router, 0, path, AVG_ENERGY, activity_factor, 0, freq);
    		cerr << "no "<<i<<" Router Eavg:" << Eavg <<" activity_factor " << activity_factor << "\n";
    		cerr << "no "<<i<<" Router Eavg:" << Eavg* freq << "\n";
    		total_router_power += Eavg * freq;
    	}

    	//cout << "Node:" << node_id << " Packets: " << stat_packets_in
    	cerr << "\n\n************** Power Stats ***************\n";
    	cerr << " Router Power:" << total_router_power << "\n";
    	cerr << " Average Router Power:" << total_router_power/no_nodes << "\n";
    	//<< "\t Link Power:" << Plink << "\n";
    }
	//
#endif
	/*  should not use sim_time_ms as quotient but simTime of simulator */
	/*    cerr << "\n\n************** Simulation Stats ***************\n";
    cerr << " Simulation Time: " << sim_time_ms << " s. " << endl;
    cerr << " No of pkts/terminal/s: " << (tot_pkts+0.0)/(no_nodes*sim_time_ms)<< endl;
    cerr << " No of flits/terminal/s: " << (tot_flits+0.0)/(no_nodes*sim_time_ms) << endl;
    cerr << " Total Mem Req serviced: " << tot_pkts_out/2 << endl;
	 */

	ullint last_pkt_arrival_time=Simulator::LastTimer() ;
	//if (((last_pkt_arrival_time+0.0)/max_sim_time)> 0.10)  // to check deadlock
	{
#ifdef _PRINTOUT
		cerr << "\n\n************** Simulation Stats ***************\n";
		cerr << " Simulation Time: " << sim_time_ms << " s. " << endl;
		cerr << " Cycles simulated: " << sim_time << " cycles. " << endl;
		cerr << " No of flits sent out  " << tot_flits_out << "  " << endl;
		cerr << " No of flits arrived  " << tot_flits_in << "  " << endl;
		cerr << " No of pkts sent out  " << tot_pkts_out << "  " << endl;
		cerr << " No of virtual pkts sent out " << tot_vs_packet_out << "  " << endl;
		cerr << " No of pkts arrived  " << tot_pkts_in << "  " << endl;
		//cerr << " No of pkts/terminal/cycle: " << (tot_pkts+0.0)/(no_nodes*last_pkt_arrival_time) << endl;
		cerr << " No of pkts/terminal/cycle: " << (tot_pkts_in+0.0)/(no_nodes*last_pkt_arrival_time) << endl; // tot_pkts_in
		cerr << " No of flits/terminal/cycle: " << (tot_flits_out+0.0)/(no_nodes*last_pkt_arrival_time) << endl;
		cerr << " Average_packet_latency: " << average_packet_latency << " cycles. " << " (measured at Destination)"<< endl;

		cerr << "------------ End SimIris ---------------------" << endl;
#endif

		if (nb_faults!=0)
		{
#ifdef _PRINTOUT
			cerr << "\n\n************** Fault Inject **************" << endl;
			cerr << " Fault Injection Rate:\t" << FAULT_INJECTION_RATE <<"%"<< endl;
			cerr << " No of faults injected total " << nb_faults << "  " << endl;
			uint nb_temp_faults=0;
			for (uint i=0; i<nb_faults;i++ )
			{
				if(fault_type[i])
					nb_temp_faults ++;
			}
			cerr << " No of temproral faults injected  " << nb_temp_faults << "  " << endl;
			cerr << " No of virtual pkts sent out " << tot_vs_packet_out << "  " << endl;
			cerr << " No of pseudo hf created  " << tot_pshf_created << "  " << endl;
			cerr << " No of pseudo tf created  " << tot_pstf_created << "  " << endl;
			cerr << " No of pkts dropped at NetworkInterface " << pkts_drop_NI << "  " << endl;
			cerr << " No of pkts dropped at Router " << pkts_drop_RT << "  " << endl;
			cerr << " No of pkts dropped totally " << tot_pkts_drop << "  " << endl;
			cerr << " Percentage of pkts dropped  " << (tot_pkts_drop+0.0)/tot_pkts_out*100<<"%"<< "  " << endl;
			cerr << " Percentage of flits not received  " << (tot_flits_out-tot_flits_in+0.0)/tot_flits_out*100<<"%"<< "  " << endl;
#endif
		}

#ifdef _DBG_METRIC_0
		if (cg_metric == FLIT_REMAIN )
			for ( uint i=0 ; i<no_nodes ; i++ )
			{
				cerr << "node " << i<< ":";
				for (uint j=0;j<5;j++) {
					if(router_model == VIRTUALm)
						cerr << static_cast<RouterVcMPm*>(topology_ptr->routers[i])->remain_flits[j] << "\t";
					if(router_model == VIRTUALft)
						cerr << static_cast<RouterVcMPft*>(topology_ptr->routers[i])->remain_flits[j] << "\t";
					if(router_model == VARIANTB)
						cerr << static_cast<VariantB*>(topology_ptr->routers[i])->remain_flits[j] << "\t";
				}
				cerr << endl;
			}
#endif

		// write results into files

		/*
    ostringstream arg_oss;
    string nomresult;
    arg_oss << argv[1];
    nomresult = arg_oss.str();
		 */
		string nomresult;
		nomresult = argv[1];
		size_t position_config=-1;
		position_config = nomresult.find("config");
		if (position_config == string::npos )
		{
			cerr << "string not found" << endl;
			exit(0);
		}
		nomresult.replace(position_config,6,"results");
		//nomresult += ".res";
		//    cerr << nomresult << endl;
		size_t position_it=-1;
		position_it = nomresult.rfind("_");
		if (position_it == string::npos )
		{
			cerr << "string not found" << endl;
			exit(0);
		}    nomresult.erase(position_it-3);
		nomresult += ".res";
		//    cerr << nomresult << endl;
		ofstream fd_result(nomresult.c_str(), ios_base::app);
		if (fd_result)
		{
			cerr << nomresult << " is created" << endl;
			fd_result << argv[1];
			fd_result << ";" << (tot_pkts_in+0.0)/(no_nodes*last_pkt_arrival_time) << ";"
					<<  (tot_flits_out+0.0)/(no_nodes*last_pkt_arrival_time) << ";"
					<<  average_packet_latency<< ";"
					<<  tot_pkts_drop<< ";"
					<<  (tot_pkts_drop+0.0)/tot_pkts_out*100 <<";"
					<<  tot_pkts_out<< ";"
					<<  tot_pkts_in<< ";"
					<<  last_pkt_arrival_time<< ";"
					<<  pkts_drop_NI<< ";"
					<<  pkts_drop_RT<< ";"
					<<  tot_vs_packet_out<< ";"
					<<  tot_pshf_created<< ";"
					<<  tot_pstf_created<< ";"
					<< endl;
		}
		fd_result.close();

	}

	/*else
	{
		ofstream log_run;
		log_run.open ("./log/log_run.txt",ios::app);
		log_run<< " Last Pkt Arrival Time: "<< Simulator::LastTimer() << endl;
		ofstream errorFile( "./log/ErrorFile.log", ios::app );
		cerr.rdbuf( errorFile.rdbuf() );
		dump_configuration();
		cerr << " Last Pkt Arrival Time: "<< Simulator::LastTimer() << endl;

	}*/

	Visual* vis = new Visual(topology_ptr, no_nodes, links, grid_size, network_type, rtr_model, total_link_utilization, total_link_cr_utilization, bytes_delivered, available_bw, tot_pkts, tot_flits_out, sim_time_ms);
	vis->create_new_connections();
	vis->create_graphml();

	delete topology_ptr;
	delete vis;

	return 0;
}				/* ----------  end of function main  ---------- */


#endif   /* ----- #ifndef _simmc2mesh_cc_INC  ----- */
