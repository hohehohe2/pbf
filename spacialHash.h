#ifndef HOHE2_spacialHash_H
#define HOHE2_spacialHash_H

#include <vector>
#include <list>
#include <iostream>
#include <assert.h>
#include "particles.h"

namespace hohe2
{
   
///Number of hash size. Must be 2^n.
const int NUMHASHARRAYSIZE = 4096;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
///Linked list node.
template <typename T>
class LinkedListNode
{
    ///Next node.
    T* next_;

    ///Prev node.
    T* prev_;

public:

    ///Constructor.
    LinkedListNode() : next_(NULL), prev_(NULL){}

    ///Destructor.
    virtual ~LinkedListNode(){}

    ///Get the previous node pointer.
    /**
       @return Pointer to the previous node, or NULL if this node is the last.
     */
    inline T* prev(){return prev_;}

    ///Get the previous node pointer.
    /**
       @return Const pointer to the previous node, or NULL if this node is the last.
     */
    inline const T* prev() const{prev_;}

    ///Get the next node pointer.
    /**
       @return Pointer to the next node, or NULL if this node is the last.
     */
    inline T* next(){return next_;}

    ///Get the next node pointer.
    /**
       @return Const pointer to the next node, or NULL if this node is the last.
     */
    inline const T* next() const{return next_;}

    ///Insert this node after a node in a list. This node must not be in a list already.
    /**
       @param [in] newPrev New previous node.
       @return None.
     */
    inline void insert(T& newPrev);

    ///Delete this node from a list. You cannot delete a start node.
    /**
       @return Previous node pointer it had.
     */
    inline T* deleteMe();
};

//---------------------------------------------------------------------------
template < typename T >
void LinkedListNode < T > ::insert(T& newPrev)
{
    next_ = newPrev.next_;
    prev_ = &newPrev;
    //Not dynamic_cast for efficiecy.
    newPrev.next_ = static_cast < T* > (this);
    if (next_)
    {
        //Not dynamic_cast for efficiecy.
        next_->prev_ = static_cast < T* > (this);
    }
}

//---------------------------------------------------------------------------
template < typename T >
T* LinkedListNode < T > ::deleteMe()
{
    if ( ! prev_)
    {
        //Not in a list.
        return NULL;
    }
    T* ret = prev_;
    prev_->next_ = next_;
    if (next_)
    {
        next_->prev_ = prev_;
    }
    prev_ = NULL;
    next_ = NULL;
    return ret;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
///Cell class.
class HashCell : public LinkedListNode < HashCell > 
{
    static const int NUM_INIT_RESERVED_IDS_IN_CELL_ = 8;

    short cx_, cy_, cz_;

    //NOTE: Should it be a list?
    //NOTE: To use it, it is much more convenient than making it a list
    //NOTE: but we also want to add/remove ids randomly, hmmm...
    ///Particle id container.
    std::vector < int > particleIds_;

    std::list < HashCell* > adjacentCells_;

    mutable int mark_;

public:

    ///Constructor.
    /**
       @param [in] cx Cell index.
       @param [in] cy Cell index.
       @param [in] cz Cell index.
     */
    HashCell(short cx=0, short cy=0, short cz=0) : cx_(cx), cy_(cy), cz_(cz), mark_(0)
    {
        particleIds_.reserve(NUM_INIT_RESERVED_IDS_IN_CELL_);
    }

    ///Destructor.
    virtual ~HashCell(){}

    ///Set cell indexes.
    /**
       @param [in] cx Cell index.
       @param [in] cy Cell index.
       @param [in] cz Cell index.
       @return None.
     */
    void setIndexes(short cx, short cy, short cz){cx_ = cx; cy_ = cy; cz_ = cz;}

    ///Get the indexes of this cell.
    /**
       @param [out] cx Cell index.
       @param [out] cy Cell index.
       @param [out] cz Cell index.
       @return None.
     */
    inline void getCellIndex(short& cx, short& cy, short& cz)const {cx = cx_; cy = cy_; cz = cz_;}

    ///Test if this cell has some given indexes.
    /**
       @param [in] cx Cell index.
       @param [in] cy Cell index.
       @param [in] cz Cell index.
       @return true if this cell has the indexes, false otherwise.
     */
    inline bool isSameCell(short cx, short cy, short cz) const{return cx_ == cx && cy_ == cy && cz_ == cz;}

    ///Calculate hash value (static).
    /**
       @param [in] cx Cell index.
       @param [in] cy Cell index.
       @param [in] cz Cell index.
       @return Hash value.
     */
    inline static int hashValue(short cx, short cy, short cz);

    ///Calculate hash value of this cell.
    /**
       @return Hash value.
     */
    inline int hashValue() const{return hashValue(cx_, cy_, cz_);}

    ///Add a particle id.
    /**
       @param [in] pid Particle id.
       @return None.
     */
    inline void addParticleId(int pid) {particleIds_.push_back(pid);}

    ///Get the particle id array.
    /**
       @return Const array of particle ids.
     */
    inline const int* getParticleIds() const{return particleIds_.data();}

    ///Get the number of particles in this cell.
    /**
       @return Number of particles.
     */
    inline int getNumParticles() const{return particleIds_.size();}
    
    ///Remove particles that are not in this cell any more.
    /**
       @param [in] positions Position data.
       @param [out] removedIds Ids of particles that are removed. It is not cleared first.
       @return true if the cell gets empty.
     */
    bool removeMovedOutParticles(const float* positions, std::list < int >& removedIds);

    ///Get the list of adjacent cells.
    /**
       @return Reference of the list of adjacent cell.
    */
    std::list < HashCell* >& adjacentCells(){return adjacentCells_;}

    ///Get the list of adjacent cells.
    /**
       @return Reference of the list of adjacent cell.
    */
    const std::list < HashCell* >& adjacentCells() const{return adjacentCells_;}

    ///Set multi purpose mark.
    /**
       @param [in] mark Mark value.
       @return None.
    */
    void setMark(int mark) const{mark_ = mark;}

    ///Get multi purpose mark value.
    /**
       @return Mark value.
    */
    int getMark() const{return mark_;}
};

//---------------------------------------------------------------------------
inline int HashCell::hashValue(short cx, short cy, short cz)
{
    const int p1 = 73856093;
    const int p2 = 19349663;
    const int p3 = 83492791;
    return ( (cx * p1) xor (cy * p2) xor (cz * p3)) & (NUMHASHARRAYSIZE - 1);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
///Free list.
class CellFreeList
{
    static const int NUM_CELL_CREATES_ATONCE_ = 256;
    //Dummy cell indicating the beggining of the list.
    //NOTE: There is a list implementation that doesn't use dummy but don't worry.
    HashCell start_;
    HashCell* end_;

public:

    ///Constructor.
    /**
     */
    CellFreeList() : end_(&start_){}

    ///Get a pointer to a free HashCell object.
    /**
       @return HashCell pointer.
     */
    HashCell* get()
    {
        if (end_ == &start_)
        {
            //No Free node found. Create some.
            //NOTE: By design, it is never deleted once it is created.
            HashCell* newCells = new HashCell[NUM_CELL_CREATES_ATONCE_];
            for (int i = 0; i < NUM_CELL_CREATES_ATONCE_; ++i)
            {
                add(newCells[i]);
            }
        }

        //Found free node, return it. Returned node is not in any list.
        HashCell* ret = end_;
        end_ = end_->deleteMe();
        return ret;
    }

    ///Register an unsued HashCell object to this free list.
    /**
       @param [in] unusedCell HashCell object to add.
     */
    void add(HashCell& unusedCell)
    {
        unusedCell.insert(*end_);
        end_ = &unusedCell;
    }
};


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
///Spacial hash.
class SpacialHash
{
    ///Hash.
    std::vector < HashCell > harray_; //Contains start nodes.

    ///Free list of cell node objects.
    CellFreeList freeList_;

public:
    ///Constructor.
    SpacialHash() : harray_(NUMHASHARRAYSIZE){}

    ///Get a cell from its indexes.
    /**
      @param [in] cx Cell index to get.
      @param [in] cy Cell index to get.
      @param [in] cz Cell index to get.
      @return Cell object, or NULL if not found.
     */
    HashCell* getCell(short cx, short cy, short cz);

    ///Get a cell from its indexes. Create one if not found. Use cell.deleteMe() to remove a cell from the hash.
    /**
      @param [in] cx Cell index to get.
      @param [in] cy Cell index to get.
      @param [in] cz Cell index to get.
      @return Cell object.
     */
    HashCell* getOrCreateCell(short cx, short cy, short cz);

    ///Add particles. Create cells if not found enough.
    /**
       @param [in] positions Particle position data.
       @param [in] numParticles Number of particles to add.
       @return None.
     */
    void addParticles(const float* positions, int numParticles);

    ///Restructure the spacial hash.
    /**
       @param [in] positions Particle position data.
       @return None.
     */
    void restructure(const float* positions);


    ///Returns the max number of particles in a cell.
    /**
       @return Max number of particles in a cell.
     */
    int numMaxCellParticles() const;


    ///Returns the list of every particle cells.
    /**
       @param [out] cells List of every cells. It is not cleared inside the method.
       @return Number of every particles.
     */
    int getEveryCells(std::vector < const HashCell* >& cells) const;


private:
    ///Iterate over every cell and get every particles ids that are not in the right cell.
    /**
       @param [in] positions Particle position data.
       @param [out] removedIds Ids of particles that are removed. It is not cleared first.
       @param [out] emptyCells List of pointers to empty cells. It is not cleared first.
       @return None.
     */
    void removeParticles_(const float* positions,
                          std::list < int >& removedIds,
                          std::list < HashCell* >& emptyCells);

    ///Add given particles to the right cell. A Cell is created if no right one found.
    /**
       @param [in] positions Position data.
       @param [in] particleIds Particle ids.
       @return None.
     */
    void addParticles_(const float* positions, const std::list < int >& particleIds);

    ///Remove empty cell. Searches only cells in a given list.
    /**
       @param [in] emptyCells List of pointers to cells to check.
       @return None.
     */
    void removeCells_(std::list < HashCell* > emptyCells);

    ///Set adjacent cell pointers to every cell.
    /**
       @return None.
     */
    void setAdjacentCells_();

    //For tests.
    friend class createSpacialHashTest;
};


}

#endif
