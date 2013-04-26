/*!
 * =====================================================================================
 *
 *       Filename:  genericRC.cc
 *
 *    Description:  All routing is here for time time being. Every node knows
 *    its position in the grid and has a table as follows
 *      node_id         grid_x_position         grid_y_position
 *      node0                   0               0
 *      node1                   0               1
 *      .. and so on
 *      
 *
 *        Version:  1.0
 *        Created:  02/19/2010 12:13:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */
#ifndef  _genericaddressdecoder_cc_INC
#define  _genericaddressdecoder_cc_INC

#include    <algorithm>
#include    <vector>
#include	"genericRC.h"

//#define _DEBUG
//#define _DEEP_DEBUG
//#define _DEBUG_VN

using namespace std;

vector<uint> GenericRC::Address::last_aport;
uint GenericRC::missrouting_counter=0;

enum HIERACHY_CONNER {WS, CS, ES, EN, CN, WN};
/*
uint dws[]={1,3,4,2};
uint dwn[]={1,4,3,2};

uint dcs[]={4,1,3,2};
uint dcn[]={1,4,3,2};

uint des[]={2,3,4,1};
uint den[]={2,4,3,1};

uint de[]={2};
uint dw[]={1};
 */
uint dws[]={1,3,2};
uint dwn[]={1,4,2};

uint dcs[]={3,2,1};
uint dcn[]={4,1,2};

uint des[]={2,3,1};
uint den[]={2,4,1};

uint de[]={2};
uint dw[]={1};


GenericRC::GenericRC()
{
	name = "AddrDecoder";
	node_ip = -1;
	address = 0;
	// initialize Address::last_aport
}

vector<uint>
GenericRC::hierachy_out(uint dst)
{
	uint dest = dst;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	vector<uint> output;
	int deltax= destx - myx;
	int deltay= desty - myy;
	if(deltax < 0 && deltay < 0)       output.assign(dws, dws+sizeof(dws)/sizeof(uint));
	else if(deltax > 0 && deltay < 0)  output.assign(des, des+sizeof(des)/sizeof(uint));
	else if(deltax > 0 && deltay > 0)  output.assign(den, den+sizeof(den)/sizeof(uint));
	else if(deltax < 0 && deltay > 0)  output.assign(dwn, dwn+sizeof(dwn)/sizeof(uint));
	else if(deltax ==0 && deltay > 0)  output.assign(dcn, dcn+sizeof(dcn)/sizeof(uint));
	else if(deltax ==0 && deltay < 0)  output.assign(dcs, dcs+sizeof(dcs)/sizeof(uint));
	else if(deltax > 0 && deltay ==0)  output.assign(de , de +sizeof(de)/sizeof(uint));
	else if(deltax < 0 && deltay ==0)  output.assign(dw , dw+sizeof(dw)/sizeof(uint));

	return output;
}
void
GenericRC::route_hierachy(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];


	if (dest==node_ip)
	{
		possible_out_ports.push_back(0);
		return;
	}

	else if (possible_out_ports.size()) {
		_DBG("possible_out_ports.size() = %d ", possible_out_ports.size());
		exit(0);
	}
	else possible_out_ports=hierachy_out(dest);
	return;
}

uint
GenericRC::route_x_y(uint dest)
{
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	if ( myx == destx && myy == desty )
		oport = 0;
	else if ( myx ==  destx )
	{
		if( desty < myy )
			oport = 3;
		else
			oport = 4;
	}
	else
	{
		if( destx < myx )
			oport = 1;
		else
			oport = 2;
	}

#ifdef _DEBUG
	_DBG(" Route HEAD dest: %d oport: %d ", dest, oport);
	_DBG(" addr: %d myid_x: %d myid_y: %d destid_x: %d destid_y: %d", node_ip, myx, myy, destx, desty);
#endif
	possible_out_ports.push_back(oport);
	return oport;
}
void
GenericRC::route_dy_xy(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	/*
        if ( myx == destx && myy == desty)
        {
        	possible_out_ports.push_back(0);
        	return;
        }
        else if ( myx ==  destx && myy != desty)
        {
        	if( desty < myy )
            	possible_out_ports.push_back(3);
            else if ( desty > myy )
            	possible_out_ports.push_back(4);
        	return;
        }
        else if ( myx !=  destx && myy == desty)
        {
        	if( destx < myx )
            	possible_out_ports.push_back(1);
            else if ( destx > myx )
            	possible_out_ports.push_back(2);
        	return;
        }
        else if ( myx <  destx && myy != desty )
        {
            	possible_out_ports.push_back(2);
            	if ( myy <  desty)
            		possible_out_ports.push_back(4);
            	else if ( myy >  desty)
                	possible_out_ports.push_back(3);
            	return;
        }
        else if ( myx >  destx && myy != desty)
        {
        	possible_out_ports.push_back(1);
        	if ( myy <  desty)
        		possible_out_ports.push_back(4);
        	else if ( myy >  desty)
            	possible_out_ports.push_back(3);
        	return;
        }
	 */
	if ( myx == destx)
	{
		if ( myy == desty)
			possible_out_ports.push_back(0);
		else if ( myy > desty)
			possible_out_ports.push_back(3);
		else if ( myy < desty)
			possible_out_ports.push_back(4);
		return;
	}
	else if ( myx <  destx )
	{
		if ( myy == desty)
			possible_out_ports.push_back(2);
		else if ( myy > desty)
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(3);
		}
		else if ( myy < desty)
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(4);
		}
		return;
	}
	else if ( myx >  destx )
	{
		if ( myy == desty)
			possible_out_ports.push_back(1);
		else if ( myy > desty)
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(3);
		}
		else if ( myy < desty)
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(4);
		}
		return;
	}
	else
	{
		_DBG(" ERROR: Some condition not covered destx: %d , desty: %d myx: %d , myy: %d", destx, desty, myx, myy);
	}

	return;
}



void
GenericRC::route_torus(HeadFlit* hf)
{
	cout << " Torus routing not supported yet " << endl;
	exit(1);
}
void
GenericRC::route_ring(HeadFlit* hf)
{
	cout << " Rings not supported yet " << endl;
	exit(1);
}

void
GenericRC::route_negative_first(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	if( deltax == 0 && deltay == 0 )
	{
		//        _DBG_NOARG(" Reached dest!" );
		possible_out_ports.push_back(0);
		return;
	}

	if ( deltax < 0 && deltay < 0 )
	{
		//        _DBG( " Both negative by x:%d y:%d", deltax, deltay );
		possible_out_ports.push_back(1);
		possible_out_ports.push_back(3);
		return;
	}
	else if ( deltax < 0 )
	{
		//        _DBG( " x negative by %d ", deltax);
		possible_out_ports.push_back(1);
		return;
	}
	else if ( deltay < 0 )
	{
		//        _DBG( " y negative by %d ", deltay );
		possible_out_ports.push_back(3);
		return;
	}
	else if (deltax > 0 && deltay > 0 )
	{
		//        _DBG( " Both positive by x:%d y;%d", deltax, deltay );
		possible_out_ports.push_back(2);
		possible_out_ports.push_back(4);
		return;
	}
	else if ( deltax == 0)
	{
		possible_out_ports.push_back(4);
		return;
	}
	else if( deltay == 0)
	{
		possible_out_ports.push_back(2);
		return;
	}
	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
	}
	return;
}

void
GenericRC::route_west_first(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}

	if ( deltax < 0 )
	{
		possible_out_ports.push_back(1);
		return;
	}
	else if ( deltax > 0 )
	{
		if ( deltay < 0)
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(3);
			return;
		}
		else if ( deltay > 0)
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(4);
			return;
		}
		else
		{
			possible_out_ports.push_back(2);
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay < 0)
		{
			possible_out_ports.push_back(3);
			return;
		}
		else if ( deltay > 0)
		{
			possible_out_ports.push_back(4);
			return;
		}
		else
		{
			cerr << "\nERROR should have exited earlier " << endl;
		}
	}
	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
	}
	return;
}
void
GenericRC::route_vn_south_last(HeadFlit* hf, vector<uint> nbs)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	bool no_minimal = false;
#ifdef _DEBUG_VN
	_DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
#endif
	assert (hf->vn == VN0);
	//assert (deltay >= 0 );
	for (uint i=0;i<nbs.size();i++)
	{
		if(dest == nbs[i])
		{
			possible_out_ports.push_back(i);
			is_misrouting.push_back(false);
			return;
		}
	}
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		is_misrouting.push_back(false);
		return;
	}
	if ( deltax < 0 )
	{
		if ( deltay < 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			else
			{
				if(nbs[3]!=-1)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}

			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else if ( deltay > 0 )
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4);
				is_misrouting.push_back(false);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay < 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			else
			{
				if(nbs[3]!=-1)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}

			return;
		}
		else if ( deltay > 0)
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay < 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[3]!=-1)
			{
				possible_out_ports.clear();
				is_misrouting.clear();
				assert(hf->virtual_source == NOVS);
				hf->virtual_source = VSSET;
				possible_out_ports.push_back(0);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}

			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay > 0)
		{
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else
		{
			cerr << "\nERROR should have exited earlier " << endl;
		}
	}

	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#ifdef _DEBUG_VN
#endif
	}
	/*if(no_minimal)
	{
		possible_out_ports.clear();
		is_misrouting.clear();
		assert(hf->virtual_source == NOVS);
		hf->virtual_source = VSSET;
		possible_out_ports.push_back(0);
		is_misrouting.push_back(false);
	}*/
	return;
}
void
GenericRC::route_vn_south_last(HeadFlit* hf, vector<uint> nbs, vector<uint> no_fon)
{

	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	bool no_minimal = false;

	uint nb_min=0;
	uint nb_nomin=0;

#ifdef _DEBUG_VN
	_DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
#endif
	assert (hf->vn == VN0);
	//assert (deltay >= 0 );
	// if found dst in neighbors
	for (uint i=0;i<nbs.size();i++)
	{
		if(dest == nbs[i])
		{
			possible_out_ports.push_back(i);
			is_misrouting.push_back(false);
			return;
		}
	}
	// included in above
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		is_misrouting.push_back(false);
		return;
	}
	// remove those neighbors which is a dead end
	for(uint i=1;i<ports;i++)
	{
		if (no_fon[i] == 3)
			nbs[i]= -1;
	}
	if ( deltax < 0 )
	{
		if ( deltay < 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			else
			{
				if(nbs[3]!=-1)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}

			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else if ( deltay > 0 )
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4);
				is_misrouting.push_back(false);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay < 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			else
			{
				if(nbs[3]!=-1)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}

			return;
		}
		else if ( deltay > 0)
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
				nb_min ++;
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4);
				is_misrouting.push_back(false);
				nb_min ++;
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
				nb_nomin ++;
			}
			/*if(nb_min > 1)
			{
				if(no_fon[2]>no_fon[4])
					//remove 2
				{

				}
			}*/
			return;
		}
		else // deltay == 0
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay < 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[3]!=-1)
			{
				possible_out_ports.clear();
				is_misrouting.clear();
				assert(hf->virtual_source == NOVS);
				hf->virtual_source = VSSET;
				possible_out_ports.push_back(0);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4); // miss routing
				is_misrouting.push_back(true);
			}

			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay > 0)
		{
			if(nbs[4]!=-1){
				possible_out_ports.push_back(4);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else
		{
			cerr << "\nERROR should have exited earlier " << endl;
		}
	}

	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#ifdef _DEBUG_VN
#endif
	}
	/*if(no_minimal)
	{
		possible_out_ports.clear();
		is_misrouting.clear();
		assert(hf->virtual_source == NOVS);
		hf->virtual_source = VSSET;
		possible_out_ports.push_back(0);
		is_misrouting.push_back(false);
	}*/
	return;

}
void
GenericRC::route_vn_north_last(HeadFlit* hf, vector<uint> nbs)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	bool no_minimal = false;
#ifdef _DEBUG_VN
	_DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
#endif

	assert (hf->vn == VN1);
	//assert (deltay <= 0 );
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		is_misrouting.push_back(false);
		return;
	}
	if ( deltax < 0 )
	{
		if ( deltay > 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			//_DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
#endif
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			else
			{
				if(nbs[4]!=-1)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}
			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay < 0 )
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3);
				is_misrouting.push_back(false);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay > 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			//_DBG(" In RC from %d to %d at %d at VN%d", hf->src_address, hf->dst_address, node_ip, hf->vn);
#endif
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			else
			{
				if(nbs[4]!=-1)
				{
									possible_out_ports.clear();
									is_misrouting.clear();
									assert(hf->virtual_source == NOVS);
									hf->virtual_source = VSSET;
									possible_out_ports.push_back(0);
									is_misrouting.push_back(false);
								}
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}
			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay < 0)
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay > 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[4]!=-1){
							possible_out_ports.clear();
							is_misrouting.clear();
							assert(hf->virtual_source == NOVS);
							hf->virtual_source = VSSET;
							possible_out_ports.push_back(0);
							is_misrouting.push_back(false);
						}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}

			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay < 0)
		{
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3);
				is_misrouting.push_back(false);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else
		{
#ifdef _DEBUG_VN
			cerr << "\nERROR should have exited earlier " << endl;
#endif
		}
	}

	else
	{
#ifdef _DEBUG_VN
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
	}

	return;
}
void
GenericRC::route_vn_north_last(HeadFlit* hf, vector<uint> nbs,vector<uint> no_fon)
{

	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	bool no_minimal = false;
#ifdef _DEBUG_VN
	_DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
#endif

	uint nb_min=0;
	uint nb_nomin=0;

	assert (hf->vn == VN1);
	//assert (deltay <= 0 );
	// if found dst in neighbors
	for (uint i=0;i<nbs.size();i++)
	{
		if(dest == nbs[i])
		{
			possible_out_ports.push_back(i);
			is_misrouting.push_back(false);
			return;
		}
	}
	// included in above
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		is_misrouting.push_back(false);
		return;
	}
	// remove those neighbors which is a dead end
	for(uint i=1;i<ports;i++)
	{
		if (no_fon[i] == 3)
			nbs[i]= -1;
	}
	if ( deltax < 0 )
	{
		if ( deltay > 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			//_DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
#endif
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);

			}
			else
			{
				if(nbs[4]!=-1)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);

				}
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);

			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);

			}
			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay < 0 )
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3);
				is_misrouting.push_back(false);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
				nb_nomin ++;
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);

			}
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay > 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			//_DBG(" In RC from %d to %d at %d at VN%d", hf->src_address, hf->dst_address, node_ip, hf->vn);
#endif
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
				nb_min ++;
			}
			else
			{
				if(nbs[4]!=-1)
				{
									possible_out_ports.clear();
									is_misrouting.clear();
									assert(hf->virtual_source == NOVS);
									hf->virtual_source = VSSET;
									possible_out_ports.push_back(0);
									is_misrouting.push_back(false);
									nb_min ++;
								}
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
				nb_nomin ++;
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
				nb_nomin ++;
			}
			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay < 0)
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3);
				is_misrouting.push_back(false);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else // deltay == 0
		{
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2);
				is_misrouting.push_back(false);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay > 0)
		{
#ifdef _DEBUG_VN
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
			if(nbs[4]!=-1){
							possible_out_ports.clear();
							is_misrouting.clear();
							assert(hf->virtual_source == NOVS);
							hf->virtual_source = VSSET;
							possible_out_ports.push_back(0);
							is_misrouting.push_back(false);
						}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3); // miss routing
				is_misrouting.push_back(true);
			}

			/*if(no_minimal)
				{
					possible_out_ports.clear();
					is_misrouting.clear();
					assert(hf->virtual_source == NOVS);
					hf->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}*/
			return;
		}
		else if ( deltay < 0)
		{
			if(nbs[3]!=-1){
				possible_out_ports.push_back(3);
				is_misrouting.push_back(false);
			}
			if(nbs[2]!=-1){
				possible_out_ports.push_back(2); // miss routing
				is_misrouting.push_back(true);
			}
			if(nbs[1]!=-1){
				possible_out_ports.push_back(1); // miss routing
				is_misrouting.push_back(true);
			}
			return;
		}
		else
		{
#ifdef _DEBUG_VN
			cerr << "\nERROR should have exited earlier " << endl;
#endif
		}
	}

	else
	{
#ifdef _DEBUG_VN
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
#endif
	}

	return;

}
void
GenericRC::route_vn_south_last(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	assert (hf->vn == VN0);
	assert (deltay >= 0 );
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}
	if ( deltax < 0 )
	{
		if ( deltay < 0)
		{
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			return;
		}
		else if ( deltay > 0 )
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(4);
			possible_out_ports.push_back(2); // miss routing
			return;
		}
		else // deltay==0
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(4); // miss routing
			possible_out_ports.push_back(2); // miss routing
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay < 0)
		{
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			return;        }
		else if ( deltay > 0)
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(4);
			possible_out_ports.push_back(1); // miss routing
			return;
		}
		else // deltay==0
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(4); // miss routing
			possible_out_ports.push_back(1); // miss routing
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay < 0)
		{
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			return;
		}
		else if ( deltay > 0)
		{
			possible_out_ports.push_back(4);
			possible_out_ports.push_back(1); // miss routing
			possible_out_ports.push_back(2); // miss routing
			return;
		}
		else
		{
			cerr << "\nERROR should have exited earlier " << endl;
		}
	}

	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
	}
	return;
}

void
GenericRC::route_vn_north_last(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	//    _DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
	assert (hf->vn == VN1);
	assert (deltay <= 0 );
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}
	if ( deltax < 0 )
	{
		if ( deltay > 0)
		{
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			//       	 _DBG(" In RC from %d pkt %lld to %d at %d at VN%d", hf->src_address, hf->addr, dest, node_ip, hf->vn);
			return;
		}
		else if ( deltay < 0 )
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(3);
			possible_out_ports.push_back(2); // miss routing
			return;
		}
		else
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(3); // miss routing
			possible_out_ports.push_back(2); // miss routing
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay > 0)
		{
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			//       	 _DBG(" In RC from %d to %d at %d at VN%d", hf->src_address, hf->dst_address, node_ip, hf->vn);
			return;
		}
		else if ( deltay < 0)
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(3);
			possible_out_ports.push_back(1); // miss routing
			return;
		}
		else
		{
			possible_out_ports.push_back(2);
			possible_out_ports.push_back(3); // miss routing
			possible_out_ports.push_back(1); // miss routing
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay > 0)
		{
			_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
			return;
		}
		else if ( deltay < 0)
		{
			possible_out_ports.push_back(3);
			possible_out_ports.push_back(2); // miss routing
			possible_out_ports.push_back(1); // miss routing
			return;
		}
		else
		{
			cerr << "\nERROR should have exited earlier " << endl;
		}
	}

	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
	}
	return;
}
void
GenericRC::route_north_last(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}
	if ( deltax < 0 )
	{
		if ( deltay < 0)
		{
			possible_out_ports.push_back(1);
			return;
		}
		else if ( deltay > 0 )
		{
			possible_out_ports.push_back(1);
			possible_out_ports.push_back(4);
			return;
		}
		else
		{
			possible_out_ports.push_back(1);
			return;
		}
	}
	else if ( deltax > 0 )
	{
		if ( deltay < 0)
		{
			possible_out_ports.push_back(2);
			return;
		}
		else if ( deltay > 0)
		{
			possible_out_ports.push_back(4);
			possible_out_ports.push_back(2);
			return;
		}
		else
		{
			possible_out_ports.push_back(2);
			return;
		}
	}
	else if ( deltax == 0 )
	{
		if ( deltay < 0)
		{
			possible_out_ports.push_back(3);
			return;
		}
		else if ( deltay > 0)
		{
			possible_out_ports.push_back(4);
			return;
		}
		else
		{
			cerr << "\nERROR should have exited earlier " << endl;
		}
	}

	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
	}
	return;
}

void
GenericRC::route_north_last_non_minimal(HeadFlit* hf)
{
	uint dest = hf->dst_address;
	uint oport = -1;
	uint myx=-1, destx=-1, myy =-1, desty=-1;
	myx = grid_xloc[node_ip];
	myy = grid_yloc[node_ip];
	destx = grid_xloc[ dest ];
	desty = grid_yloc[ dest ];
	int deltax = destx - myx;
	int deltay = desty - myy;
	bool is_fringe_router = false;
	if ( node_ip%grid_size == 0 || (node_ip+1)%grid_size == 0
			|| (node_ip+grid_size) > no_nodes || ( node_ip-grid_size)<0)
		is_fringe_router = true;

	/* Allow all paths to fringe routers. However fringe routers follow DOR */
	if( is_fringe_router )
	{
		possible_out_ports.push_back(route_x_y(hf->dst_address));
		return;
	}

	if( deltax == 0 && deltay == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}
	else if( hf->inport == 4 )
	{
		possible_out_ports.push_back(3);
		return;
	}
	if ( deltax != 0 || (deltax == 0 && deltay > 0)  )
	{
		/*
        bool dest_on_fringe = false;
        if ( dest%grid_size == 0 || (dest+1)%grid_size == 0 
             || (dest+grid_size) > no_nodes || ( dest-grid_size)<0)
             dest_on_fringe = true;

        if( dest_on_fringe )
        {
            possible_out_ports.push_back(route_x_y(hf->dst_address));
            return;
        }
		 * */
		//        cout << " Cannot add 3 inport:" << hf->inport <<endl;
		if( hf->inport != 1 )
		{
			possible_out_ports.push_back(1);
			//        cout << " :Add 1" << endl;
		}
		if( hf->inport != 2)
		{
			possible_out_ports.push_back(2);
			//        cout << " :Add 2" << endl;
		}
		if( hf->inport != 4)
		{
			possible_out_ports.push_back(4);
			//        cout << " :Add 4" << endl;
		}
		return;
	}
	else if ( deltax == 0 && deltay < 0)
	{
		possible_out_ports.push_back(3);
		//        cout << " Can add 3" << endl;
		/*
        for ( uint j=1; j<5; j++)
            if ( j != hf->inport )
                possible_out_ports.push_back(j);
		 */
		return;
	}
	else
	{
		_DBG(" ERROR: Some condition not covered deltax: %d , deltay:%d", deltax, deltay);
	}
	return;
}

void
GenericRC::route_odd_even(HeadFlit* hf)
{
	uint src = hf->src_address;
	uint dest = hf->dst_address;
	uint oport = -1;
	uint c0=-1, d0=-1, c1=-1, d1=-1, s0=-1, s1=-1;
	c0 = grid_xloc[node_ip];
	c1 = grid_yloc[node_ip];
	d0 = grid_xloc[ dest ];
	d1 = grid_yloc[ dest ];
	s0 = grid_xloc[ src ];
	s1 = grid_yloc[ src ];
	int e0 = d0-c0;
	int e1 = d1-c1;

	if( e0 == 0 && e1 == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}

	if( e0 == 0 )
	{
		if( e1<0 )
			possible_out_ports.push_back(3);
		else
			possible_out_ports.push_back(4);
	}
	else if ( e0 > 0 )
	{
		if ( e1 == 0 )
			possible_out_ports.push_back(2);
		else
		{
			if ((c0%2) != 0 || c0==s0 )
			{
				if ( e1 < 0 )
					possible_out_ports.push_back(3);
				else
					possible_out_ports.push_back(4);
			}

			if ( (d0%2)!=0 || e0 != 1 )
				possible_out_ports.push_back(2);
		}
	}
	else
	{
		possible_out_ports.push_back(1);
		if ( (c0%2)==0 )
		{
			if ( e1 < 0 )
				possible_out_ports.push_back(3);
			else
				possible_out_ports.push_back(4);
		}
	}

	if( possible_out_ports.size() == 0 )
	{
		_DBG(" ERROR: Some condition not covered d0: %d , d1:%d", d0, d1);
	}
	return;
}

void
GenericRC::route_odd_even_m(HeadFlit* hf)
{
	uint src = hf->src_address;
	uint dest = hf->dst_address;
	uint oport = -1;
	uint c0=-1, d0=-1, c1=-1, d1=-1, s0=-1, s1=-1;
	c0 = grid_xloc[node_ip];
	c1 = grid_yloc[node_ip];
	d0 = grid_xloc[ dest ];
	d1 = grid_yloc[ dest ];
	s0 = grid_xloc[ src ];
	s1 = grid_yloc[ src ];
	int e0 = d0-c0;
	int e1 = d1-c1;

	if( e0 == 0 && e1 == 0 )
	{
		possible_out_ports.push_back(0);
		return;
	}

	if( e0 == 0 )
	{
		if( e1 > 0 )
			possible_out_ports.push_back(4);
		else
			possible_out_ports.push_back(3);
	}
	else if ( e0 > 0 )
	{
		if ( e1 == 0 )
			possible_out_ports.push_back(2);
		else
		{
			if ((c0%2) != 0 || c0==s0 )
			{
				if ( e1 > 0 )
					possible_out_ports.push_back(4);
				else
					possible_out_ports.push_back(3);
			}

			if ( (d0%2)!=0 || e0 != 1 )
				possible_out_ports.push_back(2);
		}
	}
	else
	{
		possible_out_ports.push_back(1);
		if ( (c0%2)==0 )
		{
			if ( e1 > 0 )
				possible_out_ports.push_back(4);
			else
				possible_out_ports.push_back(3);
		}
	}

	if( possible_out_ports.size() == 0 )
	{
		_DBG(" ERROR: Some condition not covered d0: %d , d1:%d", d0, d1);
	}
	else
	{
#ifdef _DEBUG
		_DBG(" Even Odd Routing Possible out ports : %d", possible_out_ports.size());
		int i=0;
		for ( vector<uint>::iterator it=possible_out_ports.begin();it!=possible_out_ports.end();++it)
		{
			_DBG(" The Possible out ports are [%d]: port[%d]", i++,*it);

		}
#endif
	}

	if ( possible_out_ports.size() < 3 )
	{
		vector <uint> v;
		v.resize(possible_out_ports.size());
		copy (possible_out_ports.begin(),possible_out_ports.end(),v.begin());
		sort (v.begin(), v.end());
		for ( int i=1; i <=4; i++)
		{
			if (!binary_search(v.begin(), v.end(), i))
				misrouting_out_ports.push_back(i);
		}
	}
#ifdef _DEBUG
	int i=0;
	for ( vector<uint>::iterator it=misrouting_out_ports.begin();it!=misrouting_out_ports.end();++it)
	{
		_DBG(" The misrouting out ports are [%d]: port[%d]", i++,*it);
	}
#endif
	return;
}

void
GenericRC::push (Flit* f, uint ch, vector <vector<uint> > const vcr)
{
	if(ch > addresses.size())
		cout << "Invalid VC Exception " << endl;

	//    static vector <uint> last_aport;
	/*    static int try_static_variable_in_method;
    _DBG("try_static_variable_in_method = %d", try_static_variable_in_method++);
	 */  //try try_static_variable_in_method, each time method is called (with different obj),this sVariable is ++

	//Route the head
	if( f->type == HEAD )
	{
		HeadFlit* header = static_cast< HeadFlit* >( f );
		//        addresses[ch].last_adaptive_port = 0;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		possible_out_ports.clear();
		possible_out_vcs.clear();


		vector <vector<uint> > const vvcr = vcr;

		if( rc_method == NEGATIVE_FIRST )
		{
			possible_out_ports.clear();
			route_negative_first( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

		}
		else if( rc_method == WEST_FIRST)
		{
			possible_out_ports.clear();
			route_west_first( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

		}
		else if( rc_method == NORTH_LAST)
		{
			possible_out_ports.clear();
			route_north_last( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

		}
		else if( rc_method == ODD_EVEN)
		{
			possible_out_ports.clear();
			route_odd_even( header );
			//addresses [ch].out_port = possible_out_ports.at(0);
#ifdef _DEBUG_ROUTER
			for ( uint i=0; i<possible_out_ports.size(); i++)
			{
				//                addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
				_DBG("possible_out_ports [%d] downstream_credits = %d",possible_out_ports[i], vvcr[possible_out_ports[i]][ch]);

			}
#endif
		}
		else if( rc_method == NORTH_LAST_NON_MINIMAL)
		{
			possible_out_ports.clear();
			route_north_last_non_minimal( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

		}
		else if( rc_method == RING_ROUTING)
		{
			possible_out_ports.clear();
			possible_out_vcs.clear();
			route_ring( header );
		}
		else
		{
			addresses [ch].out_port = route_x_y(header->dst_address);
			addresses [ch].possible_out_ports.push_back(route_x_y(header->dst_address));
		}

		if( rc_method != TORUS_ROUTING && rc_method != RING_ROUTING )
		{
			if( do_request_reply_network )
			{
				if( header->msg_class == RESPONSE_PKT )
					addresses[ch].possible_out_vcs.push_back(1);
				else
					addresses[ch].possible_out_vcs.push_back(0);
			}
			else
			{
				for ( uint i=0;i<vcs;i++)
					addresses[ch].possible_out_vcs.push_back(i);
			}

		}

		addresses [ch].route_valid = true;

#ifdef _DEEP_DEBUG
		_DBG(" computed oport %d %d dest: %d src:%d addr:%lld",addresses [ch].out_port, addresses [ch].channel, header->dst_address, header->src_address, header->addr);
#endif

	}
	else if(f->type == TAIL)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("TAIL InvalidAddrException" );
		}

		addresses[ch].route_valid = false;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		addresses[ch].last_adaptive_port = 0;
		possible_out_ports.clear();
		possible_out_vcs.clear();
	}
	else if (f->type == BODY)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("BODY InvalidAddrException" );
		}
	}
	else
	{
		_DBG(" InvalidFlitException fty: %d", f->type);
	}

	return ;
}		/* -----  end of method genericRC::push  ----- */
void
GenericRC::push (Flit* f, uint ch, vector<uint> nbs, vector<uint> fnp)
{
	if(ch > addresses.size())
	{
		cout << "\n Invalid VC Exception " << endl;
		cout << " ch="<< ch << endl;
		cout << " addresses.size()=" << addresses.size() << endl;
		cout << " headflit.destnation = " << static_cast<HeadFlit*>(f)->dst_address << endl;
		//cout << " headflit.source_address = " << static_cast<HeadFlit*>(f)->addr << endl;
		cout << " node_ip = " << node_ip << endl;

	}

	//Route the head
	if( f->type == HEAD )
	{
		HeadFlit* header = static_cast< HeadFlit* >( f );
		addresses[ch].last_adaptive_port = 0;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		addresses[ch].is_misrouting.clear();
		possible_out_ports.clear();
		possible_out_vcs.clear();
		is_misrouting.clear();
		addresses [ch].is_pkt_rejoin = false;

#ifdef _DEBUG_VN
		_DBG(" In RC from %d pkt of add %lld to %d at VN%d", header->src_address, header->addr, header->dst_address, header->vn);
#endif



		if( rc_method == VIRTUALNETWORK )
		{
			possible_out_ports.clear();

			vector<uint> eligible_nbs;
			eligible_nbs.clear();
			//eligible_nbs.resize(ports);

			for(uint m=0;m<nbs.size();m++)
			{
				bool found = false;
				for(uint n=0;n<fnp.size();n++)
				{
					if(m==fnp[n]) found= true;
				}
				if (found) eligible_nbs.push_back(-1);
				else eligible_nbs.push_back(nbs[m]);
			}

			for ( uint i=0; i<eligible_nbs.size(); i++) {
				for ( uint j=0; j<header->path.size(); j++) {
					if (eligible_nbs[i] == header->path[j])
						eligible_nbs[i]=-1;
				}
			}
			if (header->vn == VN0) {
				//_DBG(" In RC no of paths: %d from %d to %d at %d at VN%d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip, header->vn);
				route_vn_south_last( header, eligible_nbs );
				/*if ((possible_out_ports.size() !=0) && (possible_out_ports.size()==is_misrouting.size()) )
				{
					if (eligible_nbs[3] !=-1)
					{
						assert(header->virtual_source == NOVS);
						header->virtual_source = VSSET;
						possible_out_ports.push_back(0);
						is_misrouting.push_back(false);
					}
				}*/
				if ((possible_out_ports.size()==0) && (eligible_nbs[3] !=-1))
				{
					assert(header->virtual_source == NOVS);
					header->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}

			}

			else if (header->vn == VN1) {
				route_vn_north_last( header, eligible_nbs );
				//_DBG(" In RC no of paths: %d from %d to %d at %d at VN%d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip, header->vn);
				/*if ((possible_out_ports.size() !=0) && (possible_out_ports.size()==is_misrouting.size()) )
				{
					if (eligible_nbs[4] !=-1)
					{
						assert(header->virtual_source == NOVS);
						header->virtual_source = VSSET;
						possible_out_ports.push_back(0);
						is_misrouting.push_back(false);
					}
				}*/
				if ((possible_out_ports.size()==0) && (eligible_nbs[4] !=-1))
				{
					assert(header->virtual_source == NOVS);
					header->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}
			}

			else cout << "\n Invalid VN Exception " << endl;

			//cerr << endl <<header->toString() <<endl;
			//cerr << "incoming port" << header->node_stamping.back();
			// cerr << endl<<toString()<<endl;

			//            addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);

			// remove node which is shown in the path


			for ( uint i=0; i<possible_out_ports.size(); i++) {
				//if (possible_out_ports[i]!= header->node_stamping.back()) //wrong
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			}
			for ( uint i=0; i<is_misrouting.size(); i++)
				addresses [ch].is_misrouting.push_back(is_misrouting[i]);

			assert(possible_out_ports.size() == is_misrouting.size());
		}
		else if( rc_method == HIERACHY )
		{
			possible_out_ports.clear();
			route_hierachy( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
							addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == NEGATIVE_FIRST )
		{
			possible_out_ports.clear();
			route_negative_first( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == WEST_FIRST)
		{
			possible_out_ports.clear();
			route_west_first( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == NORTH_LAST)
		{
			possible_out_ports.clear();
			route_north_last( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == ODD_EVEN)
		{
			possible_out_ports.clear();
			route_odd_even( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == DY_XY)
		{
			possible_out_ports.clear();
			route_dy_xy( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == NORTH_LAST_NON_MINIMAL)
		{
			possible_out_ports.clear();
			route_north_last_non_minimal( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);
		}
		else if( rc_method == RING_ROUTING)
		{
			possible_out_ports.clear();
			possible_out_vcs.clear();
			route_ring( header );
		}
		else if( rc_method == XY)
		{
			addresses [ch].out_port = route_x_y(header->dst_address);
			addresses [ch].possible_out_ports.push_back(addresses [ch].out_port);
			//addresses [ch].possible_out_ports.push_back(route_x_y(header->dst_address));
			//for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);
		}
		else {
			_DBG( " : Routing Method %d is not found \n", rc_method);
			exit(0);
		}

		if( rc_method != TORUS_ROUTING && rc_method != RING_ROUTING )
		{
			if( do_request_reply_network )
			{
				if( header->msg_class == RESPONSE_PKT )
					addresses[ch].possible_out_vcs.push_back(1);
				else
					addresses[ch].possible_out_vcs.push_back(0);
			}
			else
			{
				for ( uint i=0;i<vcs;i++)
					addresses[ch].possible_out_vcs.push_back(i);
			}

		}

		if(possible_out_ports.size()>0)
		{
			if((header->virtual_source == NOVS) && (f->pseudo) && (possible_out_ports[0]==0))
			{
				// pkt rejoin = true
				addresses [ch].is_pkt_rejoin = true;
			}
		}

		addresses [ch].route_valid = true;

#ifdef _DEEP_DEBUG
		_DBG(" computed oport %d %d dest: %d src:%d addr:%lld",addresses [ch].out_port, addresses [ch].channel, header->dst_address, header->src_address, header->addr);
#endif



	}
	else if(f->type == TAIL)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("TAIL InvalidAddrException" );
		}

		addresses[ch].route_valid = false;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		addresses[ch].last_adaptive_port = 0;
		possible_out_ports.clear();
		possible_out_vcs.clear();
	}
	else if (f->type == BODY)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("BODY InvalidAddrException" );
		}
	}
	else
	{
		_DBG(" InvalidFlitException fty: %d", f->type);
	}

	return ;
}		/* -----  end of method genericRC::push  ----- */
void
GenericRC::push (Flit* f, uint ch, const vector<uint> nbs, const vector<uint> fnp, const vector<vector<uint*> > fon)
{

	if(ch > addresses.size())
	{
		cout << "\n Invalid VC Exception " << endl;
		cout << " ch="<< ch << endl;
		cout << " addresses.size()=" << addresses.size() << endl;
		cout << " headflit.destnation = " << static_cast<HeadFlit*>(f)->dst_address << endl;
		//cout << " headflit.source_address = " << static_cast<HeadFlit*>(f)->addr << endl;
		cout << " node_ip = " << node_ip << endl;

	}

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
		if(nbs[p]!= -1)
		{
			for(uint i=1;i<ports;i++)
			{
				if(i!=ip)
					no_fault_neighbor[p] += *(fon[p][i]);// wrong for nodes at border
			}
		}
	}
	//Route the head
	if( f->type == HEAD )
	{
		HeadFlit* header = static_cast< HeadFlit* >( f );
		addresses[ch].last_adaptive_port = 0;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		addresses[ch].is_misrouting.clear();
		possible_out_ports.clear();
		possible_out_vcs.clear();
		is_misrouting.clear();
		addresses [ch].is_pkt_rejoin = false;

#ifdef _DEBUG_VN
		_DBG(" In RC from %d pkt of add %lld to %d at VN%d", header->src_address, header->addr, header->dst_address, header->vn);
#endif



		if( rc_method == VIRTUALNETWORK )
		{
			possible_out_ports.clear();

			vector<uint> eligible_nbs;
			eligible_nbs.clear();
			//eligible_nbs.resize(ports);

			for(uint m=0;m<nbs.size();m++)
			{
				bool found = false;
				for(uint n=0;n<fnp.size();n++)
				{
					if(m==fnp[n]) found= true;
				}
				if (found) eligible_nbs.push_back(-1);
				else eligible_nbs.push_back(nbs[m]);
			}

			for ( uint i=0; i<eligible_nbs.size(); i++) {
				for ( uint j=0; j<header->path.size(); j++) {
					if (eligible_nbs[i] == header->path[j])
						eligible_nbs[i]=-1;
				}
			}
			if (header->vn == VN0) {
				//_DBG(" In RC no of paths: %d from %d to %d at %d at VN%d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip, header->vn);
				route_vn_south_last( header, eligible_nbs , no_fault_neighbor);
				/*if ((possible_out_ports.size() !=0) && (possible_out_ports.size()==is_misrouting.size()) )
				{
					if (eligible_nbs[3] !=-1)
					{
						assert(header->virtual_source == NOVS);
						header->virtual_source = VSSET;
						possible_out_ports.push_back(0);
						is_misrouting.push_back(false);
					}
				}*/
				if ((possible_out_ports.size()==0) && (eligible_nbs[3] !=-1))
				{
					assert(header->virtual_source == NOVS);
					header->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}

			}

			else if (header->vn == VN1) {
				route_vn_north_last( header, eligible_nbs, no_fault_neighbor );
				//_DBG(" In RC no of paths: %d from %d to %d at %d at VN%d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip, header->vn);
				/*if ((possible_out_ports.size() !=0) && (possible_out_ports.size()==is_misrouting.size()) )
				{
					if (eligible_nbs[4] !=-1)
					{
						assert(header->virtual_source == NOVS);
						header->virtual_source = VSSET;
						possible_out_ports.push_back(0);
						is_misrouting.push_back(false);
					}
				}*/
				if ((possible_out_ports.size()==0) && (eligible_nbs[4] !=-1))
				{
					assert(header->virtual_source == NOVS);
					header->virtual_source = VSSET;
					possible_out_ports.push_back(0);
					is_misrouting.push_back(false);
				}
			}

			else cout << "\n Invalid VN Exception " << endl;

			//cerr << endl <<header->toString() <<endl;
			//cerr << "incoming port" << header->node_stamping.back();
			// cerr << endl<<toString()<<endl;

			//            addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);

			// remove node which is shown in the path


			for ( uint i=0; i<possible_out_ports.size(); i++) {
				//if (possible_out_ports[i]!= header->node_stamping.back()) //wrong
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			}
			for ( uint i=0; i<is_misrouting.size(); i++)
				addresses [ch].is_misrouting.push_back(is_misrouting[i]);

			assert(possible_out_ports.size() == is_misrouting.size());
		}
		else if( rc_method == HIERACHY )
		{
			possible_out_ports.clear();
			route_hierachy( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
							addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == NEGATIVE_FIRST )
		{
			possible_out_ports.clear();
			route_negative_first( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == WEST_FIRST)
		{
			possible_out_ports.clear();
			route_west_first( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == NORTH_LAST)
		{
			possible_out_ports.clear();
			route_north_last( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == ODD_EVEN)
		{
			possible_out_ports.clear();
			route_odd_even( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == DY_XY)
		{
			possible_out_ports.clear();
			route_dy_xy( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);

		}
		else if( rc_method == NORTH_LAST_NON_MINIMAL)
		{
			possible_out_ports.clear();
			route_north_last_non_minimal( header );
			addresses [ch].out_port = possible_out_ports.at(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);
		}
		else if( rc_method == RING_ROUTING)
		{
			possible_out_ports.clear();
			possible_out_vcs.clear();
			route_ring( header );
		}
		else if( rc_method == XY)
		{
			addresses [ch].out_port = route_x_y(header->dst_address);
			addresses [ch].possible_out_ports.push_back(addresses [ch].out_port);
			//addresses [ch].possible_out_ports.push_back(route_x_y(header->dst_address));
			//for ( uint i=0; i<possible_out_ports.size(); i++)
										addresses [ch].is_misrouting.push_back(false);
		}
		else {
			_DBG( " : Routing Method %d is not found \n", rc_method);
			exit(0);
		}

		if( rc_method != TORUS_ROUTING && rc_method != RING_ROUTING )
		{
			if( do_request_reply_network )
			{
				if( header->msg_class == RESPONSE_PKT )
					addresses[ch].possible_out_vcs.push_back(1);
				else
					addresses[ch].possible_out_vcs.push_back(0);
			}
			else
			{
				for ( uint i=0;i<vcs;i++)
					addresses[ch].possible_out_vcs.push_back(i);
			}

		}

		if(possible_out_ports.size()>0)
		{
			if((header->virtual_source == NOVS) && (f->pseudo) && (possible_out_ports[0]==0))
			{
				// pkt rejoin = true
				addresses [ch].is_pkt_rejoin = true;
			}
		}

		addresses [ch].route_valid = true;

#ifdef _DEEP_DEBUG
		_DBG(" computed oport %d %d dest: %d src:%d addr:%lld",addresses [ch].out_port, addresses [ch].channel, header->dst_address, header->src_address, header->addr);
#endif



	}
	else if(f->type == TAIL)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("TAIL InvalidAddrException" );
		}

		addresses[ch].route_valid = false;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		addresses[ch].last_adaptive_port = 0;
		possible_out_ports.clear();
		possible_out_vcs.clear();
	}
	else if (f->type == BODY)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("BODY InvalidAddrException" );
		}
	}
	else
	{
		_DBG(" InvalidFlitException fty: %d", f->type);
	}

	return ;

}
void
GenericRC::push (Flit* f, uint ch)
{
	if(ch > addresses.size())
	{
		cout << "\n Invalid VC Exception " << endl;
		cout << " ch="<< ch << endl;
		cout << " addresses.size()=" << addresses.size() << endl;
		cout << " headflit.destnation = " << static_cast<HeadFlit*>(f)->dst_address << endl;
		//cout << " headflit.source_address = " << static_cast<HeadFlit*>(f)->addr << endl;
		cout << " node_ip = " << node_ip << endl;

	}

	//Route the head
	if( f->type == HEAD )
	{
		HeadFlit* header = static_cast< HeadFlit* >( f );
		addresses[ch].last_adaptive_port = 0;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		possible_out_ports.clear();
		possible_out_vcs.clear();
		addresses [ch].is_pkt_rejoin = false;
#ifdef _DEBUG
		_DBG(" In RC from %d pkt of add %lld to %d at VN%d", header->src_address, header->addr, header->dst_address, header->vn);
#endif

		if (header->virtual_source == true)
		{
			possible_out_ports.push_back(0);
			for ( uint i=0; i<possible_out_ports.size(); i++)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
		}
		else
		{
			if( rc_method == VIRTUALNETWORK )
			{
				possible_out_ports.clear();
				if (header->vn == VN0) {
					//_DBG(" In RC no of paths: %d from %d to %d at %d at VN%d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip, header->vn);
					route_vn_south_last( header );
				}

				else if (header->vn == VN1) {
					route_vn_north_last( header );
					//_DBG(" In RC no of paths: %d from %d to %d at %d at VN%d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip, header->vn);
				}

				else cout << "\n Invalid VN Exception " << endl;

				//cerr << endl <<header->toString() <<endl;
				//cerr << "incoming port" << header->node_stamping.back();

				//            addresses [ch].out_port = possible_out_ports.at(0);
				//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
				for ( uint i=0; i<possible_out_ports.size(); i++) {
					//if (possible_out_ports[i]!= header->node_stamping.back()) //wrong
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
				}


			}
			else if( rc_method == HIERACHY )
			{
				possible_out_ports.clear();
				route_hierachy( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

			}
			else if( rc_method == NEGATIVE_FIRST )
			{
				possible_out_ports.clear();
				route_negative_first( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

			}
			else if( rc_method == WEST_FIRST)
			{
				possible_out_ports.clear();
				route_west_first( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

			}
			else if( rc_method == NORTH_LAST)
			{
				possible_out_ports.clear();
				route_north_last( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				//            _DBG(" In RC no of paths: %d from %d to %d at %d", possible_out_ports.size(), header->src_address, header->dst_address, node_ip);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

			}
			else if( rc_method == ODD_EVEN)
			{
				possible_out_ports.clear();
				route_odd_even( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

			}
			else if( rc_method == DY_XY)
			{
				possible_out_ports.clear();
				route_dy_xy( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);

			}
			else if( rc_method == NORTH_LAST_NON_MINIMAL)
			{
				possible_out_ports.clear();
				route_north_last_non_minimal( header );
				addresses [ch].out_port = possible_out_ports.at(0);
				for ( uint i=0; i<possible_out_ports.size(); i++)
					addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
			}
			else if( rc_method == RING_ROUTING)
			{
				possible_out_ports.clear();
				possible_out_vcs.clear();
				route_ring( header );
			}
			else if( rc_method == XY)
			{
				addresses [ch].out_port = route_x_y(header->dst_address);
				//addresses [ch].possible_out_ports.push_back(route_x_y(header->dst_address));
				addresses [ch].possible_out_ports.push_back(possible_out_ports[0]);
			}
			else {
				_DBG( " : Routing Method %d is not found \n", rc_method);
				exit(0);
			}

			if( rc_method != TORUS_ROUTING && rc_method != RING_ROUTING )
			{
				if( do_request_reply_network )
				{
					if( header->msg_class == RESPONSE_PKT )
						addresses[ch].possible_out_vcs.push_back(1);
					else
						addresses[ch].possible_out_vcs.push_back(0);
				}
				else
				{
					for ( uint i=0;i<vcs;i++)
						addresses[ch].possible_out_vcs.push_back(i);
				}

			}
		}
		//if(addresses [ch].possible_out_ports.size()>0)
		if(possible_out_ports.size()>0)
		{
			if((header->virtual_source == NOVS) && (f->pseudo) && (possible_out_ports[0]==0))
			{
				// pkt rejoin = true
				addresses [ch].is_pkt_rejoin = true;
			}
		}
		addresses [ch].route_valid = true;

#ifdef _DEEP_DEBUG
		_DBG(" computed oport %d %d dest: %d src:%d addr:%lld",addresses [ch].out_port, addresses [ch].channel, header->dst_address, header->src_address, header->addr);
#endif



	}
	else if(f->type == TAIL)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("TAIL InvalidAddrException" );
		}

		addresses[ch].route_valid = false;
		addresses[ch].possible_out_ports.clear();
		addresses[ch].possible_out_vcs.clear();
		addresses[ch].last_adaptive_port = 0;
		possible_out_ports.clear();
		possible_out_vcs.clear();
	}
	else if (f->type == BODY)
	{
		if( !addresses[ch].route_valid)
		{
			_DBG_NOARG("BODY InvalidAddrException" );
		}
	}
	else
	{
		_DBG(" InvalidFlitException fty: %d", f->type);
	}

	return ;
}		/* -----  end of method genericRC::push  ----- */
uint
GenericRC::get_output_port ( uint ch, vector <vector<uint> > const vcr,uint ip,uint inport)
{
	uint oport = -1;
	/*
	    if ( addresses[ch].possible_out_ports.size() > 1)
	    {
	    _DBG("addresses[%d].last_adaptive_port = %d", ch, addresses[ch].last_adaptive_port);
	    if (addresses[ch].last_adaptive_port >= addresses[ch].possible_out_ports.size())
	        addresses[ch].last_adaptive_port = 0;

	    oport  = addresses[ch].possible_out_ports[addresses[ch].last_adaptive_port];

	    _DBG("addresses[%d].last_adaptive_port = %d", ch, addresses[ch].last_adaptive_port);
	    addresses[ch].last_adaptive_port++;//=(addresses[ch].last_adaptive_port)++;

	    }

	    else
	    	oport  = addresses[ch].possible_out_ports[0];

	 */


	vector<uint> check;
	//check.begin()=possible_out_ports.at(0);
	for (int i=0; i<possible_out_ports.size();i++)
	{
		//if ( possible_out_ports[i]>=0)
		check.push_back(possible_out_ports[i]);
	}
#ifdef _DEBUG_ROUTER
	_DBG(" check size : [%d]", check.size());
	int i=0;
	for (vector<uint>::iterator it=check.begin();it!=check.end();++it)
	{
		_DBG(" check[%d]: port[%d]", i++,*it);
	}
#endif
	//check credits of possible out ports
	vector <vector<uint> > const vvcr = vcr;
	for (int i=0; i<check.size();i++)
	{
		if ( vcr[check[i]][ch]<=0)
			check.erase(check.begin()+i);
	}

	if ((check.size()<=0 || ip==5)||(missrouting_counter>1))
	{
#ifdef _DEBUG_ROUTER
		printf("\n ERROR: no credits in possible out ports");
#endif
		//_DBG()
		//assert
		//exit(0);
		for (int i=0; i<misrouting_out_ports.size();i++)
		{
			if (misrouting_out_ports[i]!=inport)
			{
				check.push_back(misrouting_out_ports[i]);
				addresses[ch].possible_out_ports.push_back(misrouting_out_ports[i]);
			}

		}
		//missrouting_counter++;
		_DBG("missrouting_counter = %d", missrouting_counter);
	}
	//if (missrouting_counter==0)
	else
	{
		addresses[ch].out_port = possible_out_ports.at(0);
		for ( uint i=0; i<possible_out_ports.size(); i++)
		{
			if (possible_out_ports[i]!=inport)
				addresses [ch].possible_out_ports.push_back(possible_out_ports[i]);
		}

	}
#ifdef _DEBUG_ROUTER
	i=0;
	for ( vector<uint>::iterator it=addresses [ch].possible_out_ports.begin();it!=addresses [ch].possible_out_ports.end();++it)
	{
		_DBG(" The final possible routing out ports are [%d]: port[%d]", i++,*it);
	}
#endif


	///*
	// just to have out port change, not a real case, coz it change each time method is called
	Address::last_aport.resize(4);
#ifdef _DEBUG_ROUTER
	_DBG("Address::last_aport[%d] = %d", ch, Address::last_aport[ch]);
#endif
	if (Address::last_aport[ch] >= addresses[ch].possible_out_ports.size())
		Address::last_aport[ch] = 0;
	oport  = addresses[ch].possible_out_ports[Address::last_aport[ch]];
#ifdef _DEBUG_ROUTER
	_DBG("Address::last_aport[%d] = %d", ch, Address::last_aport[ch]);
#endif
	Address::last_aport[ch]=(Address::last_aport[ch]++)%4;//=(addresses[ch].last_adaptive_port)++;
	//*/
#ifdef _DEBUG_ROUTER
	_DBG(" The final routing out port is : port[%d]",oport);
#endif
	return oport;
}		/* -----  end of method genericRC::get_output_port  ----- */
uint
GenericRC::get_output_port ( uint ch)
{
	uint oport = -1;
	if (addresses[ch].last_adaptive_port == addresses[ch].possible_out_ports.size())
		addresses[ch].last_adaptive_port = 0;
	oport  = addresses[ch].possible_out_ports[addresses[ch].last_adaptive_port];
	addresses[ch].last_adaptive_port++;
#ifdef _DEBUG_ROUTER
	//		 	   _DBG("get_output_port: The final routing out port is : port[%d]",oport);
#endif
	return oport;
}		/* -----  end of method genericRC::get_output_port  ----- */

bool
GenericRC::get_output_port_status ( uint ch)
{
	bool is_misrt = false;
	if (addresses[ch].last_adaptive_port == addresses[ch].is_misrouting.size())
		addresses[ch].last_adaptive_port = 0;
	is_misrt  = addresses[ch].is_misrouting[addresses[ch].last_adaptive_port];
	addresses[ch].last_adaptive_port++;
#ifdef _DEBUG_ROUTER
	//		 	   _DBG("get_output_port: The final routing out port is : port[%d]",oport);
#endif
	return is_misrt;
}		/* -----  end of method genericRC::get_output_port  ----- */

uint
GenericRC::no_adaptive_vcs( uint ch )
{
	return addresses[ch].possible_out_vcs.size();
}

uint
GenericRC::no_adaptive_ports( uint ch )
{
#ifdef _DEBUG
	if( addresses[ch].possible_out_ports.size() == 0 )
	{
		_DBG(" ERROR: GenericRC::no_adaptive_ports(%d) : possible_out_ports.size()==0",ch);
	}
#endif
	return addresses[ch].possible_out_ports.size();
}

bool
GenericRC::get_is_pkt_rejoin ( uint ch )
{
	return addresses[ch].is_pkt_rejoin;
}		/* -----  end of method genericRC::get_is_pkt_rejoin  ----- */

uint
GenericRC::get_virtual_channel ( uint ch )
{
	uint och = -1;
	if (addresses[ch].last_vc == addresses[ch].possible_out_vcs.size())
		addresses[ch].last_vc = 0;

	och = addresses[ch].possible_out_vcs[addresses[ch].last_vc];
	addresses[ch].last_vc++;

	return och;
}		/* -----  end of method genericRC::get_vc  ----- */

void
GenericRC::resize ( uint ch )
{
	vcs = ch;
	addresses.resize(ch);
	for ( uint i = 0 ; i<ch ; i++ )
	{
		addresses[i].route_valid = false;
		addresses[i].last_vc = 0;
		addresses[i].is_pkt_rejoin = false;
	}
	return ;
}		/* -----  end of method genericRC::set_no_channels  ----- */

uint
GenericRC::get_no_channels()
{
	return addresses.size();
}		/* -----  end of method genericRC::get_no_channels  ----- */

bool
GenericRC::is_empty ()
{
	uint channels = addresses.size();
	for ( uint i=0 ; i<channels ; i++ )
		if(addresses[i].route_valid)
			return false;

	return true;
}		/* -----  end of method genericRC::is_empty  ----- */

string
GenericRC::toString () const
{
	stringstream str;
	str << "GenericRC"
			<< "\tchannels: " << addresses.size();

	for (uint j=0;j<addresses.size();j++){
		cerr <<"\t ch:"<<j<<" port:";
		for (uint i=0;i<addresses[j].possible_out_ports.size();i++)
			cerr<<addresses[j].possible_out_ports[i]<<" ";
	}

	return str.str();
}		/* -----  end of function GenericRC::toString  ----- */
#endif   /* ----- #ifndef _genericaddressdecoder_cc_INC  ----- */

