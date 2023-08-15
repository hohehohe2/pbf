#include <iostream>
#include <CppUnitLite/TestHarness.h>
#include "spacialHash.h"

using namespace hohe2;

class DerivedLinkedListNode : public LinkedListNode < DerivedLinkedListNode >{};
TEST(LinkedListNode, insert)
{
    DerivedLinkedListNode  start;
    DerivedLinkedListNode node1;
    DerivedLinkedListNode node2;
    DerivedLinkedListNode  node3;
    node3.insert(start);
    node2.insert(start);
    node1.insert(start);
    CHECK_EQUAL(true, &node1 == start.next());
    CHECK_EQUAL(true, &node2 == node1.next());
    CHECK_EQUAL(true, &node3 == node2.next());
    CHECK_EQUAL(true, NULL == node3.next());
}

TEST(LinkedListNode, delete)
{
    DerivedLinkedListNode start;
    DerivedLinkedListNode node1;
    DerivedLinkedListNode node2;
    DerivedLinkedListNode node3;
    node3.insert(start);
    node2.insert(start);
    node1.insert(start);
    node3.deleteMe();
    CHECK_EQUAL(true, &node1 == start.next());
    CHECK_EQUAL(true, &node2 == node1.next());
    CHECK_EQUAL(true, NULL == node2.next());
    CHECK_EQUAL(true, NULL == node3.next());
    node1.deleteMe();
    CHECK_EQUAL(true, &node2 == start.next());
    CHECK_EQUAL(true, NULL == node1.next());
}

TEST(HashCell, create)
{
    HashCell hc(1, 2, 3);
    CHECK_EQUAL(true, hc.isSameCell(1, 2, 3));
    CHECK_EQUAL(false, hc.isSameCell(2, 2, 3));
}

TEST(HashCell, hashValue)
{
    HashCell hc(1, 2, 3);
    CHECK_EQUAL(2630, hc.hashValue());
}

TEST(HashCell, add)
{
    HashCell hc;
    hc.addParticleId(0);
    hc.addParticleId(1);
    hc.addParticleId(2);
    CHECK_EQUAL(1, hc.getParticleIds()[1]);
    CHECK_EQUAL(3, hc.getNumParticles());
}

TEST(HashCell, remove)
{
    HashCell hc;
    hc.addParticleId(0);
    hc.addParticleId(1);
    hc.addParticleId(2);
    float positions[] = {0, 0, 0, 1.5, 1.5, 1.5, 5, 5, 5};
    std::list < int > removedIds;
    bool result = hc.removeMovedOutParticles(positions, removedIds);
    CHECK_EQUAL(false, result);
    CHECK_EQUAL(1, (int)removedIds.size());
    CHECK_EQUAL(2, removedIds.front());
    CHECK_EQUAL(2, hc.getNumParticles());
    CHECK_EQUAL(0, hc.getParticleIds()[0]);
    CHECK_EQUAL(1, hc.getParticleIds()[1]);
}

TEST(CellFreeList, create)
{
    CellFreeList cfl;
    HashCell* hc1 = cfl.get();
    HashCell* hc2 = cfl.get();
    CHECK_EQUAL(false, hc1 == hc2);
    cfl.add(*hc2);
    HashCell* hc3 = cfl.get();
    CHECK_EQUAL(true, hc3 == hc2);
}

namespace hohe2
{

TEST(SpacialHash, create)
{
    float positions[] = {0, 0, 0, 1.5, 1.5, 1.5, 5, 5, 5};
    SpacialHash sh;
    sh.addParticles(positions, 3);
    sh.harray_.size();
}

TEST(SpacialHash, get)
{
    float positions[] = {0, 0, 0, 1.5, 1.5, 1.5, 5, 5, 5};
    SpacialHash sh;
    sh.addParticles(positions, 3);
    HashCell* hc000 = sh.getCell(0, 0, 0);
    CHECK_EQUAL(false, hc000==NULL);
    HashCell* hc999 = sh.getCell(9, 9, 9);
    CHECK_EQUAL(true, hc999==NULL);
    CHECK_EQUAL(2, hc000->getNumParticles());
    CHECK_EQUAL(0, hc000->getParticleIds()[0]);
    CHECK_EQUAL(1, hc000->getParticleIds()[1]);
}

TEST(SpacialHash, restructure1)
{
    float positions[] = {0, 0, 0, 1.5, 1.5, 1.5, 5, 5, 5};
    SpacialHash sh;
    sh.addParticles(positions, 3);
    positions[0] = 9;
    sh.restructure(positions);

    HashCell* hc000 = sh.getCell(0, 0, 0);
    CHECK_EQUAL(false, hc000==NULL);
    CHECK_EQUAL(1, hc000->getNumParticles());
    CHECK_EQUAL(1, hc000->getParticleIds()[0]);
    HashCell* hc004 = sh.getCell(4, 0, 0);
    CHECK_EQUAL(false, hc004==NULL);
    CHECK_EQUAL(1, hc004->getNumParticles());
    CHECK_EQUAL(0, hc004->getParticleIds()[0]);
}

TEST(SpacialHash, restructure2)
{
    float positions[] = {0, 0, 0, 1.5, 1.5, 1.5, 5, 5, 5};
    SpacialHash sh;
    sh.addParticles(positions, 3);
    positions[0] = 9;
    positions[3] = 9.5;
    sh.restructure(positions);
    HashCell* hc000 = sh.getCell(0, 0, 0);
    CHECK_EQUAL(true, hc000==NULL);
}

}
