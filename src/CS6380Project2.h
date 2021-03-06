/*
 * CS6380Project2.h
 *
 *  Created on: Sep 25, 2014
 *  Authors:
 *      Kenneth Platz
 *      Joshua Olson
 *      Brian Snedic
 */

#ifndef CS6380PROJECT2_H_
#define CS6380PROJECT2_H_

#include <iosfwd>
#include <string>
#include <vector>

#if __cplusplus <= 199711L
#    error  "This project requires a compiler that complies with the"\
" ISO/IEC 14882:2011 standard (aka C++11)"
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define TESTING

// Parse the configuration file.
// Parameters:
//    cfg_file    : Name of the configuration file
//    return_vals : (Reference to) vector of ints to return the node ID's in
//    return_nbrs : (Reference to) vector of vector of ints to return the neighbor array
// Return value:
//    int         : Number of neighbors found

int parse_config( std::string cfg_file,
                  std::vector<int>& return_vals,
                  std::vector<std::vector<int>>& return_nbrs );

#include "Neighbor.h"
//
// Function to run a node
// Parameters:
//    myNode      : The node ID to represent
//    mySock      : My socket to send/receive connections   
//    master      : Address of master thread
//    neighbor    : The addresses of my neighbors
void run_node( int myNode, 
               int mySock,
               const struct sockaddr_in& master,  
               std::vector<Neighbor> neighbors );

extern bool verbose;

#include "Message.h"

#endif /* CS6380PROJECT2_H_ */
