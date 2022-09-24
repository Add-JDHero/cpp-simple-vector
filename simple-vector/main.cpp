#include <iostream>
#include <cassert>

#include "simple_vector.h"
#include "tests.h"


/*
int main() {SimpleVector tmp(other.GetSize());
    Test1();
    return 0;
}
*/


int main() { 
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
}
