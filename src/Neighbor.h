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
	int                  id;
	int                  vecpos;       // My position in the vector
	float                weight;
        enum EdgeType {
            NBR_CANDIDATE,
            NBR_TREE,
            NBR_REJECT
        }                    edgeType;
public:
	Neighbor( const int i, const int v, const int w ) :
		id(i), vecpos(v), weight(w), edgeType(NBR_CANDIDATE) {};
	Neighbor( const Neighbor& ) = default;
	Neighbor( Neighbor&& ) = default;

	inline Neighbor& operator=( const Neighbor& ) = default;
	inline bool operator<( const Neighbor& n ) { return id < n.id; }   // to be used in sorting
	inline bool operator==( const Neighbor& n ) { return id == n.id; } // to be used in sets

	inline Edge getEdge(int me) {
		return Edge{ me, id, weight };
	}
	inline int getWeight() { return weight; }
        inline enum EdgeType getEdgeType() { return edgeType; }
};

#endif /* NEIGHBOR_H_ */
