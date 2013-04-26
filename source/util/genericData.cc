/*
 * =====================================================================================
 *
 *       Filename:  genericData.cc
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  02/21/2010 05:06:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _genericdata_cc_INC
#define  _genericdata_cc_INC

#include	"genericData.h"

LinkArrivalData::LinkArrivalData()
{
    valid = false;
}

LinkArrivalData::~LinkArrivalData()
{
}

VirtualChannelDescription::VirtualChannelDescription()
{
}

string
InputBufferState::toString() const
{
    stringstream str;
    str << "input_buffer_state"
//        << " inport: " << input_port
//        << " inch: " << input_channel
//        << " outport: " << output_port
//        << " outch: " << output_channel
//        << " destination: " << destination
        << " address: " << address;
    switch( pipe_stage)
    {
        case INVALID:
            str << " INVALID";
            break;
        case IB:
            str << " IB";
            break;
        case EMPTY:
            str << " EMPTY";
            break;
        case FULL:
            str << " FULL";
            break;
        case ROUTED:
            str << " ROUTED";
            break;
        case SWA_REQUESTED:
            str << " SWA_REQUESTED";
            break;
        case SW_ALLOCATED:
            str << " SW_ALLOCATED";
            break;
        case SW_TRAVERSAL:
            str << " SW_TRAVERSAL";
            break;
        case REQ_OUTVC_ARB:
            str << " REQ_OUTVC_ARB";
            break;
    }

    return str.str();
}

InputBufferState::InputBufferState()
{
    pipe_stage = INVALID;
    copy_hf = NULL;
    remain_flits = 0;
    input_port   = -1;
    input_channel= -1;
    output_port  = -1;
    output_channel = -1;
    stat_pkt_intime = 0;
    arrival_time = 0;
    length = 0;
    credits_sent = 0;
    possible_ovcs.clear();
    possible_oports.clear();
    //msg_class;
    address = -1;
    destination = -1;
    source = -1;
    clear_message = false;
    sa_head_done = false;
    flits_in_ib = 0;
    sent_head = false;
    has_pending_pkts = false;
    pkt_in_progress = false;
    bodies_sent = 0;
    copy_hf = NULL;
    ptr_pstf= NULL;
    //vn;
    is_misrouting.clear(); /* indicate whether the possible output port is misrouting or not */
    is_pseudo_tf_created = false;
    is_pseudo_hf_created = false;
    is_pktrj = false;
    has_pstf = false;
    is_resent = false;

 }


 #endif   /* ----- #ifndef _genericdata_cc_INC  ----- */

