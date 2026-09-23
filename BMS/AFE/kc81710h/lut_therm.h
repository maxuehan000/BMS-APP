/*****************************************************************************
* Copyright(c) QCG, 2023. All rights reserved.
*
* QCG [S8] Source Code Reference Design
* File: lut_therm.h
*
* This Source Code Reference Design for QCG [S8] access
* ("Reference Design") is solely for the use of PRODUCT INTEGRATION REFERENCE ONLY,
* and contains confidential and privileged information of QCG International
* Limited. QCG shall have no liability to any PARTY FOR THE RELIABILITY,
* SERVICEABILITY FOR THE RESULT OF PRODUCT INTEGRATION, or results from: (i) any
* modification or attempted modification of the Reference Design by any party, or
* (ii) the combination, operation or use of the Reference Design with non-QCG
* Reference Design. Use of the Reference Design is at user's discretion to qualify
* the final work result.
*****************************************************************************/

#ifndef _KC81710H__THERMAL_TABLE_H_
#define _KC81710H__THERMAL_TABLE_H_

/*****************************************************************************
 * #include section
 * add #include here
 *****************************************************************************/

typedef struct tag_one_latitude_data
{
	int32_t x;
	int32_t y;
} one_latitude_data_t;

//This table is built with 0.1C and resistor( R )table
// The resistor R at 25'C is 10K ohm.
#if 0
#define TEMPERATURE_DATA_NUM   25

one_latitude_data_t temp_data_table[TEMPERATURE_DATA_NUM] =
{
	{1268,    900}, {1452,   850}, {1668,   800},
	{1924,    750}, {2227,   700}, {2586,   650},
	{3014,    600}, {3535,   550}, {4160,   500},
	{4916,    450}, {5833,   400}, {6947,   350},
	{8314,    300}, {10000,  250}, {12080,  200},
	{14673,   150}, {17925,  100}, {22021,   50},
	{27218,     0}, {33892,  -50}, {42506, -100},
	{53649,  -150}, {68236, -200}, {87558, -250},
	{113347, -300},

};
#endif
#define TEMPERATURE_CELL_NUM			34
#define TEMPERATURE_MOSFET_NUM			34

one_latitude_data_t temp_cell_table[TEMPERATURE_CELL_NUM] =
{
	{532,   1250},{598,    1200},{673,    1150},{759,    1100},{860,   1050},
	{976,   1000},{1112,    950},{1270,    900},{1455,    850},{1673,   800},
	{1929,   750},{2233,    700},{2593,    650},{3024,    600},{3539,   550},
	{4160,   500},{4911,    450},{5824,    400},{6939,    350},{8209,   300},
	{10000,  250},{12099,   200},{14720,   150},{18010,   100},{22165,   50},
	{27445,    0},{34196,   -50},{42889,  -100},{54166,  -150},{68915, -200},
	{88381, -250},{114340, -300},{149390, -350},{197390, -400},
};

one_latitude_data_t temp_mosfet_table[TEMPERATURE_MOSFET_NUM] =
{
	{540,   1250},{600,    1200},{676,    1150},{762,    1100},{861,   1050},
	{977,   1000},{1111,    950},{1268,    900},{1451,    850},{1667,   800},
	{1921,   750},{2222,    700},{2579,    650},{3007,    600},{3520,   550},
	{4138,   500},{4888,    450},{5800,    400},{6918,    350},{8295,   300},
	{10000,  250},{12126,   200},{14795,   150},{18166,   100},{22459,   50},
	{27954,    0},{35085,   -50},{44369,  -100},{56587,  -150},{72818, -200},
	{94597, -250},{124135, -300},{164649, -350},{208084, -400},
};


#endif

