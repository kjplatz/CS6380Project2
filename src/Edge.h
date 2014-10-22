/*
 * Edge.h
 *
 *  Created on: Oct 18, 2014
 *      Author: ken
 */

#ifndef EDGE_H_
#define EDGE_H_

class Edge {
    int end1, end2, weight;
public:
    Edge( int w, int e1, int e2 ) : weight(w) {
        if ( e1 < e2 ) {
            end1 = e1, end2 = e2;
        } else {
            end1 = e2, end2 = e1;
        }
    }
    Edge( const Edge& ) = default;
    Edge( Edge&& ) = default;

    Edge& operator=( const Edge& ) = default;

    bool operator<( const Edge& e2 ) {
        return ( weight == e2.weight ) ?
               (( end1 == e2.end1 ) ? ( end2 < e2.end2 ) : ( end1 < e2.end1 ))
               : ( weight < e2.weight );
    }
};




#endif /* EDGE_H_ */
