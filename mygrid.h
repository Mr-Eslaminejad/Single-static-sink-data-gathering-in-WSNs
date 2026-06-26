#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"
#include "mobilenode.h"

#define MAX_NODE 50
#define CURRENT_TIME    Scheduler::instance().clock()

struct hdr_mygrid {
	int packet_id;
	int source_id;
        float X,Y;
        float sink_xp;
        float sink_yp;
        int sink_id;
        float energy;
        int recv_id;
	int original_id;
	int packet_type;
//	bool nei1[MAX_NODE];
        int hopcount;
     //   bool nei2[MAX_NODE][MAX_NODE];
	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static hdr_mygrid* access(const Packet* p) {
		return (hdr_mygrid*) p->access(offset_);
	}
};

class MyGridAgent : public Agent {
public:
	MyGridAgent();
	virtual int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*);
	void mysend(int send_type, int pckt, int orgnl, const char* packet_type, float xpos, float ypos, float initenergy, int sink, float sink_x, float sink_y, int hopcount);
//        void sink_pos_update(float sink1_xpos, float sink1_ypos, float sink2_xpos, float sink2_ypos, int sink1, int sink2, float initenergy);
//	void MPR_selection();
//	void new_MPR_selection();
//	bool is_covered(bool *a, bool *b);
//	int fitness(bool *a, bool *b);
//	int find_cover(bool *benchmark, bool *input);
	void print_nei1();
	void print_pos();
	void update_position();
	void update_pos();
	void printsinkpos();
//	void find_nearest_sink(const char* packet_type, float xpos, float ypos);
	void result(int non, float nop, int tot_event_nodes);
//        void random_source_select();

//----------counter for counting the number of node neighbors----
        int rid;  

//----------node structure and routing table array---------------
        struct nodestruct {
        int id;
        float x;
        float y;
        float residual_e; 
        };
        struct nodestruct routingtable[12];
//---------------------------------------------------------------

        //float s1_y;
        //float s2_x;
        //float s2_y;
        int R;   //radio range
        int sw;
	int source_id;
	int rcvd_packet_id;
	int rcvd_original_id;
        float rcvd_x;
        float rcvd_y;
        float rcvd_sinkx;
        float rcvd_sinky;
	int rcvd_hopcount;
//	float posx,posy;
	float sink1_x,sink1_y;
        int rcvd_sinkid;
	bool nei1[MAX_NODE], nei2[MAX_NODE][MAX_NODE], MPR[MAX_NODE], as_MPR[MAX_NODE];
	bool already_recieved,already_MPR;
//        float d1, d2;
//        float min_d;
//        int min_nodeid;
        float nearsink_x;
        float nearsink_y;
        float nearsink_id;
        int rcvd_packets;  //number of packets received at sink node succesfully
        float min_e;       //minimum energy for communicating
        long int total_hopcount; //total hopcount of packets received at sink node succesfully 
protected:
	float posx,posy;
        double iEnergy;
        MobileNode *iNode;
	double energy;
private:
        double s1_x;
};
