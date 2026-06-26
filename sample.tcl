# for each scenario we should set max_event_nodes, R (radio range), ST, val(x), val(y), nn, initenergy, num_row, nop, src and sink positions and sink movement

#Define a 'pr_nei1' function for the class 'Agent/MyGrid'

#Agent/MyGrid instproc pr_dead {sid t deadno} {
#	$self instvar node_
#	puts "node $sid is dead at time= $t while the number of dead nodes is $deadno"
#}

Agent/MyGrid instproc pr_nei1 {sid xid yid} {
	$self instvar node_
	puts "node [$node_ id] : node $sid is my 1st hop neighboor. x= $xid and y= $yid"
}

Agent/MyGrid instproc pr_pos {xid yid} {
	$self instvar node_
	puts "node [$node_ id] :  x= $xid and y= $yid"
}

Agent/MyGrid instproc pr_sink_pos {xid1 yid1} {
	$self instvar node_
	puts "========================================================"
	puts "node [$node_ id] : Sink1 Position  x= $xid1 and y= $yid1"
	puts "========================================================"
}

Agent/MyGrid instproc up_pos1 {xid yid ctime} {
	global ns_ p n
	$self instvar node_
	for {set i 0} {$i < $n } {incr i} {
		$ns_ at $ctime "$p($i) sink1_update_pos $xid $yid"
	}
}

Agent/MyGrid instproc pr_ack {sid t rp th} {
	$self instvar node_
	puts "Sink [$node_ id] receives $rp packets from source $sid till $t -tot_hop $th"
}

Agent/MyGrid instproc pr_relay {sid t} {
	$self instvar node_
	puts "Relay node [$node_ id] forwards a packet from source node $sid at $t"
}

# ===========================================================================
# General Layer 2 and 3 Options
# ===========================================================================
set val(chan)           Channel/WirelessChannel    ;# channel type
set val(prop)           Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)          Phy/WirelessPhy            ;# network interface type
set val(mac)            Mac/802_11	               ;# MAC type
set val(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)             LL                         ;# link layer type
set val(ant)            Antenna/OmniAntenna        ;# antenna model
set val(ifqlen)         500                        ;# max packet in ifq
set opt(nn)	      530		      ;# number of mobilenodes
set val(rp)             DumbAgent                  ;# routing protocol
set val(x)	      2000.00		      ;# Width od the train
set val(y)	      2000.00		      ;# Length of train
set val(energymodel)   EnergyModel		    ;# Energy Model
set val(initialenergy) 4			    ;# value
# ===========================================================================
# Wireless Physical Layer Options
# ===========================================================================
#Phy/WirelessPhy set CPThresh_ 10.0
#Phy/WirelessPhy set CSThresh_ 2.78869e-9	  ;# Carrier Sense Threshold 	
#Phy/WirelessPhy set RXThresh_ 2.78869e-9 	  ;# Receiving Threshold
#Phy/WirelessPhy set Rb_ 1e6 			  ;# Bandwidth Rate
#Phy/WirelessPhy set Pt_ 0.281838 		  ;# Power Transmission
#Phy/WirelessPhy set freq_ 2.4e+9		  ;# Transmission Range 100m 
#Phy/WirelessPhy set L_ 1.0

Phy/WirelessPhy set CPThresh_ 10.0
#Phy/WirelessPhy set CSThresh_ 3.652e-10
Phy/WirelessPhy set CSThresh_ 3.41828e-08
  #Phy/WirelessPhy set RXThresh_ 3.652e-10 ;# Receiving Threshold
Phy/WirelessPhy set RXThresh_ 3.41828e-08 ;# Receiving Threshold 75m
Phy/WirelessPhy set Rb_ 1e6 ;# Bandwidth
Phy/WirelessPhy set Pt_ 0.28183815 ;# 40 m
Phy/WirelessPhy set freq_ 914e+6
Phy/WirelessPhy set L_ 1.0

# ===========================================================================
# Main Program Body
# ===========================================================================

#
# Initialize Global Variables
#

set ns_         [new Simulator]
set tracefd     [open simple_grid.tr w]
set n $opt(nn)
set ST 3600             ;#simulation time
#set nf [open simple_grid.nam w]

$ns_ trace-all $tracefd
set namtrace [open simple_grid.nam w]
$ns_ namtrace-all-wireless $namtrace $val(x) $val(y)

# set up topography object
set topo       [new Topography]

$topo load_flatgrid $val(x) $val(y)

#
# Create God
#
create-god $opt(nn)

#Configuring wireless channel
#Phy/WirelessPhy set CPThresh_ 10.0
#Phy/WirelessPhy set CSThresh_ 3.652e-10
  #Phy/WirelessPhy set RXThresh_ 3.652e-10 ;# Receiving Threshold
#Phy/WirelessPhy set RXThresh_ 3.00435e-08 ;# Receiving Threshold
#Phy/WirelessPhy set Rb_ 1e6 ;# Bandwidth
#Phy/WirelessPhy set Pt_ 0.281838 ;# 40 m
#Phy/WirelessPhy set freq_ 914e+6
#Phy/WirelessPhy set L_ 1.0

#  Create the specified number of mobilenodes [$opt(nn)] and "attach" them
#  to the channel. 
#  Here two nodes are created : node(0) and node(1)
# configure node
set chan_1_ [new $val(chan)]

        $ns_ node-config -adhocRouting $val(rp) \
                         -llType $val(ll) \
                         -macType $val(mac) \
                         -ifqType $val(ifq) \
                         -ifqLen $val(ifqlen) \
                         -antType $val(ant) \
                         -propType $val(prop) \
                         -phyType $val(netif) \
                         -channel $chan_1_ \
                         -topoInstance $topo \
                         -agentTrace ON \
                         -routerTrace OFF \
                         -macTrace OFF \
                         -movementTrace OFF \
                         -energyModel $val(energymodel) \
                         -initialEnergy $val(initialenergy) \
                         -rxPower 35.28e-3 \
                         -txPower 31.32e-3 \
	                 -idlePower 712e-6 \
	                 -sleepPower 144e-9                  
                      

for {set i 0} {$i < $opt(nn) } {incr i} {
        set node_($i) [$ns_ node]       
        $node_($i) random-motion 0                      ;# enable random motion
}

# ------- Provide initial (X,Y, for now Z=0) co-ordinates for mobilenodes----------
set num_row 23
set R 75
#set R 70
#set R1 70
set k 0;
while {$k < $num_row} {
 for {set i 0} {$i < $num_row} {incr i} {
   set m [expr ($i+$k*$num_row)+1];
   $node_($m) set X_ [expr $i*$R+20.0];
   $node_($m) set Y_ [expr $k*$R+20.0];
   $node_($m) set Z_ 0.0;
   set postable(0,$m) [expr $i*$R+20.0];
   set postable(1,$m) [expr $k*$R+20.0];
   }
   incr k;
}

# ------------ (0) is mobile sink ----------

set midx [expr ((($num_row-1)/2)*$R)+20]
set midy [expr ((($num_row-1)/2)*$R)+20]
set fullx [expr (($num_row-1)*$R)+20]
set fully [expr (($num_row-1)*$R)+20]

$node_(0) set X_ $midx 
$node_(0) set Y_ $midy 
$node_(0) set Z_ 0.0 

set postable(0,0) $midx 
set postable(1,0) $midy 



# --------------print the positions of nodes------------

#for {set i 0} {$i < $opt(nn) } {incr i} {
#puts " the x position is $postable(0,$i) and the Y position is $postable(1,$i)"
#}

#---------------set size of nodes-----------------------

for {set i 0} {$i < $opt(nn) } {incr i} {
	$ns_ initial_node_pos $node_($i) 20             ;# set size of nodes
}

#---------------insert agent----------------------------

for {set i 0} {$i < $opt(nn) } {incr i} {
	set p($i) [new Agent/MyGrid]
	$ns_ attach-agent $node_($i) $p($i)
	$p($i) set node_source_id $i
}

#---------------send HELLO packets----------------------
set initenergy 2
set j 0.001
set sensor $opt(nn)
for {set i 0} {$i < $sensor} {incr i} {
        set xpos $postable(0,$i)
        set ypos $postable(1,$i)
       # puts "$xpos and $ypos"
	$ns_ at $j "$p($i) send HLO $xpos $ypos $initenergy"            ;# send HELLO packets
	set j [ expr $j + 0.003 ]
}

#---------------start multo-hop communication_src (source) sends data packet for sinks (destination)---------

set firetime 5.505
set n_sensor [expr $sensor-1]
set period 6.000	;#period of time between two packet sending
set nop 10              ;#number of packets sent by a source node
set total_event_nodes 0	;#sum of all event nodes during the simulation used for calculating total number of packets
set R_event 75
while {$firetime < $ST} {
	set rnd_f [expr {1 + floor(rand()* $n_sensor)}]         ;#Random variable as source between 0 to 24 
#	set src [expr int($rnd_f)]
	set event [expr int($rnd_f)]				;#define a node as event point
	set event_nodes 0
	for {set i 1} {$i < $sensor } {incr i} {	;#finding neighboring nodes at event point
		set xx [expr $postable(0,$event)-$postable(0,$i)]
	        set yy [expr $postable(1,$event)-$postable(1,$i)]
	        set pow_xx [expr $xx*$xx]
	        set pow_yy [expr $yy*$yy]
	        set sum_xy [expr $pow_xx + $pow_yy]
	        set dton [expr sqrt($sum_xy)]
		if {$dton <= $R_event} {
			set src($event_nodes) $i
			set event_nodes [expr $event_nodes + 1]
		}
	}

	set total_event_nodes [expr $total_event_nodes + $event_nodes]

	set event_x $postable(0,$event)
	set event_y $postable(1,$event)

	for {set i 0} {$i < $nop } {incr i} {
		set multifire $firetime
		for {set j 0} {$j < $event_nodes } {incr j} {
			set xpos $postable(0,$src($j))
			set ypos $postable(1,$src($j))
			$ns_ at $multifire "$p($src($j)) send DATA $xpos $ypos" 
			set multifire [ expr $multifire + 0.020 ]
		}      
		set firetime [ expr $firetime + $period ]
	}
}

#--------------Sink movement in TCL-------------------

# THE SINK (O) IS STATIC IN THIS METHOD

#---------Sink position update in TCL---------------
set max_event_nodes 2
for {set updatetime 5.5} {$updatetime < $ST } {set updatetime [ expr $updatetime + $period]} {
#	puts $updatetime
	set online $updatetime
	for {set u 0} {$u < $max_event_nodes} {incr u} {
		$ns_ at $online "$p(0) updatepos"
#		$ns_ at $updatetime "$p(0) updatepos"
		set online [expr $online + 0.040]
	}	
}

#---------------send Energy Update packets----------------------

# energy_update_interval value is 100 now
set energy_update_interval 100
while {$energy_update_interval <= $ST} {
#	set packet_send_period $energy_update_interval
	for {set e 0} {$e < $n} {incr e} {
		$ns_ at $energy_update_interval "$p($e) send EUPDATE"     ;# send ENERGY UPDATE packets
#		$ns_ at $packet_send_period "$p($e) send EUPDATE"     ;# send ENERGY UPDATE packets
#		set packet_send_period [ expr $packet_send_period + 0.009 ]
	}
	set energy_update_interval [ expr $energy_update_interval + 100 ]
}

#---------------getting results at the end of program-----------

proc result {} {
	global ns_ p sensor ST nop total_event_nodes
	set nodes [expr $sensor-1]
	for {set e 1} {$e < $sensor} {incr e} {
#		$ns_ at $ST "$p($e) results $nodes $period"     ;# get results like residual energy
		$ns_ at $ST "$p($e) results $nodes $nop $total_event_nodes"     ;# get results like residual energy
	}
}

$ns_ at 1.50000 "$p(2) printsinkpos"

$ns_ at 1.4 "$p(9) print_nei1"

# -----end of program------------------------
$ns_ at $ST "$p(0) print_sink_energy"
$ns_ at $ST "result"
$ns_ at [expr $ST + 0.01] "stop"
$ns_ at [expr $ST + 0.02] "puts \"NS EXITING...\" ; $ns_ halt"

proc stop {} {
    global ns_ tracefd namtrace
    $ns_ flush-trace
    close $tracefd
    close $namtrace
    exec nam simple_grid.nam &
    exit 0
}

puts "Starting Simulation..."
$ns_ run

