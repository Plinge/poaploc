#pragma once

namespace Filtering {
	/** positional information of input data */
    typedef enum { 
        HAVE_NONE=0,  ///< contains neither start or end of buffer
        HAVE_START=1, ///< is start of buffer
        HAVE_END=2,   ///< ends with end of buffer 
        HAVE_BOTH=3,  ///< contains both start and end, i.e. all the data
		AFTER_END=4   ///< past input end, flush any leftovers
    } PositionInfo;
};
