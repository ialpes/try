/*!
 * =====================================================================================
 *
 *       Filename:  genericPktGenVn.h
 *
 *    Description:  stochastic pkt gen.
 *    can be used as a PE 
 *    uses the gsl fuctions for distributions ( can pick each of these
 *    parameters: dest, injection_rate,length based on a distribution.)
 *    Note inclusion of files such as gsl_rng
 *
 *        Version:  1.0
 *        Created:  02/20/2010 02:09:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _genericPktGenVn_h_INC
#define  _genericPktGenVn_h_INC

#include        <gsl/gsl_rng.h>
#include        <gsl/gsl_randist.h>
#include        "genericInterfaceNB.h"
#include        "../../../util/genericData.h"
#include        "../interfaces/processor.h"
#include        "../../data_types/impl/highLevelPacket.h"
#include        "../../data_types/impl/lowLevelPacket.h"
#include	"../../../memctrl/request.h"
#include        "../../../memctrl/mshr.h"
#include	"../../../util/mc_constants.h"
#include        <math.h>
#include        <fstream>
#include        <deque>
#include        <string.h>
#include        <sys/time.h>

#define GSL_RNG_SEED 12345

#define DEFAULT_RAN_MAX_TIME 100
#define MAX_ADDRESS 3

extern uint no_nodes;
extern uint grid_size;
extern uint no_mcs;
extern uint mean_irt;
extern uint pkt_payload_length;
extern message_class terminal_msg_class;
extern bool do_request_reply_network;
extern pkt_pattern pattern_type;

class GenericPktGenVn : public Processor
{

    private:
        uint vcs;
        uint no_nodes;
        ullint max_sim_time;
        uint irt;
        string out_filename;
        string trace_name;
        uint no_outstanding;
        ofstream out_file;
        ifstream *trace_filename;
        unsigned int last_vc;
        void handle_new_packet_event(IrisEvent* e);
        void handle_ready_event(IrisEvent* e);
        void handle_out_pull_event(IrisEvent* e);
        void handle_reset_stat_event( IrisEvent* e);
        void handle_vs_pkt_arrival_event( IrisEvent* e);
        vector<bool> vs_request;
        bool flag_enable;



    public :
        GenericPktGenVn();
        ~GenericPktGenVn();
        /* stats variables */
        //volatile unsigned int stat_packets_out;
        unsigned int stat_packets_out;
        unsigned int stat_packets_out_count;
        volatile unsigned int stat_packets_in;
        double stat_min_pkt_latency;
        double stat_last_packet_out_cycle;
        double stat_last_packet_in_cycle;
        ullint stat_total_lat;
        ullint stat_hop_count;
        uint stat_vs_pkt_out;

        vector< bool > ready;
        bool sending;
        ullint max_time;
        void setup(uint no_nodes, uint vcs, uint max_sim_time);
        void finish();
        void process_event(IrisEvent* e);
        string toString() const;
        string print_stats() const;
        void set_trace_filename( string filename );
        void set_no_vcs ( uint v );
        void set_output_path( string outpath );
        vector <uint> mc_node_ip;
        void enable(uint vcs, bool enable);
        void log_enable();
        uint node_transpose();
        ullint get_average_packet_latency();
        ullint get_vs_packet_out();
        ullint get_packets_out () ;
        ullint get_packets_in () ;


        /* for debugging.. break if not sending for too long */
        ullint lastSentTime;

        /* For statistical distrib */
        const gsl_rng_type * T;
        gsl_rng * arate_gen;
        gsl_rng * dest_gen;
        gsl_rng * plen_gen;
};



#endif   /* ----- #ifndef _genericPktGenVn_h_INC  ----- */

