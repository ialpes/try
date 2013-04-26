/*!
 * =====================================================================================
 *
 *       Filename:  genericInterfaceNBb.cc
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

#ifndef  _genericInterfaceNBb_cc_INC
#define  _genericInterfaceNBb_cc_INC

#include	"genericInterfaceNBb.h"

//#define _DEBUG
//#define _DEBUG_PKT
//#define _DEBUG_PKT_SPLIT
//#define _DEBUG_CREDIT

GenericInterfaceNBb::GenericInterfaceNBb()
{
    name = "GenericInterfaceNBb";
    ticking =false;
}		/* -----  end of function GenericInterfaceNBb::GenericInterfaceNBb  ----- */

GenericInterfaceNBb::~GenericInterfaceNBb()
{
    in_packets.clear();
    out_packets.clear();
}		/* -----  end of function GenericInterfaceNBb::~GenericInterfaceNBb  ----- */

void
GenericInterfaceNBb::setup (uint v, uint cr, uint bs)
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
    is_requested_inbf.resize(vcs);
    inpkt_ch_for_inbf.resize(vcs);
    in_requests.clear();
    is_equal.resize(vcs);
    is_vs.resize(vcs);
    is_tf_received.resize(vcs);
    is_tf_sent.resize(vcs);

    for ( uint i=0; i<vcs ; i++ )
    {
        out_packet_flit_index[i] = 0;
        in_ready[i] = true;
        in_packet_complete[i] = false;
        in_packets[i].destination = address;
        in_packets[i].virtual_channel = i;
        in_packets[i].length= 0;
        in_packets[i].addr=-1;
        in_packets_flit_index[i]=0;
        downstream_credits[i] = credits;

        in_packets_ready[i] = true;
        is_requested_inbf[i] = false;
        inpkt_ch_for_inbf[i] = -1;

        is_equal[i] = true;
        is_vs[i] = false;
        is_tf_received[i] = false;
        is_tf_sent[i] = false;
    }

    /*  init stats */
    packets_out = 0;
    flits_out = 0;
    packets_in = 0;
    flits_in = 0;
    vs_packets_in = 0;
    total_packets_in_time = 0;
    stat_pkts_drop = 0;

    ticking = false;
    /* Init complete */

	IrisEvent* event = new IrisEvent();
	    event->type = CHECK_EVENT;
	    event->time = 500;
	    Simulator::Schedule( 500,
	                         &NetworkComponent::process_event,
	                         this,
	                         event);


    return;
}		/* -----  end of function GenericInterfaceNBb::setup  ----- */
void
GenericInterfaceNBb::handle_check_event (IrisEvent* e)
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
				uint ibch=-1;

				for (uint ch=0; ch<vcs;ch++)
				{
					if(inpkt_ch_for_inbf[ch]==i)
						ibch=ch;
				}
				if(ibch != -1)
				{
					if(in_buffer.get_occupancy(ibch)==0)
					{
						_DBG("; pkt %lld is dropped at NI due to large waiting time %lld",in_packets[i].addr, waiting_time);
						in_packets[i].clear();
						in_packets[i].addr=-1;
						in_packets_ready[i] = true;
						is_equal[i] = true;
						in_packets_flit_index[i] = 0;
						in_packets[i].length= 0;
						stat_pkts_drop +=1;
						inpkt_ch_for_inbf[ibch] = -1; // reset
						is_requested_inbf[ibch] = false; // reset
					}
				}

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
	    delete e; e=NULL;
		return ;
}

string
GenericInterfaceNBb::toString () const
{
    stringstream str;
    str << "GenericInterfaceNBb: "
        << "\t VC: " << out_packets.size()
        << "\t address: " << address << " node_ip: " << node_ip
        << "\t OutputBuffers: " << out_buffer.toString()
        << "\t InputBuffer: " << in_buffer.toString()
        ;
    return str.str();
}		/* -----  end of function GenericInterfaceNBb::toString  ----- */

void
GenericInterfaceNBb::set_no_credits( int cr )
{
}		/* -----  end of function GenericInterfaceNBb::toString  ----- */

uint
GenericInterfaceNBb::get_no_credits() const
{
    return credits;
}		/* -----  end of function GenericInterfaceNBb::toString  ----- */

void
GenericInterfaceNBb::set_no_vcs( uint cr )
{
}		/* -----  end of function GenericInterfaceNBb::toString  ----- */

void
GenericInterfaceNBb::set_buffer_size( uint cr )
{
}		/* -----  end of function GenericInterfaceNBb::toString  ----- */
void
GenericInterfaceNBb::set_buffer_size( uint vcs, uint bs )
{
    in_buffer.resize(vcs, bs);
    out_buffer.resize(vcs, bs);
}		/* -----  end of function GenericInterfaceNBb::toString  ----- */
void
GenericInterfaceNBb::process_event(IrisEvent* e)
{
    switch(e->type)
    {
        /*! READY_EVENT: A ready is the credit at a packet level with the
         * injection/ejection point */
        case READY_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBb::READY_EVENT ; time:%f", Simulator::Now());
#endif
        	handle_ready_event(e);
            break;

            /*! LINK_ARRIVAL_EVENT: This is a flit/credit arrival from the
             * network. (Incoming path)*/
        case LINK_ARRIVAL_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBb::LINK_ARRIVAL_EVENT ; time:%f", Simulator::Now());
#endif
            handle_link_arrival(e);
            break;

            /*! NEW_PACKET_EVENT: This is a high level packet from the
             * processor connected to this node */
        case NEW_PACKET_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBb::NEW_PACKET_EVENT ; time:%f", Simulator::Now());
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
            _DBG("; GenericInterfaceNBb::TICK_EVENT ; time:%f", Simulator::Now());
#endif
            handle_tick_event(e);
            break;

        case RESET_STAT_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBb::RESET_STAT_EVENT time:%f", Simulator::Now());
#endif
        	handle_reset_stat_event(e);
            break;

        case VS_TICK_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBb::VS_TICK_EVENT time:%f", Simulator::Now());
#endif
            handle_vs_tick_event(e);
            break;

        case CHECK_EVENT:
#ifdef _DEBUG
            _DBG("; GenericInterfaceNBb::CHECK_EVENT time:%f", Simulator::Now());
#endif
        	handle_check_event(e);
            break;


        default:
            cout << "**Unkown event exception! " << e->type << endl;
            break;
    }
}		/* -----  end of function GenericInterfaceNBb::process_event ----- */

void
GenericInterfaceNBb::handle_ready_event( IrisEvent* e)
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

    delete e;e=NULL;
    return;
}		/* -----  end of function GenericInterfaceNBb::handle_ready_event  ----- */

void
GenericInterfaceNBb::handle_link_arrival ( IrisEvent* e)
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
#ifdef _DEBUG_PKT_SPLIT
        if(uptr->ptr->pseudo)
        {
        	if( uptr->ptr->type == HEAD )
        	{
        		_DBG("; Pseudo Flit arrival; ch=%d; addr=%lld; 0x%x Head ",uptr->vc, static_cast<HeadFlit*>(uptr->ptr)->addr, uptr->ptr);
        	}
        	if( uptr->ptr->type == TAIL )
        	{
        		_DBG("; Pseudo Flit arrival; ch=%d; addr=%lld; 0x%x Tail ",uptr->vc, static_cast<TailFlit*>(uptr->ptr)->addr, uptr->ptr);
        	}
        }
#endif


    	flits_in++;
        in_buffer.change_push_channel(uptr->vc);
        in_buffer.push(uptr->ptr);

        if( uptr->ptr->type == HEAD )
        {
        	in_buffer.change_pull_channel(uptr->vc);
        	Flit* f=in_buffer.peek();
#ifdef _DEBUG_PKT_SPLIT
        	_DBG("; New packet %lld; 0x%x arrived ch=%d  ", static_cast<HeadFlit*>(f)->addr, uptr->ptr, uptr->vc);
#endif
        }

        if( uptr->ptr->type == TAIL && !uptr->ptr->pseudo)//TODO: statistic
        {
            //packets_in++;
            total_packets_in_time += (Simulator::Now() - static_cast<TailFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<TailFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now());
            		//-static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency;
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

        if( uptr->ptr->type == HEAD && static_cast<HeadFlit*>(uptr->ptr)->msg_class == ONE_FLIT_REQ && !uptr->ptr->pseudo)
        {
            //packets_in++;
            total_packets_in_time += (Simulator::Now() - static_cast<HeadFlit*>(uptr->ptr)->packet_originated_time);
            static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency = ceil(Simulator::Now())
                -static_cast<HeadFlit*>(uptr->ptr)->avg_network_latency;
        }
        if( uptr->ptr->type == HEAD && !uptr->ptr->pseudo)
        {
            if (static_cast<HeadFlit*>(uptr->ptr)->dst_address == node_ip)
            	packets_in++;
            //else
            	//vs_packet_in++;
        }
        uptr->valid = false; 

/*
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
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfaceNBb::process_event, this, new_event);
    }
    delete uptr; uptr=NULL;
    delete e; e=NULL;
    return;
}		/* -----  end of function GenericInterfaceNBb::handle_link_arrival  ----- */

void
GenericInterfaceNBb::handle_new_packet_event(IrisEvent* e)
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
#ifdef _DEBUG_PKT
       _DBG(";NI new packet event ; addr:%d; 0x%x head ",
    		   static_cast<HeadFlit*>(out_packets[pkt->virtual_channel].at(0))->addr,
    		   out_packets[pkt->virtual_channel].at(0));//get_next_flit
#endif

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
//    delete pkt;

    if( !ticking)
    {
        ticking = true;
        IrisEvent* event = new IrisEvent();
        event->type = TICK_EVENT;
        event->vc = e->vc;
        Simulator::Schedule(Simulator::Now()+1, &NetworkComponent::process_event, this, event);
    }

    delete e; e=NULL;
    return;
}		/* -----  end of function GenericInterfaceNBb::handle_new_packet_event  ----- */

void
GenericInterfaceNBb::handle_reset_stat_event( IrisEvent* e)
{
    /*  init stats */
    packets_out = 0;
    vs_packets_in = 0;
    flits_out = 0;
    packets_in = 0;
    flits_in = 0;
    total_packets_in_time = 0;
    stat_pkts_drop = 0;
    delete e; e=NULL;
}
void
GenericInterfaceNBb::handle_vs_tick_event(IrisEvent* e)
{
	// check if packet is all arrived
	// if yes, send vs_request_event to proc asking for vc
	// if no, continue vs_ticking
	// could be a handle_vs_tick_event sent back by proc with authorizing vc
	// send out packet through giving vc, arbiter for switch traversal
	// put into output buffer!!! then use handle_tick_event
}
void
GenericInterfaceNBb::handle_tick_event(IrisEvent* e)
{
    ticking = false;

    //out packet to out buffer
    for ( uint i=0; i<vcs ; i++)
        if ( out_packets[i].size() > 0 && out_packet_flit_index[i] < out_packets[i].length )//if there is in_pkt, and not send out completely
        {
            out_buffer.change_push_channel(i); //out_packets[i].virtual_channel);
            Flit* ptr = out_packets[i].get_next_flit();
            //assert(i == out_packets[i].virtual_channel);
            if(ptr->type == TAIL)
            	is_tf_sent[i] = true;
            ptr->vc = i; //out_packets[i].virtual_channel;
            out_buffer.push( ptr);
            out_packet_flit_index[i]++;

            if((out_packet_flit_index[i] == out_packets[i].length) || is_tf_sent[i])
            {
                out_packet_flit_index[i] = 0;
                out_packets[i].flits.clear();
                in_packet_complete[i] = true;
                is_tf_sent[i] = false;
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
//        _DBG("; TICK ;;request_ch: %d occ:%d credit[vc%d]:%d ",i,out_buffer.get_occupancy(i),i,downstream_credits[i]);
//        cout <<" comp:"<<in_packet_complete[i];
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
                     _DBG(";Head Flit out ; ft:%d vc:%d ; address:%d ; addr:%d; 0x%x",
                    		 f->type,arrival->vc, event->src_id, static_cast<HeadFlit*>(f)->addr, f);
#endif
            HeadFlit* hf = static_cast<HeadFlit*>(f);

            if (hf->src_address == node_ip)
            	packets_out++;
            //else
            	//vs_packets_out++;
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
                     _DBG(";Tail/One_Flit_Req Flit out ; ft:%d vc:%d ; src_id:%d ; addr:%d; 0x%x",
                    		 f->type,arrival->vc, event->src_id, static_cast<TailFlit*>(f)->addr, f);
#endif

        	//packets_out++;
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
    bool vs_pkt_fnd = false;
    uint comp_pkt_index = -1;
    uint vs_pkt_vc = -1;

    for ( uint i=0; i<vcs ; i++)
    {
        //if (( in_packets_flit_index[i]!=0 )&& (in_packets_flit_index[i] == in_packets[i].length))
        if ( (in_packets_flit_index[i]!=0) && (in_packets_flit_index[i] == in_packets[i].length) && (is_equal[i]) )
        {
//        	cerr <<in_packets[i].toString();
        	pkt_fnd = true;
        	comp_pkt_index = i;
#ifdef _DEBUG_PKT
        _DBG("; pkt_fnd comp_pkt_index=%d %lld",comp_pkt_index, in_packets[i].addr );
#endif
        	break;
            //cerr <<endl <<" in_bufer NI : "<< "channel "<<i<<":";
            //cerr<< in_buffer.get_occupancy(i) <<" " << in_packets_flit_index[i] <<" " << in_packets[i].length;
            //cerr <<endl;

            /*
            if (in_buffer.get_occupancy(i)>0) {
            in_buffer.change_pull_channel(i);
        	Flit* f=in_buffer.peek();
        	cout << "in_buffer.get_occupancy "<< in_buffer.get_occupancy(i)<< " ";
        	cout << f->toString();*/  // the buffer is empty

        }
        else if (is_vs[i] && is_tf_received[i]) // if virtual source used, dont wait for whole pkt
        {
        	pkt_fnd = true;
        	comp_pkt_index = i;
#ifdef _DEBUG_PKT
        _DBG("; vs pkt_fnd comp_pkt_index=%d %lld",comp_pkt_index, in_packets[i].addr );
#endif
        	break;
        }
    }
    // if vs_pkt found, send a vc request to proc asking for a vc in another VN
    // how to prevent send it more than one time ???
/*
        if (vs_pkt_fnd) {
        IrisEvent* event = new IrisEvent();
        event->type = VS_REQUEST_EVENT;
        event->dst_id = in_packets[vs_pkt_vc].destination;
        event->vc = vs_pkt_vc;
        event->event_data.push_back(&in_packets[vs_pkt_vc]);
        Simulator::Schedule(floor(Simulator::Now())+ 1,
                            &NetworkComponent::process_event, processor_connection, event);

        LinkArrivalData* arrival = new LinkArrivalData();
        arrival->vc = vs_pkt_vc;
        arrival->type = CREDIT_ID;
        IrisEvent* event2 = new IrisEvent();
        event2->type = LINK_ARRIVAL_EVENT;
        event2->vc = vs_pkt_vc;
        event2->event_data.push_back(arrival);
        event2->src_id = address;
        Simulator::Schedule( floor(Simulator::Now())+0.75,
                             &NetworkComponent::process_event,
                             static_cast<GenericLink*>(input_connection)->input_connection, event2);
        static_cast<GenericLink*>(input_connection)->credits_passed++;
        istat->stat_link[static_cast<GenericLink*>(input_connection)->link_id]->credits_transferred++;
        ticking = true;

    }
*/

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

            /* This should be done inside from_low_level_packet. FIXME TODO*/
            pkt->avg_network_latency = in_packets[comp_pkt_index].avg_network_latency;
            pkt->hop_count = in_packets[comp_pkt_index].hop_count;
            pkt->req_start_time = in_packets[comp_pkt_index].req_start_time;
            pkt->waiting_in_ni = in_packets[comp_pkt_index].waiting_in_ni;

#ifdef _DEBUG
            if(is_mc_interface)
            {
                _DBG("; PKT->NI ; in_packet[%d] add:%lld vc%d",comp_pkt_index, pkt->addr, pkt->virtual_channel);
            }
            else
            {
                _DBG("; PKT->PROC ; in_packet[%d] add:%lld vc%d",comp_pkt_index, pkt->addr,pkt->virtual_channel);
            }
#endif

            in_packets_flit_index[comp_pkt_index] = 0;
            in_packets[comp_pkt_index].length= 0;
            in_packets[comp_pkt_index].addr=-1;
            in_packets_ready[comp_pkt_index] = true;
            is_tf_received[comp_pkt_index] = false;
            if(is_vs[comp_pkt_index])
            	is_vs[comp_pkt_index] = false;
            //assert(is_equal[comp_pkt_index]==true); //vs pkt
            is_equal[comp_pkt_index]=true;


            IrisEvent* event = new IrisEvent();
            event->type = NEW_PACKET_EVENT;
            event->event_data.push_back(pkt);
            event->src_id = address;
            event->vc = pkt->virtual_channel;
            Simulator::Schedule(floor(Simulator::Now())+ 1, 
                               &NetworkComponent::process_event, processor_connection, event);

			uint ibch=-1;
			for (uint ch=0; ch<vcs;ch++)
			{
				if(inpkt_ch_for_inbf[ch]==comp_pkt_index)
					ibch=ch;
			}
			if(ibch != -1)
			{
				//if(is_requested_inbf[ibch]== false)
				{
					inpkt_ch_for_inbf[ibch] = -1; // reset
					is_requested_inbf[ibch] = false; // reset
				}
			}
/*
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
*/
            ticking = true;

        }
        //else
            //ticking =true;
    }  // if ( found && pkt_fnd )

    // arbitrate for the winner and push packets to in_buffer
//#define _OLD
#ifdef _OLD

    for ( uint i=0; i<vcs ; i++ ) {
        if( in_buffer.get_occupancy(i) > 0 && in_packets_flit_index[i] < in_packets[i].length )
        {
            in_buffer.change_pull_channel(i);
            Flit* ptr = in_buffer.pull();
            in_packets[i].add(ptr);

            LinkArrivalData* arrival = new LinkArrivalData();
            arrival->vc = i;//orig_vc; //pkt->virtual_channel;
            arrival->type = CREDIT_ID;
            IrisEvent* event2 = new IrisEvent();
            event2->type = LINK_ARRIVAL_EVENT;
            event2->vc = i;//orig_vc; //pkt->virtual_channel;
            event2->event_data.push_back(arrival);
            event2->src_id = address;
            Simulator::Schedule( floor(Simulator::Now())+0.75,
                                 &NetworkComponent::process_event,
                                 static_cast<GenericLink*>(input_connection)->input_connection, event2);
            static_cast<GenericLink*>(input_connection)->credits_passed++;
            istat->stat_link[static_cast<GenericLink*>(input_connection)->link_id]->credits_transferred++;
#ifdef _DEBUG
            _DBG(";Send back a credit vc: %d buffer_occupancy: %d", i, in_buffer.get_occupancy(i));
#endif

            in_packets_flit_index[i]++;
            ticking = true;

/*
            if( ptr->type == HEAD && static_cast<HeadFlit*>(ptr)->dst_address != node_ip)
            {
                HeadFlit* hf = static_cast<HeadFlit*>(ptr);
                if (hf->virtual_source != VSSET)
                _DBG(";ERROR IncorrectDestinationException pkt_dest: %d node_ip: %d addr:%lld", in_packets[i].destination, node_ip, hf->addr);
            }
*/
        }
    }
#else
    /* scan in in_buffer if some pseudo hf match some pending in_packets */
    /*for ( uint i=0; i<vcs ; i++ )
    {
    	if (! in_packets_ready[i]) // for those in_packets is not ready
    	{
    		//if(in_packets[i].is_pending)
    		{
    			//assert(inpkt_ch_for_inbf[i] == -1);  // wrong assert
    			//assert(is_requested_inbf[i] == false); // wrong assert
    			bool found_rest = false;
    			uint found_channel = -1;
    			// scan in_buffer if the rest pkt arrive with pseudo headflit
    			for ( uint j=0; j<vcs ; j++ )
    			{
    				if(in_buffer.get_occupancy(j)>0)
    				{
    					if(!is_requested_inbf[j])
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
#ifdef _DEBUG_PKT_SPLIT
    								_DBG("; pkt %lld from in_buffer[%d] matches in_packets[%d] %lld; 0x%x",
    										static_cast<HeadFlit*>(ptr)->addr,found_channel,i,in_packets[i].addr, ptr);
#endif
    								break;
    							}
    						}

    					}
    				}
    			}
    			if(found_rest)
    			{
    				//is_requested_inbf[i] = true;
    				//inpkt_ch_for_inbf[i] = found_channel;
    				is_requested_inbf[found_channel] = true;
    				inpkt_ch_for_inbf[found_channel] = i;
    				ticking = true;
    				break;
    			}

    		}
    	}
    }*/

    /* in_buffer to in_packets : loop for each ch of in_packets */
    for ( uint ipch=0;ipch<vcs ; ipch++ )  // process existing requests
    {
    	if (in_packets_ready[ipch]) // if there has empty in_packets
    	{
    		if (in_requests.size()>0)
    		{
    			uint winner_ib = in_requests.front();
    			in_requests.pop_front();
    			assert(inpkt_ch_for_inbf[winner_ib] == -1);
    			assert(in_packets_flit_index[ipch]==0);
    			inpkt_ch_for_inbf[winner_ib] = ipch;

    			assert (in_buffer.get_occupancy(winner_ib) > 0);

    			in_buffer.change_pull_channel(winner_ib);
    			Flit* ptr = in_buffer.pull();
    			assert(ptr->type == HEAD);

                if( ptr->type == HEAD && static_cast<HeadFlit*>(ptr)->dst_address != node_ip
                		&& static_cast<HeadFlit*>(ptr)->virtual_source!=VSSET)
                {
                    HeadFlit* hf = static_cast<HeadFlit*>(ptr);
                    _DBG(";ERROR IncorrectDestinationException pkt_dest: %d node_ip: %d addr:%lld",
                    		in_packets[ipch].destination, node_ip, hf->addr);
                }

                if(static_cast<HeadFlit*>(ptr)->virtual_source == VSSET)
                	is_vs[ipch] = true;
                if(ptr->pseudo)
                {
                	/*if(is_equal[ipch])
                		is_equal[ipch]=false;
                	else
                		is_equal[ipch]=true;
                	*/
                	is_equal[ipch] =!is_equal[ipch];
#ifdef _DEBUG_PKT_SPLIT
                	_DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d Pseudo Head 0x%x is_equal=%s",
                			static_cast<HeadFlit*>(ptr)->addr,ipch,  ipch, in_packets_flit_index[ipch],ptr,
                			(is_equal[ipch])?"true":"false");
#endif
                }
                else
                {
                	in_packets_flit_index[ipch]++;
#ifdef _DEBUG_PKT_SPLIT
//                	_DBG("; pkt %lld 0x%x from in_buffer[%d] to in_packets[%d] in_packets_flit_index[%d]=%d",static_cast<HeadFlit*>(ptr)->addr,ptr,winner_ib,i,i, in_packets_flit_index[i]);
                    _DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d Head 0x%x is_equal=%s",
                    		static_cast<HeadFlit*>(ptr)->addr,ipch,ipch, in_packets_flit_index[ipch], ptr,
                    		(is_equal[ipch])?"true":"false");
#endif
                }

                assert (ptr->vc == winner_ib);
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
#ifdef _DEBUG_CREDIT
                _DBG("; create a credit downstream_credits sending back ; vc=%d",ptr->vc);
#endif
    			in_packets[ipch].add(ptr);
    			in_packets_ready[ipch] = false; // reset to true only when in_packets[i] sent to processor

                ticking = true;

    		}
    	}
    }

    /* /* in_buffer to in_packets : loop for each ch of in_buffer*/
    for ( uint i=0; i<vcs ; i++ )
    {
    	if(in_buffer.get_occupancy(i)>0)
    	{
    		if(inpkt_ch_for_inbf[i] == -1)
    		{
    			if(!is_requested_inbf[i])  // make request
    			/*{
    				// pseudo hf does not request
    				in_buffer.change_pull_channel(i);
    				Flit* ptr = in_buffer.peek();
    				assert(ptr->type == HEAD);
    				in_requests.push_back(i);
    				is_requested_inbf[i] = true;
    			}*/
    			{

    				/* pseudo hf does not request */
    				in_buffer.change_pull_channel(i);
    				Flit* ptr = in_buffer.peek();
    				if(ptr->type ==BODY)
    				{
#ifdef _DEBUG_PKT_SPLIT
    					_DBG("; pkt %lld is dropped at NI already Flit 0x%x Body", static_cast<BodyFlit*>(ptr)->addr, ptr);
#endif
    					in_buffer.pull();
    					delete ptr;
    				}
    				else if(ptr->type ==TAIL)
    				{
#ifdef _DEBUG_PKT_SPLIT
    					_DBG("; pkt %lld is dropped at NI already Flit 0x%x Tail", static_cast<TailFlit*>(ptr)->addr, ptr);
#endif
    					in_buffer.pull();
    					delete ptr;
    				}
    				//assert(ptr->type == HEAD);
    				else if(ptr->type == HEAD)
    				{
    					in_requests.push_back(i);
    					is_requested_inbf[i] = true;
    				}

    			}

    		}
    		else //got 1 channel of in_packets
    		{
    			uint ch=inpkt_ch_for_inbf[i];
    			assert(is_requested_inbf[i]==true);
    			assert(in_packets_ready[ch]==false);

    			if(( in_packets_flit_index[ch] < in_packets[ch].length ) || !is_equal[ch])
    			{
    				in_buffer.change_pull_channel(i);
    				Flit* ptr = in_buffer.pull();

    				assert (ptr->vc == i);
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
#ifdef _DEBUG_CREDIT
                    _DBG("; create a credit downstream_credits sending back ; vc=%d",ptr->vc);
#endif

    				//assert(ptr->type != HEAD);// could be 2eme hf of pkt
    				if ( ptr->type == BODY )
    				{
    					in_packets[ch].add(ptr);
    					in_packets_flit_index[ch]++;
#ifdef _DEBUG_PKT
//                     _DBG("; pkt %lld 0x%x Body flit moved to in_packets[%d] in_packets_flit_index[%d]=%d", static_cast<BodyFlit*>(ptr)->addr, ptr,ch, ch, in_packets_flit_index[ch]);
	                    _DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d Body 0x%x "
	                    		"is_equal=%s",static_cast<BodyFlit*>(ptr)->addr, ch, ch,
	                    		in_packets_flit_index[ch], ptr, (is_equal[ch])?"true":"false");

#endif
    				}
    				else if( ptr->type == TAIL )   // in_buffer is free after then
    				{
    					//assert(is_tf_received[ch] == false);
    					if(is_tf_received[ch] == false)
    							is_tf_received[ch] = true;
    					if (ptr->pseudo)
    					{
    						//in_packets[ch].is_pending = true;
/*if(is_equal[ch])
    							is_equal[ch]=false;
    						else
    							is_equal[ch]=true;
    							*/
    						is_equal[ch] =!is_equal[ch];
#ifdef _DEBUG_PKT_SPLIT
    						_DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d "
    								"Pseudo Tail 0x%x is_equal=%s",static_cast<TailFlit*>(ptr)->addr,
    								ch, ch, in_packets_flit_index[ch], ptr,(is_equal[ch])?"true":"false");
//    	                    _DBG("; pkt 0x%x Tail %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d", ptr,static_cast<HeadFlit*>(ptr)->addr, ch, ch, in_packets_flit_index[ch]);
#endif
    						// drop pkt
    						ullint waiting_time = Simulator::Now() - in_packets[ch].in_packet_arrival;
    						_DBG("; pkt %lld is dropped at NI due to transient fault",in_packets[ch].addr);
    						in_packets_flit_index[ch] = 0;
    						in_packets[ch].length= 0;
    						in_packets[ch].addr=-1;
    						in_packets_ready[ch] = true;
    						is_tf_received[ch] = false;
    						in_packets[ch].clear();
    						is_equal[ch] = true;

    						stat_pkts_drop ++;
    						packets_in --;
    					}
    					else
    					{
    						in_packets_flit_index[ch]++;
#ifdef _DEBUG_PKT
//                     _DBG("; pkt %lld 0x%x Tail flit moved to in_packets[%d] in_packets_flit_index[%d]=%d", static_cast<TailFlit*>(ptr)->addr, ptr,ch, ch, in_packets_flit_index[ch]);
                     _DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d Tail 0x%x "
                    		 "is_equal=%s",static_cast<TailFlit*>(ptr)->addr, ch, ch, in_packets_flit_index[ch],
                    		 ptr, (is_equal[ch])?"true":"false");
#endif
    					}
    					in_packets[ch].add(ptr);
    					is_requested_inbf[i] = false;
    					inpkt_ch_for_inbf[i] = -1;

    				}
    				else if( ptr->type == HEAD )
    				{
    					//_DBG("; NI error , pkt %lld", static_cast<HeadFlit*>(ptr)->addr);
    					if(static_cast<HeadFlit*>(ptr)->virtual_source == VSSET)
    						is_vs[i] = true;

    					if(ptr->pseudo)
    	                {
    	                	/*if(is_equal[ch])
    	                		is_equal[ch]=false;
    	                	else
    	                		is_equal[ch]=true;*/
    						is_equal[ch] =!is_equal[ch];
#ifdef _DEBUG_PKT_SPLIT
    						_DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d "
    								"Pseudo Head 0x%x is_equal=%s",static_cast<HeadFlit*>(ptr)->addr,
    								ch, ch, in_packets_flit_index[ch], ptr,(is_equal[ch])?"true":"false");
#endif
    	                }
    					else
    					{
    						in_packets_flit_index[ch]++;
#ifdef _DEBUG_PKT
//                     _DBG("; pkt %lld 0x%x Head flit moved to in_packets[%d]", static_cast<HeadFlit*>(ptr)->addr, ptr,ch);
//                     _DBG("; pkt %lld 0x%x Head flit moved to in_packets[%d] in_packets_flit_index[%d]=%d", static_cast<HeadFlit*>(ptr)->addr, ptr,ch, ch, in_packets_flit_index[ch]);
                    _DBG("; pkt %lld flit moved to in_packets_%d in_packets_flit_index_%d=%d heaD 0x%x "
                    		"is_equal=%s", static_cast<HeadFlit*>(ptr)->addr,ch, ch, in_packets_flit_index[ch],
                    		ptr, (is_equal[ch])?"true":"false");
#endif
    					//_DBG("; in_buffer %lld , in_packets %lld", static_cast<HeadFlit*>(ptr)->addr, in_packets[inpkt_ch_for_inbf[i]].addr);
    					}

    					ullint add_temp=in_packets[inpkt_ch_for_inbf[i]].addr;
    					in_packets[ch].add(ptr);
    					assert(static_cast<HeadFlit*>(ptr)->addr == add_temp);
    				}


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
        Simulator::Schedule( floor(Simulator::Now())+1, &GenericInterfaceNBb::process_event, this, new_event);
    }

    delete e; e=NULL;
}

string
GenericInterfaceNBb::print_stats()
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
GenericInterfaceNBb::get_flits_in()
{
    return flits_in;
}

ullint
GenericInterfaceNBb::get_flits_out()
{
    return flits_out;
}

ullint
GenericInterfaceNBb::get_packets_out()
{
    return packets_out;
}
ullint
GenericInterfaceNBb::get_vs_packets_in()
{
    return vs_packets_in;
}
ullint
GenericInterfaceNBb::get_packets_in()
{
    return packets_in;
}

ullint
GenericInterfaceNBb::get_packets()
{
    return packets_out + packets_in;
}
ullint
GenericInterfaceNBb::get_packets_drop()
{
    return stat_pkts_drop;
}
#endif   /* ----- #ifndef _GenericInterfaceNBb_cc_INC  ----- */

