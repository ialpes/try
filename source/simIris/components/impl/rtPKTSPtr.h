/*
 * =====================================================================================
 *
 *       Filename:  routerVcMP.h
 *
 *    Description:  Router that can be used with meshes/multiple_vcs/DOR 
 *    1. Usess vcs purely for improving buffer and link utilization.
 *    2. Self contained: Example of how you can not use too many pseudo
 *    components
 *    3. WARNING: Does not check if the buffer is empty before sending a new
 *    pkt out. Uses the non blocking interface.
 *
 *        Version:  1.0
 *        Created:  03/11/2010 08:56:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _rtPKTSPtr_h_INC
#define  _rtPKTSPtr_h_INC

#include	"../interfaces/router.h"
#include	"genericBuffer.h"
#include	"genericRC.h"
//#include	"pvtopv_swa.h"
//#include	"genericVcAllocator.h"
#include	"genericVca.h"
#include	"ptopSwaVcs.h"
#include	"genericCrossbar.h"
#include	"../../../util/genericData.h"
#include	"../../../util/stats.h"
#include	"genericLink.h"
#include	<sys/time.h>
#include	<algorithm>
#include	<list>

extern uint send_early_credit;
extern uint stat_print_level;
extern uint do_two_stage_router;
extern void print_state_at_deadlock(void);
extern IrisStats* istat;
extern uint pkt_size;
extern CONGEST_METRIC cg_metric;

class RtPKTSPtr: public Router
{
    public:
        RtPKTSPtr ();                             /*! constructor */
        ~RtPKTSPtr();                             /*! destructor */
        void init(uint ports, uint vcs, uint credits, uint buffer_size);
        /*! Initialise the router for ports, vcs and credits. TODO buffer size
         * not used right now. Just make sure its larger than credits.*/

        void set_no_nodes( uint nodes);                 /*! For DOR the no of nodes in the network 
                                                          and the grid sizes are to be set and 
                                                          passed to RC. */
        void set_grid_x_location ( uint a, uint b, uint c);
        void set_grid_y_location ( uint a, uint b, uint c);

        void send_credit_back( uint i);

        void process_event(IrisEvent* e);
        string toString() const;
        string print_stats();
        void set_edge_links();
        vector< vector<uint> > downstream_credits;
        vector<uint> remain_flits;  // remain_flits
        void dump_buffer_state();
        double get_average_packet_latency();
        double get_last_flit_out_cycle();
        double get_flits_per_packet();
        double get_buffer_occupancy();
        double get_swa_fail_msg_ratio();
        double get_swa_load();
        double get_vca_fail_msg_ratio();
        double get_vca_load();
        double get_stat_packets();
        double get_stat_flits();
        double get_stat_ib_cycles();
        double get_stat_rc_cycles();
        double get_stat_vca_cycles();
        double get_stat_swa_cycles();
        double get_stat_st_cycles();

        double get_packets_drop();
        double get_pshf_created();
        double get_pstf_created();

        /* NoP Algorithm */
        vector< vector<uint> > free_slots_in;
        vector< vector<uint> > free_slots_out;
        struct NopData
        {
        	uint free_slots;
        	bool available;

        };
        uint nop_slect();
        uint get_sum_bf(uint port);
        void find_neighbors(uint ports, uint vcs, uint credits, uint buffer_size);

        //vector<uint>  fault_ports;//fault flag:0-nofault,1-yes
        void set_fault(uint p);
        void set_temp_opfault(uint p,double t,ullint d);
        void set_temp_ipfault(uint p,double t,ullint d);


    protected:

    private:
        vector <GenericBuffer> in_buffers;
        vector <GenericRC> decoders;
        vector <GenericRC> nop_rc;
        vector <InputBufferState> input_buffer_state;
        GenericVca vca;
        PToPSwitchArbiterVcs swa;
        vector< list< uint> > vc_alloc;
        vector< vector< uint> > sw_alloc;
        vector< vector <uint> >request_op;
        vector <uint> available_ports;
        vector <uint> available_vcs;
        vector < vector<uint> > cr_time;
        vector <pair<int,int> >link_number;

        bool ticking;

        vector<uint>  fault_inports;//fault flag:0-nofault,1-yes
        vector<uint>  fault_outports;//fault flag:0-nofault,1-yes

        void handle_link_arrival_event(IrisEvent* e);
        void handle_tick_event(IrisEvent* e);
        void handle_detect_deadlock_event(IrisEvent*);
        void handle_reset_stat_event( IrisEvent* e);
        void handle_fault_inject_event(IrisEvent* e);
        void handle_fault_remove_event(IrisEvent* e);

        void do_switch_traversal();
        void do_switch_allocation();
        void do_switch_allocation_m();
        void do_input_buffering();
        void do_virtual_channel_allocation();
        void do_vca();
        void do_swa();
        void do_st();
        void request_switch_allocation();

        void set_temp_fault(uint p,uint type);
        void remove_fault(uint p,uint type);

        /*! These are the statistics variables */
        double stat_buffer_occupancy;
        uint stat_packets;
        uint stat_flits;
        double stat_total_packet_latency;
        double last_flit_out_cycle;  /* indicates the last active cycle for the router */
        double stat_swa_fail_msg_ratio;
        double stat_swa_load;
        double stat_vca_fail_msg_ratio;
        double stat_vca_load;
        ullint stat_sim_total_time;
        vector< vector<uint> > stat_packet_out;
        vector< vector<uint> > stat_flit_out;
        bool is_mc_router;
        ullint stat_ib_cycles;
        ullint stat_rc_cycles;
        ullint stat_vca_cycles;
        ullint stat_swa_cycles;
        ullint stat_st_cycles;

        uint stat_pkts_drop;
        uint stat_pshf_created;
        uint stat_pstf_created;

        vector<pair<uint, uint> >metrics;

        struct sort_pred_increase
            {
            bool operator () (const pair<uint,uint>& left, const pair<uint,uint>& right)
                {
                return left.second < right.second;
                }
            };
        struct sort_pred_decrease
        {
        	bool operator () (const pair<uint,uint>& left, const pair<uint,uint>& right)
        	{
        		return left.second > right.second;
        	}
        };
        inline void sort_metrics_increase()
        {
            sort(metrics.begin(), metrics.end(), sort_pred_increase());
        }
        inline void sort_metrics_decrease()
        {
            sort(metrics.begin(), metrics.end(), sort_pred_decrease());
        }


}; /* -----  end of class RtPKTSPtr  ----- */

#endif   /* ----- #ifndef _rtPKTSPtr_h_INC  ----- */

