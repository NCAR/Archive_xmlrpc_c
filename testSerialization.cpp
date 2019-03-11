// testSerialization.cpp
//  Created on: Jan 31, 2019
//      Author: Chris Burghart <burghart@ucar.edu>

/// Test Archive_xmlrpc_c serialization

#include <cstdint>
#include <iostream>
#include <xmlrpc-c/base.hpp>
#include <boost/serialization/nvp.hpp>
#include "Archive_xmlrpc_c.h"

class TestClass {
public:
    TestClass() :
        _i8Bit(INT8_MIN),
        _ui8Bit(UINT8_MAX),
        _i16Bit(INT16_MIN),
        _ui16Bit(UINT16_MAX),
        _i32Bit(INT32_MIN),
        _ui32Bit(UINT32_MAX),
        _i64Bit(INT64_MIN),
        _ui64Bit(UINT64_MAX) {}

    virtual ~TestClass() {}

    /// @brief Serialize our members to a boost save (output) archive or populate
    /// our members from a boost load (input) archive.
    /// @param ar the archive to load from or save to.
    /// @param version the version
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        // Map named entries to our member variables using serialization's
        // name/value pairs (nvp). If anything is changed in this section, be
        // sure to increment the version number in STATUS_VERSION above.
        ar & BOOST_SERIALIZATION_NVP(_i8Bit);
        ar & BOOST_SERIALIZATION_NVP(_ui8Bit);
        ar & BOOST_SERIALIZATION_NVP(_i16Bit);
        ar & BOOST_SERIALIZATION_NVP(_ui16Bit);
        ar & BOOST_SERIALIZATION_NVP(_i32Bit);
        ar & BOOST_SERIALIZATION_NVP(_ui32Bit);
        ar & BOOST_SERIALIZATION_NVP(_i64Bit);
        ar & BOOST_SERIALIZATION_NVP(_ui64Bit);
    }

    int8_t _i8Bit;
    uint8_t _ui8Bit;
    int16_t _i16Bit;
    uint16_t _ui16Bit;
    int32_t _i32Bit;
    uint32_t _ui32Bit;
    int64_t _i64Bit;
    uint64_t _ui64Bit;
};

//xmlrpc_c::value_struct
//TestClass::toXmlRpcValue() const {
//    std::map<std::string, xmlrpc_c::value> statusDict;
//    // Clone ourself to a non-const instance
//    TestClass clone(*this);
//    // Stuff our content into the statusDict, i.e., _serialize() to an
//    // output archiver wrapped around the statusDict.
//    Oarchive_xmlrpc_c oar(statusDict);
//    oar << clone;
//    // Finally, return the statusDict
//    return(xmlrpc_c::value_struct(statusDict));
//}

int
main(int argc, char *argv[]) {
    TestClass tc;

    xmlrpc_c::cstruct outMap;
    Oarchive_xmlrpc_c oa(outMap);
    oa << tc;

    xmlrpc_c::value_struct xmlStruct(outMap);
    Iarchive_xmlrpc_c ia(xmlStruct);
    XmlrpcSerializable<TestClass> newTc(xmlStruct);

    bool fail = false;
    bool ok;
    ok = (newTc._i8Bit == INT8_MIN);
    std::cout << "int8_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._ui8Bit == UINT8_MAX);
    std::cout << "uint8_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._i16Bit == INT16_MIN);
    std::cout << "int16_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._ui16Bit == UINT16_MAX);
    std::cout << "uint32_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._i32Bit == INT32_MIN);
    std::cout << "int32_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._ui32Bit == UINT32_MAX);
    std::cout << "uint32_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._i64Bit == INT64_MIN);
    std::cout << "int64_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

    ok = (newTc._ui64Bit == UINT64_MAX);
    std::cout << "uint64_t " << (ok ? "GOOD" : "BAD") << std::endl;
    fail |= !ok;

}
