// radio range (R) should be change based on the related value in TCL_ min_e (minimum energy for communicate)

#include "mygrid.h"
#include "mobilenode.h"

FILE *pfile1_, *pfile2_, *pfile3_, *pfile4_, *pfile5_, *pfile6_, *pfile7_;
int counter = 0;
float residual = 0;
float accumolator = 0;
int alive = 0;
int totalpacket = 0;
float totalhop = 0;
int sw_life = 0; //switch to register lifetime only one time
float lifetime;

//int dead_nodes = 0;
int hdr_mygrid::offset_;

static class MyGridHeaderClass : public PacketHeaderClass {
public:
	MyGridHeaderClass() : PacketHeaderClass("PacketHeader/MyGrid", sizeof(hdr_mygrid)) {
		bind_offset(&hdr_mygrid::offset_);
	}
} class_mygridhdr;

static class MyGridClass : public TclClass {
public:
	MyGridClass() : TclClass("Agent/MyGrid") {}
	TclObject* create(int, const char*const*) {
		return (new MyGridAgent());
	}
} class_mygrid;

MyGridAgent::MyGridAgent() : Agent(PT_MYGRID)
{
        MobileNode *iNode;      // for energy
        iEnergy = 0.0;         // for energy
        posx=0.0;
        posy=0.0;
        min_e = 0.2;       //minimum energy for communicating
	total_hopcount = 0; 
//        srand(clock());
//        bind("X_", &posx);	
//        bind("Y_", &posy);	
//        bind("sink1_y", &s1_y);	
//        bind("sink2_x", &s2_x);	
//        bind("sink2_y", &s2_y);	
        bind("packetSize_", &size_);
	bind("node_source_id", &source_id);
        sw = 0;
        rid = 0;
        R = 75;
        rcvd_packets = 0;
	energy = 2;
	for (int i=0; i<MAX_NODE; i++) {
		nei1[i] = FALSE;
		for (int j=0; j<MAX_NODE; j++)
			nei2[i][j] = FALSE;
	}

}

void MyGridAgent::update_position() 
{
	iNode->update_position();
	posx = iNode->X();
	posy = iNode->Y();
}
void MyGridAgent::mysend(int send_type, int pckt, int orgnl, const char* packet_type, float xpos, float ypos, float initenergy, int sink, float sink_x, float sink_y, int hopcount)
{
	Packet* pkt = allocpkt();
	hdr_mygrid* hdr = hdr_mygrid::access(pkt);
	hdr_cmn *cmnhdr = hdr_cmn::access(pkt);
	hdr_ip*  iphdr  = hdr_ip::access(pkt);

	cmnhdr->next_hop() = IP_BROADCAST;
//	cmnhdr->size()    = size;

	iphdr->src_.addr_ = here_.addr_;  //LL will fill MAC addresses in the MAC header
	iphdr->dst_.addr_ = IP_BROADCAST;
	iphdr->dst_.port_ = this->port();
	
	if (send_type == 0) {                  // the source node send the message
		rcvd_original_id = source_id;
		hdr->original_id = source_id;
		rcvd_packet_id = 1;
		hdr->packet_id = 1;

	} else {                               // the relay node send the message
		hdr->original_id = orgnl;
		hdr->packet_id = pckt;
	}

	hdr->source_id = source_id;

        if (strcmp(packet_type,"HLO") == 0) {
		hdr->X = xpos; 
		hdr->Y = ypos;
                hdr->energy = initenergy;	
		hdr->packet_type = 1;
		energy = energy - 0.0007290625;
		send(pkt, 0);
        }
        else if (strcmp(packet_type,"EUPDATE") == 0) {
/*    		pfile2_ = fopen("out_residual.txt","a");  //file for report dead nodes
		fprintf (pfile2_, "%d-%.4f-%.4f \n", source_id, CURRENT_TIME, initenergy);
		fclose(pfile2_);  */ 
		//hdr->X = xpos; 
		//hdr->Y = ypos;
		if (initenergy < min_e) {
    			pfile1_ = fopen("out_dead.txt","a");  //file for report dead nodes
			// Scheduler::instance().clock() 
		        fprintf (pfile1_, "%d-%.4f-%.4f \n", source_id, CURRENT_TIME, initenergy);
			fclose(pfile1_);   
		}
                hdr->energy = initenergy;	
		hdr->packet_type = 4;
		energy = energy - 0.0007290625;
		send(pkt, 0);
        }
        else if (strcmp(packet_type,"SINKHLO") == 0) {

                hdr->energy = initenergy;	
		hdr->packet_type = 4;
		energy = energy - 0.0007290625;
		send(pkt, 0);
        }
/*	else if (strcmp(packet_type,"NEI") == 0) {
		hdr->X = xpos; 
		hdr->Y = ypos;
                hdr->energy = initenergy;	
		hdr->packet_type = 2;
		for (int i=0; i < MAX_NODE; i++)
			hdr->nei1[i] = nei1[i];
	} */ 
	else if (strcmp(packet_type,"DATA") == 0) {

                //Finding best neighbor by calculating minimum distanse to sink
                float d1,d2; 
		d1 = sqrt(((xpos-sink_x) * (xpos-sink_x))+((ypos-sink_y) * (ypos-sink_y)));
                int t,min_nodeid; 
                float min_d = 5000; //minimum distance to sink
                float min_dis;
                if (d1 <= R){
                   min_nodeid = sink;
                } else {
		        for (int i=0; i < rid; i++) {
			   d2 = sqrt((((routingtable[i].x)-sink_x) * ((routingtable[i].x)-sink_x))+(((routingtable[i].y)-sink_y) * ((routingtable[i].y)-sink_y)));
			   if ((d2 < d1) && (d2 <= min_d) && ((routingtable[i].residual_e) > min_e)) {
		              t=i;  
			      min_nodeid = t; 
		              min_dis = d2;
		              min_d = d2;
		           }                                  
		        }
                }
                printf("d1= %f and d2= %f in c++ and min_nodeid= %d \n", d1, min_dis, min_nodeid); 
                //End of finding best neighbor---------------------------------  
		if ((min_nodeid >= 0) && (min_nodeid < rid)) {            
			hdr->hopcount = hopcount;
			hdr->X = routingtable[min_nodeid].x; 
			hdr->Y = routingtable[min_nodeid].y;
        	        hdr->sink_xp = sink_x;
        	        hdr->sink_yp = sink_y;
        	        hdr->sink_id = sink;
        	        if (d1 > R) {
        	        	hdr->recv_id = routingtable[min_nodeid].id;
        	        } 
        	        else {
        	        	hdr->recv_id = sink;
        	        }	
			hdr->packet_type = 3;
			//for (int i=0; i < MAX_NODE; i++)
			//	hdr->nei1[i] = nei1[i];
			energy = energy - 0.0007290625;
			send(pkt, 0);
		} else {
    			pfile4_ = fopen("out_loss.txt","a");  //file for report packet loss caused by energy deplation
		        fprintf (pfile4_, "Can not send packet-%d at time %.4f with energy %.4f \n", source_id, CURRENT_TIME, initenergy);
			fclose(pfile4_);   
		}

	} 

        //-------------------------------------------------------------1
        iNode = (MobileNode *) (Node::get_node_by_address(source_id));
//        iEnergy = iNode->energy_model()->energy();
//        printf("updated energy for node %d is energy %.4f at time %.4f \n", source_id, iEnergy, CURRENT_TIME);
        printf("updated energy for node %d is energy %.4f at time %.4f \n", source_id, energy, CURRENT_TIME);
        //----------------------------------------------------------
        //------------- Lifetime measurment ------------------------
//	if ((iEnergy < min_e) && (sw_life == 0) && (source_id != 0)) {
	if ((energy < min_e) && (sw_life == 0) && (source_id != 0)) {
		lifetime = CURRENT_TIME;
		sw_life = 1;
    		pfile3_ = fopen("out_lifetime.txt","a");  //file for report dead nodes
//	        fprintf (pfile3_, "S-%d-%.4f-%.4f \n", source_id, CURRENT_TIME, iEnergy);
	        fprintf (pfile3_, "S-%d-%.4f-%.4f \n", source_id, CURRENT_TIME, energy);
		fclose(pfile3_);   
	}

}

void MyGridAgent::recv(Packet* pkt, Handler*)
{
	hdr_ip* hdrip = hdr_ip::access(pkt);
	hdr_mygrid* hdr = hdr_mygrid::access(pkt);

        //-------------------------------------------------------------1
        iNode = (MobileNode *) (Node::get_node_by_address(source_id));
//        iEnergy = iNode->energy_model()->energy();
        //----------------------------------------------------------
        //------------- Lifetime measurment ------------------------
//	if ((iEnergy < min_e) && (sw_life == 0) && (source_id != 0)) {
	if (source_id != 0) {
		energy = energy - 0.0004;
	}
	if ((energy < min_e) && (sw_life == 0) && (source_id != 0)) {
		lifetime = CURRENT_TIME;
		sw_life = 1;
    		pfile3_ = fopen("out_lifetime.txt","a");  //file for report dead nodes
//	        fprintf (pfile3_, "R-%d-%.4f-%.4f \n", source_id, CURRENT_TIME, iEnergy);
	        fprintf (pfile3_, "S-%d-%.4f-%.4f \n", source_id, CURRENT_TIME, energy);
		fclose(pfile3_);   
	}

	//Hello Message
	
        if (hdr->packet_type == 1) {  
           if (hdr->source_id != 0) {
		routingtable[rid].x = hdr->X;
		routingtable[rid].y = hdr->Y;
		routingtable[rid].residual_e = hdr->energy;
		routingtable[rid].id = hdr->source_id;
		rid++;
		nei1[hdr->source_id] = TRUE;
           } 
		Packet::free(pkt);
	}
        else if (hdr->packet_type == 4)  {  //Energy Update Message for Neighboring nodes
           if (hdr->source_id != 0) {
		for (int i=0; i < rid; i++) {
			if (routingtable[i].id == hdr->source_id) {
				routingtable[i].residual_e = hdr->energy;
			}
		}				
           } 
		Packet::free(pkt);
	}
        else  if (hdr->packet_type == 3)  {  //Data Message
                 if ((hdr->recv_id == source_id) && (source_id != hdr->sink_id)) {  //forward packet
		    rcvd_original_id = hdr->original_id;
                    rcvd_x = hdr->X;
                    rcvd_y = hdr->Y;
                    rcvd_sinkx = hdr->sink_xp;
                    rcvd_sinky = hdr->sink_yp;
                    rcvd_sinkid = hdr->sink_id;
		    rcvd_hopcount = hdr->hopcount;
		    rcvd_hopcount++;  //Increase the hop count
                    Packet::free(pkt);
	            char out[100];
	            Tcl& tcl = Tcl::instance(); 
		    sprintf(out, "%s pr_relay %d %f", name(), rcvd_original_id, CURRENT_TIME); 
		    tcl.eval(out);
                    //printf("the x pos of sink(0) is %lf", &s1_x);
	            mysend(1,0,rcvd_original_id,"DATA",rcvd_x,rcvd_y,0,rcvd_sinkid,sink1_x,sink1_y,rcvd_hopcount);                    	
                 }
                 else if ((hdr->recv_id == source_id) && (source_id == hdr->sink_id)) {  //sink reseives packet
                    ++rcvd_packets;
		    ++totalpacket;	
		    rcvd_original_id = hdr->original_id;
	    	    rcvd_hopcount = hdr->hopcount;
		    rcvd_hopcount++;  //Increase the hop count
		    total_hopcount = total_hopcount + rcvd_hopcount; 
		    totalhop = totalhop + rcvd_hopcount; 
			if (source_id == 0) {
    			pfile5_ = fopen("out_delay1.txt","w");  //Sink1 (SS)_file for report hop count (delay) and packet number 
		        fprintf (pfile5_, "Sink %d receives %d packets from source %d till %f -tot_hop %ld \n", source_id, rcvd_packets, rcvd_original_id, CURRENT_TIME, total_hopcount);
			fclose(pfile5_);   
			}
/*			else {
    			pfile6_ = fopen("out_delay2.txt","w");  //Sink2 (MS)_file for report hop count (delay) and packet number
		        fprintf (pfile6_, "Sink %d receives %d packets from source %d till %f -tot_hop %ld \n", source_id, rcvd_packets, rcvd_original_id, CURRENT_TIME, total_hopcount);
			fclose(pfile6_);   
			} */
		    Packet::free(pkt);
	            char out[100];
	            Tcl& tcl = Tcl::instance(); 
		    sprintf(out, "%s pr_ack %d %f %d %ld", name(), rcvd_original_id, CURRENT_TIME, rcvd_packets, total_hopcount); 
		    tcl.eval(out);
                 }                                            
		 else Packet::free(pkt);
        }
}

void MyGridAgent::print_nei1() 
{
	char out[100];
	Tcl& tcl = Tcl::instance();
/*	for (int i=0; i < MAX_NODE; i++) {
		if (nei1[i]) {
			sprintf(out, "%s pr_nei1 %d", name(),i);  */
	for (int i=0; i < rid; i++) {
		
		sprintf(out, "%s pr_nei1 %d %f %f", name(), routingtable[i].id, routingtable[i].x, routingtable[i].y); 
		tcl.eval(out);
		
	}
}

void MyGridAgent::print_pos()
{
	char out[100];
	Tcl& tcl = Tcl::instance();
	update_position();
	sprintf(out, "%s pr_pos %f %f", name(), posx,posy); 
	tcl.eval(out);
}

void MyGridAgent::printsinkpos()
{
	char out[100];
	Tcl& tcl = Tcl::instance();
	update_position();
	sprintf(out, "%s pr_sink_pos %f %f", name(), sink1_x,sink1_y); 
	tcl.eval(out);
}

void MyGridAgent::update_pos()
{
	char out[100];
	Tcl& tcl = Tcl::instance();
	update_position();
	if (source_id == 0) {
		sprintf(out, "%s up_pos1 %f %f %f", name(), posx,posy,CURRENT_TIME);
	} 
	tcl.eval(out);
}

void MyGridAgent::result(int non, float nop, int tot_event_nodes)	//(non)= number of nodes (nop)= number of packets produced by a source
{
//        iNode = (MobileNode *) (Node::get_node_by_address(source_id));
//        iEnergy = iNode->energy_model()->energy();
//	if (iEnergy >= min_e) {
	if (energy >= min_e) {
		++alive;
	}
	++counter;
//	accumolator = accumolator + iEnergy;
	accumolator = accumolator + energy;
	residual = accumolator / counter;
	pfile2_ = fopen("out_residual.txt","a");  //file for report dead nodes
//	fprintf (pfile2_, "%d-%.4f-%.4f \n", source_id, CURRENT_TIME, iEnergy);
	fprintf (pfile2_, "%d-%.4f-%.4f \n", source_id, CURRENT_TIME, energy);
	fclose(pfile2_);
	if (source_id == non) {
		float hop_per_pkt; //rate of hops per packet
		int loss; //number of packet loss
		int tot_nop; //total number of packets that should be produced during simulation time 
		long int int_tt_hop;
		float throughput;
		tot_nop = nop * tot_event_nodes;
		loss = tot_nop-totalpacket;
		hop_per_pkt = totalhop/totalpacket;
		throughput = (totalpacket*100.0)/tot_nop;	//throughput based on percent 

//		int_tt_hop = (long int) totalhop;
//		int_tt_hop = (long int) (totalhop + 0.5);
		int_tt_hop = (long int) floor(totalhop + 0.5);
		pfile7_ = fopen("out_result.txt","a");  //file for report dead nodes
//		fprintf (pfile7_, "CURRENT_TIME-Lifetime-Residual-Alive-Totalpacket-Loss-Totalhop-Hop/PerPacket \n");
		fprintf (pfile7_, "%.4f-%.4f-%.4f-%d-%d-%d-%6.1f-%.4f-%ld-%6.2f \n", CURRENT_TIME, lifetime, residual, alive, totalpacket, loss, totalhop, hop_per_pkt, int_tt_hop, throughput);
		fclose(pfile7_);
	}
}

int MyGridAgent::command(int argc, const char*const* argv)
{
	if (argc == 2) {
		if (strcmp(argv[1], "print_nei1") == 0) {
			print_nei1();
			return (TCL_OK);
		}
		else if (strcmp(argv[1], "printpos") == 0) {
			print_pos();
			return (TCL_OK);
		}
		else if (strcmp(argv[1], "updatepos") == 0) {
			update_pos();
			return (TCL_OK);
		}
		else if (strcmp(argv[1], "printsinkpos") == 0) {
			printsinkpos();
			return (TCL_OK);
		}
		else if (strcmp(argv[1], "print_sink_energy") == 0) {
			pfile2_ = fopen("out_residual.txt","a");  //open file for report sink energy
			fprintf (pfile2_, "%d-%.4f-%.4f \n", source_id, CURRENT_TIME, energy);
			fclose(pfile2_);
			return (TCL_OK);
		}
	}
        else if (argc == 3) {                            //send energy update message
		if ((strcmp(argv[1], "send") == 0) && (strcmp(argv[2], "EUPDATE"))) {
        		iNode = (MobileNode *) (Node::get_node_by_address(source_id));
//		        iEnergy = iNode->energy_model()->energy();
//                      float energy = iEnergy;
			mysend(0,0,0,argv[2],0,0,energy,0,0,0,0);
			return (TCL_OK);
		}
		else if ((strcmp(argv[1], "send") == 0) && (strcmp(argv[2], "SINKHLO"))) {
        		iNode = (MobileNode *) (Node::get_node_by_address(source_id));
//		        iEnergy = iNode->energy_model()->energy();
//                      float energy = iEnergy;
			mysend(0,0,0,argv[2],0,0,energy,0,0,0,0);
			return (TCL_OK);
		}
	}	
        else if (argc == 6) {                            //send hello message
		if (strcmp(argv[1], "send") == 0) {
                        float x = atof(argv[3]);
                        float y = atof(argv[4]);
//                      float energy = atof(argv[5]);
//			mysend(0,0,0,argv[2],x,y,energy,0,0,0,0);
                        float i_energy = atof(argv[5]);
			mysend(0,0,0,argv[2],x,y,i_energy,0,0,0,0);
			return (TCL_OK);
		}
	}
        else if (argc == 5) {                            //send data message
		if (strcmp(argv[1], "send") == 0) {
                        float xpos = atof(argv[3]);
                        float ypos = atof(argv[4]);
		        nearsink_id = 0;
	                mysend(0,0,0,argv[2],xpos,ypos,0,nearsink_id,sink1_x,sink1_y,0);
			return (TCL_OK);
		}
		else if (strcmp(argv[1], "results") == 0) {
                        int nodes = atoi(argv[2]);
                        float nop = atof(argv[3]);
                        int tot_event_nodes = atoi(argv[4]);						
                        result(nodes,nop,tot_event_nodes);
			return (TCL_OK);
		}
	}
        else if (argc == 4) {                            //sink position update
		if (strcmp(argv[1], "sink1_update_pos") == 0) {
                        sink1_x = atof(argv[2]);
                        sink1_y = atof(argv[3]);
			return (TCL_OK);
		}
	}

	return (Agent::command(argc, argv));
}


