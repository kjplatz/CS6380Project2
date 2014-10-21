/*
 * Message.h
 *
 *  Created on: Sep 25, 2014
 *  Authors:
 *      Kenneth Platz
 *      Joshua Olson
 *      Brian Snedic
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>

/*
 * class Message:
 *     This creates an abstraction of a Message that can be sent and
 *     received between peers or between a child and the master.
 */
class Message {
    int rcvd;
public:
    enum MsgType {  MSG_NULL=0, MSG_TICK, MSG_DONE, MSG_EXPLORE,
                    MSG_REJECT, MSG_LEADER,
                    MSG_HELLO,         // Initiate contact with neighbor
					MSG_ACK,           // Acknowledge neighbor's HELLO
					MSG_BEGIN,         // Message from master->children to start algorithm
					MSG_QUERY,         // What is your component ID?         (MWOE->MWOE)
					MSG_COMPONENT_ID,  // Reply with my component ID & level (MWOE->MWOE)
					MSG_NEW_COMPONENT, // Announce the ID, Leader, and Level of the new component
					MSG_INITIATE_PHASE,// Leader sends to initiate a phase of the MST algorithm
					                   // (leader->leaf)
					MSG_BEST_EDGE,     // My best edge (leaf->leader)
					MSG_ATTEMPT_EDGE,  // The edge to try (leader->leaf)
					MSG_MERGE,         // Attempt to merge with other component (MWOE->MWOE)
					MSG_ABSORBME       // Tell other component to absorb me     (MWOE->MWOE)
                 } msgType;
    int  id;
    // Read a message from a file descriptor
    Message() = delete;
    Message( const Message& ) = default;
    Message( Message&& );
    Message( int fd );

    Message& operator=( const Message& m ) = default;

    // Create a message
    Message( enum MsgType mt, int _id=-1 ) : msgType(mt), id(_id) {};

    // Send this message to the named file descriptor
    int send( int fd );

    int send( int fd, const struct sockaddr_in& dest );

    // Generate a string representation of this message
    std::string toString() const;
};


#endif /* MESSAGE_H_ */
