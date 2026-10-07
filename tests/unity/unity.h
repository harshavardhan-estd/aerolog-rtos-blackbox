/* ==========================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [MIT License]
========================================== */

#ifndef UNITY_FRAMEWORK_H
#define UNITY_FRAMEWORK_H

#include <stdio.h>
#include <math.h>

#define TEST_ASSERT(condition) if (!(condition)) { printf("FAIL: line %d\n", __LINE__); return; }
#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))
#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT((expected) == (actual))
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) TEST_ASSERT(fabsf((float)(expected) - (float)(actual)) <= (float)(delta))

#endif
