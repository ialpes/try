/*!
 * =====================================================================================
 *
 *       Filename:  genericInterfaceNBb.h
 *
 *    Description:  Component that connects a PE to a router. This converts
 *    a HLP t flits and sends them out on the respective channel. It is
 *    non-blocking in the sense that flits are sent as long as there are
 *    credits for the downstream buffer available. In cases where you need to
 *    check send a new packet only if the next buffer is completely empty this
 *    wont work.
 *
 *        Version:  1.0
 *        Created:  02/24/2010 02:04:22 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _genericInterfaceNBb_h_INC
#define  _genericInterfaceNBb_h_INC

#include	"../../../util/genericData.h"
#include	"../../../util/stats.h"
#include	"../../data_types/impl/highLevelPacket.h"
#include	"../../data_types/impl/irisEvent.h"
#include	"../interfaces/interface.h"
#include	"../interfaces/networkComponent.h"
#include	"../interfaces/processor.h"
#include	"../interfaces/buffer.h"
#include	"../impl/genericLink.h"
#include	"genericBuffer.h"
#include	"genericArbiter.h"
#include	<queue>
#include	<vector>
#include	<math.h>
#include	<algorithm>

extern uint do_two_stage_router;
extern vector <uint> mc_positions;
extern IrisStats* istat;

using namespace std;

class GenericInterfaceNBb : public Interface
{
    public:
        GenericInterfaceNBb();                             /* constructor */
        ~GenericInterfaceNBb();                             /* constructor */

        uint get_no_credits() const;
        void set_no_credits( int credits );
        void set_no_vcs ( uint v );
        void set_buffer_size( uint b );
        void set_buffer_size( uint v, uint b );
        void setup( uint v, uint cr, uint bs);

        string toString() const;
        void process_event( IrisEvent* e);
        string print_stats();
        ullint get_packets_out();
        ullint get_vs_packets_in();
        ullint get_packets();
        ullint get_flits_out();
        bool is_mc_interface;
        ullint get_packets_in();
        ullint get_flits_in();
        ullint get_packets_drop();

    protected:

    private:
        uint vcs;
        uint buffer_size;
        int credits;
        bool in_packet_cleared;
        vector <bool> in_packet_complete;
        uint flast_vc;

        bool ticking;
        bool vs_ticking;
        GenericBuffer out_buffer;
        GenericBuffer in_buffer;
        GenericArbiter out_arbiter;
        vector < int > downstream_credits;

        GenericBuffer vs_buffer;

        /* The current packet being pushed into the output buffers */
        vector < LowLevelPacket> out_packets;
        vector < uint > out_packet_flit_index;

        /* The current packet being pulled from the input buffers */
        vector < LowLevelPacket> in_packets;
        vector < uint > in_packets_flit_index;
        vector < bool> in_packets_valid;

        vector < bool > in_ready; //indicate whether the vc in proc is free or not

        /* in_buffer to in_packets */
        vector <bool > in_packets_ready;
        vector <bool > is_requested_inbf;// if a in_buffer[] has made request, index by in_buffer[]
        vector <uint > inpkt_ch_for_inbf;  // store a distributed channel in_packets[] for a in_buffer[], index by in_buffer[]
        deque  <uint > in_requests; // fifo of request of a channel in_packets[] for a in_buffer[]

        vector <bool > is_equal; // whether the number of pseudo HeadFlit equals to that of pseudo TailFlit

        vector <bool > is_vs; // whether the pkt is sent to NI for virtual source
        vector <bool > is_tf_received; // when is_vs check if is_tf_received, then pkt_fnd is true
        vector <bool > is_tf_sent; //

        /* event handlers */
        void handle_new_packet_event( IrisEvent* e);
        void handle_ready_event( IrisEvent* e);
        void handle_tick_event( IrisEvent* e);
        void handle_link_arrival( IrisEvent* e);
        void handle_reset_stat_event( IrisEvent* e);
        void handle_vs_link_arrival( IrisEvent* e);
        void handle_vs_tick_event( IrisEvent* e);
        void handle_check_event (IrisEvent* e);

        /* stats */
        ullint flits_in;
        ullint packets_in;
        ullint flits_out;
        ullint packets_out;
        ullint vs_packets_in;
        ullint total_packets_in_time;
        ullint stat_pkts_drop;

}; /* -----  end of class GenericInterfaceNBb  ----- */

#endif   /* ----- #ifndef _GenericInterfaceNBb_h_INC  ----- */


