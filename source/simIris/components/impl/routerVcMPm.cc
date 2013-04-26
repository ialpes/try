/*!
 * =====================================================================================
 *
 *       Filename: \file  routerVcMP.cc
 *       Description: Simple router with multiple virtual channels.
 *
 *        Version:  1.0
 *        Created:  03/11/2010 09:20:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _routerVcMPm_cc_INC
#define  _routerVcMPm_cc_INC

#include	"routerVcMPm.h"
using namespace std;

//#define _DEBUG_ROUTER
//#define _DEBUG_ROUTER_HEAD
//#define _DEBUG_PKT
//#define _DEBUG_VCA
//#define _DEBUG_NOP
//#define _NOP
//#define _DEBUG_FI
#define  _DEBUG_METRIC
//#define _OLDVCA

RouterVcMPm::RouterVcMPm ()
{
    name = "RouterVcMPm" ;
    ticking = false;
}  /* -----  end of method RouterVcMPm::RouterVcMPm  (constructor)  ----- */

RouterVcMPm::~RouterVcMPm()
{
}

void
RouterVcMPm::find_neighbors(uint p, uint v, uint cr, uint bs)
{
    ports =p;
    vcs =v;
    credits =cr;
    buffer_size = bs;

    neighbors.resize(ports);
    {
   		for (uint i=1; i<ports; i++)
    		{
   			neighbors.push_back(-1);
    		if( static_cast<GenericLink*>(input_connections[i])->input_connection!=NULL )

    				neighbors[i]= static_cast<GenericLink*>(input_connections[i])->input_connection->node_ip; //link_id;
    		}
    }

    for(uint i=0; i<ports; i++)
    {
    	if (neighbors[i]!= -1)
        {
        	nop_rc[i].node_ip = neighbors[i];
        	nop_rc[i].resize( vcs );
        }

    }

    return ;
}
void
RouterVcMPm::set_fault(uint p)
{
	//fault_ports[p]=1; //used as flag
	fault_ports.push_back(p);
	//nb_faults++;
	return ;
}
void
RouterVcMPm::remove_fault(uint p)
{
	fault_ports.erase(remove(fault_ports.begin(),fault_ports.end(),p),fault_ports.end());
	return ;
}
void
RouterVcMPm::handle_fault_remove_event(IrisEvent* e )
{
	remove_fault(e->dst_id);
    delete e;
	return;
}
void
RouterVcMPm::set_temp_opfault(uint p, double t, ullint d)
{
    IrisEvent* event = new IrisEvent();
    event->type = FAULT_INJECT_EVENT;
    event->dst_id = p;//faulty port
    event->time = d;//fault duration

    Simulator::Schedule( t,
                         &NetworkComponent::process_event,
                         this,
                         event);
	return ;
}
void
RouterVcMPm::set_temp_ipfault(uint p, double t, ullint d)
{
	return ;
}

void
RouterVcMPm::handle_fault_inject_event(IrisEvent* e )
{

	set_fault(e->dst_id);
	IrisEvent* event = new IrisEvent();
	    event->type = FAULT_REMOVE_EVENT;
	    event->dst_id = e->dst_id;//faulty port
	    Simulator::Schedule( Simulator::Now()+e->time,
	                         &NetworkComponent::process_event,
	                         this,
	                         event);
	    delete e;
		return ;

}

void
RouterVcMPm::init (uint p, uint v, uint cr, uint bs)
{
    ports =p;
    vcs =v;
    credits =cr;
    buffer_size = bs;

    address = myId();

    /*  set_input_ports(ports); */
    in_buffers.resize(ports);
    decoders.resize(ports);

    fault_ports.clear();
    nop_rc.resize(ports);

    vca.resize(ports, vcs);
    vca.node_ip = node_ip;
    vca.address = address;
    swa.resize(ports, vcs);
    swa.node_ip = node_ip;
    swa.address = address;

    input_buffer_state.resize(ports*vcs);
    vc_alloc.resize(ports);
    sw_alloc.resize(ports);
    downstream_credits.resize(ports);
    cr_time.resize(ports);
    request_op.resize(ports);
    request_output.resize(ports);
    remain_flits.resize(ports);

    /*Resize per port stats */
    stat_packet_out.resize(ports);
    stat_flit_out.resize(ports);

	cvr.resize(ports);
	local_metric.resize(ports);

    /* All decoders and vc arbiters need to know the node_ip for routing */
    for(uint i=0; i<ports; i++)
    {
        decoders[i].node_ip = node_ip;
        decoders[i].address = address;
        remain_flits[i]=0;
        stat_packet_out[i].resize(ports);
        stat_flit_out[i].resize(ports);
    }

    /*  set_no_virtual_channels(vcs); */
    for(uint i=0; i<ports; i++)
    {
        downstream_credits[i].resize(vcs);
        cr_time[i].resize(vcs);
        in_buffers[i].resize( vcs, buffer_size );
        decoders[i].resize( vcs );
    }

    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            downstream_credits[i][j] = credits;
            cr_time[i][j] = -1;
            input_buffer_state[i*vcs+j].pipe_stage = EMPTY;
            vc_alloc[i].push_back(j);
        }

    /* init the countes */
    stat_packets = 0;
    stat_flits = 0;
    stat_total_packet_latency = 0;
    stat_buffer_occupancy = 0;
    stat_swa_fail_msg_ratio = 0;
    stat_swa_load = 0;
    stat_vca_fail_msg_ratio = 0;
    stat_vca_load = 0;
    stat_ib_cycles = 0;
    stat_rc_cycles = 0;
    stat_vca_cycles = 0;
    stat_swa_cycles = 0;
    stat_st_cycles = 0;

    stat_pkts_drop = 0;
    /*
    std::ostringstream buffer;
    buffer << "debug/router_";
    buffer << node_ip;
    string debug_log_string= buffer.str();
    debug_log.open(debug_log_string.c_str());

    // uncomment the next three lines to turn on deadlock detection
    // NOTE: his creates a lot of events and can slow down simulation
    IrisEvent* event = new IrisEvent();
    event->type = DETECT_DEADLOCK_EVENT;
    Simulator::Schedule( floor(Simulator::Now())+1, &NetworkComponent::process_event, this, event);
     * */

    return ;
}		/* -----  end of function RouterVcMPm::init  ----- */

/*! \brief These functions are mainly for DOR routing and are seperated so as to not
 * force DOR modelling in all designs */
void
RouterVcMPm::set_no_nodes( uint nodes )
{
    for ( uint i=0; i<decoders.size(); i++)
    {
        decoders[i].grid_xloc.resize(nodes);
        decoders[i].grid_yloc.resize(nodes);
    }
    for ( uint i=0; i<nop_rc.size(); i++)
    {
    	nop_rc[i].grid_xloc.resize(nodes);
    	nop_rc[i].grid_yloc.resize(nodes);
    }
}

void
RouterVcMPm::set_grid_x_location( uint port, uint x_node, uint value)
{
    decoders[port].grid_xloc[x_node]= value;
    nop_rc[port].grid_xloc[x_node]= value;
}

void
RouterVcMPm::set_grid_y_location( uint port, uint y_node, uint value)
{
    decoders[port].grid_yloc[y_node]= value;
    nop_rc[port].grid_yloc[y_node]= value;
}

/*  End of DOR grid location functions */

void
RouterVcMPm::process_event ( IrisEvent* e )
{
    //for ( uint i=0; i<input_buffer_state.size(); i++)
/*    for ( uint i=0; i<ports*vcs; i++)
    {
    	//if((input_buffer_state[i].pipe_stage == IB)||(input_buffer_state[i].pipe_stage == SW_TRAVERSAL))
    	//if((input_buffer_state[i].pipe_stage != EMPTY)||(input_buffer_state[i].pipe_stage != INVALID))
        	if(input_buffer_state[i].pipe_stage == SW_TRAVERSAL)
        	{
    		_DBG("; input_buffer_state: %d pipe_stage %d addr:%lld dst:%d  arrival_time:%f",
    				i, input_buffer_state[i].pipe_stage, input_buffer_state[i].address
    				, input_buffer_state[i].destination, input_buffer_state[i].arrival_time);
        	}
    	//cerr << input_buffer_state.at(i).toString()<<endl;
    }
*/
    switch(e->type)
    {
        case LINK_ARRIVAL_EVENT:
#ifdef _DEBUG_ROUTER
            _DBG("; RouterVcMPm::LINK_ARRIVAL_EVENT ; time:%f", Simulator::Now());
#endif
            handle_link_arrival_event(e);
            break;
        case TICK_EVENT:
#ifdef _DEBUG_ROUTER
            _DBG("; RouterVcMPm::TICK_EVENT ; time:%f", Simulator::Now());
#endif
            handle_tick_event(e);
            break;
        case DETECT_DEADLOCK_EVENT:
            handle_detect_deadlock_event(e);
            break;
        case FAULT_INJECT_EVENT:
#ifdef _DEBUG_ROUTER
            _DBG("; RouterVcMPm::FAULT_INJECT_EVENT ; time:%f", Simulator::Now());
#endif
            handle_fault_inject_event(e);
            break;
        case FAULT_REMOVE_EVENT:
#ifdef _DEBUG_ROUTER
            _DBG("; RouterVcMPm::FAULT_REMOVE_EVENT ; time:%f", Simulator::Now());
#endif
            handle_fault_remove_event(e);
            break;
        case RESET_STAT_EVENT:
#ifdef _DEBUG_ROUTER
            _DBG("; RouterVcMPm::RESET_STAT_EVENT time:%f", Simulator::Now());
#endif
        	handle_reset_stat_event(e);
            break;

        default:
            _DBG(";RouterVcMPm:: Unk event exception %d", e->type);
            break;
    }
    return ;
}		/* -----  end of function RouterVcMPm::process_event  ----- */

void 
RouterVcMPm::handle_detect_deadlock_event(IrisEvent* e )
{
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
        {
            if( cr_time[i][j] != -1)
            {
                cr_time[i][j]++;
                if( cr_time[i][j] > 50000 )
                {
                    _DBG_NOARG("************ KILLING SIMULATION *************");
                    cout << "\ndeadlock detected op:"<< i << " ovc: " << j <<  endl;
                    cout << "\ndeadlock mssg:"<< endl;
                    cout << "ip\tic\top\toc\taddr:"<< endl;
                    for ( uint ii=0; ii<ports*vcs; ii++)
                        if(input_buffer_state[ii].output_port == i && input_buffer_state[ii].output_channel == j)
                        {
                            cout << dec << input_buffer_state[ii].input_port;
                            cout << dec << "\t" << input_buffer_state[ii].input_channel;
                            cout << dec << "\t" << input_buffer_state[ii].output_port;
                            cout << dec << "\t" << input_buffer_state[ii].output_channel;
                            cout << hex << "\t" << input_buffer_state[ii].address << endl;
                        }
                    cout << "*************************" << endl;

                    //                    print_state_at_deadlock(); //not implemented yet
                    exit(1);
                }
            }
        }

    e->type = DETECT_DEADLOCK_EVENT;
    Simulator::Schedule( floor(Simulator::Now())+50, &NetworkComponent::process_event, this, e);
}

void 
RouterVcMPm::dump_buffer_state()
{
    for ( uint i=0; i<ports*vcs; i++)
        if(input_buffer_state[i].pipe_stage != EMPTY && input_buffer_state[i].pipe_stage != INVALID)
        {
            cout << " Router[" << node_ip << "]->buff["<<i<<"] " << input_buffer_state[i].toString() << endl;
        }
    cout << endl;

    return;
}

double RouterVcMPm::get_average_packet_latency()
{
	return ((stat_total_packet_latency+0.0)/stat_packets);
}

double RouterVcMPm::get_last_flit_out_cycle()
{
	return last_flit_out_cycle;
}

double RouterVcMPm::get_flits_per_packet()
{
	return (stat_flits+0.0)/(stat_packets) ;
}

double RouterVcMPm::get_buffer_occupancy()
{
	return stat_buffer_occupancy;	
}

double RouterVcMPm::get_packets_drop()
{
	return stat_pkts_drop;
}

double RouterVcMPm::get_swa_fail_msg_ratio()
{
	return stat_swa_fail_msg_ratio;	
}

double RouterVcMPm::get_swa_load()
{
	return stat_swa_load;
}

double RouterVcMPm::get_vca_fail_msg_ratio()
{
	return stat_vca_fail_msg_ratio;
}

double RouterVcMPm::get_vca_load()
{
	return stat_vca_load;
}

double RouterVcMPm::get_stat_packets()
{
	return stat_packets;	
}

double RouterVcMPm::get_stat_flits()
{
	return stat_flits;
}

double RouterVcMPm::get_stat_ib_cycles()
{
	return stat_ib_cycles;	
}

double RouterVcMPm::get_stat_rc_cycles()
{
	return stat_rc_cycles;
}

double RouterVcMPm::get_stat_vca_cycles()
{
	return stat_vca_cycles;
}

double RouterVcMPm::get_stat_swa_cycles()
{
	return stat_swa_cycles;
}

double RouterVcMPm::get_stat_st_cycles()
{
	return stat_st_cycles;
}


string
RouterVcMPm::print_stats()
{

    stringstream str;
    str << name << " node_ip = "<< node_ip ;
	for (uint i=1; i<ports; i++)
	{

		if (neighbors[i])
		str << "\n neighbors["<<i<<"]=" << neighbors[i];
	}
    str << endl;

    str << "\n router[" << node_ip << "] packet latency: " << stat_total_packet_latency
        << "\n router[" << node_ip << "] flits/packet: " << (stat_flits+0.0)/(stat_packets)
        << "\n router[" << node_ip << "] average packet latency: " << (stat_total_packet_latency+0.0)/stat_packets
        << "\n router[" << node_ip << "] last_flit_out_cycle: " << last_flit_out_cycle
        << "\n router[" << node_ip << "] stat_buffer_occupancy: " << stat_buffer_occupancy
        << "\n router[" << node_ip << "] stat_swa_fail_msg_ratio: " << stat_swa_fail_msg_ratio
        << "\n router[" << node_ip << "] stat_swa_load: " << stat_swa_load
        << "\n router[" << node_ip << "] stat_vca_fail_msg_ratio: " << stat_vca_fail_msg_ratio
        << "\n router[" << node_ip << "] stat_vca_load: " << stat_vca_load
        << "\n router[" << node_ip << "] packets: " << stat_packets
        << "\n router[" << node_ip << "] flits: " << stat_flits
        << "\n router[" << node_ip << "] stat_ib_cycles: " << stat_ib_cycles
        << "\n router[" << node_ip << "] stat_rc_cycles: " << stat_rc_cycles
        << "\n router[" << node_ip << "] stat_vca_cycles: " << stat_vca_cycles
        << "\n router[" << node_ip << "] stat_swa_cycles: " << stat_swa_cycles
        << "\n router[" << node_ip << "] stat_st_cycles: " << stat_st_cycles
        << " ";

    str << "\n router[" << node_ip << "] out_port_links_utilization: ";
    for( uint i=0; i<ports; i++)
       if( output_connections[i] )
        str << static_cast<GenericLink*>(output_connections[i])->flits_passed*1.0/max_sim_time << " ";
    str << endl;

    if( stat_print_level > 2 )
    {
        for( uint i=0; i<ports; i++)
            for ( uint j=0; j<ports; j++)
            {
                string in_port = "Inv";
                switch( i )
                {
                    case 0 :
                        in_port = "Inj";
                        break;
                    case 1:
                        in_port = 'E';
                        break;
                    case 2:
                        in_port = 'W';
                        break;
                    case 3:
                        in_port = 'S';
                        break;
                    case 4:
                        in_port = 'N';
                        break;
                    default:
                        in_port = "Invalid";
                        break;
                }

                string out_port = "Inv";
                switch( j )
                {
                    case 0 :
                        out_port = "Ejection";
                        break;
                    case 1:
                        out_port = 'W';
                        break;
                    case 2:
                        out_port = 'E';
                        break;
                    case 3:
                        out_port = 'N';
                        break;
                    case 4:
                        out_port = 'S';
                        break;
                    default:
                        out_port = "Invalid";
                        break;
                }
                if ( i != j)
                {
                    str << "\n    router[" << node_ip << "] Packets out " << in_port
                        << " going " << out_port << " : " << stat_packet_out[i][j];
                    str << "\n    router[" << node_ip << "] Flits out " << in_port
                        << " going " << out_port << " : " << stat_flit_out[i][j];
                }

            }
    }


    return str.str();
}

/*! \brief Event handle for the LINK_ARRIVAL_EVENT event. Entry from DES kernel */
void
RouterVcMPm::handle_link_arrival_event ( IrisEvent* e )
{
    LinkArrivalData* data = static_cast<LinkArrivalData*>(e->event_data.at(0));
    if(data->type == FLIT_ID)
    {

        /*Find the port the flit came in on */
        bool found = false;
        uint port = -1;
        /*  Need to check for null here as there may be null links to corner routers */
        for ( uint i=0 ; i< ports ; i++ )
            if(static_cast<GenericLink*>(input_connections[i])->input_connection)
                if( e->src_id == static_cast<GenericLink*>(input_connections[i])->input_connection->address)
                {
                    found = true;
                    port = i;
                    break;
                }

        /* Throw and exception if it was not found */
        if( !found )
        {
            _DBG("; Input port not found src_addr: %d", e->src_id);
        }

        /* Push the flit into the buffer */
        in_buffers[port].change_push_channel(data->vc);
        in_buffers[port].push(data->ptr);

        /* counting ib for body/teail can be complicated as the msg state
         * could have progressed to some other stage. Hence counting it at
         * link arrival makes it easy. */
        if(data->ptr->type !=HEAD)
        {
            istat->stat_router[node_ip]->ib_cycles++;
            stat_ib_cycles++;
        }
#ifdef _DEBUG_PKT
        {
            _DBG("; BW ; inport:%d invc:%d oport:%d ovc:%d ft:%d", port, data->vc,
                 input_buffer_state[port*vcs+data->vc].output_port, input_buffer_state[port*vcs+data->vc].output_channel, data->ptr->type);
        }
        if(data->ptr->type ==HEAD)
        {
        	_DBG("; Router BW; Flit Arrival ; Head; addr %lld; ",static_cast<HeadFlit*>(data->ptr)->addr);
        }
        else if(data->ptr->type ==BODY)
        {
        	_DBG("; Router BW; Flit Arrival ; Body; addr %lld;",static_cast<BodyFlit*>(data->ptr)->addr);
        }
        else if(data->ptr->type ==TAIL)
        {
        	_DBG("; Router BW; Flit Arrival ; Tail; addr %lld;",static_cast<TailFlit*>(data->ptr)->addr);
        }
#endif
        //        _DBG("; handle_link_arrival_event: port %d vc %d ft %d ps:%d", port, data->vc, data->ptr->type, input_buffer_state[port].pipe_stage);
    }
    else if ( data->type == CREDIT_ID)
    {
        /* Find the corresponding output port */
        bool found = false;
        uint port = -1;
        for ( uint i=0 ; ports ; i++ )
            if(static_cast<GenericLink*>(output_connections[i])->output_connection) /* Some links of corner routers may be null */
                if( static_cast<GenericLink*>(output_connections[i])->output_connection->address == e->src_id)
                {
                    port = i;
                    found = true;
                    break;
                }
        if(!found)
        {
            _DBG("; Output port not found src_addr: %d", e->src_id);
        }

        downstream_credits[port][data->vc]++;

        cr_time[port][data->vc] = -1;

#ifdef _DEBUG_ROUTER
        _DBG("; Got a credit ; port %d vc:%d  cr:%d", port, data->vc, downstream_credits[port][data->vc]);
        vector < uint > pending_ops ;
        vector < uint > pending_ips ;
        for( uint i=0; i<ports*vcs; i++)
            if(input_buffer_state[i].output_port == port && input_buffer_state[i].output_channel == data->vc)
            {
                pending_ops.push_back(port*vcs+data->vc);
                pending_ips.push_back(i);
            }

        cout << " Can send msgs (i-o): " ;
        for( uint i=0; i<pending_ops.size(); i++)
            cout << pending_ips[i] <<"-" << pending_ops[i] << "; occ/" << in_buffers[(int)(pending_ops[i]/vcs)].get_occupancy(pending_ops[i]%vcs)
                <<" bst:"<<input_buffer_state[i].pipe_stage << "; ";
        pending_ops.clear();
        pending_ips.clear();
#endif

    }
    else
    {
        _DBG( "handle_link_arrival_event Unk data type %d ", data->type);
    }

    /* Tick since you update a credit or flit */
    if(!ticking)
    {
        ticking = true;
        IrisEvent* event = new IrisEvent();
        event->type = TICK_EVENT;
        event->vc = e->vc;
        Simulator::Schedule( floor(Simulator::Now())+1, &NetworkComponent::process_event, this, event);
    }

    delete data;
    delete e;
    return ;
}		/* -----  end of function RouterVcMPm::handle_link_arrival_event  ----- */

void
RouterVcMPm::handle_reset_stat_event( IrisEvent* e)
{
    /* init the countes */
    stat_packets = 0;
    stat_flits = 0;
    stat_total_packet_latency = 0;
    stat_buffer_occupancy = 0;
    stat_swa_fail_msg_ratio = 0;
    stat_swa_load = 0;
    stat_vca_fail_msg_ratio = 0;
    stat_vca_load = 0;
    stat_ib_cycles = 0;
    stat_rc_cycles = 0;
    stat_vca_cycles = 0;
    stat_swa_cycles = 0;
    stat_st_cycles = 0;
    delete e;
}

void
RouterVcMPm::do_input_buffering()
{
    for( uint i=0; i<ports*vcs; i++)
    {
        uint inport = (uint)(i/vcs);
        uint invc = (uint)(i%vcs);
        /*  */
        if( input_buffer_state[i].pipe_stage == FULL || input_buffer_state[i].pipe_stage == EMPTY )
        {
            if( in_buffers[inport].get_occupancy(invc) > 0)
            {

                in_buffers[inport].change_pull_channel(invc);
                Flit* f = in_buffers[inport].peek();
                //                _DBG("; do_input_buffering %d ft%d",inport*vcs+invc, f->type);
#ifdef _DEBUG_ROUTER 
//                                _DBG("; do_input_buffering %d ft%d",inport*vcs+invc, f->type);
                _DBG("; IB ; do_input_buffering ; address ; %lld ; inport_%d*vcs+invc_%d=%d ft(f->type)=%d",input_buffer_state[i].address, inport,invc,inport*vcs+invc, f->type);
#endif
                if( f->type == HEAD )
                {
                    HeadFlit* hf = static_cast<HeadFlit*>(f);
                    /*  Update stats for head */
                    istat->stat_router[node_ip]->ib_cycles++;
                    stat_ib_cycles++;
                    //_DBG("; HEAD IN %d %lld" ,f->type, hf->addr);
#ifdef _DEBUG_ROUTER_HEAD
                    _DBG("; IB ; HEAD IN ; addr=%lld ; dst=%d", hf->addr, hf->dst_address);
#endif
                    input_buffer_state[inport*vcs+f->vc].input_port = inport;
                    hf->inport = inport;
                    decoders[inport].push(f,f->vc);
                    input_buffer_state[inport*vcs+f->vc].input_channel = f->vc;
                    input_buffer_state[inport*vcs+f->vc].address= hf->addr;
                    input_buffer_state[inport*vcs+f->vc].destination= hf->dst_address;
                    input_buffer_state[inport*vcs+f->vc].source= hf->src_address;
                    input_buffer_state[inport*vcs+f->vc].pipe_stage = IB;
                    input_buffer_state[inport*vcs+f->vc].msg_class = hf->msg_class;
                    input_buffer_state[inport*vcs+f->vc].possible_oports.clear();
                    input_buffer_state[inport*vcs+f->vc].possible_ovcs.clear();
                    uint no_adaptive_ports = decoders[inport].no_adaptive_ports(f->vc);
                    uint no_adaptive_vcs = decoders[inport].no_adaptive_vcs(f->vc);
                    istat->stat_router[node_ip]->rc_cycles++;
                    stat_rc_cycles++;
                    for ( uint i=0; i<no_adaptive_ports; i++ )
                    {
                        uint rc_port = decoders[inport].get_output_port(f->vc); // cope with last_adaptive_port in GenericRC

                        if( static_cast<GenericLink*>(output_connections[rc_port])->output_connection != NULL )
                        {
                            input_buffer_state[inport*vcs+f->vc].possible_oports.push_back(rc_port);
                            input_buffer_state[inport*vcs+f->vc].is_misrouting.push_back(0);
                        }
                    }

                    for ( uint i=0; i<no_adaptive_vcs; i++ )
                    {
                        uint rc_vc = decoders[inport].get_virtual_channel(f->vc);
                        input_buffer_state[inport*vcs+f->vc].possible_ovcs.push_back(rc_vc);
                    }

                    assert ( input_buffer_state[inport*vcs+f->vc].possible_oports.size() > 0);
                    assert ( input_buffer_state[inport*vcs+f->vc].possible_ovcs.size() > 0);

                    input_buffer_state[inport*vcs+f->vc].length= hf->length;
                    input_buffer_state[inport*vcs+f->vc].credits_sent= hf->length;
                    input_buffer_state[inport*vcs+f->vc].arrival_time= Simulator::Now();
                    input_buffer_state[inport*vcs+f->vc].clear_message= false;
                }
                /* Dont need this only heads go thru RC
                   else
                   {
                // Body flits are pushed thru the decoder but
                // they dont do anything. Tail flits clear the state
                decoders[inport].push(f,f->vc);
                input_buffer_state[inport*vcs+f->vc].flits_in_ib++;
                cout  << "came here " << endl;

                }
                 * */
                ticking =true;
            }
        }
    }

    /* Body and tail flits get written in link arrival and since the message
     * state may already been pushed to ST because of the header we want to
     * ensure that all flits go thru an IB and ST stage. Hence ST is done on
     * the flits_in_ib information and not buffer occupancy.
     for( uint i=0; i<(ports*vcs); i++)
     {
     uint ip = input_buffer_state[i].input_port;
     uint ic = input_buffer_state[i].input_channel;

     if (input_buffer_state[i].pipe_stage == VCA_COMPLETE || input_buffer_state[i].pipe_stage == SWA_REQUESTED 
     || input_buffer_state[i].pipe_stage == SW_TRAVERSAL)
     {
     in_buffers[ip].change_pull_channel(ic);
     Flit* f = in_buffers[ip].peek();
     if( f->type != HEAD && (input_buffer_state[i].flits_in_ib < in_buffers[ip].get_occupancy(ic)))
     {
     input_buffer_state[i].flits_in_ib++;
     ticking=true;
     }
     }

     }
     * */
}

void
RouterVcMPm::do_switch_traversal()
{
#ifdef _DEBUG_ROUTER
	for( uint i=0; i<ports; i++)
	{
		for( uint j=0; j<vcs; j++)
		cout << "\n in_buffers["<<i<<"].get_occupancy("<<j<<")="<< in_buffers[i].get_occupancy(j);
	}
	cout <<endl;
	for( uint i=0; i<ports; i++)
	{
		for( uint j=0; j<vcs; j++)
		cout << "\n downstream_credits["<<i<<"]["<<j<<"]="<< downstream_credits[i][j];
	}
	cout <<endl;
#endif
	for( uint i=0; i<(ports*vcs); i++)
    {
        if( input_buffer_state[i].pipe_stage == SW_TRAVERSAL)
        {
#ifdef _DEBUG_ROUTER
            _DBG("; ST ; input_buffer_state[%d].pipe_stage =%d",i,input_buffer_state[i].pipe_stage);
#endif
            uint op = input_buffer_state[i].output_port;
            uint oc = input_buffer_state[i].output_channel;
            uint ip = input_buffer_state[i].input_port;
            uint ic= input_buffer_state[i].input_channel;
#ifdef _DEBUG_ROUTER
            _DBG("; ST ; downstream_credits[op_%d][oc_%d]=%d",op,oc,downstream_credits[op][oc]);
            _DBG("; ST ; in_buffers[ip_%d].get_occupancy(ic_%d)=%d",ip,ic,in_buffers[ip].get_occupancy(ic));
#endif
            if( in_buffers[ip].get_occupancy(ic)>0
                && downstream_credits[op][oc]>0 )
            {
                in_buffers[ip].change_pull_channel(ic);
                Flit* f = in_buffers[ip].pull();
                f->vc = oc;

                if(f->type != TAIL && !f->is_single_flit_pkt) // if f is not end of pkt, do SWA REQ for rest of pkt
                {
                    input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                    request_switch_allocation();
                    ticking = true;
                }

                /* stats */
                last_flit_out_cycle = Simulator::Now();
                stat_flit_out[ip][op]++;
                stat_flits++;
                static_cast<GenericLink*>(output_connections[op])->flits_passed++;
                istat->stat_link[static_cast<GenericLink*>(output_connections[op])->link_id]->flits_transferred++;

                /* For energy computation: the per access energy reported is
                 * for one flit traversing the crossbar. Unlike arbiter access
                 * energy which is for multiple digraph matching in one
                 * access. This count may also be used as the local link
                 * access count if the ST and LT are considered as the same
                 * stage for very short wire lengths. */
                istat->stat_router[node_ip]->st_cycles++;
                stat_st_cycles++;


                LinkArrivalData* data = new LinkArrivalData();
                data->type = FLIT_ID;
                data->vc = oc;
                data->ptr = f;
#ifdef _DEBUG_PKT
                _DBG("; ST ; COMP ; Fout ; (ip_%d*vcs_%d+ic_%d)%d=(op_%d*vcs_%d+oc_%d)%d ft=%d",ip,vcs,ic,ip*vcs+ic,op,vcs,oc,op*vcs+oc, f->type);
        if(data->ptr->type ==HEAD)
        {
        	_DBG("; Router ST; Flit Out ; addr %lld; Head;",static_cast<HeadFlit*>(data->ptr)->addr);
        }
        else if(data->ptr->type ==BODY)
        {
        	_DBG("; Router ST; Flit Out ; addr %lld; Body;",static_cast<BodyFlit*>(data->ptr)->addr);
        }
        else if(data->ptr->type ==TAIL)
        {
        	_DBG("; Router ST; Flit Out ; addr %lld; Tail;",static_cast<TailFlit*>(data->ptr)->addr);
        }
#endif

                /*  Updating all pkt stats in the head irrespective of msg
                 *  class. Assuming all pkts do have a head. However latency
                 *  for determining switch throughput is to be determined when
                 *  the tail exits.
                 *  */
                if( f->type == HEAD)
                {
                    HeadFlit* hf = static_cast<HeadFlit*>(f);
                    hf->hop_count++;
                    //                    cout << "  " << hf->addr;
                }
                if( f->type == TAIL || f->is_single_flit_pkt )
                {
                    /* Update packet level stats */
                    uint lat = Simulator::Now() - input_buffer_state[i].arrival_time + 1;
                    stat_total_packet_latency += lat;
                    stat_packets++;

                    if( f->type == HEAD)
                    {
                        HeadFlit* hf = static_cast<HeadFlit*>(f);
                        hf->avg_network_latency += lat;
                    }
                    else
                    {
                        TailFlit* tf = static_cast<TailFlit*>(f);
                        tf->avg_network_latency += lat;
                    }

                    stat_packet_out[ip][op]++;

                    input_buffer_state[i].clear_message = true;
                    if(in_buffers[ip].get_occupancy(ic) > 0)
                        input_buffer_state[i].pipe_stage = FULL;
                    else
                        input_buffer_state[i].pipe_stage = EMPTY;
                    vc_alloc[op].push_back(oc);
                    input_buffer_state[i].possible_oports.clear();
                    input_buffer_state[i].possible_ovcs.clear();
                }

                cr_time[op][oc] = 0;
                IrisEvent* event = new IrisEvent();
                event->type = LINK_ARRIVAL_EVENT;
                event->event_data.push_back(data);
                event->src_id = address;
                event->dst_id = output_connections[op]->address;
                event->vc = data->vc;

                Simulator::Schedule( Simulator::Now()+0.75,
                                     &NetworkComponent::process_event,
                                     static_cast<GenericLink*>(output_connections[op])->output_connection,event);
                /*! \brief Send a credit back and update buffer state for the
                 * downstream router buffer */
                send_credit_back(i); // send upstream router that one more credit in input_buffer
                downstream_credits[op][oc]--; // decrease one credit since downstream router is received one flit
//            	_DBG("; Decrease Credits ; port %d vc:%d  cr:%d", op, oc, downstream_credits[op][oc]);

            }
            else
            {
#ifdef _DEBUG_ROUTER
                _DBG("; ST ; FAIL ; downstream_credits[%d][%d]=%d",op,ic,downstream_credits[op][oc]);
#endif

                input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                ticking = true;
            }
        }
        else
        {
#ifdef _DEBUG_ROUTER
        if (input_buffer_state[i].pipe_stage!=EMPTY)
        	{
//        	_DBG("; WAITING ; input_buffer_state[%d].pipe_stage =%d",i,input_buffer_state[i].pipe_stage);

        	}
#endif
        }
    }
    return;

}

void
RouterVcMPm::do_switch_allocation()
{
    bool was_swa_active = false; /* flag to make sure swa accesses are not double counted */
    uint tot_swa_req = 0;
    uint swa_won=0;
    for( uint i=0; i<ports; i++)
    {
        if(  sw_alloc[i].size() )
        {
            uint winner_op = 0; /* take the head of the queue as the winner */

            /* Since only one msg can win. counting failure cycles here as a
             * measure of congestion. */
            swa_won++;
            tot_swa_req += (sw_alloc[i].size()+1);

            uint msg_id = sw_alloc[i][winner_op];
            uint ip = input_buffer_state[msg_id].input_port;
            uint ic = input_buffer_state[msg_id].input_channel;
            uint op = input_buffer_state[msg_id].output_port;
            uint oc = input_buffer_state[msg_id].output_channel;
            assert(op == i);
            vector<uint>::iterator it = find (sw_alloc[i].begin(), sw_alloc[i].end(), msg_id);;
            if( input_buffer_state[msg_id].pipe_stage == SWA_REQUESTED && downstream_credits[op][oc] )
            {
                sw_alloc[i].erase(it);
                input_buffer_state[msg_id].pipe_stage = SW_TRAVERSAL;
                //_DBG("; SW_TRAV %d=%d cr:%d occ:%d",ip*vcs+ic,op*vcs+oc,downstream_credits[op][oc], in_buffers[ip].get_occupancy(ic));
#ifdef _DEBUG_ROUTER
       _DBG("; SWA ; COMP ; %d=%d (ip_%d*vcs_%d+ic_%d,op_%d*vcs_%d+oc_%d) cr:%d occ:%d",ip*vcs+ic,op*vcs+oc,ip,vcs,ic,op,vcs,oc,downstream_credits[op][oc], in_buffers[ip].get_occupancy(ic));
#endif
                	 ticking = true;
                was_swa_active = true;
//#ifdef _DEBUG_ROUTER
//            _DBG("; SWA ; COMP ; input_buffer_state[%d].pipe_stage =%d",msg_id,input_buffer_state[msg_id].pipe_stage);
//#endif
            }
            else
            {
#ifdef _DEBUG_ROUTER
            _DBG("; SWA ; FAIL ; downstream_credits[op_%d][oc_%d]=%d",op,oc,downstream_credits[op][oc]);
//            _DBG("; SWA ; FAIL ; input_buffer_state[%d].pipe_stage =%d",msg_id,input_buffer_state[msg_id].pipe_stage);
#endif
//       	 ticking = true;
//       was_swa_active = true;
            }
        }
    }
    if( tot_swa_req )
    {
        stat_swa_fail_msg_ratio += ((tot_swa_req - swa_won+0.0)/tot_swa_req);
        stat_swa_fail_msg_ratio = stat_swa_fail_msg_ratio/2;
        stat_swa_load += ((tot_swa_req+0.0)/(ports*vcs));
        stat_swa_load = stat_swa_load/2;
    }

    if( was_swa_active )
    {
        istat->stat_router[node_ip]->sa_cycles++;
        stat_swa_cycles++;
    }

    return;
}

void
RouterVcMPm::do_switch_allocation_m() //Yi: m=modified 
{
    bool was_swa_active = false; /* flag to make sure swa accesses are not double counted */

    uint tot_swa_req = 0;
    uint swa_won=0;

    uint ip;
    uint ic;
    uint op;
    uint oc;
    bool swa_active = false;
    for( uint i=0; i<ports; i++)
    {


    	if(  sw_alloc[i].size() )
        {
            uint winner_op = 0; /* take the head of the queue as the winner */
            /* Since only one msg can win. counting failure cycles here as a
             * measure of congestion. */
            swa_won++;
            tot_swa_req += (sw_alloc[i].size()+1);

            for (winner_op;winner_op<sw_alloc[i].size();winner_op++)
            {
            uint msg_id = sw_alloc[i][winner_op];
//            uint msg_id = static_cast<uint>(*itr);
            ip = input_buffer_state[msg_id].input_port;
            ic = input_buffer_state[msg_id].input_channel;
            op = input_buffer_state[msg_id].output_port;
            oc = input_buffer_state[msg_id].output_channel;
            assert(op == i);
            vector<uint>::iterator it = find (sw_alloc[i].begin(), sw_alloc[i].end(), msg_id);;
            if( swa_active==false && input_buffer_state[msg_id].pipe_stage == SWA_REQUESTED && downstream_credits[op][oc]>0 )
            {
                sw_alloc[i].erase(it);
                input_buffer_state[msg_id].pipe_stage = SW_TRAVERSAL;
                //_DBG("; SW_TRAV %d=%d cr:%d occ:%d",ip*vcs+ic,op*vcs+oc,downstream_credits[op][oc], in_buffers[ip].get_occupancy(ic));
#ifdef _DEBUG_ROUTER
       _DBG("; SWA ; COMP ; %d=%d (ip_%d*vcs_%d+ic_%d,op_%d*vcs_%d+oc_%d) cr:%d occ:%d",ip*vcs+ic,op*vcs+oc,ip,vcs,ic,op,vcs,oc,downstream_credits[op][oc], in_buffers[ip].get_occupancy(ic));
#endif
                ticking = true;
                was_swa_active = true;
                swa_active = true;
            }
            }

#ifdef _DEBUG_ROUTER
            if (swa_active == false)
            {

            _DBG("; SWA ; FAIL ; downstream_credits[op_%d][oc_%d]=%d",op,oc,downstream_credits[op][oc]);
//            _DBG("; SWA ; FAIL ; input_buffer_state[%d].pipe_stage =%d",msg_id,input_buffer_state[msg_id].pipe_stage);
            }
#endif
        ticking=true;
        }
    }
    if( tot_swa_req )
    {
        stat_swa_fail_msg_ratio += ((tot_swa_req - swa_won+0.0)/tot_swa_req);
        stat_swa_fail_msg_ratio = stat_swa_fail_msg_ratio/2;
        stat_swa_load += ((tot_swa_req+0.0)/(ports*vcs));
        stat_swa_load = stat_swa_load/2;
    }

    if( was_swa_active )
    {
        istat->stat_router[node_ip]->sa_cycles++;
        stat_swa_cycles++;
    }

    return;
}

void
RouterVcMPm::do_st()
{
    for ( uint i=0; i<input_buffer_state.size(); ++i)
        if( input_buffer_state.at(i).pipe_stage==SW_TRAVERSAL )
        {
        	InputBufferState& curr_pkt = input_buffer_state.at(i);
            uint op = curr_pkt.output_port;
            uint oc = curr_pkt.output_channel;
            uint ip = curr_pkt.input_port;
            uint ic = curr_pkt.input_channel;
            bool found_fault = false;
            //printf("%d@%lu: credits%d  \n",node_id, _TICK_NOW, downstream_credits.at(op).at(oc));
            if ( downstream_credits.at(op).at(oc) )//&& (in_buffers[ip].get_occupancy(ic) > 0))
            {
                in_buffers[ip].change_pull_channel(ic);
                Flit* f = in_buffers[ip].pull();
                f->vc = oc;

                for (uint u=0;u<fault_ports.size();u++)
                {
                	if(fault_ports[u]==op) found_fault = true;
                }

                if (!found_fault) // no output fault found
                {
                if(f->type != TAIL && !f->is_single_flit_pkt) // if f is not end of pkt, do SWA REQ for rest of pkt
                {
                    input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                    request_switch_allocation();
                    ticking = true;
                }  // todo : adapt to sst iris do_st()


                /* stats */
                last_flit_out_cycle = Simulator::Now();
                stat_flit_out[ip][op]++;
                stat_flits++;
                static_cast<GenericLink*>(output_connections[op])->flits_passed++;
                istat->stat_link[static_cast<GenericLink*>(output_connections[op])->link_id]->flits_transferred++;

                /* For energy computation: the per access energy reported is
                 * for one flit traversing the crossbar. Unlike arbiter access
                 * energy which is for multiple digraph matching in one
                 * access. This count may also be used as the local link
                 * access count if the ST and LT are considered as the same
                 * stage for very short wire lengths. */
                istat->stat_router[node_ip]->st_cycles++;
                stat_st_cycles++;

                LinkArrivalData* data = new LinkArrivalData();
                data->type = FLIT_ID;
                data->vc = oc;
                data->ptr = f;
#ifdef _DEBUG_PKT
                _DBG("; ST ; COMP ; Fout ; (ip_%d*vcs_%d+ic_%d)%d=(op_%d*vcs_%d+oc_%d)%d ft=%d",ip,vcs,ic,ip*vcs+ic,op,vcs,oc,op*vcs+oc, f->type);
        if(data->ptr->type ==HEAD)
        {
        	_DBG("; Router ST; Flit Out ; addr %lld; Head;",static_cast<HeadFlit*>(data->ptr)->addr);
        	_DBG("; Router ST; Flit Out ; addr %lld; InputBufferState;",curr_pkt.address);
//        	if((static_cast<HeadFlit*>(data->ptr)->addr) != curr_pkt.address)
//        		cerr << "Warning !"<<" IBS: "<<curr_pkt.address<<" PKT: "<< static_cast<HeadFlit*>(data->ptr)->addr<<endl;
        }
        else if(data->ptr->type ==BODY)
        {
        	_DBG("; Router ST; Flit Out ; addr %lld; Body;",static_cast<BodyFlit*>(data->ptr)->addr);
        	_DBG("; Router ST; Flit Out ; addr %lld; InputBufferState;",curr_pkt.address);
//        	if((static_cast<HeadFlit*>(data->ptr)->addr) != curr_pkt.address)
//        		cerr << "Warning !"<<" IBS: "<<curr_pkt.address<<" PKT: "<< static_cast<BodyFlit*>(data->ptr)->addr<<endl;
        }
        else if(data->ptr->type ==TAIL)
        {
        	_DBG("; Router ST; Flit Out ; addr %lld; Tail;",static_cast<TailFlit*>(data->ptr)->addr);
        	_DBG("; Router ST; Flit Out ; addr %lld; InputBufferState;",curr_pkt.address);
//        	if((static_cast<HeadFlit*>(data->ptr)->addr) != curr_pkt.address)
//        		cerr << "Warning !"<<" IBS: "<<curr_pkt.address<<" PKT: "<< static_cast<TailFlit*>(data->ptr)->addr<<endl;
        }
#endif

                /*  Updating all pkt stats in the head irrespective of msg
                 *  class. Assuming all pkts do have a head. However latency
                 *  for determining switch throughput is to be determined when
                 *  the tail exits.
                 *  */
                if( f->type == HEAD)
                {
                    HeadFlit* hf = static_cast<HeadFlit*>(f);
                    hf->hop_count++;
                    //                    cout << "  " << hf->addr;
                }
                if( f->type == TAIL || f->is_single_flit_pkt )
                {
                    /* Update packet level stats */
                    uint lat = Simulator::Now() - input_buffer_state[i].arrival_time + 1;
                    stat_total_packet_latency += lat;
                    stat_packets++;

                    if( f->type == HEAD)
                    {
                        HeadFlit* hf = static_cast<HeadFlit*>(f);
                        hf->avg_network_latency += lat;
                    }
                    else
                    {
                        TailFlit* tf = static_cast<TailFlit*>(f);
                        tf->avg_network_latency += lat;
                    }

                    stat_packet_out[ip][op]++;

                    input_buffer_state[i].clear_message = true;
                    if(in_buffers[ip].get_occupancy(ic) > 0)
                        input_buffer_state[i].pipe_stage = FULL;
                    else
                        input_buffer_state[i].pipe_stage = EMPTY;
                    vc_alloc[op].push_back(oc);
#ifdef _DEBUG_VCA
						 _DBG("; ST VC release %lld ip_ic_%d_%d, op_oc_%d_%d",
								 input_buffer_state[i].address ,ip,ic,op,oc);
#endif
                    vca.clear_winner(op,oc,ip,ic);
                    swa.clear_requestor(op, ip, oc);
                    input_buffer_state[i].possible_oports.clear();
                    input_buffer_state[i].possible_ovcs.clear();
                    input_buffer_state[i].address=-1;
                    input_buffer_state[i].destination=-1;

                }

                cr_time[op][oc] = 0;
                IrisEvent* event = new IrisEvent();
                event->type = LINK_ARRIVAL_EVENT;
                event->event_data.push_back(data);
                event->src_id = address;
                event->dst_id = output_connections[op]->address;
                event->vc = data->vc;

                Simulator::Schedule( Simulator::Now()+0.75,
                                     &NetworkComponent::process_event,
                                     static_cast<GenericLink*>(output_connections[op])->output_connection,event);
                /*! \brief Send a credit back and update buffer state for the
                 * downstream router buffer */
                send_credit_back(i); // send upstream router that one more credit in input_buffer
                downstream_credits[op][oc]--; // decrease one credit since downstream router is received one flit
                remain_flits[op]--;
                input_buffer_state[i].remain_flits --;
#ifdef _DEBUG_METRIC0
        	_DBG(";remain_flits ; remain_flits[%d] Remain Flits = %d --",op,remain_flits[op]);
#endif
//            	_DBG("; Decrease Credits ; port %d vc:%d  cr:%d", op, oc, downstream_credits[op][oc]);


                }
                    else if(found_fault)// found output fault
                    {
                        if(f->type ==HEAD )// || f->is_single_flit_pkt )
                        {
                        	stat_pkts_drop++;
                        	_DBG("; Fault found during ST Drop Flit Head of %lld at port:%d",static_cast<HeadFlit*>(f)->addr, op);
                        }
                        if(f->type ==TAIL || f->is_single_flit_pkt ) // no drop !!!!
                        {
                        	input_buffer_state[i].clear_message = true;
                        	if(in_buffers[ip].get_occupancy(ic) > 0)
                        		input_buffer_state[i].pipe_stage = FULL;
                        	else
                        		input_buffer_state[i].pipe_stage = EMPTY;
                            vc_alloc[op].push_back(oc);
                            vca.clear_winner(op,oc,ip,ic);
                            swa.clear_requestor(op, ip, oc);
            	    	        input_buffer_state[i].possible_oports.clear();
            	    	        input_buffer_state[i].possible_ovcs.clear();
                            _DBG("; Fault found during ST Drop Flit Tail of %lld at port:%d",static_cast<TailFlit*>(f)->addr, op);

                        }
                        if(f->type ==BODY )
                        {
                        	_DBG("; Fault found during ST Drop Flit Body of %lld at port:%d",static_cast<BodyFlit*>(f)->addr, op);
                        }
                        cr_time[op][oc] = 0;
                        send_credit_back(i); // send upstream router that one more credit in input_buffer
                        remain_flits[op]--;
                    }


            	/*
                Flit* f = in_buffers.at(ip).pull(ic);
                f->vc = oc;

                irisEvent* ev = new irisEvent;
                f->vc = oc;
                ev->type = irisEvent::Packet;
                ev->packet = *f;
                //printf("%d: Send pkt at%lu", node_id, _TICK_NOW);
                links.at(op)->Send(ev);

                //  I have one slot in the next buffer less now.
                downstream_credits.at(op).at(oc)--;

                irisEvent* cr_ev = new irisEvent; //(CREDIT, ic);
                cr_ev->type = irisEvent::Credit;
                cr_ev->credit.vc = ic;
                cr_ev->credit.num = 1;

                // make sure to send back 8 credits to the interface because it
                // assumes it sent out 8 flits
                if ( ip == 0 ) cr_ev->credit.num  = 8;
                links.at(ip)->Send(cr_ev);

                //* Update packet stats
                stat_packets_out++;
                stat_flits_out++;
                stat_last_flit_cycle = _TICK_NOW;
                uint lat = _TICK_NOW - curr_pkt.pkt_arrival_time;
                stat_total_pkt_latency_nano += lat;
                //printf("%d: pkt out @%lu\n",node_id, _TICK_NOW);

                curr_pkt.pipe_stage = EMPTY;
                curr_pkt.input_port = -1;
                curr_pkt.input_channel = -1;
                curr_pkt.output_port = -1;
                curr_pkt.output_channel = -1;
                vca.clear_winner(op, oc, ip, ic);
                swa.clear_requestor(op, ip,oc);
                curr_pkt.oport_list.clear();
                curr_pkt.ovc_list.clear();
                */
            }
        }

    return ;

}
void
RouterVcMPm::do_swa()
{
    /* Switch Allocation */
	bool was_swa_active = false;
	uint swa_won = 0;
	uint tot_swa_req = 0;
    for( uint i=0; i<ports*vcs; ++i)
    {
        if( input_buffer_state.at(i).pipe_stage == SWA_REQUESTED)
            if ( !swa.is_empty())
            {
            	swa_won ++ ;
            	tot_swa_req += swa.nb_request();
            	InputBufferState& sa_msg = input_buffer_state.at(i);
                uint op = -1, oc = -1;
                SA_unit sa_winner;
                uint ip = sa_msg.input_port;
                uint ic = sa_msg.input_channel;
                op = sa_msg.output_port;
                oc= sa_msg.output_channel;
                sa_winner = swa.pick_winner(op);

                if( sa_winner.port == ip && sa_winner.ch == ic
                    && in_buffers.at(ip).get_occupancy(ic) > 0
                    && downstream_credits.at(op).at(oc)>0 )
                {
                    //                    printf(" swa won %d-%d|%d-%d \n",ip,ic,op,oc);
                    sa_msg.pipe_stage = SW_TRAVERSAL;
                    was_swa_active = true;

                    ticking = true;
                }
                else
                {
                    sa_msg.pipe_stage = VCA_COMPLETE;
                    swa.clear_requestor(op, ip, oc);
                    //ticking = true;
                }

            }
    }
        if( tot_swa_req )
        {
            stat_swa_fail_msg_ratio += ((tot_swa_req - swa_won+0.0)/tot_swa_req);
            stat_swa_fail_msg_ratio = stat_swa_fail_msg_ratio/2;
            stat_swa_load += ((tot_swa_req+0.0)/(ports*vcs));
            stat_swa_load = stat_swa_load/2;
        }

        if( was_swa_active )
        {
            istat->stat_router[node_ip]->sa_cycles++;
            stat_swa_cycles++;
        }

    return ;

}

void
RouterVcMPm::do_vca()
{
    /* flag to count accesses. In effect multiple w
     * inners can be chosen for one access of the arbiter */
    bool was_vca_active = false;
    uint tot_vca_req = 0;
    uint vca_won=0;

    if ( !vca.is_empty() )
    {
        vca.pick_winner();
        // update vca access stats
    }
    else
    {
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; vca.is_empty() =%d",vca.is_empty());
#endif
    }

    for( uint i=0; i< ports; ++i)
        for ( uint ai=0; ai<vca.current_winners.at(i).size(); ++ai)
        {
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; vca.current_winners.at(%d).size() =%d",i,vca.current_winners.at(i).size());
#endif
            Vca_unit winner = vca.current_winners.at(i).at(ai);
            uint ip = winner.in_port;
            uint ic = winner.in_vc;
            uint op = winner.out_port;
            uint oc = winner.out_vc;
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; ip=%d;ic=%d;op=%d;oc=%d",ip,ic,op,oc);
#endif
            InputBufferState& curr_msg= input_buffer_state.at(ip* vcs+ic);
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; curr_msg.pipe_stage=%d",curr_msg.pipe_stage);
#endif

            if ( curr_msg.pipe_stage == VCA_REQUESTED )
                // &&                  downstream_credits[op)[oc)==credits)
            {

            	tot_vca_req++;
                curr_msg.output_port = op;
                curr_msg.output_channel= oc;
                curr_msg.pipe_stage = VCA_COMPLETE;
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; COMP ; (ip_%d*vcs_%d+ic_%d=op_%d*vcs_%d+oc_%d)%d=%d",ip,vcs,ic,op,vcs,oc,ip*vcs+ic,op*vcs+oc);
#endif
           	    ticking = true;
                was_vca_active = true;
                vca_won++;
#ifdef _DEBUG_VCA
						 _DBG("; VCA COMP %lld ip_ic_%d_%d, op_oc_%d_%d",
								 curr_msg.address,ip,ic,op,oc );
#endif
                // if requesting multiple out ports make sure to cancel them as
                // pkt will no longer be VCA_REQUESTED
                if(in_buffers.at(ip).get_occupancy(ic))
                {
#ifdef _DEBUG_ROUTER
            	 _DBG("; SWA ; REQ ; %d-%d|%d-%d",ip,ic,op,oc);
#endif                    //                printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);
                    swa.request(op, oc, ip, ic);
                    curr_msg.pipe_stage = SWA_REQUESTED;
                    ticking = true;
                }
            }

        }

    // If swa request failed and your stuck in vca you can request here
    for( uint i=0; i<( ports* vcs); ++i)
        if( input_buffer_state.at(i).pipe_stage == VCA_COMPLETE)
        {
        	InputBufferState& curr_msg = input_buffer_state.at(i);
            uint ip = curr_msg.input_port;
            uint ic = curr_msg.input_channel;
            uint op = curr_msg.output_port;
            uint oc = curr_msg.output_channel;
#ifdef _DEBUG_ROUTER
            	 _DBG("; SWA ; REQ ; %d-%d|%d-%d",ip,ic,op,oc);
#endif            //printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);

            if(in_buffers.at(ip).get_occupancy(ic))
            {
                swa.request(op, oc, ip, ic);
                curr_msg.pipe_stage = SWA_REQUESTED;
                ticking = true;
            }
        }

    if( tot_vca_req )
    {
        stat_vca_fail_msg_ratio += ((tot_vca_req - vca_won+0.0)/tot_vca_req);
        stat_vca_fail_msg_ratio = stat_vca_fail_msg_ratio/2;
        stat_vca_load += ((tot_vca_req+0.0)/(ports*vcs));
        stat_vca_load = stat_vca_load/2;
    }

    if( was_vca_active )
    {
        istat->stat_router[node_ip]->vca_cycles++;
        stat_vca_cycles++;
    }
}

void
RouterVcMPm::do_virtual_channel_allocation()
{
/* flag to count accesses. In effect multiple w
 * inners can be chosen for one access of the arbiter */
bool was_vca_active = false;
uint tot_vca_req = 0;
uint vca_won=0;

/* pool all the requestors based on outport */
for( uint i=0; i<(ports*vcs); i++)
    if( input_buffer_state[i].pipe_stage == VCA_REQUESTED)
    {
        uint op=input_buffer_state[i].output_port;
        //request_op[op].push_back(i); // request_op[i] keeps all the outport request from channels[j](j=port*vcs+ic) to the port i
        tot_vca_req++;
    }

for( uint i=0; i<ports; i++)
{
    bool vca_active = false;
	ullint min_t = -1;
    vector<uint> sorted_msgs;
//        uint br_test=request_op[i].size();
//        while(br_test!=0)
    while( request_op[i].size())
{
        uint msg_index = -1;
        for( uint j=0; j<request_op[i].size(); j++)
            if( input_buffer_state[request_op[i][j]].arrival_time < min_t ) /* FCFS based alloc */
                {
            	msg_index = request_op[i][j];
                }
        if(msg_index != -1)
            sorted_msgs.push_back(msg_index);
        vector<uint>::iterator it = find (request_op[i].begin(), request_op[i].end(), msg_index);;
        request_op[i].erase(it);
    }

    for( uint j=0; j<sorted_msgs.size(); j++)
    {
        uint msg_index = sorted_msgs[j];
        uint op=input_buffer_state[msg_index].output_port;
        uint ip = input_buffer_state[msg_index].input_port;
        uint ic = input_buffer_state[msg_index].input_channel;
        assert( op == i);
        if( vc_alloc[op].size() )
        {
            //uint oc  = vc_alloc[op].front();

        	uint oc=-1;
            /* can request only if there are credits improving
             * chances of winning for other pkts that can actually
             * exit the router */
            for (list<uint>::iterator itr=vc_alloc[op].begin();itr!=vc_alloc[op].end();++itr)
            //	for (vector<uint>::iterator itr=vc_alloc[op].begin();itr!=vc_alloc[op].end();++itr)
            {
            	oc = *itr;
            	if ( (downstream_credits[op][oc] > 0) && was_vca_active == false )
              {
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; COMP ; (ip_%d*vcs_%d+ic_%d=op_%d*vcs_%d+oc_%d)%d=%d",ip,vcs,ic,op,vcs,oc,ip*vcs+ic,op*vcs+oc);
#endif
        	// _DBG("; VCA COMP %d=%d",ip*vcs+ic,op*vcs+oc);
#ifdef _DEBUG_VCA
						 _DBG("; VCA COMP %lld ip_ic_%d_%d, op_oc_%d_%d",
								 input_buffer_state[msg_index].address,ip,ic,op,oc );
#endif
            	 //cout << endl << "oc = " << oc << endl;
            	 //cout << endl << "downstream_credits[op][oc] = " << downstream_credits[op][oc] << endl;

            	 input_buffer_state[msg_index].output_channel = oc;
                //input_buffer_state[msg_index].output_channel = vc_alloc[op][winner_oc];
                //vc_alloc[op].pop_front();
                //vc_alloc[op].erase(itr);

                input_buffer_state[msg_index].pipe_stage = VCA_COMPLETE;
           	    ticking = true;
                was_vca_active = true;
                vca_active = true;
                vca_won++;
              }
            }
            if (vca_active == true )
             {
            	vc_alloc[op].remove(input_buffer_state[msg_index].output_channel);
             }

#ifdef _DEBUG_ROUTER
            if (vca_active == false )
            {
            	cout << endl <<" op="<<op<<" vca_active == false "<< endl;
            }
#endif
            }
    }
}

if( tot_vca_req )
{
    stat_vca_fail_msg_ratio += ((tot_vca_req - vca_won+0.0)/tot_vca_req);
    stat_vca_fail_msg_ratio = stat_vca_fail_msg_ratio/2;
    stat_vca_load += ((tot_vca_req+0.0)/(ports*vcs));
    stat_vca_load = stat_vca_load/2;
}

if( was_vca_active )
{
    istat->stat_router[node_ip]->vca_cycles++;
    stat_vca_cycles++;
}

//request_switch_allocation();
// If swa request failed and your stuck in vca you can request here
for( uint i=0; i<( ports* vcs); ++i)
    if( input_buffer_state.at(i).pipe_stage == VCA_COMPLETE)
    {
    	InputBufferState& curr_msg = input_buffer_state.at(i);
        uint ip = curr_msg.input_port;
        uint ic = curr_msg.input_channel;
        uint op = curr_msg.output_port;
        uint oc = curr_msg.output_channel;
#ifdef _DEBUG_ROUTER
        	 _DBG("; SWA ; REQ ; %d-%d|%d-%d",ip,ic,op,oc);
#endif            //printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);

        if(in_buffers.at(ip).get_occupancy(ic))
        {
            swa.request(op, oc, ip, ic);
            curr_msg.pipe_stage = SWA_REQUESTED;
            ticking = true;
        }
    }
}
void
RouterVcMPm::do_virtual_channel_allocation_m()
{

	/* flag to count accesses. In effect multiple w
	 * inners can be chosen for one access of the arbiter */
	bool was_vca_active = false;
	uint tot_vca_req = 0;
	uint vca_won=0;

	/* pool all the requestors based on outport */
	for( uint i=0; i<(ports*vcs); i++)
	    if( input_buffer_state[i].pipe_stage == VCA_REQUESTED)
	    {
	        //uint op=input_buffer_state[i].output_port;
	        //request_output[op].push_back(i); // request_op[i] keeps all the outport request from channels[j](j=port*vcs+ic) to the port i
	        tot_vca_req++;
	    }

	for( uint i=0; i<ports; i++)
	{
		bool vca_active = false;

		while( request_output[i].size())
		{
			uint msg_index = request_output[i].front();
			request_output[i].pop_front();
			uint op=input_buffer_state[msg_index].output_port;
			uint ip = input_buffer_state[msg_index].input_port;
			uint ic = input_buffer_state[msg_index].input_channel;
			assert( op == i);
			if( vc_alloc[op].size() )
			{
				//uint oc  = vc_alloc[op].front();

				uint oc=-1;
				/* can request only if there are credits improving
				 * chances of winning for other pkts that can actually
				 * exit the router */
				for (list<uint>::iterator itr=vc_alloc[op].begin();itr!=vc_alloc[op].end();++itr)
				//	for (vector<uint>::iterator itr=vc_alloc[op].begin();itr!=vc_alloc[op].end();++itr)
				{
					oc = *itr;
					//if ( (downstream_credits[op][oc] > 0) && was_vca_active == false )
					if ( was_vca_active == false )
					{
#ifdef _DEBUG_ROUTER
						_DBG("; VCA ; COMP ; (ip_%d*vcs_%d+ic_%d=op_%d*vcs_%d+oc_%d)%d=%d",ip,vcs,ic,op,vcs,oc,ip*vcs+ic,op*vcs+oc);
#endif
						// _DBG("; VCA COMP %d=%d",ip*vcs+ic,op*vcs+oc);
#ifdef _DEBUG_VCA
						 _DBG("; VCA COMP %lld ip_ic_%d_%d, op_oc_%d_%d",
								 input_buffer_state[msg_index].address,ip,ic,op,oc );
#endif
						//cout << endl << "oc = " << oc << endl;
						//cout << endl << "downstream_credits[op][oc] = " << downstream_credits[op][oc] << endl;

						input_buffer_state[msg_index].output_channel = oc;
						//input_buffer_state[msg_index].output_channel = vc_alloc[op][winner_oc];
						//vc_alloc[op].pop_front();
						//vc_alloc[op].erase(itr);

						input_buffer_state[msg_index].pipe_stage = VCA_COMPLETE;
						ticking = true;
						was_vca_active = true;
						vca_active = true;
						vca_won++;
		                if(in_buffers.at(ip).get_occupancy(ic))
		                {
		#ifdef _DEBUG_ROUTER
		            	 _DBG("; SWA ; REQ ; %d-%d|%d-%d",ip,ic,op,oc);
		#endif                    //                printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);
		                    swa.request(op, oc, ip, ic);
		                    input_buffer_state[msg_index].pipe_stage = SWA_REQUESTED;
		                    ticking = true;
		                }
						break;
					}
				}
				if (vca_active == true )
				{
					vc_alloc[op].remove(input_buffer_state[msg_index].output_channel);
				}

#ifdef _DEBUG_ROUTER
				if (vca_active == false )
				{
					cout << endl <<" op="<<op<<" vca_active == false "<< endl;
				}
#endif
			}
		ticking=true;
		}
	}

	if( tot_vca_req )
	{
	    stat_vca_fail_msg_ratio += ((tot_vca_req - vca_won+0.0)/tot_vca_req);
	    stat_vca_fail_msg_ratio = stat_vca_fail_msg_ratio/2;
	    stat_vca_load += ((tot_vca_req+0.0)/(ports*vcs));
	    stat_vca_load = stat_vca_load/2;
	}

	if( was_vca_active )
	{
	    istat->stat_router[node_ip]->vca_cycles++;
	    stat_vca_cycles++;
	}

	//request_switch_allocation();
    // If swa request failed and your stuck in vca you can request here
    for( uint i=0; i<( ports* vcs); ++i)
        if( input_buffer_state.at(i).pipe_stage == VCA_COMPLETE)
        {
        	InputBufferState& curr_msg = input_buffer_state.at(i);
            uint ip = curr_msg.input_port;
            uint ic = curr_msg.input_channel;
            uint op = curr_msg.output_port;
            uint oc = curr_msg.output_channel;
#ifdef _DEBUG_ROUTER
            	 _DBG("; SWA ; REQ ; %d-%d|%d-%d",ip,ic,op,oc);
#endif            //printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);

            if(in_buffers.at(ip).get_occupancy(ic))
            {
                swa.request(op, oc, ip, ic);
                curr_msg.pipe_stage = SWA_REQUESTED;
                ticking = true;
            }
        }

}

/* This is called from two places. First when head completes vca and requests
 * for swa. Second when flit goes out and msg requests for swa again as long
 * as there the tail goes out*/
void
RouterVcMPm::request_switch_allocation() 
{
    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == VCA_COMPLETE)
        {
            uint ip = input_buffer_state[i].input_port;
            uint ic = input_buffer_state[i].input_channel;
            uint op = input_buffer_state[i].output_port;
            uint oc = input_buffer_state[i].output_channel;

            /* Can skip this and speculatively request sa. May end up wasting
             * an st cycle during wrong speculation. */
            if(in_buffers[ip].get_occupancy(ic) )
            {
#ifdef _DEBUG_ROUTER
            	_DBG("; SWA ; REQ ; %d=%d (ip_%d*vcs_%d+ic_%d,op_%d*vcs_%d+oc_%d)",ip*vcs+ic,op*vcs+oc,ip,vcs,ic,op,vcs,oc);
#endif                // _DBG("; SWA_REQ %d=%d",ip*vcs+ic,op*vcs+oc);
                sw_alloc[op].push_back(i);
                input_buffer_state[i].pipe_stage = SWA_REQUESTED;
                ticking = true;
            }
            else
            {
#ifdef _DEBUG_ROUTER
            	_DBG("; SWA ; REQ ; in_buffers[%d].get_occupancy(%d) =_%d",ip,ic,in_buffers[ip].get_occupancy(ic));
#endif
            }
        }
    return;
}

/*! \brief Event handle for the TICK_EVENT. Entry from DES kernel */
void
RouterVcMPm::handle_tick_event ( IrisEvent* e )
{
	ticking = false;
	//do_switch_traversal();
	do_st();
	//do_switch_allocation_m();
	do_swa();
#ifdef _OLDVCA
	//do_virtual_channel_allocation_m();
	do_virtual_channel_allocation();
#else
	do_vca();
#endif

	/*!  Input buffering
	 *  Flits are pushed into the input buffer in the link arrival handler
	 *  itself. To ensure the pipeline stages are executed in reverse pipe
	 *  order IB is done here and all link_traversals have higher priority and get done before
	 *  tick. Head/Body and Tail flits go thru the IB stage.*/
	do_input_buffering();

	// update metrics
	{
		uint temp_cg_metric=0;
		// 1: west; 2: east; 3: south; 4: north
		//vector <uint> local_metric;
		//local_metric.resize(ports);

		uint op=0; // output port for propagation

		for(uint p=1;p<ports;p++)
		{

			switch(p)
			{
			case 1: // west
				op=2;
				break;
			case 2: // east
				op=1;
				break;
			case 3: // south
				op=4;
				break;
			case 4: // north
				op=3;
				break;
			default:
				printf("\n do_rca() wrong port for rca \n");
				exit(-1);
			}


			if (cg_metric == NOMETRIC )
			{
				local_metric[p]=0;

			}
			else if (cg_metric == BUFFER_SIZE )
			{
				temp_cg_metric=0;
				uint lowlimit = 0;
				uint highlimit=vcs;

				if(neighbors[p]== -1)
				{
					temp_cg_metric = 0;
				}
				else
				{
					for (uint k=lowlimit;k<highlimit;k++)  // todo: virtual network
					{
						temp_cg_metric += (downstream_credits[p][k]);
					}
				}
				local_metric[p]=temp_cg_metric;
			}
			else if (cg_metric == VIRTUAL_CHANNEL )
			{
				//if(in_buffers[p].get_no_empty_vc()<vcs/2)
				//	local_metric[p]=1;
				//else
				//	local_metric[p]=0;
				if(neighbors[p]== -1)
				{
					local_metric[p] = 0;
				}
				else
				{
					local_metric[p] = vca.no_vc_available(p);
				}

			}
			else if (cg_metric == FLIT_REMAIN )
			{
				temp_cg_metric=0;
				if(neighbors[p]== -1)
				{
					temp_cg_metric = 0;
				}
				else
				{
					for( uint l=0; l<(ports*vcs); l++)
					{
						if(input_buffer_state[l].output_port == p)
							temp_cg_metric += (input_buffer_state[l].remain_flits);
					}
				}
				local_metric[p]=temp_cg_metric;

			}
			else if (cg_metric == MIX1 )
			{
				temp_cg_metric=0;
				if(neighbors[p]== -1)
				{
					temp_cg_metric = 0;
				}
				else
				{
					for( uint l=0; l<(ports*vcs); l++)
					{
						if(input_buffer_state[l].output_port == p)
						{
							assert(input_buffer_state[l].remain_flits<=20);
							if(input_buffer_state[l].output_channel == -1)
								temp_cg_metric += (input_buffer_state[l].remain_flits);
							else
								temp_cg_metric += (input_buffer_state[l].remain_flits)*1000;
						}
					}
					local_metric[p]=temp_cg_metric;
				}


			}
			else if (cg_metric == CROSSBAR_DEMANDE )
			{
				local_metric[p]=0;

			}
			else if (cg_metric == XB_VC )
			{
				local_metric[p]=0;

			}
			else if (cg_metric == VC_BF )
			{
				local_metric[p]=0;

			}
			else if (cg_metric == XB_BF )
			{
				local_metric[p]=0;

			}

			if(cvr[p]!=local_metric[p])
			{
				cvr[p]=local_metric[p];
			}


		}


	}

#ifdef _NOP
	nop_slect();
#else
	/*! Request VCA at the end of IB*/

	uint op;//=-1;
	uint oc;//=-1;
	for( uint i=0; i<(ports*vcs); i++)
		if( input_buffer_state[i].pipe_stage == IB )
		{
			uint ip = input_buffer_state[i].input_port;
			uint ic = input_buffer_state[i].input_channel;
			op=-1;
			oc=-1;

			if( in_buffers[ip].get_occupancy(ic)>0 ) {

				for (uint u=0;u<fault_ports.size();u++) // remove faulty ports in possible_oports
					//            for (uint u=0;u<nb_faults;u++)
				{
					for (uint v=0;v<input_buffer_state[i].possible_oports.size(); v++)
					{
						if(fault_ports[u]==input_buffer_state[i].possible_oports[v]) {
							input_buffer_state[i].possible_oports.erase(input_buffer_state[i].possible_oports.begin()+v);
#ifdef _DEBUG_FI
							_DBG("fault_ports[%d]=%d; input_buffer_state[%d].possible_oports[%d]=%d", u, fault_ports[u], i, v, input_buffer_state[i].possible_oports[v]);
#endif
						}
					}
				}

				if ( input_buffer_state[i].possible_oports.size() != 0 )
				{

					if (cg_metric == NOMETRIC )
					{
						if (op== -1)
							op = input_buffer_state[i].possible_oports[0];
						else
						{
							// todo: fixme RC should be done only once
							uint temp=op;
							op = input_buffer_state[i].possible_oports[0];
							assert(temp==op);
						}
					}
					else if (cg_metric == BUFFER_SIZE )
					{
						if(input_buffer_state[i].possible_oports.size() >1)
						{
							for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
							{
								uint temp_op = input_buffer_state[i].possible_oports.at(j);
								if(!input_buffer_state[i].is_misrouting.at(j))
								{
									metrics.push_back(pair<uint,uint>(temp_op, cvr[temp_op]));
								}

							}
							if (metrics.size()>=1)
							{
								sort_metrics_decrease();
#ifdef _DEBUG_METRIC
								for(uint j=0;j<metrics.size();j++)
								{
									_DBG("; ip%d_ic%d sorted metrics BUFFER_SIZE: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
								}
#endif
								op = metrics.front().first;
							}
							else if (metrics.size()<1)
							{
								for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
								{
									uint temp_op = input_buffer_state[i].possible_oports.at(j);
									if(input_buffer_state[i].is_misrouting.at(j))
									{
										metrics.push_back(pair<uint,uint>(temp_op, cvr[temp_op]));
									}

								}
								if (metrics.size()>=1)
								{
									sort_metrics_decrease();
#ifdef _DEBUG_METRIC
									for(uint j=0;j<metrics.size();j++)
									{
										_DBG("; ip%d_ic%d sorted metrics BUFFER_SIZE: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
									}
#endif
									op = metrics.front().first;
								}

							}
							assert (op!=-1);
							metrics.clear();
						}
						else
						{
							op = input_buffer_state[i].possible_oports[0];
						}

					}
					else if (cg_metric == VIRTUAL_CHANNEL )
					{

						if(input_buffer_state[i].possible_oports.size() >1)
						{
							for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
							{
								uint temp_op = input_buffer_state[i].possible_oports.at(j);
								if(!input_buffer_state[i].is_misrouting.at(j))
								{
									metrics.push_back(pair<uint,uint>(temp_op, cvr[temp_op]));
								}

							}
							if (metrics.size()>=1)
							{
								sort_metrics_decrease();
#ifdef _DEBUG_METRIC
								for(uint j=0;j<metrics.size();j++)
								{
									_DBG("; ip%d_ic%d sorted metrics VIRTUAL_CHANNEL: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
								}
#endif
								op = metrics.front().first;
							}
							else if (metrics.size()<1)
							{
								for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
								{
									uint temp_op = input_buffer_state[i].possible_oports.at(j);
									if(input_buffer_state[i].is_misrouting.at(j))
									{
										metrics.push_back(pair<uint,uint>(temp_op, cvr[temp_op]));
									}

								}
								if (metrics.size()>=1)
								{
									sort_metrics_decrease();
#ifdef _DEBUG_METRIC
									for(uint j=0;j<metrics.size();j++)
									{
										_DBG("; ip%d_ic%d sorted metrics VIRTUAL_CHANNEL: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
									}
#endif
									op = metrics.front().first;
								}

							}
							assert (op!=-1);
							metrics.clear();
						}
						else
						{
							op = input_buffer_state[i].possible_oports[0];
						}

					}
					else if ((cg_metric == FLIT_REMAIN ) || (cg_metric == MIX1 ))
					{
						if(input_buffer_state[i].possible_oports.size() >1)
						{
							for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
							{
								uint temp_op = input_buffer_state[i].possible_oports.at(j);
								if(!input_buffer_state[i].is_misrouting.at(j))
								{
									metrics.push_back(pair<uint,uint>(temp_op, cvr[temp_op]));
								}

							}
							if (metrics.size()>=1)
							{
								sort_metrics_increase();
#ifdef _DEBUG_METRIC
								for(uint j=0;j<metrics.size();j++)
								{
									_DBG("; ip%d_ic%d sorted metrics FLIT_REMAIN: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
								}
#endif
								op = metrics.front().first;
							}
							else if (metrics.size()<1)
							{
								for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
								{
									uint temp_op = input_buffer_state[i].possible_oports.at(j);
									if(input_buffer_state[i].is_misrouting.at(j))
									{
										metrics.push_back(pair<uint,uint>(temp_op, cvr[temp_op]));
									}

								}
								if (metrics.size()>=1)
								{
									sort_metrics_increase();
#ifdef _DEBUG_METRIC
									for(uint j=0;j<metrics.size();j++)
									{
										_DBG("; ip%d_ic%d sorted metrics FLIT_REMAIN: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
									}
#endif
									op = metrics.front().first;
								}

							}
							assert (op!=-1);
							metrics.clear();
						}
						else
						{
							op = input_buffer_state[i].possible_oports[0];
						}
					}



					//oc = input_buffer_state[i].possible_ovcs[0];
					// todo: fixme optput vc allocation
					if (oc== -1)
						oc = ic;
#ifdef _OLDVCA
					input_buffer_state[i].pipe_stage = VCA_REQUESTED;
					input_buffer_state[i].output_port = op;
					request_output[op].push_back(i);
					request_op[op].push_back(i);
					ticking = true;
					// update   remain_flits
					remain_flits[op] += pkt_size;
					input_buffer_state[i].remain_flits  += pkt_size;
#else
					if( !vca.is_requested(op, oc, ip, ic) )
						//if( !vca.is_requested(op, oc, ip, ic) && downstream_credits[op][ic]>0)
						//if( downstream_credits[op][oc]>0)
					{
						input_buffer_state[i].pipe_stage = VCA_REQUESTED;
						input_buffer_state[i].output_port = op;

						vca.request(op, oc, ip, ic);
#ifdef _DEBUG_ROUTER
						_DBG("; VCA ; REQUEST; ip%d-ic%d|op%d-oc%d ",ip,ic,op,oc);
#endif
						ticking = true;
						// update   remain_flits
						remain_flits[op] += pkt_size;
						input_buffer_state[i].remain_flits  += pkt_size;
#ifdef _DEBUG_METRIC0
						_DBG(";pkt %lld +3 ; remain_flits ; remain_flits[%d] Remain Flits = %d",input_buffer_state[i].address,op,remain_flits[op]);
#endif
					}
#endif
#ifdef _DEBUG_VCA
						 _DBG("; VCA REQ %lld ip_ic_%d_%d, op_%d",
								 input_buffer_state[i].address,ip,ic,op );
#endif
#ifdef _DEBUG_ROUTER
						_DBG("; VCA ; REQUEST; ip%d-ic%d|op%d-oc%d ",ip,ic,op,oc);
#endif

#ifdef _DEBUG_METRIC0
						_DBG(";pkt %lld +3 ; remain_flits ; remain_flits[%d] Remain Flits = %d",input_buffer_state[i].address,op,remain_flits[op]);
#endif

				}

				else // delete flit & send back credit
				{
					if( in_buffers[ip].get_occupancy(ic)>0 )
					{
						in_buffers[ip].change_pull_channel(ic);
						Flit* f=in_buffers[ip].pull();
#ifdef _DEBUG_FI
						_DBG("; RC ; Fail ; ip_%d*vcs_%d+ic_%d(=%d) ft=%d",ip,vcs,ic,ip*vcs+ic, f->type);
						if(f->type ==HEAD)
						{
							_DBG("; Router ; Drop Flit HEAD ; addr %lld; Head;",static_cast<HeadFlit*>(f)->addr);
						}
						else if(f->type ==BODY)
						{
							_DBG("; Router ; Drop Flit BODY ; addr %lld; Body;",static_cast<BodyFlit*>(f)->addr);
						}
						else if(f->type ==TAIL)
						{
							_DBG("; Router ; Drop Flit TAIL ; addr %lld; Tail;",static_cast<TailFlit*>(f)->addr);
						}
#endif

						if(f->type ==HEAD )// || f->is_single_flit_pkt )
						{
							stat_pkts_drop++;
						}

						send_credit_back(i); // send upstream router that one more credit in input_buffer
						/*                  remain_flits[op]--;
#ifdef _DEBUG_METRIC
        	_DBG(";remain_flits ; remain_flits[%d] Remain Flits = %d",op,remain_flits[op]);
#endif
						 */
						if(f->type ==TAIL || f->is_single_flit_pkt )
						{

							input_buffer_state[i].clear_message = true;
							if(in_buffers[ip].get_occupancy(ic) > 0)
								input_buffer_state[i].pipe_stage = FULL;
							else
								input_buffer_state[i].pipe_stage = EMPTY;
							//vc_alloc[op].push_back(oc);
							input_buffer_state[i].possible_oports.clear();
							input_buffer_state[i].possible_ovcs.clear();
							input_buffer_state[i].output_channel =-1;
							input_buffer_state[i].output_port =-1;
							input_buffer_state[i].input_channel =-1;
							input_buffer_state[i].input_port =-1;
						}
					}

				}
			}
		}

#endif
	/*Per cycle router stats */
	for( uint i=0; i<ports; i++)
		for( uint j=0; j<vcs; j++)
		{
			if(in_buffers[i].get_occupancy(j) )
			{
				double buff_occ =(in_buffers[i].get_occupancy(j)+0.0)/credits;
				stat_buffer_occupancy += buff_occ;
				stat_buffer_occupancy = stat_buffer_occupancy/2;
			}
		}

	/*! The router can be set to not tick and therefore speed up simulation if
	 * there are no messages to route. To be replaced by the start and stop
	 * clock functionality in the next release of the kernel. Under high load
	 * you also register the component to a clock so that it will be called
	 * every cycle and hence an event need not be explicitly generated.
	 * */
	if(ticking)
	{
		ticking = true;
		IrisEvent* event = new IrisEvent();
		event->type = TICK_EVENT;
		event->vc = e->vc;
		Simulator::Schedule(Simulator::Now()+1, &NetworkComponent::process_event, this, event);
	}

	delete e;

	return;

}		/* -----  end of function RouterVcMPm::handle_input_arbitration_event  ----- */

string
RouterVcMPm::toString() const
{
    stringstream str;
    str << "RouterVcMPm"
        << "\t addr: " << address
        << " node_ip: " << node_ip
        << "\n Input buffers: " << in_buffers.size() << " ";
    if( in_buffers.size() > 0)
        str << in_buffers[0].toString();

    str << "\n decoders: " << decoders.size() << " ";
    if( decoders.size() > 0)
        str << decoders[0].toString();

    return str.str();
}

void
RouterVcMPm::send_credit_back(uint i)
{
    input_buffer_state[i].credits_sent--;
    LinkArrivalData* data = new LinkArrivalData();
    uint port = input_buffer_state[i].input_port;
    data->type = CREDIT_ID;
    data->vc = input_buffer_state[i].input_channel;
    IrisEvent* event = new IrisEvent();
    event->type = LINK_ARRIVAL_EVENT;
    event->event_data.push_back(data);
    event->src_id = address;
    event->vc = data->vc;
    Simulator::Schedule(Simulator::Now()+0.75, &NetworkComponent::process_event,
                        static_cast<GenericLink*>(input_connections[port])->input_connection, event);

    static_cast<GenericLink*>(input_connections[port])->credits_passed++;
    istat->stat_link[static_cast<GenericLink*>(input_connections[port])->link_id]->credits_transferred++;
}

uint
RouterVcMPm::get_sum_bf(uint port)
{
    uint sum_bf=0;
	for( uint j=0; j<vcs; j++)
        {
            {
                sum_bf=sum_bf+downstream_credits[port][j];

            }
        }
	return sum_bf;
}

uint
RouterVcMPm::nop_slect()
{

    uint op=-1;
    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == IB )
        {
        	HeadFlit* hf=new HeadFlit();
        	hf->dst_address=input_buffer_state[i].destination;
        	hf->src_address=input_buffer_state[i].source;
        	//hf->addr=input_buffer_state[i].address;
       	    hf->type = HEAD;
       	    hf->vc= input_buffer_state[i].input_channel;
        	if ( input_buffer_state[i].possible_oports.size() == 2 )
        	{
            	uint porta=input_buffer_state[i].possible_oports[0];
            	uint portb=input_buffer_state[i].possible_oports[1];
            	nop_rc[porta].push(hf,hf->vc);
            	nop_rc[portb].push(hf,hf->vc);

            	vector <uint> a_aoc;
            	a_aoc.resize(2);
            	uint sum_free_buffer_a=0;
                uint no_adaptive_aports = nop_rc[porta].no_adaptive_ports(hf->vc);

                for ( uint i=0; i<no_adaptive_aports; i++ )
                {
                	uint temp=0;
                	uint rc_port = nop_rc[porta].get_output_port(hf->vc);
                    if( static_cast<RouterVcMPm*>(static_cast<GenericLink*>(output_connections[porta])->output_connection)->neighbors[rc_port] != NULL )  // should check if exits
                    {
                    	a_aoc.push_back(rc_port);
                     temp=static_cast<RouterVcMPm*>(static_cast<GenericLink*>(output_connections[porta])->output_connection)->get_sum_bf(rc_port);
                    	sum_free_buffer_a=sum_free_buffer_a+temp;

                    }
                }

            	vector <uint> b_aoc;
            	b_aoc.resize(2);
            	uint sum_free_buffer_b=0;
                uint no_adaptive_bports = nop_rc[portb].no_adaptive_ports(hf->vc);

                for ( uint i=0; i<no_adaptive_bports; i++ )
                {
                	uint temp=0;
                    uint rc_port = nop_rc[portb].get_output_port(hf->vc);
                    if( static_cast<RouterVcMPm*>(static_cast<GenericLink*>(output_connections[portb])->output_connection)->neighbors[rc_port] != NULL )  // should check if exits
                    {
                        b_aoc.push_back(rc_port);
                        temp=static_cast<RouterVcMPm*>(static_cast<GenericLink*>(output_connections[portb])->output_connection)->get_sum_bf(rc_port);
                    	sum_free_buffer_b=sum_free_buffer_b+temp;
                    }

                }

                assert ( a_aoc.size() > 0);
                assert ( b_aoc.size() > 0);


                if (sum_free_buffer_a>sum_free_buffer_b) op=porta;
                else  op=portb;

#ifdef _DEBUG_NOP
            _DBG("; VCA ; NOP ; no_adaptive_aports=%d; no_adaptive_bports=%d",no_adaptive_aports,no_adaptive_bports);
#endif
#ifdef _DEBUG_NOP
            _DBG("; VCA ; NOP ; \n port_%d(sum_free_buffer_a=%u); \n port_%d(sum_free_buffer_b=%u); \n op= %d",porta,sum_free_buffer_a,portb, sum_free_buffer_b,op);
#endif

        	}

	else if (input_buffer_state[i].possible_oports.size()==1)
		 op=input_buffer_state[i].possible_oports[0];


            input_buffer_state[i].pipe_stage = VCA_REQUESTED;
            input_buffer_state[i].output_port = op;
#ifdef _DEBUG_NOP
            _DBG("; VCA ; REQ ; i=%d op=%d",i,op);
#endif
            //                _DBG("; VCA REQ %d=%d(op)",ip*vcs+ic,op);
            ticking = true;
            free(hf);

        }
}


#endif   /* ----- #ifndef _routerVcMP_cc_INC  ----- */

