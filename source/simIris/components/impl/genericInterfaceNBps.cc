/*!
 * =====================================================================================
 *
 *       Filename:  genericInterfaceNBVn.cc
 *
 *    Description:  Implements the class described in genericInterfaceNB.h
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

#ifndef  _genericInterfaceNBVn_cc_INC
#define  _genericInterfaceNBVn_cc_INC

#include	"genericInterfaceNBVn.h"

//#define _DEBUG
//#define _DEBUG_PKT


GenericInterfaceNBVn::GenericInterfaceNBVn()
{
    name = "GenericInterfaceNBVn";
    ticking =false;
}		/* -----  end of function GenericInterfaceNBVn::GenericInterfaceNBVn  ----- */

GenericInterfaceNBVn::~GenericInterfaceNBVn()
{
    in_packets.clear();
    out_packets.clear();
}		/* -----  end of function GenericInterfaceNBVn::~GenericInterfaceNBVn  ----- */

void
GenericInterfaceNBVn::setup (uint v, uint cr, uint bs)
{
    vcs =v;
    credits = cr;
    buffer_size = bs;
    /* All this is part of the init */
    address = myId();
    flast_vc = 0;
//    convert_packet_cycles = DEFAULT_CONVERT_PACKET_CYCLES;

    downstream_credits.resize(vcs);

    out_packet_flit_index.resize(vcs);
    in_ready.resize(vcs);
    in_packets_flit_index.resize(vcs);
    in_packet_complete.resize(vcs);

    in_packets.resize(vcs);
    in_packets_valid.resize(vcs);
    out_packets.resize(vcs);
    out_arbiter.set_no_requestors(vcs);


    for ( uint i=0; i<vcs ; i++ )
    {
        out_packet_flit_index[i] = 0;
        in_ready[i] = true;
        in_packet_complete[i] = false;
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
    stat_pkts_drop = 0;

    ticking = false;
    /* Init complete */


    return;
}		/* -----  end of function GenericInterfaceNBVn::setup  ----- */


string
GenericInterfaceNBVn::toString () const
{
    stringstream str;
    str << "GenericInterfaceNBVn: "
        << "\t VC: " << out_packets.size()
        << "\t address: " << address << " node_ip: " << node_ip
        << "\t OutputBuffers: " << out_buffer.toString()
        << "\t InputBuffer: " << in_buffer.toString()
        ;
    return str.str();
}		/* -----  end of function GenericInterfaceNBVn::toString  ----- */

void
GenericInterfaceNBVn::set_no_credits( int cr )
{
}		/* -----  end of function GenericInterfaceNBVn::toString  ----- */

uint
GenericInterfaceNBVn::get_no_credits() const
{
    return credits;
}		/* -----  end of function GenericInterfaceNBVn::toString  ----- */

void
GenericInterfaceNBVn::set_no_vcs( uint cr )
{
}		/* -----  end of function GenericInterfaceNBVn::toString  ----- */

void
GenericInterfaceNBVn::set_buffer_size( uint cr )
{
}		/* -----  end of function GenericInterfaceNBVn::toString  ----- */
void
GenericInterfaceNBVn::set_buffer_size( uint vcs, uint bs )
{
    in_buffer.resize(vcs, bs);
    out_buffer.resize(vcs, bs);
}		/* -----  end of function GenericInterfaceNBVn::toString  ----- */
void
GenericInterfaceNBVn::process_event(IrisEvent* e)
{
    switch(e->type)
    {
        /*! READY_EVENT: A ready is the credit at a packet level with the
         * injection/ejection point */
        case READY_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBVn::READY_EVENT ; time:%f", Simulator::Now());
#endif
        	handle_ready_event(e);
            break;

            /*! LINK_ARRIVAL_EVENT: This is a flit/credit arrival from the
             * network. (Incoming path)*/
        case LINK_ARRIVAL_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBVn::LINK_ARRIVAL_EVENT ; time:%f", Simulator::Now());
#endif
            handle_link_arrival(e);
            break;

            /*! NEW_PACKET_EVENT: This is a high level packet from the
             * processor connected to this node */
        case NEW_PACKET_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBVn::NEW_PACKET_EVENT ; time:%f", Simulator::Now());
#endif
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
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBVn::TICK_EVENT ; time:%f", Simulator::Now());
#endif
            handle_tick_event(e);
            break;

        case RESET_STAT_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBVn::RESET_STAT_EVENT time:%f", Simulator::Now());
#endif
        	handle_reset_stat_event(e);
            break;

        default:
            cout << "**Unkown event exception! " << e->type << endl;
            break;
    }
}		/* -----  end of function GenericInterfaceNBVn::process_event ----- */

void
GenericInterfaceNBVn::handle_ready_event( IrisEvent* e)
{
    /*! These are credits from the processor and hence do not update flow
     *  control state. Flow control state is updated at the link arrival event
     *  for a credit. */
#ifdef _DEBUG_INTERFACE
if(is_mc_interface)
{
    _DBG("; IntM got ready from NI %d ", e->vc);
}
else
{
    _DBG("; Int got ready %d ", e->vc);
}
#endif

if( in_ready[e->vc] )
{
    cout << " Error Recv incorrect READY !" << endl;
    exit(1);
}
    in_ready[e->vc] = true;
    if(!ticking)
    { 
        ticking = true;
        IrisEvent* event = new IrisEvent();
        event->type = TICK_EVENT;
        event->vc = e->vc;
        Simulator::Schedule( floor(Simulator::Now())+1, &NetworkComponent::process_event, this, event);
    }

    delete e;
    return;
}		/* -----  end of function GenericInterfaceNBVn::handle_ready_event  ----- */

void
GenericInterfaceNBVn::handle_link_arrival ( IrisEvent* e)
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
        in_buffer.change_push_channel(uptr->vc);
        in_buffer.push(uptr->ptr);
        if( uptr->ptr->type == TAIL )
        {
            packets_in++;
            total_packets_in_time += (Simulator::Now() - static_cast<TailFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<TailFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now());
#ifdef _DEBUG_PKT
         TailFlit* tf = static_cast<TailFlit*>(uptr->ptr);
        	 _DBG("; Flit arrival; ch=%d; addr=%d; Tail",uptr->vc, tf->addr);
#endif
        }

#ifdef _DEBUG
        if( uptr->ptr->type == HEAD)
        {
            HeadFlit* hf = static_cast<HeadFlit*>(uptr->ptr);
#ifdef _DEBUG_PKT
        	 _DBG("; Flit arrival; ch=%d; addr=%d; Head",uptr->vc, hf->addr);
#endif

/*            if( is_mc_interface)
            {
                _DBG("; ROUTER->MINT ; vc%d %llx",uptr->vc,hf->addr);
            }
            else
            {
                _DBG("; ROUTER->INT ; vc%d %llx",uptr->vc,hf->addr);
            }
*/
        }
#endif

        if( uptr->ptr->type == HEAD && static_cast<HeadFlit*>(uptr->ptr)->msg_class == ONE_FLIT_REQ )
        {
            packets_in++;
            total_packets_in_time += (Simulator::Now() - static_cast<HeadFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now())
                -static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency;
        }
        uptr->valid = false; 

        if( !(uptr->ptr->type == TAIL ||
              ( uptr->ptr->type == HEAD && static_cast<HeadFlit*>(uptr->ptr)->msg_class == ONE_FLIT_REQ )))
        {
            LinkArrivalData* arrival = new LinkArrivalData();
            arrival->vc = uptr->ptr->vc;
            arrival->type = CREDIT_ID;
            IrisEvent* event = new IrisEvent();
            event->type = LINK_ARRIVAL_EVENT;
            event->vc = uptr->ptr->vc;
            event->event_data.push_back(arrival);
            event->src_id = address;
            Simulator::Schedule( floor(Simulator::Now())+0.75, 
                                 &NetworkComponent::process_event, 
                                 static_cast<GenericLink*>(input_connection)->input_connection, event);
            static_cast<GenericLink*>(input_connection)->credits_passed++;
            istat->stat_link[static_cast<GenericLink*>(input_connection)->link_id]->credits_transferred++;
#ifdef _DEBUG
        _DBG("; create a credit downstream_credits sending back ; [uptr->vc=%d]",uptr->vc);
#endif
        }
    }
    else if ( uptr->type == CREDIT_ID)
    {
    	downstream_credits[uptr->vc]++;
    	assert(downstream_credits[uptr->vc]<=credits);
#ifdef _DEBUG
        _DBG("; got a credit downstream_credits ; [uptr->vc=%d]=%d",uptr->vc,downstream_credits[uptr->vc]);
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
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfaceNBVn::process_event, this, new_event);
    }
    delete uptr;
    delete e;
    return;
}		/* -----  end of function GenericInterfaceNBVn::handle_link_arrival  ----- */

void
GenericInterfaceNBVn::handle_new_packet_event(IrisEvent* e)
{
    HighLevelPacket* pkt = static_cast<HighLevelPacket*>(e->event_data.at(0));
    /* 
       if( pkt->msg_class == RESPONSE_PKT)
       pkt->virtual_channel = 1;
       else
       pkt->virtual_channel = 0;
     * */

    /*  corresponding low level packet to output virtual channel */
    pkt->to_low_level_packet(&out_packets[pkt->virtual_channel]);
    _DBG("; New Packet -> ch:%d add:%lld dst:%d vn:%d", pkt->virtual_channel,pkt->addr, pkt->destination,pkt->vn);

#ifdef _DEBUG_INTERFACE
    _DBG("; IntM NI->MInt ch%d %llx", pkt->virtual_channel,pkt->addr);
    cout << out_packets[pkt->virtual_channel].toString() ;
    cout << "HLP: " << pkt->toString() ;
    if(is_mc_interface)
    {
        _DBG("; IntM NI->MInt ch%d %llx", pkt->virtual_channel,pkt->addr);
    }
    else
    {
        _DBG("; Int Proc->Int ch%d %llx", pkt->virtual_channel,pkt->addr);
    }
#endif
#ifdef _DEBUG
    _DBG("; New Packet ch %d ; addr %lld ; dst %d", pkt->virtual_channel,pkt->addr, pkt->destination);
#endif
    out_packet_flit_index[ pkt->virtual_channel ] = 0; // initial the index
    delete pkt; 

    if( !ticking)
    {
        ticking = true;
        IrisEvent* event = new IrisEvent();
        event->type = TICK_EVENT;
        event->vc = e->vc;
        Simulator::Schedule(Simulator::Now()+1, &NetworkComponent::process_event, this, event);
    }

    delete e;
    return;
}		/* -----  end of function GenericInterfaceNBVn::handle_new_packet_event  ----- */

void
GenericInterfaceNBVn::handle_reset_stat_event( IrisEvent* e)
{
    /*  init stats */
    packets_out = 0;
    flits_out = 0;
    packets_in = 0;
    flits_in = 0;
    total_packets_in_time = 0;
    stat_pkts_drop = 0;
    delete e;
}

void
GenericInterfaceNBVn::handle_tick_event(IrisEvent* e)
{
    ticking = false;

    //out packet to out buffer
    for ( uint i=0; i<vcs ; i++)
    {

    }
    for ( uint i=0; i<vcs ; i++)
        if ( out_packets[i].size() > 0 && out_packet_flit_index[i] < out_packets[i].length )//if there is in_pkt, and not send out completely
        {
            out_buffer.change_push_channel(i); //out_packets[i].virtual_channel);
            Flit* ptr = out_packets[i].get_next_flit();
            ptr->vc = i; //out_packets[i].virtual_channel;
            out_buffer.push( ptr);
            out_packet_flit_index[i]++;

            if(out_packet_flit_index[i] == out_packets[i].length )
            {
                out_packet_flit_index[i] = 0;
                out_packets[i].flits.clear();
                in_packet_complete[i] = true;
            }
            ticking = true;
        }

    /* Request if there are enough credits , remove arbiter since vc is not changed */
    for ( uint i=0; i<vcs ; i++)
    {
        /* A you need to interpret the pattern at which the interface output
         * flits for a given input rate uncomment the next two lines and track
         * the occupancy with the pkt complete flag
         _DBG(";request: %d occ:%d cr:%d ",i,out_buffer.get_occupancy(i),downstream_credits[i]);
         cout <<" comp:"<<in_packet_complete[i];
         * */
#ifdef _DEBUG
        _DBG("; TICK ;;request_ch: %d occ:%d credit[vc%d]:%d ",i,out_buffer.get_occupancy(i),i,downstream_credits[i]);
        cout <<" comp:"<<in_packet_complete[i];
#endif

        if( downstream_credits[i]>0 && out_buffer.get_occupancy(i)>0
            && in_packet_complete[i] )
        {
            out_arbiter.request(i);
            ticking = true;
        }
    }
        if ( !out_arbiter.is_empty())
        {
            uint winner = out_arbiter.pick_winner();
            if ( winner > vcs )
            {
                cout << " ERROR: Interface Arbiter picked incorrect vc to send " << endl;
                exit(1);
            }
        IrisEvent* event = new IrisEvent();
        LinkArrivalData* arrival =  new LinkArrivalData();
        arrival->type = FLIT_ID;
        arrival->vc = winner; //out_packets[winner].virtual_channel;
        event->vc = arrival->vc;
        out_buffer.change_pull_channel(arrival->vc);
        arrival->ptr = out_buffer.pull();
        arrival->ptr->vc = arrival->vc;
        downstream_credits[arrival->vc]--; 
        assert(downstream_credits[arrival->vc]>=0);
        flits_out++;
        out_arbiter.clear_winner();

        event->event_data.push_back(arrival);
        event->type = LINK_ARRIVAL_EVENT;
        event->src_id = address;
        event->vc = arrival->vc;

        Flit* f = arrival->ptr;

        Simulator::Schedule(Simulator::Now()+0.75, 
                            &NetworkComponent::process_event, 
                            static_cast<GenericLink*>(output_connection)->output_connection, event);
        static_cast<GenericLink*>(output_connection)->flits_passed++;
        istat->stat_link[static_cast<GenericLink*>(output_connection)->link_id]->flits_transferred++;
        ticking = true;

        if ( f->type == HEAD )
        {
#ifdef _DEBUG_PKT
                     _DBG(";Head Flit out ; ft:%d vc:%d ; address:%d ; addr:%d", f->type,arrival->vc, event->src_id, static_cast<HeadFlit*>(f)->addr);
#endif
            HeadFlit* hf = static_cast<HeadFlit*>(f);

#ifdef _DEBUG_MC
            if(is_mc_interface)
            {
                _DBG(";INTM->ROUTER ; vc:%d %llx",arrival->vc,hf->addr);
            }
            else
            {
                _DBG(";INT->ROUTER ; vc:%d %llx",arrival->vc, hf->addr);
            }
#endif

            static_cast<HeadFlit*>(f)->avg_network_latency = Simulator::Now() 
                - static_cast<HeadFlit*>(f)->avg_network_latency;
        }

        if (f->type == TAIL || ( f->type == HEAD && static_cast<HeadFlit*>(f)->msg_class == ONE_FLIT_REQ) )
        {
#ifdef _DEBUG_PKT
                     _DBG(";Tail/One_Flit_Req Flit out ; ft:%d vc:%d ; src_id:%d ; addr:%d", f->type,arrival->vc, event->src_id, static_cast<TailFlit*>(f)->addr);
#endif

        	packets_out++;
            in_packet_complete[winner] = false;

            IrisEvent* event = new IrisEvent();
            event->type = READY_EVENT;
            event->vc = winner;
            Simulator::Schedule(floor(Simulator::Now())+1, &NetworkComponent::process_event, processor_connection, event);
        }
        out_arbiter.clear_winner();

        }


    /*---------- This is on the input side. From network in to the processor node --------- */
    // in packets to processor
    bool found = false;
    for ( uint i=flast_vc+1; i<vcs ; i++)  
        if ( in_ready[i] )  
        {
            flast_vc = i;
            found = true;
            break;
        }

    if(!found)
    {    for ( uint i=0; i<=flast_vc ; i++)
        if ( in_ready[i] )  
        {
            flast_vc = i;
            found = true;
            break;
        }
    }

    bool pkt_fnd = false;
    uint comp_pkt_index = -1;
    for ( uint i=0; i<vcs ; i++)
        if ( in_packets_flit_index[i]!=0 && in_packets_flit_index[i] == in_packets[i].length)
        {
            pkt_fnd = true;
            comp_pkt_index = i;
            //            break;
        }


    if ( found && pkt_fnd )
    {
        if(!static_cast<Processor*>(processor_connection)->ni_recv)
        {
            static_cast<Processor*>(processor_connection)->ni_recv = true;
            in_ready[flast_vc] = false;	
            HighLevelPacket* pkt = new HighLevelPacket();
            pkt->from_low_level_packet(&in_packets[comp_pkt_index]);
            uint orig_vc = pkt->virtual_channel;
            pkt->virtual_channel = flast_vc;
            in_packets[comp_pkt_index].virtual_channel = flast_vc;
            pkt->recv_time = Simulator::Now();

            /* This should be done inside from_low_level_packet. FIXME */
            pkt->avg_network_latency = in_packets[comp_pkt_index].avg_network_latency;
            pkt->hop_count = in_packets[comp_pkt_index].hop_count;
            pkt->req_start_time = in_packets[comp_pkt_index].req_start_time;
            pkt->waiting_in_ni = in_packets[comp_pkt_index].waiting_in_ni;

#ifdef _DEBUG
            if(is_mc_interface)
            {
                _DBG("; PKT->NI ; %lld vc%d",pkt->addr, pkt->virtual_channel); 
            }
            else
            {
                _DBG("; PKT->PROC ; vc%d",pkt->virtual_channel);
            }
#endif

            in_packets_flit_index[comp_pkt_index] = 0;
            IrisEvent* event = new IrisEvent();
            event->type = NEW_PACKET_EVENT;
            event->event_data.push_back(pkt);
            event->src_id = address;
            event->vc = pkt->virtual_channel;
            Simulator::Schedule(floor(Simulator::Now())+ 1, 
                                &NetworkComponent::process_event, processor_connection, event);

            LinkArrivalData* arrival = new LinkArrivalData();
            arrival->vc = orig_vc; //pkt->virtual_channel;
            arrival->type = CREDIT_ID;
            IrisEvent* event2 = new IrisEvent();
            event2->type = LINK_ARRIVAL_EVENT;
            event2->vc = orig_vc; //pkt->virtual_channel;
            event2->event_data.push_back(arrival);
            event2->src_id = address;
            Simulator::Schedule( floor(Simulator::Now())+0.75, 
                                 &NetworkComponent::process_event, 
                                 static_cast<GenericLink*>(input_connection)->input_connection, event2);
            static_cast<GenericLink*>(input_connection)->credits_passed++;
            istat->stat_link[static_cast<GenericLink*>(input_connection)->link_id]->credits_transferred++;
            ticking = true;

        }
        else
            ticking =true;
    }

    // arbitrate for the winner and push packets to in_buffer
    for ( uint i=0; i<vcs ; i++ )
        if( in_buffer.get_occupancy(i) > 0 && in_packets_flit_index[i] < in_packets[i].length )
        {
            in_buffer.change_pull_channel(i);
            Flit* ptr = in_buffer.pull();
            in_packets[i].add(ptr);

            in_packets_flit_index[i]++;
            ticking = true;

            if( ptr->type == HEAD && static_cast<HeadFlit*>(ptr)->dst_address != node_ip)
            {
                HeadFlit* hf = static_cast<HeadFlit*>(ptr);
                _DBG(";ERROR IncorrectDestinationException pkt_dest: %d node_ip: %d addr:%lld", in_packets[i].destination, node_ip, hf->addr);
            }

        }

    if(ticking)
    {
        ticking = true;
        IrisEvent* new_event = new IrisEvent();
        new_event->type = TICK_EVENT;
        new_event->vc = e->vc;
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfaceNBVn::process_event, this, new_event);
    }

    delete e;
}


string
GenericInterfaceNBVn::print_stats()
{
    stringstream str;
    str << "\n interface[" << node_ip <<"] flits_in: " << flits_in
        << "\n interface[" << node_ip <<"] packets_in: " << packets_in
        << "\n interface[" << node_ip <<"] flits_out: " << flits_out
        << "\n interface[" << node_ip <<"] packets_out: " << packets_out
        << "\n interface[" << node_ip <<"] total_packet_latency(In packets): " << total_packets_in_time;
    if(packets_in != 0)
        str << "\n interface[" << node_ip <<"] avg_packet_latency(In packets): " << (total_packets_in_time+0.0)/packets_in
                                                                                     ;
    return str.str();

}

ullint
GenericInterfaceNBVn::get_flits_out()
{
    return flits_out;
}

ullint
GenericInterfaceNBVn::get_flits_in()
{
    return flits_in;
}

ullint
GenericInterfaceNBVn::get_packets_out()
{
    return packets_out;
}
ullint
GenericInterfaceNBVn::get_packets_in()
{
    return packets_in;
}

ullint
GenericInterfaceNBVn::get_packets()
{
    return packets_out + packets_in;
}
ullint
GenericInterfaceNBVn::get_packets_drop()
{
    return stat_pkts_drop;
}
#endif   /* ----- #ifndef _GenericInterfaceNBVn_cc_INC  ----- */

