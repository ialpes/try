/*!
 * =====================================================================================
 *
 *       Filename: \file genericInterface.cc
 *
 *    Description: \brief This file is the implementation for the
 *    GenericInterfacePhy class. It implements a thin interface layer that
 *    converts between high level and low level packets and outputs flits to a
 *    router. The model is suited for use with a single channel physical router.
 *    Does not handle multiple channels(phy or virtual).
 *
 *        Version:  1.0
 *        Created:  02/21/2010 03:46:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _genericInterfacePhy_cc_INC
#define  _genericInterfacePhy_cc_INC

#include        "genericInterface.h"
#include	<algorithm>

GenericInterfacePhy::GenericInterfacePhy ()
{
    name = "Interface";
    ticking =false;
}               /* -----  end of function GenericInterfacePhy::GenericInterfacePhy  ----- */

GenericInterfacePhy::~GenericInterfacePhy ()
{
    in_packets.clear();
    out_packets.clear();

}               /* -----  end of function GenericInterfacePhy::~GenericInterfacePhy  ----- */

void
GenericInterfacePhy::setup (uint v, uint cr)
{
    vcs =v;
    int credits = cr;
    /* All this is part of the init */
    address = myId();
    //    convert_packet_cycles = DEFAULT_CONVERT_PACKET_CYCLES;

    vector <uint>::iterator itr = find(mc_positions.begin(), mc_positions.end(), node_ip);
    if ( itr != mc_positions.end())
        is_for_mc = true;
    else
        is_for_mc = false;

    in_packet_complete = false;
    in_buffer.resize(vcs, buffer_size);
    out_buffer.resize(vcs, buffer_size);
    downstream_credits.resize(vcs);

    out_packet_flit_index.resize(vcs);
    in_ready.resize(vcs);
    in_packets_flit_index.resize(vcs);

    in_packets.resize(vcs);
    in_packets_valid.resize(vcs);
    out_packets.resize(vcs);


    for ( uint i=0; i<vcs ; i++ )
    {
        out_packet_flit_index[i] = 0;
        in_ready[i] = true;
        in_packets[i].destination = address;
        in_packets[i].virtual_channel = i;
        in_packets[i].length= 10;
        downstream_credits[i] = credits;
    }

    /*  init stats */
    packets_out = 0;
    flits_out = 0;
    packets_in = 0;
    flits_in = 0;
    total_packets_in_time = 0;

    /* Init complete */


    return;
}               /* -----  end of function GenericInterfacePhy::setup  ----- */

void
GenericInterfacePhy::set_no_vcs( uint v )
{
    vcs = v;
}

void
GenericInterfacePhy::set_buffer_size( uint b )
{
    buffer_size = b;
}

/* We care about credits on the interface only for the output buffers ie on
 * the output side maintain credit info for the downstream buffers */
uint
GenericInterfacePhy::get_no_credits () const
{
    return credits;
}               /* -----  end of function GenericInterfacePhy::get_no_credits  ----- */

void
GenericInterfacePhy::set_no_credits ( int c)
{
    credits = c;
    return;
}               /* -----  end of function GenericInterfacePhy::set_no_credits  ----- */

string
GenericInterfacePhy::toString () const
{
    stringstream str;
    str << "GenericInterfacePhy: "
        << "\t VC: " << out_packets.size()
        << "\t address: " << address << " node_ip: " << node_ip
        << "\t OutputBuffers: " << out_buffer.toString()
        << "\t InputBuffer: " << in_buffer.toString()
        ;
    return str.str();
}               /* -----  end of function GenericInterfacePhy::toString  ----- */

void
GenericInterfacePhy::process_event(IrisEvent* e)
{
    switch(e->type)
    {
        /*! READY_EVENT: A ready is the credit at a packet level with the
         * injection/ejection point */
        case READY_EVENT:
            handle_ready_event(e);
            break;

            /*! LINK_ARRIVAL_EVENT: This is a flit/credit arrival from the
             * network. (Incoming path)*/
        case LINK_ARRIVAL_EVENT:
            handle_link_arrival(e);
            break;

            /*! NEW_PACKET_EVENT: This is a high level packet from the
             * processor connected to this node */
        case NEW_PACKET_EVENT:
            handle_new_packet_event(e);
            break;

            /*! TICK_EVENT: Update internal state of the interface. This involves
             * two distinct paths ( Incoming and Outgoing ). Push flits into the
             * output buffer and send them out after all flits for a packet are
             * moved to the output buffer(Outgoing path). Push flits from the
             * input buffer to a low level packet. Convert the low level packet to
             * a high level packet when complete and then send it to the injection
             * processor connected to this node (Incoming path). */
        case TICK_EVENT:
            handle_tick_event(e);
            break;

        default:
            cout << "**Unkown event exception! " << e->type << endl;
            break;
    }
}               /* -----  end of function GenericInterfacePhy::process_event ----- */

void
GenericInterfacePhy::handle_ready_event( IrisEvent* e)
{
    /*! \brief  These are credits from the processor and hence do not update flow
     *  control state. Flow control state is updated at the link arrival event
     *  for a credit. */

#ifdef _DEBUG_INTERFACE
    _DBG(" handle_ready_event %d ", e->vc);
#endif

    in_ready[0] = true;
    if(!ticking)
    { 
        ticking = true;
        IrisEvent* event = new IrisEvent();
        event->type = TICK_EVENT;
        event->vc = 0;
        Simulator::Schedule( floor(Simulator::Now())+1, &NetworkComponent::process_event, this, event);
    }

    delete e;
    return;
}               /* -----  end of function GenericInterfacePhy::handle_ready_event  ----- */

void
GenericInterfacePhy::handle_link_arrival ( IrisEvent* e)
{
    /*! link arrival can be for a flit or credit.
     * FLIT: Add incoming flit to buffer and send a credit back. Credit for
     * the tail flit is not sent here. This is held till we know the ejection
     * port is free. ( This is modelled here for the additional MC
     * backpressure modelling and need not be this way).
     * CREDIT: Update state for downstream credits. 
     * Generate a TICK event at the end of it. This will restart the interface
     * if it is blocked on a credit(outgoing path) or initiate a new
     * packet(incoming path).*/
    LinkArrivalData* uptr = static_cast<LinkArrivalData* >(e->event_data.at(0));

    if(uptr->type == FLIT_ID)
    {
        flits_in++;
        in_buffer.change_push_channel(0);
        in_buffer.push(uptr->ptr);
        if( uptr->ptr->type == TAIL )
        {
            packets_in++;
            total_packets_in_time += (ullint)(ceil(Simulator::Now() - 
                                                   static_cast<TailFlit*>(uptr->ptr)->packet_originated_time));
            static_cast<TailFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now());
        }
#ifdef _DEBUG_INTERFACE
        _DBG(" handle_link_arrival FLIT ticking: %d type: %d type: %d", uptr->type, ticking, uptr->ptr->type);
#endif

        if( uptr->ptr->is_single_flit_pkt )
        {
            packets_in++;
            total_packets_in_time += (ullint)(Simulator::Now() - static_cast<HeadFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now())-static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency;
        }
        uptr->valid = false;

        if( !(uptr->ptr->type == TAIL || uptr->ptr->is_single_flit_pkt))
        {
            LinkArrivalData* arrival = new LinkArrivalData();
            arrival->vc = 0;
            arrival->type = CREDIT_ID;
            IrisEvent* event = new IrisEvent();
            event->type = LINK_ARRIVAL_EVENT;
            event->vc = 0;
            event->event_data.push_back(arrival);
            event->src_id = address;
            Simulator::Schedule( floor(Simulator::Now())+0.75, 
                                 &NetworkComponent::process_event, static_cast<GenericLink*>(input_connection)->input_connection, event);
            static_cast<GenericLink*>(input_connection)->credits_passed++;

        }
    }
    else if ( uptr->type == CREDIT_ID)
    {
        downstream_credits[uptr->vc]++;
#ifdef _DEBUG_INTERFACE
        _DBG(" got a credit vc: %d ftype: %d no_of_credits: %d ", uptr->vc, uptr->type, downstream_credits[uptr->vc] );
#endif
    }
    else
    {
        cout << "Exception: Unk link arrival data type! " << endl;
    }

    if(!ticking)
    {
        ticking = true;
        IrisEvent* new_event = new IrisEvent();
        new_event->type = TICK_EVENT;
        new_event->vc = e->vc;
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfacePhy::process_event, this, new_event);
    }
    delete uptr;
    delete e;
    return;
}               /* -----  end of function GenericInterfacePhy::handle_link_arrival  ----- */

void
GenericInterfacePhy::handle_new_packet_event(IrisEvent* e)
{
    HighLevelPacket* pkt = static_cast<HighLevelPacket*>(e->event_data.at(0));
    if( out_packets[0].size() !=0 )
    {
        cout << "Error got new pkt when old not emptied" << endl;
        exit(1);
    }
    pkt->to_low_level_packet(&out_packets[0]);


    out_packet_flit_index[ pkt->virtual_channel ] = 0;
    delete pkt;

#ifdef _DEBUG_INTERFACE
    _DBG("handle_new_packet_event vc: %d ", static_cast<unsigned int >(pkt->virtual_channel) );
#endif

    if( !ticking)
    {
        ticking = true;
        IrisEvent* event = new IrisEvent();
        event->type = TICK_EVENT;
        event->vc = e->vc;
        Simulator::Schedule(floor(Simulator::Now())+1, &NetworkComponent::process_event, this, event);
    }

    delete e;
    return;
}               /* -----  end of function GenericInterfacePhy::handle_new_packet_event  ----- */

void
GenericInterfacePhy::handle_tick_event(IrisEvent* e)
{
    ticking = false;
    /*---------- This is on the output side. From processor out to network --------- */
    if( downstream_credits[0]>0 && out_buffer.get_occupancy(0)>0
        && in_packet_complete )
    {
        Flit* f;
        bool send_flit = true;
        out_buffer.change_pull_channel(0);
        /* Needed for adaptive VCS and ensures the check for one msg per input
         * port of a router. However in DOR with packets of variable length
         * this leads to poor buffer utilization. Hence removing.*/
        f= out_buffer.peek();
        if(f->type == HEAD  && downstream_credits[0] != credits)
            send_flit = false;

        if( send_flit )
        {
            IrisEvent* event = new IrisEvent();
            LinkArrivalData* arrival =  new LinkArrivalData();
            arrival->type = FLIT_ID;
            arrival->vc = 0;
            out_buffer.change_pull_channel(0);
            f = out_buffer.pull();
            downstream_credits[arrival->vc]--; 

            if ( f->type == HEAD )
                static_cast<HeadFlit*>(f)->avg_network_latency = Simulator::Now() - static_cast<HeadFlit*>(f)->avg_network_latency;

            if (f->type == TAIL || ( f->is_single_flit_pkt) )
            {
                packets_out++;
                in_packet_complete = false;

                IrisEvent* event = new IrisEvent();
                event->type = READY_EVENT;
                event->vc = 0;
                Simulator::Schedule(floor(Simulator::Now())+1, &NetworkComponent::process_event, processor_connection, event);
            }
            flits_out++;

            arrival->ptr = f;

            event->event_data.push_back(arrival);
            event->type = LINK_ARRIVAL_EVENT;
            event->src_id = address;
            event->vc = arrival->vc;

            ticking = true;

            if(do_two_stage_router)
                Simulator::Schedule(Simulator::Now()+0.75, 
                                    &NetworkComponent::process_event, 
                                    static_cast<GenericLink*>(output_connection)->output_connection, event);
            else
                Simulator::Schedule(Simulator::Now()+1.75, 
                                    &NetworkComponent::process_event, 
                                    static_cast<GenericLink*>(output_connection)->output_connection, event);
            static_cast<GenericLink*>(output_connection)->flits_passed++;
#ifdef _DEBUG_INTERFACE
            _DBG(" FLIT_OUT_EVENT credits_now: %d fty:%d", downstream_credits[0], arrival->ptr->type);
#endif
        }
    }

    //out packet to out buffer
    if ( out_packets[0].size() > 0 && out_packet_flit_index[0] < out_packets[0].length ) //&& out_buffer.get_occupancy(0)>0)
    {
        out_buffer.change_push_channel(0);
        Flit* ptr = out_packets[0].get_next_flit();
        out_buffer.push( ptr);
        out_packet_flit_index[0]++;

#ifdef _DEBUG_INTERFACE
        _DBG("Flit->OutBuffer ftype:%d outpkt_len:%d index:%d OB_size: %d ",ptr->type, out_packets[0].length, out_packet_flit_index[0], out_buffer.get_occupancy(0));
#endif

        if(out_packet_flit_index[0] == out_packets[0].length )
        {
            out_packet_flit_index[0] = 0;
            out_packets[0].flits.clear();
            in_packet_complete = true;

            /* 
               if( out_packets[0].size()<1)
               {

               IrisEvent* event = new IrisEvent();
               event->type = READY_EVENT;
               event->vc = 0;
               Simulator::Schedule(floor(Simulator::Now())+1, &NetworkComponent::process_event, processor_connection, event);
               }
             * */

        }
        ticking = true;
    }


    /*---------- This is on the input side. From processor out to ntwk --------- */
    // in packets to processor

    if ( in_ready[0]  && in_packets_flit_index[0]!=0 && in_packets_flit_index[0] == in_packets[0].length)
    {
        in_ready[0] = false;    
        HighLevelPacket* pkt = new HighLevelPacket();
        pkt->from_low_level_packet(&in_packets[0]);
        pkt->avg_network_latency = in_packets[0].avg_network_latency;
        pkt->hop_count = in_packets[0].hop_count;
        pkt->recv_time = (ullint)Simulator::Now();
        pkt->req_start_time = in_packets[0].req_start_time;
        pkt->waiting_in_ni = in_packets[0].waiting_in_ni;
#ifdef _DEBUG_INTERFACE
        _DBG( "Interface got a complete llp: %s ", in_packets[0].toString().c_str());
        _DBG("converted it %s",pkt->toString().c_str());
#endif

        /* 
           LowLevelPacket* llp = &in_packets[0];
           for( uint k=0; k<llp->flits.size(); k++)
           {
           for ( uint j=0;j<llp->flits[k]->phits.size() ;j++ )
           {
           llp->flits[k]->phits[j]->data.clear();
           delete (llp->flits[k]->phits[j]);
           }
           llp->flits[k]->phits.erase(llp->flits[k]->phits.begin(),llp->flits[k]->phits.end());
           llp->flits[k]->phits.clear();
           delete (llp->flits[k]);
           }

           llp->flits.erase(llp->flits.begin(),llp->flits.end());
           llp->flits.clear();
         * */

        in_packets_flit_index[0] = 0;
        IrisEvent* event = new IrisEvent();
        event->type = NEW_PACKET_EVENT;
        event->event_data.push_back(pkt);
        event->src_id = address;
        event->vc = 0;
        Simulator::Schedule(floor(Simulator::Now())+ 1, 
                            &NetworkComponent::process_event, processor_connection, event);

        LinkArrivalData* arrival = new LinkArrivalData();
        arrival->vc = 0;
        arrival->type = CREDIT_ID;
        IrisEvent* event2 = new IrisEvent();
        event2->type = LINK_ARRIVAL_EVENT;
        event2->vc = 0;
        event2->event_data.push_back(arrival);
        event2->src_id = address;
        if(do_two_stage_router)
            Simulator::Schedule( floor(Simulator::Now())+0.75, 
                                 &NetworkComponent::process_event, static_cast<GenericLink*>(input_connection)->input_connection, event2);
        else
            Simulator::Schedule( floor(Simulator::Now())+1.75, 
                                 &NetworkComponent::process_event, static_cast<GenericLink*>(input_connection)->input_connection, event2);
        static_cast<GenericLink*>(input_connection)->credits_passed++;
        ticking = true;
#ifdef _DEBUG_INTERFACE
        _DBG("Packet to Processor. Sent credit back: %s", pkt->toString().c_str());
#endif

    }
    else if ( !in_ready[0] && (in_packets_flit_index[0]!=0 && in_packets_flit_index[0] == in_packets[0].length))
    {
        ticking = true;
    }

    // arbitrate for the winner and push packets to in_buffer
    if( in_buffer.get_occupancy(0) > 0 && in_packets_flit_index[0] < in_packets[0].length )
    {
        in_buffer.change_pull_channel(0);
        Flit* ptr = in_buffer.pull();
        in_packets[0].add(ptr);

        in_packets_flit_index[0]++;
        ticking = true;
#ifdef _DEBUG_INTERFACE
        _DBG("Inpush flit ftype:%d", ptr->type);
#endif
        if( ptr->type == HEAD && static_cast<HeadFlit*>(ptr)->dst_address != node_ip)
        {
            _DBG("ERROR IncorrectDestinationException pkt_dest: %d node_ip: %d", in_packets[0].destination, node_ip); 
        }

    }

    if(ticking)
    {
        ticking = true;
        IrisEvent* new_event = new IrisEvent();
        new_event->type = TICK_EVENT;
        new_event->vc = e->vc;
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfacePhy::process_event, this, new_event);
    }

    delete e;
}

string
GenericInterfacePhy::print_stats()
{
    stringstream str;
    str << "\n interface[" << node_ip <<"] flits_in: " << flits_in
        << "\n interface[" << node_ip <<"] packets_in: " << packets_in
        << "\n interface[" << node_ip <<"] flits_out: " << flits_out
        << "\n interface[" << node_ip <<"] packets_out: " << packets_out
        << "\n interface[" << node_ip <<"] total_packet_latency(In packets): " << total_packets_in_time;
    if(packets_in != 0)
        str << "\n interface[" << node_ip <<"] avg_packet_latency(In packets): " << (total_packets_in_time+0.0)/packets_in;
    return str.str();

}

ullint
GenericInterfacePhy::get_flits_out()
{
    return flits_out;
}

ullint
GenericInterfacePhy::get_packets_in()
{
    return packets_in;
}

ullint
GenericInterfacePhy::get_packets_out()
{
    return packets_out;
}

ullint
GenericInterfacePhy::get_packets()
{
    return packets_out + packets_in;
}

bool
GenericInterfacePhy::is_pkt_in_progress(uint vc)
{
    return pkt_in_progress[vc];
}
ullint
GenericInterfacePhy::get_packets_drop()
{
    return stat_pkts_drop;
}
#endif   /* ----- #ifndef _genericInterfacePhy_cc_INC  ----- */

