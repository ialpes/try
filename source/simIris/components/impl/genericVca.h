/*
 * =====================================================================================
// Copyright 2010 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2010, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.
 * =====================================================================================
 */

#ifndef  _GENERICVCA_H_INC
#define  _GENERICVCA_H_INC

#include	"../interfaces/networkComponent.h"
#include	"../../data_types/impl/flit.h"
#include	"../../../util/genericData.h"
#include	<vector>
#include	<fstream>
#include	<algorithm>

//using namespace std;
//extern SW_ARBITRATION sw_arbitration;
extern message_class priority_msg_type;

//extern Router_params* r_param; //used in sst

struct Vca_unit
{
    Vca_unit(): in_port(-1), in_vc(-1), out_port(-1), out_vc(-1), is_valid(false){}
    uint in_port;
    uint in_vc;
    uint out_port;
    uint out_vc;
    bool is_valid;
};  /* -----  struct VCA_unit  ----- */

class GenericVca
{
    public:
        GenericVca ();
        ~GenericVca ();

        void resize ( uint p, uint v );
        bool request( uint op, uint ovc, uint ip, uint ivc);
        void pick_winner( void );
        void clear_winner( uint op, uint oc, uint ip, uint ic);
        void change_owner( uint pip, uint pic, uint op, uint oc, uint nip, uint nic);//pip:previous ip, nip:new ip
        uint no_requestors ( uint op );

        std::string toString() const;
        uint address;
        std::string name;
        uint node_ip;

        // should be visible in router class
        std::vector < std::vector <Vca_unit> > current_winners;

        // useful inlines
        inline bool is_empty ( uint port ) const
        {
            for ( uint i=0; i< ports* vcs; ++i )
                if (requesting_inputs.at(port).at(i).is_valid)
                    return false;

            return true;
        }	

        inline bool is_empty ( void ) const
        {
            for ( uint p=0; p< ports; ++p )
                if (!is_empty(p))
                    return false;

            return true;
        }

        inline bool is_requested ( uint op, uint ovc, uint ip, uint ivc ) const
        {
            //return requesting_inputs.at(op).at(ip+ ports*ivc).is_valid ;
            return requesting_inputs.at(op).at(ip*vcs + ivc).is_valid ;
        }

        inline uint no_vc_available (uint op)
        {
        	return ovc_tokens.at(op).size();
        }

    private:
        //Requestor matrix for each port
        std::vector < std::vector <Vca_unit> > requesting_inputs;
        // pool of grants to issue from, nb of tokens means nb of vc available
        std::vector < std::vector <uint> > ovc_tokens;
        // Winner list and index of last winner to round robin
        // out_ports x out_vc matrix, for given op oc who is last_winner
        std::vector < std::vector <uint> > last_winner;
        uint ports;
        uint vcs;

}; /* -----  end of class GenericVca  ----- */

#endif   /* ----- #ifndef _GENERICVCA_H_INC  ----- */
