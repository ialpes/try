/*
 * =====================================================================================
 *
 *       Filename:  genericData.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/21/2010 04:59:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */
#ifndef  _genericdata_h_INC
#define  _genericdata_h_INC

#include	"../simIris/data_types/impl/flit.h"
#include	"simIrisComponentHeader.h"
//#include    "../frontend/impl/topology.h"

using namespace std;

//Topology * topology_ptr = NULL;

enum ROUTING_SCHEME { XY, DY_XY, HIERACHY, VIRTUALNETWORK, NEGATIVE_FIRST, ODD_EVEN, WEST_FIRST, NORTH_LAST, NORTH_LAST_NON_MINIMAL, TORUS_ROUTING, RING_ROUTING };
enum SW_ARBITRATION { ROUND_ROBIN, FCFS, ROUND_ROBIN_PRIORITY };
enum RouterPipeStage { 
   /* 0 */ INVALID, 
   /* 1 */ EMPTY,
   /* 2 */ IB, 
   /* 3 */ FULL, 
   /* 4 */ ROUTED,  // temporary used for the state where inport fault node waiting for the rest part of pkt
   /* 5 */ VCA_REQUESTED, 
   /* 6 */ VCA_COMPLETE,
   /* 7 */ SWA_REQUESTED,
   /* 8 */ SW_ALLOCATED,
   /* 9 */ SW_TRAVERSAL,
   /* 10*/ REQ_OUTVC_ARB,

};

enum  ROUTER_MODEL { PHYSICAL, VIRTUAL,VIRTUAL4Stg,VIRTUALm,VIRTUALft,VARIANTA,VARIANTB,RTDBAR,RTRCA1D,PRDBAR,VBDBAR,SPDBAR,RTPGCP,RTPKTSP,RTPKTSPa,RTPKTSPb,RTPKTPR,RTPKTPRa,RTPKTPRtr,RTPKTSPtr};
enum  MC_MODEL { GENERIC_MC, FLAT_MC, SINK};
enum  TERMINAL_MODEL { GENERIC_PKTGEN, TPG, PKTGENVN, CUSTOMPG};
enum  CONGEST_METRIC { NOMETRIC, BUFFER_SIZE, VIRTUAL_CHANNEL, CROSSBAR_DEMANDE, FLIT_REMAIN,XB_VC,VC_BF,XB_BF, MIX1 , MIX2, MIX3, MIX4, MIX5};


class InputBufferState
{
    public:
        InputBufferState();
        ~InputBufferState(){}
        uint input_port;
        uint input_channel;
        uint output_port;
        uint output_channel;
        double stat_pkt_intime;
        double arrival_time;
        int length;
        int credits_sent;
        vector < uint > possible_ovcs;
        vector < uint > possible_oports;
        RouterPipeStage pipe_stage;
        message_class msg_class;
        ullint address;
        uint destination;
        uint source;
        bool clear_message;
        bool sa_head_done;
        uint flits_in_ib;
        bool sent_head;
        bool has_pending_pkts;
        bool pkt_in_progress;
        int bodies_sent;
        HeadFlit* copy_hf;
        TailFlit* ptr_pstf;
        string toString () const;
        virtual_network vn;
        vector < bool > is_misrouting; /* indicate whether the possible output port is misrouting or not */
        uint remain_flits;
        bool is_pseudo_tf_created;
        bool is_pseudo_hf_created;
        bool is_pktrj;
        bool has_pstf;
        bool is_resent;

};

/*
 * =====================================================================================
 *        Class:  LinkArrivalData
 *  Description:  
 * =====================================================================================
 */
class LinkArrivalData
{
    public:
        LinkArrivalData ();                             /* constructor */
        ~LinkArrivalData ();                             /* constructor */
        uint type;
        uint vc;
        Flit* ptr;
        bool valid;
        void* p;

    protected:

    private:

}; /* -----  end of class LinkArrivalData  ----- */

/*
 * =====================================================================================
 *        Class:  VirtualChannelDescription
 *  Description:  
 * =====================================================================================
 */
class VirtualChannelDescription
{
    public:
        VirtualChannelDescription ();                             /* constructor */
        uint vc;
        uint port;

    protected:

    private:

}; /* -----  end of class VirtualChannelDescription  ----- */

/*
 * =====================================================================================
 *        Class:  RouteEntry
 *  Description:  
 * =====================================================================================
 */
class RouteEntry
{
    public:
        RouteEntry ();                             /* constructor */
        uint destination;
        vector< vector<uint> > ports;
        vector< vector<uint> > channels;

    protected:

    private:

}; /* -----  end of class RouteEntry  ----- */

class SA_unit
{
    public:
        SA_unit(){};
        uint port;
        uint ch;
        ullint in_time;
        ullint win_cycle;
};

#endif   /* ----- #ifndef _genericdata_h_INC  ----- */


