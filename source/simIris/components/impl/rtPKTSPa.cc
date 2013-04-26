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

#ifndef  _rtPKTSPa_cc_INC
#define  _rtPKTSPa_cc_INC

#include	"rtPKTSPa.h"
using namespace std;

//#define _DEBUG_ROUTER0
//#define _DEBUG_ROUTER
//#define _DEBUG_PKT
//#define _DEBUG_MEMORY
//#define _DEBUG_ROUTER_HEAD
//#define _DEBUG_FI
//#define _DEBUG_METRIC
//#define _DEBUG_CREDIT

//#define _DEBUG_NOP
//#define _NOP
//#define _DEBUG_METRIC0

RtPKTSPa::RtPKTSPa ()
{
    name = "RtPKTSPa" ;
    ticking = false;
}  /* -----  end of method RtPKTSPa::RtPKTSPa  (constructor)  ----- */

RtPKTSPa::~RtPKTSPa()
{
}


void
RtPKTSPa::find_neighbors(uint p, uint v, uint cr, uint bs)
{
    ports =p;
    vcs =v;
    credits =cr;
    buffer_size = bs;

    //neighbors.resize(ports); // resize with all set to 0
    //neighbors.insert(neighbors.begin(),neighbors.size(),-1);
    for (uint i=0; i<ports; i++)
    {
    	neighbors.push_back(-1);
    }
    for (uint i=0; i<ports; i++)
    {
    	if( static_cast<GenericLink*>(input_connections[i])->input_connection!=NULL )
    	{
    		neighbors[i]= static_cast<GenericLink*>(input_connections[i])->input_connection->node_ip; //link_id;
    	}
    }

    // for NOP
    for(uint i=0; i<ports; i++)
    {
    	if (neighbors[i]!= -1)
        {
        	nop_rc[i].node_ip = neighbors[i];
        	nop_rc[i].resize( vcs );
        }

    }
    // for link_number

    // for fault on neighbors
    for(uint i=1; i<ports; i++)
    {
    	if (neighbors[i]!= -1)
        {
    		for(uint j=0; j<ports; j++)
    		{
    			fon[i].push_back(&(static_cast<RtPKTSPa*>(static_cast<GenericLink*>(input_connections[i])->input_connection)->local_ft[j]));
    		}
        }
    	else
    	{
    		local_ft[i] = 1;
    		for(uint j=0; j<ports; j++)
    			fon[i].push_back(NULL);
    	}
    }
    return ;
}
void
RtPKTSPa::set_fault(uint p)
{
	//fault_ports[p]=1; //used as flag
	fault_outports.push_back(p);
	//nb_faults++;
	local_ft[p]=1;
	return ;
}
void
RtPKTSPa::set_temp_fault(uint p,uint type)
{
	//fault_outports[p]=1; //used as flag
	//nb_faults++;
	if(type==6000)
	{
		fault_outports.push_back(p);
		local_ft[p]=1;
	}
	else if(type==9000)
		fault_inports.push_back(p);
	else
	{
		_DBG("; Not expected type %d",type );
		exit(0);
	}
	return ;
}
void
RtPKTSPa::remove_fault(uint p,uint type)
{
	if(type==6000)
	{
		fault_outports.erase(remove(fault_outports.begin(),fault_outports.end(),p),fault_outports.end());
		local_ft[p]=0;
	}
	else if(type==9000)
		fault_inports.erase(remove(fault_inports.begin(),fault_inports.end(),p),fault_inports.end());
	else
	{
		_DBG("; Not expected type %d",type );
		exit(0);
	}
	return ;
}
void
RtPKTSPa::set_temp_opfault(uint p, double t, ullint d)
{
    IrisEvent* event = new IrisEvent();
    event->type = FAULT_INJECT_EVENT;
    event->src_id = 6000;//6000 output port fault; 9000 input port fault
    event->dst_id = p;//faulty port
    event->time = d;//fault duration

    Simulator::Schedule( t,
                         &NetworkComponent::process_event,
                         this,
                         event);
	return ;
}
void
RtPKTSPa::set_temp_ipfault(uint p, double t, ullint d)
{
    IrisEvent* event = new IrisEvent();
    event->type = FAULT_INJECT_EVENT;
    event->src_id = 9000;//6000 output port fault; 9000 input port fault
    event->dst_id = p;//faulty port
    event->time = d;//fault duration

    Simulator::Schedule( t,
                         &NetworkComponent::process_event,
                         this,
                         event);
	return ;
}
void
RtPKTSPa::handle_fault_remove_event(IrisEvent* e )
{
	remove_fault(e->dst_id, e->src_id);
#ifdef _DEBUG_FI

	_DBG("; Fault Injection Remove port %d ",e->dst_id);

#endif
    delete e; e=NULL;
	return;
}
void
RtPKTSPa::handle_fault_inject_event(IrisEvent* e )
{
#ifdef _DEBUG_FI
	if(e->src_id==6000)
	{
		_DBG("; Fault Injection out_port %d duration %lld",e->dst_id, e->time);
	}
	else if(e->src_id==9000)
	{
			_DBG("; Fault Injection in_port %d duration %lld",e->dst_id, e->time);
	}
	else
	{
		_DBG("; Not expected type %d",type );
		exit(0);
	}
#endif
	set_temp_fault(e->dst_id, e->src_id);
	//set_fault(e->dst_id);
	IrisEvent* event = new IrisEvent();
	    event->type = FAULT_REMOVE_EVENT;
	    event->src_id = e->src_id;
	    event->dst_id = e->dst_id;//faulty port
	    Simulator::Schedule( Simulator::Now()+e->time,
	                         &NetworkComponent::process_event,
	                         this,
	                         event);
	    delete e; e=NULL;
		return ;

}

void
RtPKTSPa::init (uint p, uint v, uint cr, uint bs)
{
    ports =p;
    vcs =v;
    credits =cr;
    buffer_size = bs;

    address = myId();

    /*  set_input_ports(ports); */
    in_buffers.resize(ports);
    decoders.resize(ports);

    fault_outports.clear();
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
    remain_flits.resize(ports);

    /*Resize per port stats */
    stat_packet_out.resize(ports);
    stat_flit_out.resize(ports);


    /* All decoders and vc arbiters need to know the node_ip for routing */
    for(uint i=0; i<ports; i++)
    {
        decoders[i].node_ip = node_ip;
        decoders[i].address = address;
        stat_packet_out[i].resize(ports);
        stat_flit_out[i].resize(ports);
        remain_flits[i]=0;
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
    {
        for(uint j=0; j<vcs; j++)
        {
            downstream_credits[i][j] = credits;
            cr_time[i][j] = -1;
            input_buffer_state[i*vcs+j].pipe_stage = EMPTY;
            input_buffer_state[i*vcs+j].output_port = -1;
            vc_alloc[i].push_back(j);
        }
    }

    fon.resize(ports);

    local_ft.resize(ports);
    for(uint i=0; i<ports; i++)
    {
    	fon[0].push_back(&(local_ft[i]));
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
    stat_pshf_created = 0;
    stat_pstf_created = 0;
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
}		/* -----  end of function RtPKTSPa::init  ----- */

/*! \brief These functions are mainly for DOR routing and are seperated so as to not
 * force DOR modelling in all designs */
void
RtPKTSPa::set_no_nodes( uint nodes )
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
RtPKTSPa::set_grid_x_location( uint port, uint x_node, uint value)
{
    decoders[port].grid_xloc[x_node]= value;
    nop_rc[port].grid_xloc[x_node]= value;
}

void
RtPKTSPa::set_grid_y_location( uint port, uint y_node, uint value)
{
    decoders[port].grid_yloc[y_node]= value;
    nop_rc[port].grid_yloc[y_node]= value;
}

/*  End of DOR grid location functions */

void
RtPKTSPa::process_event ( IrisEvent* e )
{
    switch(e->type)
    {
        case LINK_ARRIVAL_EVENT:
#ifdef _DEBUG_ROUTER
        	_DBG_NOARG("; RtPKTSPa::LINK_ARRIVAL_EVENT ;");
#endif
            handle_link_arrival_event(e);
            break;
        case TICK_EVENT:
#ifdef _DEBUG_ROUTER
        	_DBG_NOARG("; RtPKTSPa::TICK_EVENT ;");
#endif
            handle_tick_event(e);
            break;
        case DETECT_DEADLOCK_EVENT:
#ifdef _DEBUG_ROUTER
        	_DBG_NOARG("; RtPKTSPa::DETECT_DEADLOCK_EVENT ;");
#endif
            handle_detect_deadlock_event(e);
            break;
        case FAULT_INJECT_EVENT:
#ifdef _DEBUG_ROUTER
        	_DBG_NOARG("; RtPKTSPa::FAULT_INJECT_EVENT ;");
#endif
            handle_fault_inject_event(e);
            break;
        case FAULT_REMOVE_EVENT:
#ifdef _DEBUG_ROUTER
        	_DBG_NOARG("; RtPKTSPa::FAULT_REMOVE_EVENT ;");
#endif
            handle_fault_remove_event(e);
            break;
        case RESET_STAT_EVENT:
#ifdef _DEBUG_ROUTER
        	_DBG_NOARG("; RtPKTSPa::RESET_STAT_EVENT ;");
#endif
        	handle_reset_stat_event(e);
            break;

        default:
            _DBG(";RtPKTSPa:: Unk event exception %d", e->type);
            break;
    }
    return ;
}		/* -----  end of function RtPKTSPa::process_event  ----- */

void 
RtPKTSPa::handle_detect_deadlock_event(IrisEvent* e )
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
RtPKTSPa::dump_buffer_state()
{
    for ( uint i=0; i<ports*vcs; i++)
        if(input_buffer_state[i].pipe_stage != EMPTY && input_buffer_state[i].pipe_stage != INVALID)
        {
            cout << " Router[" << node_ip << "]->buff["<<i<<"] " << input_buffer_state[i].toString() << endl;
        }
    cout << endl;

    return;
}

double RtPKTSPa::get_average_packet_latency()
{
	return ((stat_total_packet_latency+0.0)/stat_packets);
}

double RtPKTSPa::get_last_flit_out_cycle()
{
	return last_flit_out_cycle;
}

double RtPKTSPa::get_flits_per_packet()
{
	return (stat_flits+0.0)/(stat_packets) ;
}

double RtPKTSPa::get_buffer_occupancy()
{
	return stat_buffer_occupancy;	
}

double RtPKTSPa::get_packets_drop()
{
	return stat_pkts_drop;
}

double RtPKTSPa::get_pshf_created()
{
	return stat_pshf_created;
}
double RtPKTSPa::get_pstf_created()
{
	return stat_pstf_created;
}
double RtPKTSPa::get_swa_fail_msg_ratio()
{
	return stat_swa_fail_msg_ratio;	
}

double RtPKTSPa::get_swa_load()
{
	return stat_swa_load;
}

double RtPKTSPa::get_vca_fail_msg_ratio()
{
	return stat_vca_fail_msg_ratio;
}

double RtPKTSPa::get_vca_load()
{
	return stat_vca_load;
}
 
double RtPKTSPa::get_stat_packets()
{
	return stat_packets;	
}

double RtPKTSPa::get_stat_flits()
{
	return stat_flits;
}

double RtPKTSPa::get_stat_ib_cycles()
{
	return stat_ib_cycles;	
}

double RtPKTSPa::get_stat_rc_cycles()
{
	return stat_rc_cycles;
}

double RtPKTSPa::get_stat_vca_cycles()
{
	return stat_vca_cycles;
}
        
double RtPKTSPa::get_stat_swa_cycles()
{
	return stat_swa_cycles;
}
 
double RtPKTSPa::get_stat_st_cycles()
{
	return stat_st_cycles;
}


string
RtPKTSPa::print_stats()
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
RtPKTSPa::handle_link_arrival_event ( IrisEvent* e )
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
        	_DBG("; Router BW; Flit Arrival ; Head; addr %lld; 0x%x",static_cast<HeadFlit*>(data->ptr)->addr, data->ptr);
        }
        else if(data->ptr->type ==BODY)
        {
        	_DBG("; Router BW; Flit Arrival ; Body; addr %lld; 0x%x",static_cast<BodyFlit*>(data->ptr)->addr, data->ptr);
        }
        else if(data->ptr->type ==TAIL)
        {
        	_DBG("; Router BW; Flit Arrival ; Tail; addr %lld; 0x%x",static_cast<TailFlit*>(data->ptr)->addr, data->ptr);
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
#ifdef _DEBUG_CREDIT
        _DBG("; Got a credit ; port %d vc:%d  cr:%d", port, data->vc, downstream_credits[port][data->vc]);
#endif
        assert(downstream_credits[port][data->vc]<=credits);
       
        cr_time[port][data->vc] = -1;

#ifdef _DEBUG_ROUTER0
        //_DBG("; Got a credit ; port %d vc:%d  cr:%d", port, data->vc, downstream_credits[port][data->vc]);
        vector < uint > pending_ops ;
        vector < uint > pending_ips ;
        for( uint i=0; i<ports*vcs; i++)
            if(input_buffer_state[i].output_port == port && input_buffer_state[i].output_channel == data->vc)
            {
                pending_ops.push_back(port*vcs+data->vc);
                pending_ips.push_back(i);
            }

        //cout << " Can send msgs (i-o): " ;
        /*for( uint i=0; i<pending_ops.size(); i++)
            cout << pending_ips[i] <<"-" << pending_ops[i] << "; occ/" << in_buffers[(int)(pending_ops[i]/vcs)].get_occupancy(pending_ops[i]%vcs)
                <<" bst:"<<input_buffer_state[i].pipe_stage << "; ";*/
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

    delete data; data=NULL;
    delete e; e=NULL;
    return ;
}		/* -----  end of function RtPKTSPa::handle_link_arrival_event  ----- */

void
RtPKTSPa::handle_reset_stat_event( IrisEvent* e)
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

    stat_pkts_drop = 0;
    stat_pshf_created = 0;
    stat_pstf_created = 0;
    delete e; e=NULL;
}

void
RtPKTSPa::do_input_buffering()
{
    for( uint i=0; i<ports*vcs; i++) 
    {
        uint inport = (uint)(i/vcs);
        uint invc = (uint)(i%vcs);
        bool found_ipfault = false;

        for (uint u=0;u<fault_inports.size();u++)
        {
        	if(fault_inports[u]==inport) found_ipfault = true;

        }

        /*  */
        if( input_buffer_state[i].pipe_stage == FULL || input_buffer_state[i].pipe_stage == EMPTY )
        {
            if( in_buffers[inport].get_occupancy(invc) > 0 || input_buffer_state[i].is_pseudo_hf_created)
            {
            	Flit* f;
            	if(input_buffer_state[i].is_pseudo_hf_created)
            	{
            		//f = &(input_buffer_state[i].copy_hf);
            		f = input_buffer_state[i].copy_hf;
            	}
            	else
            	{
            		in_buffers[inport].change_pull_channel(invc);
            		f = in_buffers[inport].peek();
            	}

                //                _DBG("; do_input_buffering %d ft%d",inport*vcs+invc, f->type);
#ifdef _DEBUG_ROUTER
//                                _DBG("; do_input_buffering %d ft%d",inport*vcs+invc, f->type);
                //                _DBG("; IB ; do_input_buffering ; address ; %lld ; inport_%d*vcs+invc_%d=%d ft(f->type)=%d",input_buffer_state[i].address, inport,invc,inport*vcs+invc, f->type);
                                _DBG("; IB ; do_input_buffering ; inport_%d*vcs+invc_%d=%d ft(f->type)=%d", inport,invc,inport*vcs+invc, f->type);
#endif
                if( f->type == HEAD )
                {
                	//if(!found_ipfault)
                	{
                    HeadFlit* hf = static_cast<HeadFlit*>(f);
                    /*  Update stats for head */
                    istat->stat_router[node_ip]->ib_cycles++;
                    stat_ib_cycles++;
                    //_DBG("; HEAD IN %d %lld" ,f->type, hf->addr);
#ifdef _DEBUG_ROUTER_HEAD
                    _DBG("; IB ; HEAD IN ip_%d ic_%d; addr=%lld; 0x%x; dst=%d",inport, invc, hf->addr, hf, hf->dst_address);
#endif
#ifdef _DEBUG_PKT
                    _DBG("; IB ; HEAD IN ip_%d ic_%d; addr=%lld; 0x%x; dst=%d",inport, invc, hf->addr, hf, hf->dst_address);
#endif
                    input_buffer_state[inport*vcs+f->vc].input_port = inport;
                    hf->inport = inport;

                    if(! input_buffer_state[i].is_pseudo_hf_created)
                    	//input_buffer_state[inport*vcs+f->vc].copy_hf = *hf;
                    	//input_buffer_state[inport*vcs+f->vc].copy_hf = hf;
                    {
                    	HeadFlit* bk_hf = new HeadFlit();
                    	//*(input_buffer_state[inport*vcs+f->vc].copy_hf) = *hf; //wrong why?
                    	//*(input_buffer_state[inport*vcs+f->vc].copy_hf) = *(static_cast<HeadFlit*>(f)); //wrong
                    	//*copy_hf = *static_cast<HeadFlit*>(f);
                    	*bk_hf = *hf;
                    	input_buffer_state[inport*vcs+f->vc].copy_hf=bk_hf;
#ifdef _DEBUG_MEMORY
                    _DBG("; IB ; bk_hf; addr=%lld; 0x%x",bk_hf->addr, bk_hf);
#endif
#ifdef _DEBUG_ROUTER_HEAD
                    _DBG("; IB ; bk_hf; addr=%lld; 0x%x",bk_hf->addr, bk_hf);
#endif
#ifdef _DEBUG_PKT
                    _DBG("; IB ; bk_hf; addr=%lld; 0x%x",bk_hf->addr, bk_hf);
#endif
                    }
                    input_buffer_state[inport*vcs+f->vc].input_channel = f->vc;
                    input_buffer_state[inport*vcs+f->vc].address= hf->addr;
                    input_buffer_state[inport*vcs+f->vc].destination= hf->dst_address;
                    input_buffer_state[inport*vcs+f->vc].source= hf->src_address;
                    input_buffer_state[inport*vcs+f->vc].vn= hf->vn;
                    input_buffer_state[inport*vcs+f->vc].pipe_stage = IB;
                    input_buffer_state[inport*vcs+f->vc].msg_class = hf->msg_class;
                    input_buffer_state[inport*vcs+f->vc].possible_oports.clear(); 
                    input_buffer_state[inport*vcs+f->vc].possible_ovcs.clear(); 
                    input_buffer_state[inport*vcs+f->vc].is_misrouting.clear();
                    if(hf->pseudo)
                    {
                    	input_buffer_state[inport*vcs+f->vc].length= hf->ps_length;
                    	input_buffer_state[inport*vcs+f->vc].remain_flits= hf->ps_length;
                    }
                    else
                    {
                    	input_buffer_state[inport*vcs+f->vc].length= hf->length;
                    	input_buffer_state[inport*vcs+f->vc].remain_flits= hf->length;
                    }

                    //decoders[inport].push(f,f->vc,neighbors, fault_outports);
                    decoders[inport].push(f,f->vc,neighbors, fault_outports, fon);

                    uint no_adaptive_ports = decoders[inport].no_adaptive_ports(f->vc);
                    //uint no_adaptive_vcs = decoders[inport].no_adaptive_vcs(f->vc);
                    // todo no_adaptive_vcs
                    uint no_adaptive_vcs = 1;//decoders[inport].no_adaptive_vcs(f->vc);


                    input_buffer_state[inport*vcs+f->vc].possible_ovcs.push_back(f->vc);

                    //assert ( input_buffer_state[inport*vcs+f->vc].possible_oports.size() > 0);
                    //assert ( input_buffer_state[inport*vcs+f->vc].possible_ovcs.size() > 0);

                    input_buffer_state[inport*vcs+f->vc].credits_sent= hf->length;
                    input_buffer_state[inport*vcs+f->vc].arrival_time= Simulator::Now();
                    input_buffer_state[inport*vcs+f->vc].clear_message= false;
                    input_buffer_state[inport*vcs+f->vc].is_pseudo_tf_created=false;

                    istat->stat_router[node_ip]->rc_cycles++;
                    stat_rc_cycles++;

                    for ( uint i=0; i<no_adaptive_ports; i++ )
                    {
                    	uint rc_port = decoders[inport].get_output_port(f->vc); // cope with last_adaptive_port in GenericRC

                    	if( static_cast<GenericLink*>(output_connections[rc_port])->output_connection != NULL )
                    		input_buffer_state[inport*vcs+f->vc].possible_oports.push_back(rc_port);
                    }

                    for ( uint i=0; i<no_adaptive_ports; i++ )
                    {
                    	bool is_misr = decoders[inport].get_output_port_status(f->vc);
                    	input_buffer_state[inport*vcs+f->vc].is_misrouting.push_back(is_misr);
                    }

                    for ( uint i=0; i<no_adaptive_vcs; i++ )
                    {
                    	uint rc_vc = decoders[inport].get_virtual_channel(f->vc);
                    	input_buffer_state[inport*vcs+f->vc].possible_ovcs.push_back(rc_vc);
                    }

                    ticking =true;
                	}
                	/*else  //todo if found in_fault
                	{
                		// drop hf and reset input_buffer_state
                		if(input_buffer_state[i].is_pseudo_hf_created)
                		{
                			delete input_buffer_state[inport*vcs+f->vc].copy_hf;
                		}

                		else
                			in_buffers[inport].pull();
                		//send_credit_back(i); // send upstream router that one more credit in input_buffer
#ifdef _DEBUG_FI
                		_DBG("; IB ; do_input_buffering ; inport fault found : Drop Flit HEAD ; addr %lld; Head;",static_cast<HeadFlit*>(f)->addr);
#endif
                	}*/
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
                //ticking =true;
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
RtPKTSPa::do_switch_traversal()
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
                _DBG("; ST ; COMP ; Fout ; (ip_%d*vcs_%d+ic_%d)%d=(op_%d*vcs_%d+oc_%d)%d ft=%d credit=%d",ip,vcs,ic,ip*vcs+ic,op,vcs,oc,op*vcs+oc, f->type, downstream_credits[op][oc]);
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
                    input_buffer_state[i].output_port=-1;
                    input_buffer_state[i].is_pseudo_hf_created = false;
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
RtPKTSPa::do_switch_allocation()
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
RtPKTSPa::do_switch_allocation_m() //Yi: m=modified
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
RtPKTSPa::do_st()
{
    for ( uint i=0; i<input_buffer_state.size(); ++i)
        if( input_buffer_state.at(i).pipe_stage==SW_TRAVERSAL )
        {
        	InputBufferState& curr_pkt = input_buffer_state.at(i);
            uint op = curr_pkt.output_port;
            uint oc = curr_pkt.output_channel;
            uint ip = curr_pkt.input_port;
            uint ic = curr_pkt.input_channel;
            bool found_opfault = false;
            bool found_ipfault = false;
            //bool send_pseudo_tf = false; // replaced by InputBufferState.is_pseudo_tf_created
            //printf("%d@%lu: credits%d  \n",node_id, _TICK_NOW, downstream_credits.at(op).at(oc));
            if ( downstream_credits.at(op).at(oc) )
            {
                for (uint u=0;u<fault_outports.size();u++)
                {
                	if(fault_outports[u]==op) found_opfault = true;
                }
                for (uint u=0;u<fault_inports.size();u++)
                {
                	if(fault_inports[u]==ip) found_ipfault = true;
                }

            	in_buffers[ip].change_pull_channel(ic);
                Flit* f;

                if(input_buffer_state[i].is_pseudo_hf_created  && !input_buffer_state[i].sent_head)
                	//f = &(input_buffer_state[i].copy_hf);
                {
                	f = input_buffer_state[i].copy_hf;
                	//assert(found_opfault==false);
                }
                else
                	f = in_buffers[ip].pull();

                f->vc = oc;



                if (!found_opfault)
                {
                	input_buffer_state[i].is_resent = false;

                	if(f->type != TAIL && !f->is_single_flit_pkt) // if f is not end of pkt, do SWA REQ for rest of pkt
                	{
                		input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                		// request_switch_allocation();
                		if(in_buffers.at(ip).get_occupancy(ic))
                		{
#ifdef _DEBUG_ROUTER
                			_DBG("; SWA ; REQ ; ip%d-ic%d|op%d-oc%d",ip,ic,op,oc);
#endif                    //                printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);
                			swa.request(op, oc, ip, ic);
                			input_buffer_state[i].pipe_stage = SWA_REQUESTED;
                		}
                		/*else {
                    	// the buffer is empty , check if inport fault,
                    	// if yes, create a pseudo link arrival event with pseudo tail flit
                    	// or copy current flit and change it to pseudo tail flit
                    	if (found_ipfault)
                    	{
                    	TailFlit* ptf = new TailFlit();
                    	ptf->type = TAIL;
                    	ptf->addr = curr_pkt.address;
                    	ptf->pseudo = true;
                    	LinkArrivalData* data = new LinkArrivalData();
                    	                data->type = FLIT_ID;
                    	                data->vc = oc;
                    	                data->ptr = ptf;
                        IrisEvent* event = new IrisEvent();
                        event->type = LINK_ARRIVAL_EVENT;
                        event->event_data.push_back(data);
                        event->src_id = static_cast<GenericLink*>(input_connections[ip])->input_connection->address;
                        event->dst_id = address;
                        event->vc = ic;

                        Simulator::Schedule( Simulator::Now()+0.75,
                                             &NetworkComponent::process_event,
                                             static_cast<GenericLink*>(input_connections[ip])->output_connection,event);
                        _DBG("; Create Pseudo Tail Flit:%lld; 0x%x; src_id:%d ",curr_pkt.address,ptf,event->src_id );
                        input_buffer_state[i].is_pseudo_tf_created = true;
                        stat_pstf_created ++;
                    	}
                    }*/
                		ticking = true;
                	}  // todo : adapt to sst iris do_st() ; done

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

                	curr_pkt.remain_flits--;

                	LinkArrivalData* data = new LinkArrivalData();
                	data->type = FLIT_ID;
                	data->vc = oc;
                	data->ptr = f;

                	/*  Updating all pkt stats in the head irrespective of msg
                	 *  class. Assuming all pkts do have a head. However latency
                	 *  for determining switch throughput is to be determined when
                	 *  the tail exits.
                	 *  */
                	if( f->type == HEAD)
                	{
                		assert(input_buffer_state[i].sent_head == false);
                		HeadFlit* hf = static_cast<HeadFlit*>(f);
                		hf->hop_count++;
                		hf->path.push_back(node_ip);
                		hf->node_stamping.push_back(op);
                		input_buffer_state[i].sent_head = true;

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
                			hf->path.push_back(node_ip);
                			hf->node_stamping.push_back(op);
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
                		vca.clear_winner(op,oc,ip,ic);
                		swa.clear_requestor(op, ip, oc);
                		input_buffer_state[i].possible_oports.clear();
                		input_buffer_state[i].possible_ovcs.clear();
                		input_buffer_state[i].address=-1;
                		input_buffer_state[i].destination=-1;
                		input_buffer_state[i].output_port=-1;
                		input_buffer_state[i].remain_flits = 0;
                		//delete input_buffer_state[i].copy_hf;
                		//input_buffer_state[i].is_pseudo_hf_created = false;
                		if(!input_buffer_state[i].is_pseudo_hf_created)
                		{
#ifdef _DEBUG_MEMORY
                			_DBG("; ST ; bk_hf; addr=%lld; 0x%x",input_buffer_state[i].copy_hf->addr, input_buffer_state[i].copy_hf);
#endif
                			delete input_buffer_state[i].copy_hf;
                			input_buffer_state[i].copy_hf=NULL;
                		}
                		input_buffer_state[i].is_pseudo_hf_created = false;
                		input_buffer_state[i].sent_head = false;
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
                	/* TODO: change it later (f->pseudo && f->type==HEAD && static_cast<HeadFlit*>(f)->src_address== address)) */
                	//if(!(input_buffer_state[i].is_pseudo_tf_created || (f->pseudo && f->type==HEAD && static_cast<HeadFlit*>(f)->src_address == address)))
                	//if(!((f->type == TAIL && input_buffer_state[i].is_pseudo_tf_created) || (f->type == HEAD && input_buffer_state[i].is_pseudo_hf_created)))
                	if(!((input_buffer_state[i].is_pseudo_tf_created) || (f->type == HEAD && input_buffer_state[i].is_pseudo_hf_created)))
                	{
                		send_credit_back(i); // dont send back a credit to upstream router if a pseudo_hf/pseudo_tf is created
                		input_buffer_state[i].is_pseudo_tf_created = false;
                	}                /* nothing to do with downstream_credits whether has pseudo_tf or not */
                	//send_credit_back(i); // send upstream router that one more credit in input_buffer
                	downstream_credits[op][oc]--; // decrease one credit since downstream router is received one flit
#ifdef _DEBUG_CREDIT
                	_DBG("; Decrease Credits ; port %d vc:%d  cr:%d", op, oc, downstream_credits[op][oc]);
#endif
                	remain_flits[op]--;

                	if(f->type == TAIL && input_buffer_state[i].is_pseudo_tf_created == false)
                		assert(curr_pkt.remain_flits == 0);

#ifdef _DEBUG_METRIC0
                	_DBG(";remain_flits ; remain_flits[%d] Remain Flits = %d --",op,remain_flits[op]);
#endif

#ifdef _DEBUG_PKT
                	_DBG("; ST ; COMP ; Fout ; (ip_%d*vcs_%d+ic_%d)%d=(op_%d*vcs_%d+oc_%d)%d ft=%d credit=%d",ip,vcs,ic,ip*vcs+ic,op,vcs,oc,op*vcs+oc, f->type, downstream_credits[op][oc]);
                	if(data->ptr->type ==HEAD)
                	{
                		_DBG("; Router ST; Flit Out ; addr %lld; Head; 0x%x",static_cast<HeadFlit*>(data->ptr)->addr,data->ptr);
                	}
                	else if(data->ptr->type ==BODY)
                	{
                		_DBG("; Router ST; Flit Out ; addr %lld; 0x%x; Body",static_cast<BodyFlit*>(data->ptr)->addr,data->ptr);
                	}
                	else if(data->ptr->type ==TAIL)
                	{
                		_DBG("; Router ST; Flit Out ; addr %lld; 0x%x; Tail",static_cast<TailFlit*>(data->ptr)->addr,data->ptr);
                	}
#endif
#ifdef _DEBUG_ROUTER_HEAD
                	if(data->ptr->type ==HEAD)
                	{
                		_DBG("; Router ST; Flit Out ; addr %lld; 0x%x; Head sent_head=%s",
                				static_cast<HeadFlit*>(data->ptr)->addr,data->ptr,
                				(input_buffer_state[i].sent_head)?"true":"false");
                	}
#endif

                }
                else if(found_opfault)// output port fault
                {
                	if(input_buffer_state[i].is_resent)
                	{
                		//#define _DROPPKT
#ifdef _DROPPKT
                		if(f->type ==HEAD )// || f->is_single_flit_pkt )
                		{
                			stat_pkts_drop++;
                			_DBG("; Fault found during ST Drop Flit Head of %lld at port:%d",static_cast<HeadFlit*>(f)->addr, op);
                		}
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
                			input_buffer_state[i].output_port=-1;
                		}
                		send_credit_back(i); // send upstream router that one more credit in input_buffer
#else
                		if(f->type ==HEAD )// || f->is_single_flit_pkt )
                		{
                			// push_front and redo IB/RC
#ifdef _DEBUG_FI
                			_DBG("; ST Output Fault found with HeadFlit %lld 0x%x => Redo RC",input_buffer_state[i].address, f);
#endif			
                			if(f!=input_buffer_state[i].copy_hf)
                			{
                				in_buffers[ip].change_push_channel(ic);
                				in_buffers[ip].push_front(f);
                			}
                			input_buffer_state[i].clear_message = true;
                			if(in_buffers[ip].get_occupancy(ic) > 0)
                				input_buffer_state[i].pipe_stage = FULL;
                			else
                				input_buffer_state[i].pipe_stage = EMPTY;
                			//vc_alloc[op].push_back(oc);
                			vca.clear_winner(op,oc,ip,ic);
                			swa.clear_requestor(op, ip, oc);
                			input_buffer_state[i].possible_oports.clear();
                			input_buffer_state[i].possible_ovcs.clear();
                			input_buffer_state[i].output_port=-1;
                			input_buffer_state[i].remain_flits -=input_buffer_state[i].length;
                			assert(input_buffer_state[i].remain_flits == 0);
                			ticking = true;

                		}
                		if(f->type ==BODY || f->type == TAIL )
                		{
                			if(!input_buffer_state[i].is_pseudo_hf_created)
                			{
                				/* push this BodyFlit back to buffer */
                				in_buffers[ip].change_push_channel(ic);
                				in_buffers[ip].push_front(f);
                				/* create pseudo HeadFlit */
                				//HeadFlit* pshf = new HeadFlit();

                				/*
                            	pshf-> dst_address = input_buffer_state[i].copy_hf.dst_address;
                            	pshf-> src_address = input_buffer_state[i].copy_hf.src_address;
                            	pshf-> inport = input_buffer_state[i].copy_hf.inport;
                            	pshf-> avg_network_latency = input_buffer_state[i].copy_hf.avg_network_latency;
                            	pshf-> hop_count = input_buffer_state[i].copy_hf.hop_count;
                            	pshf-> vn = input_buffer_state[i].copy_hf.vn;
                            	pshf-> virtual_source = input_buffer_state[i].copy_hf.virtual_source;
                            	pshf-> path = input_buffer_state[i].copy_hf.path;
                            	pshf-> node_stamping = input_buffer_state[i].copy_hf.node_stamping;
                				 */
                				//*pshf = input_buffer_state[i].copy_hf;
                				//in_buffers[ip].push_front(pshf);

                				//HeadFlit* pshf = &(input_buffer_state[i].copy_hf);
                				HeadFlit* pshf = input_buffer_state[i].copy_hf;
                				//if (input_buffer_state[i].remain_flits != pshf->length-1) //if original pkt only hf sent out, just copy again hf
                				{
                					//pshf->src_address = address;
                					pshf->ps_length = input_buffer_state[i].remain_flits+1;
                				}
                				pshf->pseudo = true;
#ifdef _DEBUG_FI
                				_DBG("; Create Pseudo HeadFlit %lld; 0x%x",pshf->addr, pshf);
#endif
                				input_buffer_state[i].clear_message = true;
                				if(in_buffers[ip].get_occupancy(ic) > 0)
                					input_buffer_state[i].pipe_stage = FULL;
                				else
                					input_buffer_state[i].pipe_stage = EMPTY;
                				//vc_alloc[op].push_back(oc);
                				vca.clear_winner(op,oc,ip,ic);
                				swa.clear_requestor(op, ip, oc);
                				input_buffer_state[i].possible_oports.clear();
                				input_buffer_state[i].is_misrouting.clear();
                				input_buffer_state[i].possible_ovcs.clear();
                				input_buffer_state[i].output_port=-1;
                				input_buffer_state[i].remain_flits=0;
                				input_buffer_state[i].is_pseudo_hf_created = true;
                				input_buffer_state[i].sent_head = false;
                				stat_pshf_created ++;
                				ticking = true;


                				/*send psth to downstream router*/
                				TailFlit* ptf = new TailFlit();
                				ptf->type = TAIL;
                				ptf->addr = curr_pkt.address;
                				ptf->pseudo = true;
                				LinkArrivalData* data = new LinkArrivalData();
                				data->type = FLIT_ID;
                				data->vc = oc;
                				data->ptr = ptf;
                				IrisEvent* event = new IrisEvent();
                				event->type = LINK_ARRIVAL_EVENT;
                				event->event_data.push_back(data);
                				event->src_id = address;
                				event->dst_id = output_connections[op]->address;
                				event->vc =  data->vc;

                				Simulator::Schedule( Simulator::Now()+0.75,
                						&NetworkComponent::process_event,
                						static_cast<GenericLink*>(output_connections[op])->output_connection,event);
#ifdef _DEBUG_FI
_DBG("; Create Pseudo Tail Flit:%lld; 0x%x; ",curr_pkt.address,ptf);
#endif
//input_buffer_state[i].is_pseudo_tf_created = true;
stat_pstf_created ++;
downstream_credits[op][oc]--;

                			}
                			else // already split , drop rest
                			{
                				//#ifdef _DEBUG_PKT

                				if(f->type ==HEAD)
                				{
                					_DBG("; ERROR : Already Split, so drop ; addr %lld; Head; 0x%x",static_cast<HeadFlit*>(f)->addr,f);
                					exit(-1);
                				}
                				else if(f->type ==BODY)
                				{
                					_DBG("; Already Split, so drop ; addr %lld; 0x%x; Body",static_cast<BodyFlit*>(f)->addr,f);
                				}
                				else if(f->type ==TAIL)
                				{
                					_DBG("; Already Split, so drop ; addr %lld; 0x%x; Tail",static_cast<TailFlit*>(f)->addr,f);
                				}
                				//#endif
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
                					input_buffer_state[i].output_port=-1;

                					vca.clear_winner(op,oc,ip,ic);
                					swa.clear_requestor(op, ip, oc);
                					input_buffer_state[i].possible_oports.clear();
                					input_buffer_state[i].possible_ovcs.clear();
                					input_buffer_state[i].address=-1;
                					input_buffer_state[i].destination=-1;
                					input_buffer_state[i].remain_flits = 0;
                					input_buffer_state[i].is_pseudo_hf_created = false;
                					input_buffer_state[i].sent_head = false;
                				}
                				send_credit_back(i); // send upstream router that one more credit in input_buffer
                			}
                		}

#endif

                	}
                	else // push front the flit for retransmission
                	{
                		in_buffers[ip].change_push_channel(ic);
                		in_buffers[ip].push_front(f);
                		input_buffer_state[i].is_resent = true;
                		ticking = true;

                	}
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
RtPKTSPa::do_swa()
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

#ifdef _DEBUG_ROUTER
            	 _DBG("; SWA ; won ip%d-ic%d|op%d-oc%d ",ip,ic,op,oc);
#endif                 //	printf(" swa won %d-%d|%d-%d \n",ip,ic,op,oc);
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

    }

    return ;

}

void
RtPKTSPa::do_vca()
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

                // if requesting multiple out ports make sure to cancel them as
                // pkt will no longer be VCA_REQUESTED
                if(in_buffers.at(ip).get_occupancy(ic))
                {
#ifdef _DEBUG_ROUTER
               	    _DBG("; SWA ; REQ ; ip%d-ic%d|op%d-oc%d",ip,ic,op,oc);
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

            if(in_buffers.at(ip).get_occupancy(ic))
            {
#ifdef _DEBUG_ROUTER
         	    _DBG("; SWA ; REQ ; ip%d-ic%d|op%d-oc%d",ip,ic,op,oc);
#endif            //printf(" request swa %d-%d|%d-%d \n",ip,ic,op,oc);

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
RtPKTSPa::do_virtual_channel_allocation()
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
        request_op[op].push_back(i); // request_op[i] keeps all the outport request from channels[j](j=port*vcs+ic) to the port i
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
            {
            	oc = *itr;
            	if ( (downstream_credits[op][oc] > 0) && was_vca_active == false ) 
              {
#ifdef _DEBUG_ROUTER
            	 _DBG("; VCA ; COMP ; (ip_%d*vcs_%d+ic_%d=op_%d*vcs_%d+oc_%d)%d=%d",ip,vcs,ic,op,vcs,oc,ip*vcs+ic,op*vcs+oc);
#endif
        	// _DBG("; VCA COMP %d=%d",ip*vcs+ic,op*vcs+oc);

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

request_switch_allocation();
}

/* This is called from two places. First when head completes vca and requests
 * for swa. Second when flit goes out and msg requests for swa again as long
 * as there the tail goes out*/
void
RtPKTSPa::request_switch_allocation()
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
RtPKTSPa::handle_tick_event ( IrisEvent* e )
{
    ticking = false;
    //do_switch_traversal();
    do_st();
    //do_switch_allocation_m();
    do_swa();
    //do_virtual_channel_allocation();
    do_vca();

    /*!  Input buffering 
     *  Flits are pushed into the input buffer in the link arrival handler
     *  itself. To ensure the pipeline stages are executed in reverse pipe
     *  order IB is done here and all link_traversals have higher priority and get done before
     *  tick. Head/Body and Tail flits go thru the IB stage.*/
    do_input_buffering();

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

/*
            for (uint u=0;u<fault_outports.size();u++) // remove faulty ports in possible_oports
//            for (uint u=0;u<nb_faults;u++)
            {
            	for (uint v=0;v<input_buffer_state[i].possible_oports.size(); v++)
            	{
            		if(fault_outports[u]==input_buffer_state[i].possible_oports[v]) {
#ifdef _DEBUG_FI
            		_DBG("fault_outports[%d]=%d; input_buffer_state[%d].possible_oports[%d]=%d", u, fault_outports[u], i, v, input_buffer_state[i].possible_oports[v]);
#endif
            		input_buffer_state[i].possible_oports.erase(input_buffer_state[i].possible_oports.begin()+v);
            		}
            	}
            }
*/// local fault is taken into account at RC

            // do_fon
        	vector<uint> no_fault_neighbor;
        	no_fault_neighbor.resize(ports);
        	for (uint p=1; p<ports; p++)
        	{
        		uint ip=-1;
        		switch (p)
        		{
        		case 1:
        			ip=2;
        			break;
        		case 2:
        			ip=1;
        			break;
        		case 3:
        			ip=4;
        			break;
        		case 4:
        			ip=3;
        			break;
        		default:
        			cout<<"wrong output port"<<endl;
        			break;
        		}
        		if(neighbors[p]!= -1)
        		{
        			for(uint i=1;i<ports;i++)
        			{
        				if(i!=ip)
        					no_fault_neighbor[p] += *(fon[p][i]);// wrong for nodes at border
        			}
        		}
        	}
        	if ( input_buffer_state[i].possible_oports.size() > 1 ) // not no solve the pb where there is only on pop
        	{
        		// could have minimal / non-minimal pop
        		if (no_fault_neighbor[input_buffer_state[i].possible_oports[0]]>=2)
        		{

        		}
        	}

            if ( input_buffer_state[i].possible_oports.size() != 0 )
            {
            	/*if(0)
            	{
            		if (op== -1)
            			op = input_buffer_state[i].possible_oports[0];
            		else
            		{
            			uint temp=op;
            			op = input_buffer_state[i].possible_oports[0];
            			assert(temp==op);
            		}
            	}*/
            	//else
            	{
      /*      		for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
            		{
            			_DBG("possible_oports %d : %d", j, input_buffer_state[i].possible_oports.at(j));
            		}
            		for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
            		{
            			metrics.push_back(pair<uint,uint>(input_buffer_state[i].possible_oports.at(j), downstream_credits[input_buffer_state[i].possible_oports.at(j)][ic]));
            		}
            		/*for(uint j=0;j<metrics.size();j++)
            		{
            			_DBG("metrics : pair_%d %d %d", j, metrics.at(j).first,metrics.at(j).second);
            		}*/
/*
            		if (metrics.size()>1)
            			{
            			    sort_metrics();
            			    for(uint j=0;j<metrics.size();j++)
            			    {
            			    	_DBG("sorted metrics : pair_%d %d %d", j, metrics.at(j).first,metrics.at(j).second);
            			    }
            			}
            		metrics.clear();
*/

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
                		for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
                		{
                			uint temp_cg_metric=0;
                			uint lowlimit = 0;
                			uint highlimit = 0;
                			uint ratio=0;
                			if (input_buffer_state[i].vn == VN0)
                			{
                				lowlimit=0;
                				highlimit=vcs/2;
                			}
                			if (input_buffer_state[i].vn == VN1)
                			{
                				lowlimit=vcs/2;
                				highlimit=vcs;
                			}
                			if(input_buffer_state[i].is_misrouting.at(j)) ratio=1;
                			else ratio=0;
                			//for (uint k=0;k<vcs;k++)  // maybe to use va_alloc (the number of vc available)
                		    for (uint k=lowlimit;k<highlimit;k++)  // todo: virtual network
                			{
                				temp_cg_metric += (downstream_credits[input_buffer_state[i].possible_oports.at(j)][k])/(1+ratio);
                			}
                			metrics.push_back(pair<uint,uint>(input_buffer_state[i].possible_oports.at(j), temp_cg_metric));

                		}
                		if (metrics.size()>1)
                		{
                			sort_metrics_decrease();
#ifdef _DEBUG_METRIC
                			for(uint j=0;j<metrics.size();j++)
                			{
                				_DBG("; ip%d_ic%d sorted metrics BUFFER_SIZE: pair_%d op_%d cgm_%d", input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
                			}
#endif
                		}
                		if (op== -1)
                			op = metrics.front().first;
                		else
                		{
                			// todo: fixme RC should be done only once
                			uint temp=op;
                			op = metrics.front().first;
                			assert(temp==op);
                		}
                		metrics.clear();
                	}
/*                	else if (cg_metric == FLIT_REMAIN )
                	{
                		for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
                		{
                			//metrics.push_back(pair<uint,uint>(input_buffer_state[i].possible_oports.at(j), remain_flits[j]));
                			uint temp_op=input_buffer_state[i].possible_oports.at(j);
                			uint temp_cgm = 0; // congestion_metric
                    		uint ratio=0;
                			if(input_buffer_state[i].is_misrouting.at(j)) ratio=1;
                			else ratio=0;
                			for( uint l=0; l<(ports*vcs); l++)
                			{
                				if(input_buffer_state[l].output_port == temp_op)
                					temp_cgm += (input_buffer_state[l].remain_flits*(1+ratio));
                			}
                			temp_cgm += 1024*ratio;  // todo: a parameter to replace 16
                			metrics.push_back(pair<uint,uint>(temp_op, temp_cgm));
                		}

                		if (metrics.size()>1)
                		{
                			sort_metrics_increase();
#ifdef _DEBUG_METRIC
                			for(uint j=0;j<metrics.size();j++)
                			{
                				_DBG("; sorted metrics ip%d_ic%d FLIT_REMAIN: pair_%d op_%d cgm_%d",input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
                			}
#endif
                		}

                		if (op== -1)
                			op = metrics.front().first;
                		else
                		{
                			// todo: fixme RC should be done only once
                			uint temp=op;
                			op = metrics.front().first;
                			assert(temp==op);
                		}
                		metrics.clear();
                	}
*/
            		else if (cg_metric == FLIT_REMAIN )
                	{
                		for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
                		{
                			//metrics.push_back(pair<uint,uint>(input_buffer_state[i].possible_oports.at(j), remain_flits[j]));
                			uint temp_op=input_buffer_state[i].possible_oports.at(j);
                			uint temp_cgm = 0; // congestion_metric
                			if(!input_buffer_state[i].is_misrouting.at(j))
                			{
                				for( uint l=0; l<(ports*vcs); l++)
                				{
                					if(input_buffer_state[l].output_port == temp_op)
                						temp_cgm += (input_buffer_state[l].remain_flits);
                				}
                				metrics.push_back(pair<uint,uint>(temp_op, temp_cgm));
                			}
                		}

                		if (metrics.size()>=1)
                		{
                			sort_metrics_increase();
#ifdef _DEBUG_METRIC
                			for(uint j=0;j<metrics.size();j++)
                			{
                				_DBG("; sorted metrics ip%d_ic%d FLIT_REMAIN: pair_%d op_%d cgm_%d",input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
                			}
#endif
                			op = metrics.front().first;
                		}
                		else if (metrics.size()<1)
                		{
                			for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
                			{
                				//metrics.push_back(pair<uint,uint>(input_buffer_state[i].possible_oports.at(j), remain_flits[j]));
                				uint temp_op=input_buffer_state[i].possible_oports.at(j);
                				uint temp_cgm = 0; // congestion_metric
                				if(input_buffer_state[i].is_misrouting.at(j))
                				{
                					for( uint l=0; l<(ports*vcs); l++)
                					{
                						if(input_buffer_state[l].output_port == temp_op)
                							temp_cgm += (input_buffer_state[l].remain_flits);
                					}
                					metrics.push_back(pair<uint,uint>(temp_op, temp_cgm));
                				}
                			}
                			if (metrics.size()>=1)
                			{
                				sort_metrics_increase();
#ifdef _DEBUG_METRIC
                			for(uint j=0;j<metrics.size();j++)
                			{
                				_DBG("; sorted metrics ip%d_ic%d FLIT_REMAIN: pair_%d op_%d cgm_%d | non minimal",input_buffer_state[i].input_port,  input_buffer_state[i].input_channel, j, metrics.at(j).first, metrics.at(j).second);
                			}
#endif
                				op = metrics.front().first;
                			}
                			//op = metrics.front().first;
                		}
                		assert (op!=-1);
                		metrics.clear();
                	}

                	else if(cg_metric == VIRTUAL_CHANNEL ){
                		for(uint j=0;j< input_buffer_state[i].possible_oports.size();j++)
                		{
                			uint temp_cg_metric=0;
                			for (uint k=0;k<vcs;k++)  // maybe to use va_alloc (the number of vc available)
                			{
                				temp_cg_metric += downstream_credits[input_buffer_state[i].possible_oports.at(j)][k];
                			}
                			metrics.push_back(pair<uint,uint>(input_buffer_state[i].possible_oports.at(j), temp_cg_metric));

                		}
                		if (metrics.size()>1)
                		{
                			sort_metrics_decrease();
#ifdef _DEBUG_METRIC0
                			for(uint j=0;j<metrics.size();j++)
                			{
                				_DBG("sorted metrics VIRTUAL_CHANNEL: pair_%d %d %d", j, metrics.at(j).first,metrics.at(j).second);
                			}
#endif
                		}
                		if (op== -1)
                			op = metrics.front().first;
                		else
                		{
                			// todo: fixme RC should be done only once
                			uint temp=op;
                			op = metrics.front().first;
                			assert(temp==op);
                		}
                		metrics.clear();
                	}


            	}

            	//oc = input_buffer_state[i].possible_ovcs[0];
            	// todo: fixme optput vc allocation
            	if (oc== -1)
            		oc = ic;

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
        #ifdef _DEBUG_METRIC0
                	_DBG(";pkt %lld +3 ; remain_flits ; remain_flits[%d] Remain Flits = %d",input_buffer_state[i].address,op,remain_flits[op]);
        #endif
           		}
            }
            else  // drop pkt
            {
            	if( in_buffers[ip].get_occupancy(ic)>0 || input_buffer_state[i].is_pseudo_hf_created)
            	{
            		Flit* f;
            		if(input_buffer_state[i].is_pseudo_hf_created)
            			f = input_buffer_state[i].copy_hf;
            		else
            		{
            			in_buffers[ip].change_pull_channel(ic);
            			f=in_buffers[ip].pull();
            		}

                    //if(f->type ==HEAD && !(f->pseudo && f->type==HEAD && static_cast<HeadFlit*>(f)->src_address == address) )// || f->is_single_flit_pkt )
                    if(f->type ==HEAD && f->pseudo==false)//!(input_buffer_state[i].is_pseudo_hf_created ))
                    {
                    	stat_pkts_drop++;
#ifdef _DEBUG_FI
                    	_DBG("; OP Fault found during RC Drop Flit Head of %lld; 0x%x ", static_cast<HeadFlit*>(f)->addr,f);
#endif			
                    }
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
        	    	        input_buffer_state[i].output_port=-1;
        	    	        input_buffer_state[i].is_pseudo_hf_created = false;
        	    	        input_buffer_state[i].is_pseudo_tf_created = false; // why comment
        	    	        //delete input_buffer_state[i].copy_hf;// in case 2 times split in same node,delete copy_hf cause pb
                    }
                    //if(!(f->pseudo && f->type==HEAD && static_cast<HeadFlit*>(f)->src_address == address))
                    // ? if(f->type ==HEAD && !(input_buffer_state[i].is_pseudo_hf_created ))
                    if(!(input_buffer_state[i].is_pseudo_hf_created ))
                    	{
                    	send_credit_back(i); // send upstream router that one more credit in input_buffer

                    	delete f;
#ifdef _DEBUG_FI
                _DBG("; RC ; Fail ; ip_%d*vcs_%d+ic_%d(=%d) ft=%d",ip,vcs,ic,ip*vcs+ic, f->type);
        if(f->type ==HEAD)
        {
        	_DBG("; Router ; Drop Flit HEAD ; addr %lld; 0x%x Head;",static_cast<HeadFlit*>(f)->addr,f);
        }
        else if(f->type ==BODY)
        {
        	_DBG("; Router ; Drop Flit BODY ; addr %lld; 0x%x Body;",static_cast<BodyFlit*>(f)->addr,f);
        }
        else if(f->type ==TAIL)
        {
        	_DBG("; Router ; Drop Flit TAIL ; addr %lld; 0x%x Tail;",static_cast<TailFlit*>(f)->addr,f);
        }
#endif

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

    delete e; e=NULL;
    return;

}		/* -----  end of function RtPKTSPa::handle_input_arbitration_event  ----- */

string
RtPKTSPa::toString() const
{
    stringstream str;
    str << "RtPKTSPa"
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
RtPKTSPa::send_credit_back(uint i)
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
RtPKTSPa::get_sum_bf(uint port)
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
RtPKTSPa::nop_slect()
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
                    if( static_cast<RtPKTSPa*>(static_cast<GenericLink*>(output_connections[porta])->output_connection)->neighbors[rc_port] != NULL )  // should check if exits
                    {
                    	a_aoc.push_back(rc_port);
                     temp=static_cast<RtPKTSPa*>(static_cast<GenericLink*>(output_connections[porta])->output_connection)->get_sum_bf(rc_port);
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
                    if( static_cast<RtPKTSPa*>(static_cast<GenericLink*>(output_connections[portb])->output_connection)->neighbors[rc_port] != NULL )  // should check if exits
                    {
                        b_aoc.push_back(rc_port);
                        temp=static_cast<RtPKTSPa*>(static_cast<GenericLink*>(output_connections[portb])->output_connection)->get_sum_bf(rc_port);
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
            //free(hf);

        }
}


#endif   /* ----- #ifndef _routerVcMP_cc_INC  ----- */

