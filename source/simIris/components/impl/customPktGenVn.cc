/*!
 * =====================================================================================
 *
 *       Filename:  customPktGenVn.cc
 *
 *    Description:  Implements the stochastic pkt gen class.
 *
 *        Version:  1.0
 *        Created:  02/20/2010 02:09:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef _customPktGenVn_cc_INC
#define _customPktGenVn_cc_INC

#include "customPktGenVn.h"
//#include	"../interfaces/router.h"
//#include	"routerVcMP.h"

//#define _DEBUG
//#define _DEBUG_VN

//#define _DEEP_DEBUG
//#define _LOG

CustomPktGenVn::CustomPktGenVn ()
{
    name = "CustomPktGenVn";
    sending = false;
    out_filename = "traceOut.tr";
    last_vc = 0;
    ni_recv = false;
    srand( time(NULL) );
} /* ----- end of function CustomPktGenVn::CustomPktGenVn ----- */

CustomPktGenVn::~CustomPktGenVn ()
{
	if(flag_enable){
    out_file.close();}
    gsl_rng_free(dest_gen);
    gsl_rng_free(arate_gen);
    gsl_rng_free(plen_gen);
} /* ----- end of function CustomPktGenVn::~CustomPktGenVn ----- */

void
CustomPktGenVn::set_trace_filename( string filename )
{
    trace_name = filename;
    cout<<"TraceName: "<<trace_name<<endl;
}

void
CustomPktGenVn::set_no_vcs ( uint v)
{
    vcs = v;
}

void
CustomPktGenVn::log_enable ()
{
#ifdef _LOG
	flag_enable = true;
#endif
}
void
CustomPktGenVn::enable (uint v, bool enable) // yi:modified to add a flag (0:disable, 1:enable)
{
    vcs =v;

    if (enable) {
    for( unsigned int i = 0; i < vcs; i++ )
    {
        IrisEvent* event = new IrisEvent();
        event->type = READY_EVENT;
        event->vc = i;
        Simulator::Schedule(Simulator::Now()+1, &NetworkComponent::process_event,this, event);
    }
//    flag_enable = true;
    }


    return ;
} /* ----- end of function CustomPktGenVn::setup ----- */


void
CustomPktGenVn::setup (uint n, uint v, uint time)
{
    vcs =v;
    no_nodes = n;
    max_sim_time = time;
    address = myId();
    node_ip = address/3;
    last_vc = 0;
    
    stat_packets_in = 0;
    stat_packets_out = 0;
    stat_packets_out_count = 0;
    stat_min_pkt_latency = 999999999;
    stat_last_packet_out_cycle = 0;
    stat_last_packet_in_cycle = 0;
    stat_total_lat = 0;
    stat_hop_count= 0;
    stat_vs_pkt_out = 0;

    gsl_rng_env_setup();
    gsl_rng_default_seed = 2^25;
    T = gsl_rng_default;
    dest_gen = gsl_rng_alloc (T);
    plen_gen = gsl_rng_alloc (T);
    arate_gen = gsl_rng_alloc (T);

    ready.resize( vcs );
    //ready.insert( ready.begin(), ready.size(), false );
    ready.insert( ready.begin(), ready.size(), true );
//    for(unsigned int i = 0; i < ready.size(); i++)
//        ready[i] = true;
    lastSentTime = 0;
    irt = 0;

    vs_request.resize(vcs);
    vs_request.insert( vs_request.begin(), vs_request.size(), false );

//   for( unsigned int i = 0; i < vcs; i++ )
//   {
//       IrisEvent* event = new IrisEvent();
//       event->type = READY_EVENT;
//       event->vc = i;
//       Simulator::Schedule(Simulator::Now()+1, &NetworkComponent::process_event,this, event);
//   }


    return ;
} /* ----- end of function CustomPktGenVn::setup ----- */

void
CustomPktGenVn::set_output_path( string name)
{
	if (flag_enable)
	{
    // open the output trace file
    stringstream str;
    str << name << "/tpg_" << node_ip << "_trace_out.tr";
    out_filename = str.str();
    out_file.open(out_filename.c_str());
    if( !out_file.is_open() )
    {
        _DBG_NOARG("Could not open output trace file ");
        cout << out_filename << endl;
    }
	}
}

void
CustomPktGenVn::finish ()
{
	if (flag_enable)
	{
	out_file.close();
    trace_filename->close();
	}
    return ;
} /* ----- end of function CustomPktGenVn::finish ----- */

void
CustomPktGenVn::process_event (IrisEvent* e)
{
    switch(e->type)
    {
        case NEW_PACKET_EVENT:
#ifdef _DEBUG
            _DBG("; CustomPktGenVn::NEW_PACKET_EVENT ; time:%f", Simulator::Now());
#endif
        	handle_new_packet_event(e);
            break;
        case OUT_PULL_EVENT:
#ifdef _DEBUG
            _DBG("; CustomPktGenVn::OUT_PULL_EVENT ; time:%f", Simulator::Now());
#endif
            handle_out_pull_event(e);
            break;
        case READY_EVENT:
#ifdef _DEBUG
            _DBG("; CustomPktGenVn::READY_EVENT ; time:%f", Simulator::Now());
#endif
            handle_ready_event(e);
            break;
        case WARMUP_EVENT:
#ifdef _DEBUG
        _DBG("; CustomPktGenVn::WARMUP_EVENT ; time:%f", Simulator::Now());
#endif
            handle_warmup_event(e);
            break;
        case ONE_PULL_EVENT:
        #ifdef _DEBUG
                    _DBG("; CustomPktGenVn::ONE_PULL_EVENT ; time:%f", Simulator::Now());
        #endif
                    handle_one_pull_event(e);
                    break;
        case RESET_STAT_EVENT:
#ifdef _DEBUG
            _DBG("; CustomPktGenVn::RESET_STAT_EVENT time:%f", Simulator::Now());
#endif
        	handle_reset_stat_event(e);
            break;
        case VS_PKT_ARRIVAL_EVENT:
#ifdef _DEBUG
            _DBG("; CustomPktGenVn::VS_PKT_ARRIVAL_EVENT time:%f", Simulator::Now());
#endif
        	handle_vs_pkt_arrival_event(e);
            break;
        default:
            cout << "\nTPG: " << address << "process_event: UNK event type" << endl;
            break;
    }
    return ;
} /* ----- end of function CustomPktGenVn::process_event ----- */


void
CustomPktGenVn::handle_new_packet_event ( IrisEvent* e)
{
//#ifdef _DEBUG
//       _DBG("; CustomPktGenVn::handle_new_packet_event at time=%f ", Simulator::Now());
//#endif

	ni_recv = false;
    // get the packet data
    HighLevelPacket* hlp = static_cast< HighLevelPacket* >( e->event_data.at(0));
    if ((hlp->virtual_source==VSSET) || (hlp->virtual_source==VSRESET))
    {
//    	cerr << "CustomPktGenVn: virtual_source arrived: "<< hlp->toString();

        IrisEvent* event = new IrisEvent();
        event->type = VS_PKT_ARRIVAL_EVENT;
        event->event_data.push_back(hlp);

        Simulator::Schedule( Simulator::Now()+2,
                             &NetworkComponent::process_event,
                             this,event);
        //delete hlp;
    }

    else
    {
    	if(hlp->transaction_id == 1000)
    	{
    		ullint lat=Simulator::Now() - hlp->sent_time;
    		ullint st=hlp->sent_time;
    		//_DBG("; CustomPktGenVn::warmup pkt  %lld arrives with latency %lld sendtime %lld ",hlp->addr,lat, st);
    	}
    	else
    	{
    		double lat = Simulator::Now() - hlp->sent_time;
    		if( stat_min_pkt_latency > lat)
    			stat_min_pkt_latency = lat;

    		stat_last_packet_in_cycle = Simulator::Now();
    		stat_packets_in++;
    		stat_hop_count += hlp->hop_count;
    		stat_total_lat += lat;

    		if(flag_enable) {
    			out_file << "last_packet_in_cycle " <<stat_last_packet_in_cycle<< ";\taddr "<<hlp->addr<<";\tlat "<<lat<<";\thop_count "<<hlp->hop_count << ";\tstat_total_lat "<<stat_total_lat<< ";\tstat_packets_in " << stat_packets_in<< endl;

    		}

#ifdef DEBUG
    		_DBG( "-------------- TPG GOT NEW PACKET --------------- pkt_latency: %f", lat);
    		// write out the packet data to the output trace file

    		if(flag_enable) {
    			if( !out_file.is_open() )
    				out_file.open( out_filename.c_str(), std::ios_base::app );

    			out_file << hlp->toString();
    			out_file << "\tPkt latency: " << lat << endl;
    		}
#endif

    		/* Note down time of last pkt */
    		Simulator::LastPkt(stat_last_packet_in_cycle);
    	}
    }

    // send back a ready event to release the channel occupied 
    IrisEvent* event2 = new IrisEvent();
    event2->type = READY_EVENT;
    event2->vc = hlp->virtual_channel;
    Simulator::Schedule( Simulator::Now()+1, &NetworkComponent::process_event, interface_connections[0], event2);
#ifdef _DEBUG
       _DBG("; READY_EVENT is created for interface at time=%d ", Simulator::Now());
#endif
//    delete hlp;
    //delete hlp; hlp=NULL;
    delete e;  e=NULL;
    return ;



} /* ----- end of function CustomPktGenVn::handle_new_packet_event ----- */

void
CustomPktGenVn::handle_vs_pkt_arrival_event ( IrisEvent* e )
{
    // change vn, vs
	HighLevelPacket* hlp = static_cast< HighLevelPacket* >( e->event_data.at(0));
	virtual_network vn_current = hlp->vn;
	virtual_network vn_found;

    sending =false;
    bool found = false;
    uint sending_vc = -1;
    uint max_vc = vcs;

    if(do_request_reply_network) //  do_request_reply_network : only half vc could be used
        max_vc = vcs/2;

    for( uint i=last_vc+1; i<max_vc; i++) // round-robin
    {
        if(ready[i])
        {
            if (i < max_vc/2) vn_found=VN0; //(VN0:South Last)
            else vn_found=VN1; //(VN1:North Last)
            if (vn_found != vn_current)
            {
            	found = true;
            	sending_vc = i;
            	last_vc = i;
            	break;
            }

        }
    }
    if ( !found)
    {
        for ( uint i=0; i<=last_vc; i++)
        {
            if(ready[i])
            {
                if (i < max_vc/2) vn_found=VN0; //(VN0:South Last)
                else vn_found=VN1; //(VN1:North Last)
                if (vn_found != vn_current)
                {
                	found = true;
                	sending_vc = i;
                	last_vc = i;
                	break;
                }
            }
        }
    }

    if( found )
    {
    	stat_vs_pkt_out ++;
    	hlp->vn = vn_found;
    	hlp->virtual_channel = sending_vc;
    	assert (hlp->virtual_source != NOVS);
    	//if (hlp->virtual_source == VSRESET) hlp->virtual_source = NOVS;
    	hlp->virtual_source = NOVS;
        IrisEvent* event = new IrisEvent();
        event->type = NEW_PACKET_EVENT;
        event->event_data.push_back(hlp);
        event->vc = sending_vc;

        ready[sending_vc] = false;

#ifdef _DEBUG_VN
        _DBG("; virtual source pkt %lld for dst %d sent out at %f from %d ", hlp->addr,hlp->destination, Simulator::Now(),node_ip);
#endif
        //Simulator::Schedule( Simulator::Now()+0.75, &NetworkComponent::process_event, interface_connections[0], event );
        Simulator::Schedule( Simulator::Now()+1, &NetworkComponent::process_event, interface_connections[0], event );

    }
    else
    {
    	//cerr << "CustomPktGenVn: no available vc for vs_pkt"<<endl;

    	        IrisEvent* event = new IrisEvent();
    	        event->type = VS_PKT_ARRIVAL_EVENT;
    	        event->event_data.push_back(hlp);

    	        Simulator::Schedule( Simulator::Now()+2,
    	                             &NetworkComponent::process_event,
    	                             this,event);
//   	        delete hlp;

    }

    delete e; e=NULL;

}
void
CustomPktGenVn::handle_warmup_event ( IrisEvent* e )
{
    sending =false;
    bool found = false;
    uint sending_vc = -1;
    uint max_vc = vcs;

    if(do_request_reply_network) //  do_request_reply_network : only half vc could be used
        max_vc = vcs/2;

    for( uint i=last_vc+1; i<max_vc; i++) // round-robin
    {
        if(ready[i])
        {
            found = true;
            sending_vc = i;
            last_vc = i;
            break;
        }
    }
    if ( !found)
    {
        for ( uint i=0; i<=last_vc; i++)
        {
            if(ready[i])
            {
                found = true;
                sending_vc = i;
                last_vc = i;
                break;
            }
        }
    }

    /* Example of how to allow some nodes to inject
       if( node_ip != (no_nodes-1))
       found = false;
     */
    if (Simulator::Now()< sim_warmup_time)
    {
    	if( found )
    	{

    		if( lastSentTime+irt <= Simulator::Now() )
    		{

    			HighLevelPacket* hlp = new HighLevelPacket();
    			hlp->virtual_channel = sending_vc;
    			hlp->source = node_ip;
    			if (pattern_type==RANDOM_UNI)
    				hlp->destination = mc_positions[stat_packets_out_count % grid_size];
    				//hlp->destination = mc_positions[gsl_rng_uniform_int( dest_gen , no_mcs) % ( no_mcs)];
    			else if (pattern_type==TRANSPOSE)
    				hlp->destination = node_transpose();
    			else if (pattern_type==HOTSPOT) {

    				if (rand()%10==0) // 10% pkt sent to hotspot
    					hlp->destination = grid_size/2*grid_size+grid_size/2-1;
    				else
    					hlp->destination = mc_positions[stat_packets_out_count % grid_size];
    				//hlp->destination = mc_positions[gsl_rng_uniform_int( dest_gen , no_mcs) % ( no_mcs)];
    			}

    			// check vc and dst corresponds
    			if (hlp->virtual_channel < max_vc/2) hlp->vn=VN0; //(VN0:South Last)
    			else hlp->vn=VN1; //(VN1:North Last)
    			virtual_network vn_dst;
    			if ((hlp->destination / grid_size) > (node_ip / grid_size)) vn_dst=VN0;
    			else if ((hlp->destination / grid_size) < (node_ip / grid_size)) vn_dst=VN1;
    			else vn_dst=hlp->vn;
    			if (hlp->vn == vn_dst)
    			{
#ifdef _DEBUG_VN
    				_DBG("stat_packets_out_count: %d; src: %d ; dst : %d ; vc : %d ; vn : %d",stat_packets_out_count,  node_ip, hlp->destination, hlp->virtual_channel, hlp->vn);
#endif
    				stat_packets_out++;
    				stat_packets_out_count++;
    				hlp->addr = node_ip*10000 + stat_packets_out_count;
    				hlp->transaction_id = 1000;


    				if( hlp->destination == node_ip )
    					hlp->destination = (hlp->destination + 1) % no_nodes ;// !!!!!

    				hlp->sent_time = (ullint)Simulator::Now();
    				uint pkt_len = pkt_payload_length; //1*max_phy_link_bits; //Static for now but can use gsl_ran
    				hlp->data_payload_length = pkt_len;
    				hlp->msg_class = terminal_msg_class;
    				/*
               for ( uint i=0 ; i < pkt_len ; i++ )
               {
               hlp->data.push_back(true);
               }
    				 * */
    				ready[sending_vc] = false;

    				assert ( hlp->destination < no_nodes);
#ifdef _DEEP_DEBUG
    				_DBG( " Sending at TPG out pull: VC %d dest:%d ", sending_vc, hlp->destination );
    				cout << " HLP: " << hlp->toString() << endl;
    				//cout << "\n HLP: " << hlp->toString() << endl;
    				//cout << " HLP addr: " << hlp->addr << endl;
    				//cout << node_ip <<" = node_ip ; "<< " Pktgen : Simulator::Now(): " << Simulator::Now();
    				_DBG( " Sending pkt: no of packets %d ", stat_packets_out);
#endif
    				if(flag_enable)
    				{
    					out_file << Simulator::Now() << " " << hlp->destination << endl;
    				}

    				IrisEvent* event = new IrisEvent();
    				event->type = NEW_PACKET_EVENT;
    				event->event_data.push_back(hlp);
    				event->vc = sending_vc;
#ifdef _DEBUG
    				_DBG("; found hlp->sent_time is %lld , but Now() is %f ", hlp->sent_time, Simulator::Now());
#endif
    				Simulator::Schedule( hlp->sent_time, &NetworkComponent::process_event, interface_connections[0], event );
    				//            Simulator::Schedule( Simulator::Now(), &NetworkComponent::process_event, interface_connections[0], event );

    				lastSentTime = (ullint)Simulator::Now();
    				//irt = gsl_ran_gaussian_tail( arate_gen ,0,mean_irt) ;
    				if(irt == 0 ) irt +=1;
    				//            irt = gsl_ran_poisson( arate_gen ,mean_irt) ;
    				stat_last_packet_out_cycle = Simulator::Now();
    				//            if( stat_packets_out >= 10000 )
    				//                irt = max_sim_time;
    				/*
               for( uint i=0; i<vcs; i++)
               if(ready[i])
               {
               irt = 10;
               break;
               }
               else
               irt=10;
    				 */
    				sending = true;
#ifdef _DEBUG
    				_DBG("; irt is %d ", irt);
    				cout << "\n irt is "<<irt;
#endif
    			}
    			else {
#ifdef _DEBUG
    				_DBG_NOARG("; dst_vn is not correspond to vc_vn ");
#endif

    				sending = true;
    			}

    		}

    	}

    	else  // not find a vc is ready
    	{
#ifdef _DEBUG
    		//            _DBG("; available vc is not found; lastSentTime+irt is %lld , but Now() is %f ", lastSentTime+irt , Simulator::Now());
    		//            cout << "\n Simulator::Now() is "<<Simulator::Now() << "; lastSentTime+irt " << lastSentTime+irt;
    		//            printf("\n printf Simulator::Now() is %f", Simulator::Now());
    		//            cout << "\n size of Simulator::Now() is "<<sizeof(Simulator::Now())<<"; size of irt is "<< sizeof(irt)<<"; size of lastSentTime is "<< sizeof(lastSentTime);
#endif
    		//sending = true; //yi:23102012, why here is true ? to get next OUT_PULL_EVENT
    		sending = true;
    		//irt = gsl_ran_poisson( arate_gen ,mean_irt) ;
    		//_DBG("; new irt is %d ", irt);
    	}



    	/* Example of how to allow some nodes to inject
       if( node_ip != (no_nodes-1))
       sending = false;
    	 */
    	//_DBG("; new irt is %d ", irt);
    	if(irt == 0 ) irt +=1;

    	if( sending)  // scheduling the next packet generation Now()+irt
    	{
    		IrisEvent* event2 = new IrisEvent();
    		event2->type = WARMUP_EVENT;
    		Simulator::Schedule(Simulator::Now()+irt, &NetworkComponent::process_event, this, event2);
    		sending = true;
#ifdef _DEBUG
    		//            _DBG("; Simulator::Now()+irt is time:%f", Simulator::Now()+irt);
#endif
    	}
    }
    /*else
    {
    	if( sending)  // scheduling the next packet generation Now()+irt
    	{
    		IrisEvent* event2 = new IrisEvent();
    		event2->type = OUT_PULL_EVENT;
    		Simulator::Schedule(Simulator::Now()+irt, &NetworkComponent::process_event, this, event2);
    		sending = true;
    	}
    }*/
    delete e;
}
void
CustomPktGenVn::handle_one_pull_event ( IrisEvent* e )
{
	//_DBG("ONE PULL EVENT : dst: %d ",e->dst_id);

	sending =false;
    bool found = false;
    uint sending_vc = -1;
    uint max_vc = vcs;

    if(do_request_reply_network) //  do_request_reply_network : only half vc could be used
        max_vc = vcs/2;

    for( uint i=last_vc+1; i<max_vc; i++) // round-robin
    {
        if(ready[i])
        {
            found = true;
            sending_vc = i;
            last_vc = i;
            break;
        }
    }
    if ( !found)
    {
        for ( uint i=0; i<=last_vc; i++)
        {
            if(ready[i])
            {
                found = true;
                sending_vc = i;
                last_vc = i;
                break;
            }
        }
    }

    /* Example of how to allow some nodes to inject
       if( node_ip != (no_nodes-1))
       found = false;
     */
    if( found )
    {
       //if( lastSentTime+irt <= Simulator::Now() )
       {

    	   HighLevelPacket* hlp = new HighLevelPacket();
    	   hlp->virtual_channel = sending_vc;
    	   hlp->source = node_ip;
    	   if (pattern_type==RANDOM_UNI)
    		   hlp->destination = e->dst_id;
    		   //hlp->destination = mc_positions[gsl_rng_uniform_int( dest_gen , no_mcs) % ( no_mcs)];
    	   /*else if (pattern_type==TRANSPOSE)
    		   hlp->destination = node_transpose();
    	   else if (pattern_type==HOTSPOT) {

    		   if (rand()%10==0) // 10% pkt sent to hotspot
    			   hlp->destination = grid_size/2*grid_size+grid_size/2-1;
    		   else
    			   hlp->destination = mc_positions[gsl_rng_uniform_int( dest_gen , no_mcs) % ( no_mcs)];
    	   }*/

    	   // check vc and dst corresponds
    	   if (hlp->virtual_channel < max_vc/2) hlp->vn=VN0; //(VN0:South Last)
    	   else hlp->vn=VN1; //(VN1:North Last)
    	   virtual_network vn_dst;
    	   if ((hlp->destination / grid_size) > (node_ip / grid_size)) vn_dst=VN0;
    	   else if ((hlp->destination / grid_size) < (node_ip / grid_size)) vn_dst=VN1;
    	   else vn_dst=hlp->vn;
    	   if (hlp->vn == vn_dst) {
#ifdef _DEBUG_VN
    		   _DBG("stat_packets_out_count: %d; src: %d ; dst : %d ; vc : %d ; vn : %d",stat_packets_out_count,  node_ip, hlp->destination, hlp->virtual_channel, hlp->vn);
#endif
    		   stat_packets_out++;
    		   stat_packets_out_count++;
    		   hlp->addr = node_ip*10000 + stat_packets_out_count;
    		   hlp->transaction_id = 2000;

    		   if( hlp->destination == node_ip )
    			   hlp->destination = (hlp->destination + 1) % no_nodes ;// !!!!!

    		   hlp->sent_time = (ullint)Simulator::Now();
    		   uint pkt_len = pkt_payload_length; //1*max_phy_link_bits; //Static for now but can use gsl_ran
    		   hlp->data_payload_length = pkt_len;
    		   hlp->msg_class = terminal_msg_class;
    		   /*
               for ( uint i=0 ; i < pkt_len ; i++ )
               {
               hlp->data.push_back(true);
               }
    		    * */
    		   ready[sending_vc] = false;

    		   assert ( hlp->destination < no_nodes);
#ifdef _DEEP_DEBUG
    		   _DBG( " Sending at TPG out pull: VC %d dest:%d ", sending_vc, hlp->destination );
    		   cout << " HLP: " << hlp->toString() << endl;
    		   //cout << "\n HLP: " << hlp->toString() << endl;
    		   //cout << " HLP addr: " << hlp->addr << endl;
    		   //cout << node_ip <<" = node_ip ; "<< " Pktgen : Simulator::Now(): " << Simulator::Now();
    		   _DBG( " Sending pkt: no of packets %d ", stat_packets_out);
#endif
    		   if(flag_enable) {
    			   out_file << Simulator::Now() << " " << hlp->destination << endl;
    		   }

    		   IrisEvent* event = new IrisEvent();
    		   event->type = NEW_PACKET_EVENT;
    		   event->event_data.push_back(hlp);
    		   event->vc = sending_vc;
#ifdef _DEBUG
    		   _DBG("; found hlp->sent_time is %lld , but Now() is %f ", hlp->sent_time, Simulator::Now());
#endif
    		   Simulator::Schedule( hlp->sent_time, &NetworkComponent::process_event, interface_connections[0], event );
    		   //            Simulator::Schedule( Simulator::Now(), &NetworkComponent::process_event, interface_connections[0], event );

    		   lastSentTime = (ullint)Simulator::Now();
    		   irt = gsl_ran_gaussian_tail( arate_gen ,0,mean_irt) ;
    		   if(irt == 0 ) irt +=1;
    		   //            irt = gsl_ran_poisson( arate_gen ,mean_irt) ;
    		   stat_last_packet_out_cycle = Simulator::Now();
    		   //            if( stat_packets_out >= 10000 )
    		   //                irt = max_sim_time;
    		   /*
               for( uint i=0; i<vcs; i++)
               if(ready[i])
               {
               irt = 10;
               break;
               }
               else
               irt=10;
    		    */
    		   sending = true;
    		   _DBG("ONE PULL EVENT : dst: %d ",e->dst_id);
#ifdef _DEBUG
    		   _DBG("; irt is %d ", irt);
    		   cout << "\n irt is "<<irt;
#endif
    	   }
    	   else {
#ifdef _DEBUG
    		   _DBG_NOARG("; dst_vn is not correspond to vc_vn ");
#endif

    		   sending = false;
    	   }

       }

    }
    if( !sending)
    {
    	IrisEvent* event2 = new IrisEvent();
    	event2->dst_id = e->dst_id;
    	event2->type = ONE_PULL_EVENT;
    	Simulator::Schedule(Simulator::Now(), &NetworkComponent::process_event, this, event2);
    }

    delete e;
}
void
CustomPktGenVn::handle_out_pull_event ( IrisEvent* e )
{
    sending =false;
    bool found = false;
    uint sending_vc = -1;
    uint max_vc = vcs;

    if(do_request_reply_network) //  do_request_reply_network : only half vc could be used
        max_vc = vcs/2;

    for( uint i=last_vc+1; i<max_vc; i++) // round-robin 
    {
        if(ready[i])
        {
            found = true;
            sending_vc = i;
            last_vc = i;
            break;
        }
    }
    if ( !found)
    {
        for ( uint i=0; i<=last_vc; i++)
        {
            if(ready[i])
            {
                found = true;
                sending_vc = i;
                last_vc = i;
                break;
            }
        }
    }

    /* Example of how to allow some nodes to inject 
       if( node_ip != (no_nodes-1))
       found = false;
     */
    if( found )
    {
       if( lastSentTime+irt <= Simulator::Now() )
       {

    	   HighLevelPacket* hlp = new HighLevelPacket();
    	   hlp->virtual_channel = sending_vc;
    	   hlp->source = node_ip;
    	   if (pattern_type==RANDOM_UNI)
    		   hlp->destination = mc_positions[gsl_rng_uniform_int( dest_gen , no_mcs) % ( no_mcs)];
    	   else if (pattern_type==TRANSPOSE)
    		   hlp->destination = node_transpose();
    	   else if (pattern_type==HOTSPOT) {

    		   if (rand()%10==0) // 10% pkt sent to hotspot
    			   hlp->destination = grid_size/2*grid_size+grid_size/2-1;
    		   else
    			   hlp->destination = mc_positions[gsl_rng_uniform_int( dest_gen , no_mcs) % ( no_mcs)];
    	   }

    	   // check vc and dst corresponds
    	   if (hlp->virtual_channel < max_vc/2) hlp->vn=VN0; //(VN0:South Last)
    	   else hlp->vn=VN1; //(VN1:North Last)
    	   virtual_network vn_dst;
    	   if ((hlp->destination / grid_size) > (node_ip / grid_size)) vn_dst=VN0;
    	   else if ((hlp->destination / grid_size) < (node_ip / grid_size)) vn_dst=VN1;
    	   else vn_dst=hlp->vn;
    	   if (hlp->vn == vn_dst) {
#ifdef _DEBUG_VN
    		   _DBG("stat_packets_out_count: %d; src: %d ; dst : %d ; vc : %d ; vn : %d",stat_packets_out_count,  node_ip, hlp->destination, hlp->virtual_channel, hlp->vn);
#endif
    		   stat_packets_out++;
    		   stat_packets_out_count++;
    		   hlp->addr = node_ip*10000 + stat_packets_out_count;
    		   hlp->transaction_id = 1000;


    		   if( hlp->destination == node_ip )
    			   hlp->destination = (hlp->destination + 1) % no_nodes ;// !!!!!

    		   hlp->sent_time = (ullint)Simulator::Now();
    		   uint pkt_len = pkt_payload_length; //1*max_phy_link_bits; //Static for now but can use gsl_ran
    		   hlp->data_payload_length = pkt_len;
    		   hlp->msg_class = terminal_msg_class;
    		   /*
               for ( uint i=0 ; i < pkt_len ; i++ )
               {
               hlp->data.push_back(true);
               }
    		    * */
    		   ready[sending_vc] = false;

    		   assert ( hlp->destination < no_nodes);
#ifdef _DEEP_DEBUG
    		   _DBG( " Sending at TPG out pull: VC %d dest:%d ", sending_vc, hlp->destination );
    		   cout << " HLP: " << hlp->toString() << endl;
    		   //cout << "\n HLP: " << hlp->toString() << endl;
    		   //cout << " HLP addr: " << hlp->addr << endl;
    		   //cout << node_ip <<" = node_ip ; "<< " Pktgen : Simulator::Now(): " << Simulator::Now();
    		   _DBG( " Sending pkt: no of packets %d ", stat_packets_out);
#endif
    		   if(flag_enable) {
    			   out_file << Simulator::Now() << " " << hlp->destination << endl;
    		   }

    		   IrisEvent* event = new IrisEvent();
    		   event->type = NEW_PACKET_EVENT;
    		   event->event_data.push_back(hlp);
    		   event->vc = sending_vc;
#ifdef _DEBUG
    		   _DBG("; found hlp->sent_time is %lld , but Now() is %f ", hlp->sent_time, Simulator::Now());
#endif
    		   Simulator::Schedule( hlp->sent_time, &NetworkComponent::process_event, interface_connections[0], event );
    		   //            Simulator::Schedule( Simulator::Now(), &NetworkComponent::process_event, interface_connections[0], event );

    		   lastSentTime = (ullint)Simulator::Now();
    		   irt = gsl_ran_gaussian_tail( arate_gen ,0,mean_irt) ;
    		   if(irt == 0 ) irt +=1;
    		   //            irt = gsl_ran_poisson( arate_gen ,mean_irt) ;
    		   stat_last_packet_out_cycle = Simulator::Now();
    		   //            if( stat_packets_out >= 10000 )
    		   //                irt = max_sim_time;
    		   /*
               for( uint i=0; i<vcs; i++)
               if(ready[i])
               {
               irt = 10;
               break;
               }
               else
               irt=10;
    		    */
    		   sending = true;
#ifdef _DEBUG
    		   _DBG("; irt is %d ", irt);
    		   cout << "\n irt is "<<irt;
#endif
    	   }
    	   else {
#ifdef _DEBUG
    		   _DBG_NOARG("; dst_vn is not correspond to vc_vn ");
#endif

    		   sending = true;
    	   }

       }

    }
    else  // not find a vc is ready
    {
#ifdef _DEBUG
//            _DBG("; available vc is not found; lastSentTime+irt is %lld , but Now() is %f ", lastSentTime+irt , Simulator::Now());
//            cout << "\n Simulator::Now() is "<<Simulator::Now() << "; lastSentTime+irt " << lastSentTime+irt;
//            printf("\n printf Simulator::Now() is %f", Simulator::Now());
//            cout << "\n size of Simulator::Now() is "<<sizeof(Simulator::Now())<<"; size of irt is "<< sizeof(irt)<<"; size of lastSentTime is "<< sizeof(lastSentTime);
#endif
    	//sending = true; //yi:23102012, why here is true ? to get next OUT_PULL_EVENT
    	sending = true;
        irt = gsl_ran_poisson( arate_gen ,mean_irt) ;
        //_DBG("; new irt is %d ", irt);
    }

    /* Example of how to allow some nodes to inject 
       if( node_ip != (no_nodes-1))
       sending = false;
     */
    //_DBG("; new irt is %d ", irt);
    if(irt == 0 ) irt +=1;

    if( sending)  // scheduling the next packet generation Now()+irt
    {
        IrisEvent* event2 = new IrisEvent();
        event2->type = OUT_PULL_EVENT;
        Simulator::Schedule(Simulator::Now()+irt, &NetworkComponent::process_event, this, event2);
        sending = true;
#ifdef _DEBUG
//            _DBG("; Simulator::Now()+irt is time:%f", Simulator::Now()+irt);
#endif
    }		
    delete e;
}

void
CustomPktGenVn::handle_ready_event ( IrisEvent* e)  // got ready event from NI which means it is ready to accept new pkt, reset available vc
{

    if ( e->vc > vcs )
    {
        _DBG("; Got ready for vc %d no_vcs %d ", e->vc, vcs);
        exit(1);
    }

    // send the first packet and schedule a out pull if ur ready
    ready[e->vc] = true;
#ifdef _DEBUG
       _DBG("; Got ready for vc %d ", e->vc);
#endif

    if( !sending && Simulator::Now()==1)// < max_sim_time ) // only vc=0 event's time=1, and only sending once

    {
        IrisEvent* event = new IrisEvent();
        //event->type = OUT_PULL_EVENT;
        event->type = WARMUP_EVENT;
        event->vc = e->vc;
//#ifdef _DEBUG
//            _DBG("; IrisEvent->vc = %d ", e->vc);
//#endif
        Simulator::Schedule(Simulator::Now()+1, &NetworkComponent::process_event, this, event);
        sending = true;
    }

    delete e; e=NULL;
    return ;
} /* ----- end of function CustomPktGenVn::handle_ready_event ----- */
uint
CustomPktGenVn::node_transpose()
{

	uint x=node_ip%grid_size;
	uint y=node_ip/grid_size;
	return x*grid_size+y;
}
void
CustomPktGenVn::handle_reset_stat_event( IrisEvent* e)
{
    stat_packets_in = 0;
    stat_packets_out = 0;
    stat_min_pkt_latency = 999999999;
    stat_last_packet_out_cycle = 0;
    stat_last_packet_in_cycle = 0;
    stat_total_lat = 0;
    stat_hop_count= 0;
    stat_vs_pkt_out = 0;
    delete e; e=NULL;
}
ullint
CustomPktGenVn::get_average_packet_latency()
{
	if (stat_total_lat==0) return 0;
	else 
	    return (stat_total_lat+0.0)/stat_packets_in;
}
ullint
CustomPktGenVn::get_vs_packet_out()
{
	if (stat_vs_pkt_out==0) return 0;
	else
	    return stat_vs_pkt_out;
}

string
CustomPktGenVn::toString () const
{
    stringstream str;
    str << "\nCustomPktGenVn: "
        << "\t vcs: " << ready.size()
        << "\t address: " <<address
        << "\t node_ip: " << node_ip
        ;
    return str.str();
} /*  ----- end of function CustomPktGenVn::toString ----- */

string
CustomPktGenVn::print_stats() const
{
    stringstream str;
    str << "\n PktGen[" << node_ip << "] packets_out: " << stat_packets_out
        << "\n PktGen[" << node_ip << "] packets_in: " << stat_packets_in
        << "\n PktGen[" << node_ip << "] min_pkt_latency: " << stat_min_pkt_latency
        << "\n PktGen[" << node_ip << "] last_packet_out_cycle: " << stat_last_packet_out_cycle
        << "\n PktGen[" << node_ip << "] last_packet_in_cycle: " << stat_last_packet_in_cycle
        << "\n PktGen[" << node_ip << "] avg_latency: " << (stat_total_lat+0.0)/stat_packets_in
        << "\n PktGen[" << node_ip << "] avg_hop_count: " << (stat_hop_count+0.0)/stat_packets_in
        << endl;
    return str.str();
} /* ----- end of function CustomPktGenVn::toString ----- */

#endif /* ----- #ifndef _customPktGenVn_cc_INC ----- */
