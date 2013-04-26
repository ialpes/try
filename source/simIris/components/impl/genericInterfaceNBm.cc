/*!
 * =====================================================================================
 *
 *       Filename:  genericInterfaceNBm.cc
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

#ifndef  _genericInterfaceNBm_cc_INC
#define  _genericInterfaceNBm_cc_INC

#include	"genericInterfaceNBm.h"

//#define _DEBUG
//#define _DEBUG_PKT

GenericInterfaceNBm::GenericInterfaceNBm()
{
    name = "GenericInterfaceNBm";
    ticking = false;
}		/* -----  end of function GenericInterfaceNBm::GenericInterfaceNBm  ----- */

GenericInterfaceNBm::~GenericInterfaceNBm()
{
    in_packets.clear();
    out_packets.clear();
}		/* -----  end of function GenericInterfaceNBm::~GenericInterfaceNBm  ----- */

void
GenericInterfaceNBm::setup (uint v, uint cr, uint bs)
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

    in_packets_ready.resize(vcs);
    is_requested.resize(vcs);
    in_channel.resize(vcs);
    in_requests.clear();
    is_equal.resize(vcs);

    for ( uint i=0; i<vcs ; i++ )
    {
        out_packet_flit_index[i] = 0;
        in_ready[i] = true;
        in_packet_complete[i] = false;
        in_packets[i].destination = address;
        in_packets[i].virtual_channel = i;
        in_packets[i].length= 10;
        downstream_credits[i] = credits;

        in_packets_ready[i] = true;
        is_requested[i] = false;
        in_channel[i] = -1;

        is_equal[i] = true;
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

	IrisEvent* event = new IrisEvent();
	    event->type = CHECK_EVENT;
	    event->time = 400;
	    Simulator::Schedule( 400,
	                         &NetworkComponent::process_event,
	                         this,
	                         event);


    return;
}		/* -----  end of function GenericInterfaceNBm::setup  ----- */

void
GenericInterfaceNBm::handle_check_event (IrisEvent* e)
{
	for(uint i=0; i<vcs; i++)
	{
		/*if vc is not free, check pkt_arrival_time, if larger then the
		 * period defined, drop the pkt and free the vc
		 */
		if(!in_packets_ready[i])
		{
			ullint waiting_time = Simulator::Now() - in_packets[i].in_packet_arrival;
			if(waiting_time > e->time)
			{
				_DBG("; pkt %lld is dropped at NI due to large waiting time %lld",in_packets[i].addr, waiting_time);
				in_packets[i].clear();
				in_packets_ready[i] = true;
				stat_pkts_drop +=1;
			}

		}
	}

	IrisEvent* event = new IrisEvent();
	    event->type = CHECK_EVENT;
	    event->time = e->time;
	    Simulator::Schedule( Simulator::Now()+e->time,
	                         &NetworkComponent::process_event,
	                         this,
	                         event);
	    delete e;
		return ;
}

string
GenericInterfaceNBm::toString () const
{
    stringstream str;
    str << "GenericInterfaceNBm: "
        << "\t VC: " << out_packets.size()
        << "\t address: " << address << " node_ip: " << node_ip
        << "\t OutputBuffers: " << out_buffer.toString()
        << "\t InputBuffer: " << in_buffer.toString()
        ;
    return str.str();
}		/* -----  end of function GenericInterfaceNBm::toString  ----- */

void
GenericInterfaceNBm::set_no_credits( int cr )
{
}		/* -----  end of function GenericInterfaceNBm::toString  ----- */

uint
GenericInterfaceNBm::get_no_credits() const
{
    return credits;
}		/* -----  end of function GenericInterfaceNBm::toString  ----- */

void
GenericInterfaceNBm::set_no_vcs( uint cr )
{
}		/* -----  end of function GenericInterfaceNBm::toString  ----- */

void
GenericInterfaceNBm::set_buffer_size( uint cr )
{
}		/* -----  end of function GenericInterfaceNBm::toString  ----- */
void
GenericInterfaceNBm::set_buffer_size( uint vcs, uint bs )
{
    in_buffer.resize(vcs, bs);
    out_buffer.resize(vcs, bs);
}		/* -----  end of function GenericInterfaceNBm::toString  ----- */
void
GenericInterfaceNBm::process_event(IrisEvent* e)
{
    switch(e->type)
    {
        /*! READY_EVENT: A ready is the credit at a packet level with the
         * injection/ejection point */
        case READY_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBm::READY_EVENT ; time:%f", Simulator::Now());
#endif
        	handle_ready_event(e);
            break;

            /*! LINK_ARRIVAL_EVENT: This is a flit/credit arrival from the
             * network. (Incoming path)*/
        case LINK_ARRIVAL_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBm::LINK_ARRIVAL_EVENT ; time:%f", Simulator::Now());
#endif
            handle_link_arrival(e);
            break;

            /*! NEW_PACKET_EVENT: This is a high level packet from the
             * processor connected to this node */
        case NEW_PACKET_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBm::NEW_PACKET_EVENT ; time:%f", Simulator::Now());
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
            _DBG("; GenericInterfaceNBm::TICK_EVENT ; time:%f", Simulator::Now());
#endif
            handle_tick_event(e);
            break;

        case RESET_STAT_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBm::RESET_STAT_EVENT time:%f", Simulator::Now());
#endif
        	handle_reset_stat_event(e);
            break;

        case CHECK_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBm::NI_CHECK_EVENT time:%f", Simulator::Now());
#endif
        	handle_check_event(e);
            break;

        default:
            cout << "**Unkown event exception! " << e->type << endl;
            break;
    }
}		/* -----  end of function GenericInterfaceNBm::process_event ----- */

void
GenericInterfaceNBm::handle_ready_event( IrisEvent* e)
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
}		/* -----  end of function GenericInterfaceNBm::handle_ready_event  ----- */

void
GenericInterfaceNBm::handle_link_arrival ( IrisEvent* e)
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

#ifdef _DEBUG
    	if(uptr->ptr->pseudo)
        {
        	if( uptr->ptr->type == HEAD )
        	{

        		_DBG("; New packet Pseudo Head Flit arrival; ch=%d; addr=%lld; ",uptr->vc, static_cast<HeadFlit*>(uptr->ptr)->addr);
        	}
        	if( uptr->ptr->type == TAIL )
        	{

        		_DBG("; Pseudo Tail Flit arrival; ch=%d; addr=%lld; ",uptr->vc, static_cast<TailFlit*>(uptr->ptr)->addr);

        	}
        }
#endif
        //else
        {

            flits_in++;
            in_buffer.change_push_channel(uptr->vc);
            in_buffer.push(uptr->ptr);

#ifdef _DEBUG
            if( uptr->ptr->type == HEAD )
            {
            	in_buffer.change_pull_channel(uptr->vc);
            	Flit* f=in_buffer.peek();
            	_DBG("; New packet %lld arrived ch=%d", static_cast<HeadFlit*>(f)->addr, uptr->vc);
            }
#endif

        if( uptr->ptr->type == TAIL && !uptr->ptr->pseudo)//TODO: statistic
        {
            packets_in++;
            total_packets_in_time += (Simulator::Now() - static_cast<TailFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<TailFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now())-static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency;;

#ifdef _DEBUG_PKT
         TailFlit* tf = static_cast<TailFlit*>(uptr->ptr);
        	 _DBG("; Flit arrival; ch=%d; addr=%d; Tail",uptr->vc, tf->addr);
#endif
        }

#ifdef _DEBUG_PKT
        if( uptr->ptr->type == HEAD)
        {
            HeadFlit* hf = static_cast<HeadFlit*>(uptr->ptr);

        	 _DBG("; Flit arrival; ch=%d; addr=%d; Head",uptr->vc, hf->addr);


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

        if( uptr->ptr->type == HEAD && static_cast<HeadFlit*>(uptr->ptr)->msg_class == ONE_FLIT_REQ && !uptr->ptr->pseudo)
        {
            packets_in++;
            total_packets_in_time += (Simulator::Now() - static_cast<HeadFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now())
                -static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency;
        }
        uptr->valid = false;
        } // end of if(uptr->ptr->pseudo)

/*
        // move send back credit after in_buffer.pull()
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
*/
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
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfaceNBm::process_event, this, new_event);
    }
    delete uptr;
    delete e;
    return;
}		/* -----  end of function GenericInterfaceNBm::handle_link_arrival  ----- */

void
GenericInterfaceNBm::handle_new_packet_event(IrisEvent* e)
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
}		/* -----  end of function GenericInterfaceNBm::handle_new_packet_event  ----- */

void
GenericInterfaceNBm::handle_reset_stat_event( IrisEvent* e)
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
GenericInterfaceNBm::handle_tick_event(IrisEvent* e)
{
    ticking = false;

    //out packet to out buffer
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

    /* Request out arbiter if there are enough credits */
    for ( uint i=0; i<vcs ; i++)
    {
        /* A you need to interpret the pattern at which the interface output
         * flits for a given input rate uncomment the next two lines and track
         * the occupancy with the pkt complete flag
         _DBG(";request: %d occ:%d cr:%d ",i,out_buffer.get_occupancy(i),downstream_credits[i]);
         cout <<" comp:"<<in_packet_complete[i];
         * */
#ifdef _DEBUG
//        _DBG("; TICK ;;request_ch: %d occ:%d credit[vc%d]:%d ",i,out_buffer.get_occupancy(i),i,downstream_credits[i]);
//        cout <<" comp:"<<in_packet_complete[i];
#endif

        if( downstream_credits[i]>0 && out_buffer.get_occupancy(i)>0
            && in_packet_complete[i] )
        {
            //            out_buffer.change_pull_channel(i);
            //            Flit* f= out_buffer.peek();
            //            if((f->type != HEAD)||( f->type == HEAD  && downstream_credits[i] == credits))
            {
                //                if ( !out_arbiter.is_requested(i) )
                out_arbiter.request(i);

                ticking = true;
            }

        }
    }

    /*---------- This is on the output side. From processor out to network --------- */
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
    	//if ( in_packets_flit_index[i]!=0 && in_packets_flit_index[i] == in_packets[i].length)
        if ( in_packets_flit_index[i]!=0 && in_packets_flit_index[i] == in_packets[i].length && is_equal[i])
        {
            pkt_fnd = true;
            comp_pkt_index = i;

            break;

            /*
            is_requested[i] = false;
			in_channel[i] = -1;
			*/
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
                _DBG("; PKT->NI ; add:%lld vc%d",pkt->addr, pkt->virtual_channel);
            }
            else
            {
                _DBG("; PKT->PROC ; add:%lld vc%d",pkt->addr,pkt->virtual_channel);
            }
#endif

            in_packets_flit_index[comp_pkt_index] = 0;
            in_packets_ready[comp_pkt_index] = true;
            IrisEvent* event = new IrisEvent();
            event->type = NEW_PACKET_EVENT;
            event->event_data.push_back(pkt);
            event->src_id = address;
            event->vc = pkt->virtual_channel;
            Simulator::Schedule(floor(Simulator::Now())+ 1,
                                &NetworkComponent::process_event, processor_connection, event);

/*            LinkArrivalData* arrival = new LinkArrivalData();
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
            */
            ticking = true;

        }
        else
            ticking =true;
    }

    // arbitrate for the winner and push packets to in_buffer
//#define _OLD
    #ifdef _OLD
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
#else
    /* scan in in_buffer if some pseudo hf match some pending in_packets */
    for ( uint i=0; i<vcs ; i++ )
    {
    	if (! in_packets_ready[i]) // there is empty in_packets
    	{
    		//if(in_packets[i].is_pending)
    		{
    			//assert(in_channel[i] == -1);  // wrong assert
    			//assert(is_requested[i] == false); // wrong assert
    			bool found_rest = false;
    			uint found_channel = -1;
    			// scan in_buffer if the rest pkt arrive with pseudo headflit
    			for ( uint j=0; j<vcs ; j++ )
    			{
    				if(in_buffer.get_occupancy(j)>0)
    				{
    					if(!is_requested[j])
    					{
    						in_buffer.change_pull_channel(j);
    						Flit* ptr = in_buffer.peek();
    						assert(ptr->type == HEAD);
    						//if(ptr->type == HEAD);
    						//if(ptr->pseudo)
    						{
    							if(static_cast<HeadFlit*>(ptr)->addr == in_packets[i].addr)
    							{
    								found_rest = true;
    								found_channel = j;
    								//HeadFlit* hf = static_cast<HeadFlit*>(in_buffer.pull()); // pull out pseudo head flit
#ifdef _DEBUG
    								_DBG("; pkt %lld from in_buffer[%d] matches in_packets[%d] %lld",static_cast<HeadFlit*>(ptr)->addr,found_channel,i,in_packets[i].addr)
#endif
    							}
    						}

    					}
    				}
    			}
    			if(found_rest)
    			{
    				is_requested[i] = true;
    				in_channel[i] = found_channel;
    				ticking = true;
    				break;
    			}

    		}
    	}
    }

    /* in_buffer to in_packets : loop for each ch of in_packets */
    for ( uint i=0; i<vcs ; i++ )  // process existing requests
    {
    	if (in_packets_ready[i]) // if there has empty in_packets
    	{
    		if (in_requests.size()>0)
    		{
    			uint winner_ib = in_requests.front();
    			in_requests.pop_front();
    			in_channel[winner_ib] = i;

    			assert (in_buffer.get_occupancy(winner_ib) > 0);

    			in_buffer.change_pull_channel(winner_ib);
    			Flit* ptr = in_buffer.pull();
    			assert(ptr->type == HEAD);

    			in_packets[i].add(ptr);
    			in_packets_ready[i] = false; // reset to true only when in_packets[i] sent to processor

                ticking = true;

                if( ptr->type == HEAD && static_cast<HeadFlit*>(ptr)->dst_address != node_ip)
                {
                    HeadFlit* hf = static_cast<HeadFlit*>(ptr);
                    _DBG(";ERROR IncorrectDestinationException pkt_dest: %d node_ip: %d addr:%lld", in_packets[i].destination, node_ip, hf->addr);
                }

                if(ptr->pseudo)
                {
                	if(is_equal[i])
                		is_equal[i]=false;
                	else
                		is_equal[i]=true;
#ifdef _DEBUG
                	_DBG("; pkt %lld by pseudo hf from in_buffer[%d] to in_packets[%d]",static_cast<HeadFlit*>(ptr)->addr,winner_ib,i)
#endif
                }
                else
                {
                	in_packets_flit_index[i]++;
#ifdef _DEBUG
                	_DBG("; pkt %lld from in_buffer[%d] to in_packets[%d]",static_cast<HeadFlit*>(ptr)->addr,winner_ib,i)
#endif
                }

                LinkArrivalData* arrival = new LinkArrivalData();
                arrival->vc = ptr->vc;
                arrival->type = CREDIT_ID;
                IrisEvent* event = new IrisEvent();
                event->type = LINK_ARRIVAL_EVENT;
                event->vc = ptr->vc;
                event->event_data.push_back(arrival);
                event->src_id = address;
                Simulator::Schedule( floor(Simulator::Now())+0.75,
                                     &NetworkComponent::process_event,
                                     static_cast<GenericLink*>(input_connection)->input_connection, event);
                static_cast<GenericLink*>(input_connection)->credits_passed++;
                istat->stat_link[static_cast<GenericLink*>(input_connection)->link_id]->credits_transferred++;
#ifdef _DEBUG
                _DBG("; create a credit downstream_credits sending back ; vc=%d",ptr->vc);
#endif

    		}
    	}
    }

    /* /* in_buffer to in_packets : loop for each ch of in_buffer*/
    for ( uint i=0; i<vcs ; i++ )
    {
    	if(in_buffer.get_occupancy(i)>0)
    	{
    		if(in_channel[i] == -1)
    		{
    			if(!is_requested[i])  // make request
    			{
    				/* pseudo hf does not request */
    				in_buffer.change_pull_channel(i);
    				Flit* ptr = in_buffer.peek();
    				assert(ptr->type == HEAD);
    				in_requests.push_back(i);
    				is_requested[i] = true;
    			}
    		}
    		else //got 1 channel of in_packets
    		{
    			uint ch=in_channel[i];
    			if(( in_packets_flit_index[ch] <= in_packets[ch].length )|| !is_equal[ch])
    			{
    				in_buffer.change_pull_channel(i);
    				Flit* ptr = in_buffer.pull();
    				//assert(ptr->type != HEAD);// could be 2eme hf of pkt
    				if ( ptr->type == BODY )
    				{
    					in_packets[ch].add(ptr);
    					in_packets_flit_index[ch]++;
    				}
    				else if( ptr->type == TAIL )   // in_buffer is free after then
    				{
    					if (ptr->pseudo)
    					{
    						//in_packets[ch].is_pending = true;
#ifdef _DEBUG
    						_DBG("; pkt %lld pseudo tail flit received", static_cast<TailFlit*>(ptr)->addr);
#endif
    						if(is_equal[i])
    							is_equal[i]=false;
    						else
    							is_equal[i]=true;

    						// drop pkt
    						ullint waiting_time = Simulator::Now() - in_packets[ch].in_packet_arrival;
    						_DBG("; pkt %lld is dropped at NI due to pseudo tail flit %lld",in_packets[ch].addr, waiting_time);
    						in_packets_flit_index[ch] = 0;
    						in_packets[ch].length= 0;
    						in_packets[ch].addr=-1;
    						in_packets_ready[ch] = true;
    						//is_tf_received[ch] = false;
    						in_packets[ch].clear();
    						is_equal[ch] = true;

    						stat_pkts_drop ++;
    						packets_in --;
    					}
    					else
    					{
    						in_packets_flit_index[ch]++;
    					}
    					in_packets[ch].add(ptr);
    					is_requested[i] = false;
    					in_channel[i] = -1;
    				}
    				else if( ptr->type == HEAD )
    				{
    					//_DBG("; NI error , pkt %lld", static_cast<HeadFlit*>(ptr)->addr);

    					in_packets[ch].add(ptr);
    					if(ptr->pseudo)
    	                {
    	                	if(is_equal[i])
    	                		is_equal[i]=false;
    	                	else
    	                		is_equal[i]=true;
    	                }
    					else
    						in_packets_flit_index[ch]++;
    					assert(static_cast<HeadFlit*>(ptr)->addr == in_packets[i].addr);


    				}

                    LinkArrivalData* arrival = new LinkArrivalData();
                    arrival->vc = ptr->vc;
                    arrival->type = CREDIT_ID;
                    IrisEvent* event = new IrisEvent();
                    event->type = LINK_ARRIVAL_EVENT;
                    event->vc = ptr->vc;
                    event->event_data.push_back(arrival);
                    event->src_id = address;
                    Simulator::Schedule( floor(Simulator::Now())+0.75,
                                         &NetworkComponent::process_event,
                                         static_cast<GenericLink*>(input_connection)->input_connection, event);
                    static_cast<GenericLink*>(input_connection)->credits_passed++;
                    istat->stat_link[static_cast<GenericLink*>(input_connection)->link_id]->credits_transferred++;
#ifdef _DEBUG
                    _DBG("; create a credit downstream_credits sending back ; vc=%d",ptr->vc);
#endif
    			}

    		}
    		ticking=true;
    	}

    }
#endif
    if(ticking)
    {
        ticking = true;
        IrisEvent* new_event = new IrisEvent();
        new_event->type = TICK_EVENT;
        new_event->vc = e->vc;
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfaceNBm::process_event, this, new_event);
    }

    delete e;
}


string
GenericInterfaceNBm::print_stats()
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
GenericInterfaceNBm::get_flits_out()
{
    return flits_out;
}

ullint
GenericInterfaceNBm::get_flits_in()
{
    return flits_in;
}

ullint
GenericInterfaceNBm::get_packets_out()
{
    return packets_out;
}
ullint
GenericInterfaceNBm::get_packets_in()
{
    return packets_in;
}
ullint
GenericInterfaceNBm::get_packets()
{
    return packets_out + packets_in;
}
ullint
GenericInterfaceNBm::get_packets_drop()
{
    return stat_pkts_drop;
}
#endif   /* ----- #ifndef _GenericInterfaceNBm_cc_INC  ----- */

