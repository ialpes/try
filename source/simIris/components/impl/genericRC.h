/*!
 * =====================================================================================
 *
 *       Filename:  genericrc.h
 *
 *    Description:  All routing algos are implemented here for the time being.
 *    Need to re arrange
 *
 *        Version:  1.0
 *        Created:  02/19/2010 11:54:57 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */
#ifndef  _genericrc_h_INC
#define  _genericrc_h_INC

#include	"../interfaces/addressDecoder.h"
#include	"../../../util/genericData.h"
#include        "../../data_types/impl/flit.h"
#include	"../../data_types/impl/lowLevelPacket.h"

using namespace std;

extern ROUTING_SCHEME rc_method;
extern uint req_rep;
extern uint grid_size;
extern uint no_nodes;
extern bool do_request_reply_network;
extern uint ports;

class GenericRC
{
    public:
        GenericRC ();
        ~GenericRC(){}
        void push( Flit* f, uint vc,  vector <vector<uint> > const vcr );
        void push( Flit* f, uint vc);
        void push( Flit* f, uint vc, vector<uint> nbs, vector<uint> fnp);
        void push (Flit* f, uint ch, const vector<uint> nbs, const vector<uint> fnp, const vector<vector<uint*> > fon);
        uint get_output_port ( uint channel);
        uint get_output_port ( uint channel, vector <vector<uint> > const vcr, uint ip,uint inport);
        uint speculate_port ( Flit* f, uint ch );
        uint speculate_channel ( Flit* f, uint ch );
        uint get_virtual_channel ( uint ch );
        void resize ( uint ch );
        uint get_no_channels ();
        uint no_adaptive_ports( uint ch );
        uint no_adaptive_vcs( uint ch );
        bool get_output_port_status( uint ch );
        bool is_empty();
        string toString() const;
        uint node_ip;
        uint address;
        vector < uint > grid_xloc;
        vector < uint > grid_yloc;
        static uint missrouting_counter;// to be moved into head flit
        bool get_is_pkt_rejoin( uint ch );

    protected:

    private:
        string name;
        uint vcs;
        uint route_x_y( uint addr );
        void route_dy_xy( HeadFlit* hf );
        void route_torus( HeadFlit* hf );
        void route_ring( HeadFlit* hf );
        void route_negative_first(HeadFlit* hf);
        void route_west_first(HeadFlit* hf);
        void route_north_last(HeadFlit* hf);
        void route_north_last_non_minimal(HeadFlit* hf);
        void route_odd_even(HeadFlit* hf);
        void route_odd_even_m(HeadFlit* hf);
        void route_hierachy(HeadFlit* hf);
        void route_vn_south_last(HeadFlit* hf);
        void route_vn_south_last(HeadFlit* hf, vector<uint> nbs);
        void route_vn_south_last(HeadFlit* hf, vector<uint> nbs, vector<uint> no_fon);
        void route_vn_north_last(HeadFlit* hf);
        void route_vn_north_last(HeadFlit* hf, vector<uint> nbs);
        void route_vn_north_last(HeadFlit* hf, vector<uint> nbs, vector<uint> no_fon);
        vector < uint > hierachy_out(uint dst);
        vector < uint > possible_out_ports;
        vector < uint > is_misrouting;
        vector < uint > misrouting_out_ports;
        vector < uint > possible_out_vcs;

        /*
         * =====================================================================================
         *        Class:  Address
         *  Description:  
         * =====================================================================================
         */
        class Address
        {
            public:
				Address(){};
				~Address(){};
                bool route_valid;
                unsigned int channel;
                unsigned int out_port;
                uint last_adaptive_port;
                static vector<uint> last_aport;
                uint last_vc;
                vector < uint > possible_out_ports;
                vector < uint > possible_out_vcs;
                vector < uint > is_misrouting;
                bool is_pkt_rejoin;

            protected:

            private:

        }; /* -----  end of class Address  ----- */

        vector<Address> addresses;

}; /* -----  end of class GenericRC  ----- */

#endif   /* ----- #ifndef _genericrc_h_INC  ----- */

