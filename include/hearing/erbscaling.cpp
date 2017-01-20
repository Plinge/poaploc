/*
 * Copyright 2010-2015 (c) Axel Plinge / TU Dortmund University
 *
 * ALL THE COMPUTER PROGRAMS AND SOFTWARE ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * WE MAKE NO WARRANTIES,  EXPRESS OR IMPLIED, THAT THEY ARE FREE OF ERROR, OR ARE CONSISTENT
 * WITH ANY PARTICULAR STANDARD OF MERCHANTABILITY, OR THAT THEY WILL MEET YOUR REQUIREMENTS
 * FOR ANY PARTICULAR APPLICATION. THEY SHOULD NOT BE RELIED ON FOR SOLVING A PROBLEM WHOSE
 * INCORRECT SOLUTION COULD RESULT IN INJURY TO A PERSON OR LOSS OF PROPERTY. IF YOU DO USE
 * THEM IN SUCH A MANNER, IT IS AT YOUR OWN RISK. THE AUTHOR AND PUBLISHER DISCLAIM ALL
 * LIABILITY FOR DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES RESULTING FROM YOUR USE OF THE
 * PROGRAMS.
 */
/**
 * @author axel
 * @date 07.12.2009
 */
#include "erbscaling.h"
#include <math.h>

using namespace Hearing;

const double ErbScaling::C1 = 24.673;
const double ErbScaling::C2 = 4.368;
const double ErbScaling::C3 = 21.366; 


ErbScaling::ErbScaling()
{
}

double ErbScaling::scale(double freq)
{
    return (C3*log10((C2 * freq/1000.0) + 1.0));
}

double ErbScaling::unscale(double erb)
{
    return 1000.0 * (pow(10.0,(erb/C3)) - 1.0) / C2;
}


