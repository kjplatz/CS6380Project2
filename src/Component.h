#ifndef __COMPONENT_H__
#define __COMPONENT_H__

class Component {
    int leader;
    int level;
    Edge id;
public:
    Component( int _ldr, int lvl, const Edge& _e ) 
        : leader(_ldr), level(lvl), id(_e) 
    {};

    Component( const Component& ) = default;
    Component( const Component&& ) = default;

    inline int getLeader() const { return leader; }
    inline int getLevel() const { return level; }
    inline Edge getID() const { return id; }
};

#endif
