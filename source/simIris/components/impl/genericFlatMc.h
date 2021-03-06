/*!
 * =====================================================================================
 *
 *       Filename:  genericFlatMc.h
 *
 *    Description:  This class can be used to stub a memmory controller with
 *    infinite queues. You can adjust the return latency(200)
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

#ifndef  _genericflatmc_h_INC
#define  _genericflatmc_h_INC

#include        "genericInterfaceNB.h"
#include        "../../../util/genericData.h"
#include        "../interfaces/processor.h"
#include        "../../data_types/impl/highLevelPacket.h"
#include        "../../../memctrl/request.h"
#include        <fstream>
#include        <deque>

#define DEFAULT_RAN_MAX_TIME 100
#define MAX_ADDRESS 3
//#define MAX(a,b) (((a)<(b))?(b):(a))
//#define MIN(a,b) (((a)<(b))?(a):(b))

using namespace std;

extern uint mc_response_pkt_payload_length;

class GenericFlatMc : public Processor
{

    private:
        uint vcs;
        uint no_nodes;
        unsigned long long int max_sim_time;
        ullint last_packet_in_cycle;
        ullint last_packet_out_cycle;
        ullint packets_in;
        deque< HighLevelPacket > out_packets;
        string out_filename;
        ofstream out_file;
        vector< bool > ready;
        bool sending;
        void handle_new_packet_event(IrisEvent* e);
        void handle_ready_event(IrisEvent* e);
        void handle_out_pull_event(IrisEvent* e);
	void convertFromBitStream(Request* req, HighLevelPacket *hlp);
        long int packets_pending;
        deque <unsigned long long int> pending_packets_time;
        deque <HighLevelPacket*> pending_packets;

    public :
        GenericFlatMc();
        ~GenericFlatMc();

        /* stats variables */
        unsigned int packets;
        ullint packets_out;
        uint last_vc;
        double min_pkt_latency;
        unsigned long long int waiting_at_injection;

        unsigned long long int max_time;
        void setup(uint no_nodes, uint vcs, uint max_sim_time);
        void finish();
        void process_event(IrisEvent* e);
        string toString() const;
        string print_stats() const;
        void set_output_path( string outpath );
        vector <uint> mc_node_ip;
        HighLevelPacket* outstanding_hlp;
        ullint get_average_packet_latency();
};



#endif   /* ----- #ifndef _genericflatmc_h_INC  ----- */

