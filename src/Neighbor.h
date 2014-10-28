/*
 * Neighbor.h
 *
 *  Created on: Oct 19, 2014
 *      Author: ken
 */

#ifndef NEIGHBOR_H_
#define NEIGHBOR_H_


#include "Edge.h"

class Neighbor {
    int id;
    struct sockaddr_in addr;
    int weight;
public:
    Neighbor( const int i, const struct sockaddr_in a, const int w ) :
        id(i), addr(a), weight(w) {};
    Neighbor( const Neighbor& ) = default;
    Neighbor( Neighbor&& ) = default;

    inline Neighbor& operator=( const Neighbor& ) = default;
    inline bool operator<( const Neighbor& n ) {
        return id < n.id;    // to be used in sorting
    }
    inline bool operator==( const Neighbor& n ) {
        return id == n.id;    // to be used in sets
    }

    inline Edge getEdge(int me) {
        return Edge { me, id, weight };
    }
    inline struct sockaddr_in getAddr() {
        return addr;
    }
    inline int getWeight() {
        return weight;
    }
};

#endif /* NEIGHBOR_H_ */
