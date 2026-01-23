#include <../inc/server.hpp>


std::string familyToString(int family) {
    switch (family) {
        case AF_INET:
            return "AF_INET";
        case AF_INET6:
            return "AF_INET6";
        case AF_UNIX:
            return "AF_UNIX";
        default:
            return "UNKNOWN_FAMILY";
    }
}