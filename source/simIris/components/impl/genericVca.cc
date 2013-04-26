/*
 * =====================================================================================
//       Filename:  genericVca.cc
 * =====================================================================================
 */
#ifndef  _GENERICVCA_CC_INC
#define  _GENERICVCA_CC_INC

#include	"genericVca.h"

GenericVca::GenericVca ()
{
    //resize();
    name = "vca";
    return ;
}

GenericVca::~GenericVca ()
{

}

void
GenericVca::resize( uint p, uint v )
{
	ports = p;
	vcs = v;
    /* Clean out previous config */
    for ( uint i=0; i<requesting_inputs.size(); ++i)
        requesting_inputs.at(i).clear();

    for ( uint i=0; i<ovc_tokens.size(); ++i)
        ovc_tokens.at(i).clear();

    for ( uint i=0; i<last_winner.size(); ++i)
        last_winner.at(i).clear();

    for ( uint i=0; i<current_winners.size(); ++i)
        current_winners.at(i).clear();

    requesting_inputs.clear();
    ovc_tokens.clear();
    last_winner.clear();
    current_winners.clear();

    /* Reset the configuration */
    requesting_inputs.resize( ports);
    current_winners.resize( ports);
    last_winner.resize( ports);
    ovc_tokens.resize( ports);

    // Init the tokens to all available output  vcs
    for ( std::vector< std::vector<uint> >::iterator it=ovc_tokens.begin();
          it != ovc_tokens.end(); ++it)
        for( uint token=0; token< vcs; ++token)
            it->push_back(token);

    /*
     * for ( std::vector< std::vector< Vca_unit> >::iterator it=current_winners.begin();
          it!=current_winners.end(); ++it)
        it->resize( ports* vcs);
        */

    for ( std::vector< std::vector< Vca_unit> >::iterator it=requesting_inputs.begin();
          it!=requesting_inputs.end(); ++it)
        it->resize( ports* vcs);

    for ( std::vector< std::vector< uint> >::iterator it=last_winner.begin();
          it!=last_winner.end(); ++it)
        it->resize( vcs);

    return ;

}

// Head flits in the IB can request for an out vc every cycle till it is
// assigned an output channel by the arbiter. If there are multiple head flits
// queued in the input buffer and the one at the head of the queue is assigned a
// channel. The one next in line can request but will not be allowed to ask for
// the grant till the tail flit of the previous packet leaves the router. (The
// freeing of the request in the matrix can be done when the tail flit is pushed
// into the output buffer for output buffered switches)
// TODO: Modify for output buffered routers
// The function returns true if the requestor was allowed to set his request in
// the requesting matrix/ false otherwise to say sorry request next cycle.
bool
GenericVca::request ( uint op, uint ovc, uint ip, uint ivc )
{
//    Vca_unit& tmp = requesting_inputs.at(op).at(ip+ ports*ivc);// ivc*ports + ip ( SST )
    Vca_unit& tmp = requesting_inputs.at(op).at(ip*vcs + ivc);// ip*vcs + ivc ( Manifold )
    if ( tmp.is_valid == false )
    {
        tmp.is_valid = true;
        tmp.in_port = ip;
        tmp.in_vc = ivc;
        tmp.out_port = op;
        tmp.out_vc = ovc;
    }

    return tmp.is_valid;

}		/* -----  end of method GenericVca::request  ----- */

uint
GenericVca::no_requestors ( uint op )
{
    uint no = 0;
    for ( uint j=0; j< ports* vcs; j++)
        if ( requesting_inputs.at(op).at(j).is_valid )
            no++;

    return no;
}		/* -----  end of method GenericVca::no_requestors  ----- */

void
GenericVca::pick_winner ( void ) // output: current_winner[p]; yi:kind of roundrobin, to add fcfs
{
    if ( !is_empty() )
    {
        for ( uint p=0; p< ports; p++)
            for ( uint v=0; v< vcs; v++)
            {
                // check if this vc is available in the token list
                std::vector<uint>::iterator it;
                it = find ( ovc_tokens.at(p).begin(), ovc_tokens.at(p).end(), v);
                if ( it != ovc_tokens.at(p).end() ) // if found one vc token of port p
                {
                    /*  For DEBUG
                        printf("\nIter for port %d vc %d tokens:-",i,j);
                        for ( uint ss=0; ss<ovc_tokens.size(); ss++)
                        printf(" %d|",ovc_tokens[i][ss]);
                        printf(" reqs:-");
                        for ( uint ss=0; ss< ports* vcs; ss++)
                        printf(" %d-%d",(int)requesting_inputs[i][ss].is_valid, requesting_inputs[i][ss].out_vc);
                     */

                    uint start_index=last_winner.at(p).at(v);  // round-robin
                    bool found_winner = false;
                    for ( uint k=start_index; k< ports* vcs; k++)
                        if ( requesting_inputs.at(p).at(k).is_valid 
                             && requesting_inputs.at(p).at(k).out_vc == v )
                        {
                            last_winner.at(p).at(v) = k;
                            found_winner = true;
                            break;
                        }

                    if ( !found_winner )
                    {
                        for ( uint k=0; k<start_index; k++)
                            if ( requesting_inputs.at(p).at(k).is_valid 
                                 && requesting_inputs.at(p).at(k).out_vc == v )
                            {
                                last_winner.at(p).at(v) = k;
                                found_winner = true;
                                break;
                            }

                    }

                    if ( found_winner )
                    {
                        // assign the channel and remove it from the token list
                        ovc_tokens.at(p).erase(it);

                        Vca_unit winner;
                        winner.is_valid = true;
                        winner.out_vc = v;
                        winner.out_port = p;
                        //winner.in_port = (int)(last_winner.at(p).at(v)% ports); // ivc*ports + ip ( SST )
                        //winner.in_vc= (int)(last_winner.at(p).at(v)/ ports);  // ivc*ports + ip ( SST )
                        //winner.in_port = (int)(last_winner.at(p).at(v) % vcs); // ip*vcs + ivc ( Manifold )
                        //winner.in_vc= (int)(last_winner.at(p).at(v) / vcs);  // ip*vcs + ivc ( Manifold )
                        winner.in_port = (int)(last_winner.at(p).at(v) / vcs); // ip*vcs + ivc ( Manifold )
                        winner.in_vc= (int)(last_winner.at(p).at(v) % vcs);  // ip*vcs + ivc ( Manifold )

                        current_winners.at(p).push_back(winner);
                    }
                }
            }
    }
    return ;
}		/* -----  end of method GenericVca::pick_winner  ----- */
void
GenericVca::change_owner( uint pip, uint pic, uint op, uint oc, uint nip, uint nic )//pip:previous ip, nip:new ip
{
    Vca_unit& tmp = requesting_inputs.at(op).at(pip*vcs+pic);
    tmp.is_valid =false;
    tmp.out_port=-1;
    tmp.out_vc=-1;
    tmp.in_port=-1;
    tmp.in_vc=-1;
    std::vector<Vca_unit> ::iterator it;
    for( it=current_winners.at(op).begin(); it!=current_winners.at(op).end(); it++)
    {
    	if (it->in_port == pip && it->in_vc == pic
    			&& it->out_port == op && it->out_vc == oc )
    	{
    		current_winners.at(op).erase(it);
    		break;
    	}
    }
    tmp=requesting_inputs.at(op).at(nip*vcs+nic);
    //tmp=&(requesting_inputs.at(op).at(nip*vcs+nic)) ;
    //tmp=static_cast<Vca_unit&>(requesting_inputs.at(op).at(nip*vcs+nic)) ;

    Vca_unit& ntmp = requesting_inputs.at(op).at(nip*vcs+nic);
    ntmp.is_valid =true;
    ntmp.out_port=op;
    ntmp.out_vc=oc;
    ntmp.in_port=nip;
    ntmp.in_vc=nic;
    current_winners.at(op).push_back(ntmp);

}
void
GenericVca::clear_winner ( uint op, uint ovc, uint ip, uint ivc )
{
    //Vca_unit& tmp = requesting_inputs.at(op).at(ip+ ports*ivc);
    Vca_unit& tmp = requesting_inputs.at(op).at(ip*vcs+ivc);
    tmp.is_valid =false;
    tmp.out_port=-1;
    tmp.out_vc=-1;
    tmp.in_port=-1;
    tmp.in_vc=-1;

    // Erase the winner from the current winners list
    /* DEBUG
       printf (" *********** Clear for op %d-%d|%d-%d\n", ip,ivc,op,ovc);
       assert(current_winners.size() > op);
       for ( uint i=0; i<current_winners.size(); i++){
       printf(" No of winners for op %d is %d\t", i, (int)current_winners.at(i).size());
       std::vector<Vca_unit> ::iterator it;
       for( it=current_winners.at(i).begin(); it!=current_winners.at(i).end(); ++it)
       {
       printf(" %d-%d|%d-%d ", it->in_port, it->in_vc, it->out_port, it->out_vc);
       }
       printf("\n");
       }
     */

    std::vector<Vca_unit> ::iterator it;
    for( it=current_winners.at(op).begin(); it!=current_winners.at(op).end(); it++)
    {
        if (it->in_port == ip && it->in_vc == ivc
            && it->out_port == op && it->out_vc == ovc )
        {
            current_winners.at(op).erase(it);
            break;
        }
    }

    // Return the token to the pool
    ovc_tokens.at(op).push_back(ovc);
    return ;
}		/* -----  end of method GenericVca::clear_winner  ----- */

#endif   /* ----- #ifndef _GENERICVCA_CC_INC  ----- */

