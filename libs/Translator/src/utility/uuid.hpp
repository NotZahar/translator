#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ts::uuid {
    inline std::string generateUUID() {
        const boost::uuids::uuid id = boost::uuids::random_generator()();
        return boost::uuids::to_string(id);
    }
}