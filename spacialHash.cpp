#include <iostream>
#include "spacialhash.h"

using namespace hohe2;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HashCell::removeMovedOutParticles(const float* positions, std::list < int >& removedIds)
{
    std::vector < int > :: iterator it = particleIds_.begin();
    for (; it != particleIds_.end();)
    {
        int pid = *it;
        short cx, cy, cz;
        BaseParticles::getCellIndexes(positions + pid * 3, cx, cy, cz);
        if ( (cx != cx_) || (cy != cy_) || (cz != cz_))
        {
            removedIds.push_back(*it);
            it = particleIds_.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return ( ! particleIds_.size());
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HashCell* SpacialHash::getCell(short cx, short cy, short cz)
{
    HashCell& start = harray_[HashCell::hashValue(cx, cy, cz)];
    HashCell* node = start.next(); //start is dummy.
    while(node)
    {
        if (node->isSameCell(cx, cy, cz))
        {
            //found.
            return node;
        }
        node = node->next();
    }

    //Not found.
    return NULL;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HashCell* SpacialHash::getOrCreateCell(short cx, short cy, short cz)
{
    HashCell* cell = getCell(cx, cy, cz);
    if (cell)
    {
        return cell;
    }

    //Not found.
    HashCell* newCell = freeList_.get();
    newCell->setIndexes(cx, cy, cz);
    HashCell& start = harray_[HashCell::hashValue(cx, cy, cz)];
    newCell->insert(start);
    return newCell;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SpacialHash::addParticles(const float* positions, int numParticles)
{
    for (int pid = 0; pid < numParticles; ++pid)
    {
        short cx, cy, cz;
        BaseParticles::getCellIndexes(positions + pid * 3, cx, cy, cz);
        getOrCreateCell(cx, cy, cz)->addParticleId(pid);
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SpacialHash::restructure(const float* positions)
{
    std::list < int > removedIds;
    std::list < HashCell* > emptyCells;
    removeParticles_(positions, removedIds, emptyCells);
    addParticles_(positions, removedIds);
    removeCells_(emptyCells);
    setAdjacentCells_();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SpacialHash::removeParticles_(const float* positions,
                      std::list < int >& removedIds,
                      std::list < HashCell* >& emptyCells)
{
    int hsize = harray_.size();
    HashCell* hdata = harray_.data();
    for (int i = 0; i < hsize; ++i)
    {
        HashCell* cell = (hdata + i)->next();
        while (cell)
        {
            if (cell->removeMovedOutParticles(positions, removedIds))
            {
                emptyCells.push_back(cell);
            }
            cell = cell->next();
        }
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SpacialHash::addParticles_(const float* positions, const std::list < int >& particleIds)
{
    std::list < int > :: const_iterator it = particleIds.begin();
    for (; it != particleIds.end(); ++it)
    {
        int pid = *it;
        short cx, cy, cz;
        BaseParticles::getCellIndexes(positions + pid * 3, cx, cy, cz);
        getOrCreateCell(cx, cy, cz)->addParticleId(pid);
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SpacialHash::removeCells_(std::list < HashCell* > emptyCells)
{
    std::list < HashCell* > :: iterator it = emptyCells.begin();
    for (; it != emptyCells.end(); ++it)
    {
        HashCell* cell = *it;
        if ( ! cell->getNumParticles())
        {
            cell->deleteMe();
            freeList_.add(*cell);
        }
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int SpacialHash::numMaxCellParticles() const
{
    int max = 0;
    int hsize = harray_.size();
    const HashCell* hdata = harray_.data();
    for (int i = 0; i < hsize; ++i)
    {
        const HashCell* cell = (hdata + i)->next();
        while (cell)
        {
            int size = cell->getNumParticles();
            if (max < size)
            {
                max = size;
            }
            cell = cell->next();
        }
    }
    return max;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int SpacialHash::getEveryCells(std::vector < const HashCell* >& cells) const
{
    int hsize = harray_.size();
    const HashCell* hdata = harray_.data();
    int numParticles = 0;
    for (int i = 0; i < hsize; ++i)
    {
        const HashCell* cell = (hdata + i)->next();
        while (cell)
        {
            numParticles += cell->getNumParticles();
            cells.push_back(cell);
            cell = cell->next();
        }
    }
    return numParticles;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SpacialHash::setAdjacentCells_()
{
    int hsize = harray_.size();
    HashCell* hdata = harray_.data();
    for (int i = 0; i < hsize; ++i)
    {
        HashCell* cell = (hdata + i)->next();
        while (cell)
        {
            short cx, cy, cz;
            cell->getCellIndex(cx, cy, cz);
            std::list < HashCell* >& ac = cell->adjacentCells();
            ac.clear();
            for (int k = -1; k < 2; ++k)
            {
                for (int j = -1; j < 2; ++j)
                {
                    for (int i = -1; i < 2; ++i)
                    {
                        if ( ! (i || j || k) )
                        {
                            continue;
                        }
                        HashCell* aCell = getCell(cx + i, cy + j, cz + k);
                        if (aCell)
                        {
                            ac.push_back(aCell);
                        }
                    }
                }
            }
            cell = cell->next();
        }
    }
}

