/*
 * main.cpp
 *
 *  Created on: Sep 25, 2014
 *  Authors:
 *      Kenneth Platz
 *      Joshua Olson
 *      Brian Snedic
 */

/*
 * CS 6380 - Distributed Computing
 * Fall 2014
 *
 * Programming Assignment #1
 * This project implements the FloodMax leader election protocol
 * in a general network.
 *
 * The program consists of three main portions:
 *     The parse_config() function reads a configuration file
 *     The main() function performs the following task:
 *         Invokes parse_config() to determine the system configuration
 *         Creates a set of socketpair() connections to communicate between
 *             the master and child threads
 *         Creates a set of socketpair() connections to simulate communications
 *             channels between peer threads
 *         Creates a vector of threads to simulate the processes
 *         Runs the main control loop:
 *             - Sends a TICK to all processes
 *             - Waits for a DONE from all processes
 *             - If any process sends a LEADER message, it in turn sends
 *               that LEADER message to all processes to indicate successful
 *               termination.
 *
 * We have tested this program on configurations of up to 80 nodes.
 */

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <unistd.h>

#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include "CS6380Project2.h"
#include "Neighbor.h"
#include "Edge.h"
#include "Message.h"

using namespace std;

bool verbose=false;

int main( int argc, char** argv ) {
    int numNodes;
    vector<vector<int>> neighborWts;
    vector<int> nodeIds;

    if ( argc == 1 ) {
        cerr << "Error: usage: " << argv[0] << " <cfg_file>" << endl;
        return 0;
    }

    string cfg_file { argv[1] };
    numNodes = parse_config( cfg_file, nodeIds, neighborWts );

    if ( argc > 2 && string {argv[2]} == string {"verbose"} ) verbose = true;

    cout << "Processed " << numNodes << " nodes." << endl;
    for( int i=0; i<numNodes; i++ ) {
        cout << "Node " << i << " has ID: " << nodeIds[i] << endl;
        cout << "    -- Neighbors: ";
        for( unsigned j=0; j<neighborWts[i].size(); j++ ) {
            if ( neighborWts[i][j] ) {
                cout << j << "(" << neighborWts[i][j] << ") ";
            }
        }
        cout << endl;
    }
    // file descriptors for threads to talk on...
    vector<int> nodeFds;
    // Addresses of all the neighbors
    vector<struct sockaddr_in> nodeAddrs;

    vector<vector<Neighbor>> neighbors( numNodes );

    // The threads vector
    vector<thread*> threads;

    // We discovered in project 1 that with a sufficiently dense graph 
    // of ~80 nodes, we would run out of file descriptors.
    //
    // As a result, we will just go ahead and attempt to bump up our
    // resources to the hard limit at the beginning of the computation.
    //
    cout << "Attempting to request additional resources..." << flush;
    struct rlimit lim;
    getrlimit( RLIMIT_NOFILE, &lim );
    lim.rlim_cur = lim.rlim_max;
    if ( setrlimit( RLIMIT_NOFILE, &lim ) != 0 ) {
        string err = string { "Unable to setrlimit: " };
        err += strerror( errno );
        throw runtime_error( err );
    }

    // We will need more than the default # of threads.  REALLY
    // Boost to the max.
    getrlimit( RLIMIT_NPROC, &lim );
    lim.rlim_cur = lim.rlim_max;
    if ( setrlimit( RLIMIT_NPROC, &lim ) != 0 ) {
        string err = string { "Unable to setrlimit: " };
        err += strerror( errno );
        throw runtime_error( err );
    }

    cerr << " (successful)" << endl;

    // Create listening sockets for every thread.
    // We're using SCTP and a one-to-many methodology here so we can get away 
    // with two file descriptors per node, instead of two file descriptors per 
    // EDGE.  We should be able to approach 2,000 nodes this way.

    // We're going to use random ports... if we fail to bind to one, we 
    // just pick the next in the random sequence.
    long long seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 generator;
    generator.seed( seed );
    uniform_int_distribution<unsigned short> distribution( 10240, 65534);
    auto rand = bind( distribution, generator );

    int masterSock = socket( AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP );
    int sockfd;

    if ( masterSock < 0 ) {
        string err {"Unable to create socket: "};
        err += strerror( errno );
        throw runtime_error { err };
    }

    // Create a socket for the 'master' thread and bind it to a port
    // Any port will do (in a storm)
    struct sockaddr_in addr, masterAddr;

    if ( listen( masterSock, 1024 ) < 0 ) {
        string err {"Cannot listen: "};
        err += string {strerror(errno)};
        throw runtime_error {err};
    }

    // We don't care what port we actually got, BUT we do need to know
    // which port it happened to be!
    socklen_t addrlen = sizeof( sockaddr_in );
    getsockname( masterSock, (struct sockaddr*)&masterAddr, &addrlen );
    cout << "Master thread listening on port " << ntohs( masterAddr.sin_port ) << endl;

    for( int i=1; i<=numNodes; i++ ) {
        usleep( 250000 );
        sockfd = socket( AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP );   // Open a new socket
        if ( sockfd < 0 ) {
            string err {"Unable to create socket: "};
            err += strerror( errno );
            throw runtime_error { err };
        }
        if ( listen( sockfd, 1024 ) < 0 ) {                         // Attempt to listen
            string err {"Cannot listen: "};
            err += string {strerror(errno)};
            throw runtime_error {err};
        }
        getsockname( sockfd, (struct sockaddr*)&addr, &addrlen );   // Record the address
        cout << "Thread " << i <<
             " is listening on port " <<
             ntohs(addr.sin_port) << endl;

        // Again, store the file descriptor & address
        nodeFds.push_back( sockfd );
        nodeAddrs.push_back( addr );
    }

    // Now create the neighbor list...
    cout << "Constructing neighbor lists..." << flush;
    for( int i=0; i<numNodes; i++ ) {
        for( int j=i+1; j<numNodes; j++ ) {
            // Assume that all paths are bidirectional and equally weighted...
            // pick the max weight of the direction in event of conflict.
            int weight = max( neighborWts[i][j], neighborWts[j][i] );
            if ( weight == 0 ) continue;


            neighbors[i].push_back( Neighbor { j, nodeAddrs[j], weight });
            neighbors[j].push_back( Neighbor { i, nodeAddrs[i], weight });
        }
        cout << "." << flush;
    }
    cout << endl;
    usleep( 250000 );

    for( int i=0; i<numNodes; i++ ) {
        threads.push_back( new thread { run_node, nodeIds[i], nodeFds[i], masterAddr, neighbors[i] } );
    }
    usleep( 250000 );

    Message begin { Message::MSG_BEGIN };
    for( auto dest : nodeAddrs ) {
        begin.send( masterSock, dest );
    }

    string str;
    cin >> str;
    /*
    int sockpair[2];


    for( int i=0; i<numNodes; i++ ) {

        // Use local socket pairs for communications -- this simulates network communications without
        // having to do the listen/connect rigamarole.
        if ( socketpair( AF_LOCAL, SOCK_SEQPACKET, 0, sockpair ) < 0 ) {
            string err( "Unable to create local socket pair: " );
            err += strerror( errno );
            cerr << err << endl << flush;

            throw runtime_error( err );
        }

        master_fds.push_back( sockpair[0] );

        // File descriptor we'll talk to our master on...
        int master_fd = sockpair[1];

        verbose && cout << "Thread " << i << " has socketpair " << sockpair[0] << ", " << sockpair[1] << endl;

        for( int j=0; j<numNodes; j++ ) {
            if ( i == j ) continue;   // A node can't be its own neighbor...
            if ( neighbors[i][j] ) {

                // Note, we are using socketpair() here.  A (successful) call to
                // socketpair() returns two local socket file descriptors at either
                // endpoint of an established communications channel.  This allows
                // us to effectively model a distributed system via message-passing.
                // These operate much like other types of sockets, except that
                // all communications happen within the kernel.

                if ( socketpair( AF_LOCAL, SOCK_SEQPACKET, 0, sockpair ) < 0 ) {
                    string err( "Unable to create local socket pair: " );
                    err += strerror( errno );
                    throw runtime_error( err );
                }

                // Add the appropriate file descriptors to process i's and j's neighbor
                // lists.
                neighbor_fds[i].push_back( sockpair[0] );
                neighbor_fds[j].push_back( sockpair[1] );

                verbose && cout << "Creating socketpair [" << sockpair[0] << ", " << sockpair[1] << "] for nodes "
                                << i << " <-> " << j << endl;

                // Set neighbors[j][i] to false so we don't create duplicate channels.
                neighbors[j][i] = false;
            }
        }

        //
        // Spawn a new thread and push it back into the thread vector array...
        threads.push_back( new thread( run_node, nodeIds[i], master_fd, neighbor_fds[i] ) );
    }

    int round=0;

    int leader = -1;
    while( leader < 0 ) {
        round++;
        sleep(1);

        Message tick( Message::MSG_TICK, round );

        // Send a tick to everyone
        cout << "Sending to " << master_fds.size() <<
             " threads: " << tick.toString() << endl;
        for( auto fd : master_fds ) {
            tick.send( fd );
        }

        sleep(1);

        int doneCount=0;
        // Read a message from each process file descriptor... this will either be a
        // LEADER message or a DONE message.
        for( unsigned i=0; i<master_fds.size(); i++ ) {
            // Get the message and parse it...
            Message msg( master_fds[i] );

            switch( msg.msgType ) {
            case Message::MSG_LEADER:  // I know who the leader is.  Ignore me from here on out
                cout << "Got message from fd " << master_fds[i] << ": " << msg.toString() << endl;
                leader = msg.id;
                break;
            // fall through
            case Message::MSG_DONE:    // I don't know, but I'm done with this round.
                doneCount++;
                break;

            default:
                throw runtime_error(
                    string {"Master thread received unexpected message: "} +
                    msg.toString() );
            }
        }
        cout << "Received DONE from " << doneCount << " threads." << endl;
    }

    cout << "got LEADER(" << leader << ")" << endl;

    queue<int, list<int>> bfsQ;
    bfsQ.push( leader );
    Message leaderMsg { Message::MSG_LEADER, leader };
    string ignore;
    while( !bfsQ.empty() ) {
        int node = bfsQ.front();

        verbose && cout << "Popped node " << node << " from queue..." << endl;
        bfsQ.pop();
        verbose && cout << "Queue length = " << bfsQ.size() << endl;
        unsigned i=0;
        verbose && cout << "Queue length = " << bfsQ.size() << endl;
        while( nodeIds[i] != node && i < nodeIds.size() ) i++;
        verbose && cout << "Node " << node << " is number " << i << endl;

        int fd = master_fds[i];
        verbose && cout << "Sending " << leaderMsg.toString() << " to node " << node << "[" << fd << "]" << endl;
        leaderMsg.send( fd );
        Message done( fd );

        ifstream children("children.txt");

        do {
            children >> node;
            if ( node != 0 ) {
                verbose && cout << "Adding node " << node << endl;
                bfsQ.push( node );
            }
        } while( node != 0 );
        children.close();

        unlink( "children.txt" );
    }

    for( auto thread : threads ) {
        thread->detach();
    }

    cout << "All threads terminated... exiting" << endl;
    */
}



