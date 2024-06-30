#pragma once

#include <string>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ts::U {
    enum class block {
        INPORT,
        SUM,
        GAIN,
        UNIT_DELAY,
        OUTPORT
    };

    inline static const boost::bimap<block, std::string> blocks = 
        boost::assign::list_of<boost::bimap<block, std::string>::relation>
        ( block::INPORT, "Inport" )
        ( block::SUM, "Sum" )
        ( block::GAIN, "Gain" )
        ( block::UNIT_DELAY, "UnitDelay" )
        ( block::OUTPORT, "Outport" );
}