
/*	Copyright (c) CCL/ITRI	1990,1991	*/
/*	  All Rights Reserved			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF CCL/ITRI	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include "tab.h"

static struct code	cnstab[CNSMAX] = {
/*0*/	{	147		, 0	},
/*1*/	{	148		, 1	},
/*2*/	{	149		, -1	},
/*3*/	{	408+1411	, 0	},
/*4*/	{	408+1835	, 1	},
/*5*/	{	408+1836	, -424	},
/*6*/	{	408+3712	, 0	},
/*7*/	{	408+3713	, 387	},
/*8*/	{	408+4100	, -1	},
/*9*/	{	408+4625	, 0	},
/*10*/	{	408+4626	, 189	},
/*11*/	{	408+4815	, -1	},
/*12*/	{	408+4899	, 0	},
/*13*/	{	408+4900	, 146	},
/*14*/	{	408+4954	, -1	},
/*15*/	{	408+4955	, 0	},
/*16*/	{	408+4956	, -2	},
/*17*/	{	408+5046	, -1	},
/*18*/	{	408+5401+9	, 0	},
/*19*/	{	408+5401+42	, 1	},
/*20*/	{	408+5401+43	, 49	},
/*21*/	{	408+5401+91	, 0	},
/*22*/	{	408+5401+137	, 1	},
/*23*/	{	408+5401+138	, 168	},
/*24*/	{	408+5401+305	, 0	},
/*25*/	{	408+5401+2145	, 1	},
/*26*/	{	408+5401+2146	, 648	},
/*27*/	{	408+5401+2254	, 1	},
/*28*/	{	408+5401+2791	, 2	},
/*29*/	{	408+5401+2792	, -645	},
/*30*/	{	408+5401+2892	, 2	},
/*31*/	{	408+5401+2893	, -637	},
/*32*/	{	408+5401+3292	, 1	},
/*33*/	{	408+5401+4929	, 2	},
/*34*/	{	408+5401+4930	, 551	},
/*35*/	{	408+5401+5076	, 1	},
/*36*/	{	408+5401+5077	, 542	},
/*37*/	{	408+5401+5364	, 0	},
/*38*/	{	408+5401+5365	, 863	},
/*39*/	{	408+5401+5481	, -1	},
/*40*/	{	408+5401+5618	, 0	},
/*41*/	{	408+5401+5722	, 1	},
/*42*/	{	408+5401+5723	, 422	},
/*43*/	{	408+5401+5945	, 0	},
/*44*/	{	408+5401+6143	, 1	},
/*45*/	{	408+5401+6225	, 2	},
/*46*/	{	408+5401+6311	, 3	},
/*47*/	{	408+5401+6312	, -366	},
/*48*/	{	408+5401+6320	, 2	},
/*49*/	{	408+5401+6473	, 3	},
/*50*/	{	408+5401+6529	, 4	},
/*51*/	{	408+5401+6530	, 304	},
/*52*/	{	408+5401+6643	, 3	},
/*53*/	{	408+5401+6644	, -321	},
/*54*/	{	408+5401+6786	, 2	},
/*55*/	{	408+5401+6787	, -310	},
/*56*/	{	408+5401+6832	, 1	},
/*57*/	{	408+5401+6903	, 2	},
/*58*/	{	408+5401+6904	, 196	},
/*59*/	{	408+5401+7098	, 1	},
/*60*/	{	408+5401+7588	, 2	},
/*61*/	{	408+5401+7589	, 45	},
/*62*/	{	408+5401+7632	, 1	},
/*63*/	{	408+5401+7633	, 2	},
/*64*/	{	408+5401+7644	, 3	},
/*65*/	{	408+5401+7645	, -9	},
/*66*/	{	408+5401+7649	, 2	}
};
