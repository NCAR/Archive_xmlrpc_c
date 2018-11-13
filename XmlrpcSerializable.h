// XmlrpcSerializable.h
//
//  Created on: Nov 13, 2018
//      Author: Chris Burghart <burghart@ucar.edu>

#ifndef XMLRPCSERIALIZABLE_H_
#define XMLRPCSERIALIZABLE_H_

#include <xmlrpc-c/base.hpp>
#include "Archive_xmlrpc_c.h"

// Mix-in class which allows objects of its class to be serialized to XML-RPC
// as composite members of other classes.
//
// Members of this class must provide a boost::serialization serialize()
// template method which serialize to/from boost::serialization::nvp
// name-value pair representation.
template<typename T>
class XmlrpcSerializable : public T {
public:
    /// @brief Default constructor
    XmlrpcSerializable() : T() {}

    /// @brief Constructor which copies from an instance of T
    /// @param t the instance of type T to copy
    XmlrpcSerializable(const T & t) : T() {
        *this = t;
    }

    /// @brief Construct from an xmlrpc_c::value (which must be
    /// xmlrpc_c::value_struct)
    /// @param xmlrpcVal the xmlrpc_c::value holding the content from which
    /// to construct
    XmlrpcSerializable(xmlrpc_c::value xmlrpcVal) : T() {
        // Cast the xmlrpc_c::value to xmlrpc_c::value_struct, then from that
        // to std::map<std::string, xmlrpc_c::value>.
        xmlrpc_c::value_struct statusStruct(xmlrpcVal);
        std::map<std::string, xmlrpc_c::value> statusMap(statusStruct);

        // Create an input archiver wrapper around the map and use serialize()
        // to populate our members from its content.
        Iarchive_xmlrpc_c iar(statusMap);
        iar >> *this;
    }

    virtual ~XmlrpcSerializable() {};

    /// @brief Cast to xmlrpc_c::value
    operator xmlrpc_c::value() const { return(_toXmlRpcValueStruct()); }

private:
    /// @brief Return an xmlrpc_c::value containing a struct (dictionary) with
    /// the object's serialized representation
    xmlrpc_c::value_struct _toXmlRpcValueStruct() const {
        std::map<std::string, xmlrpc_c::value> statusMap;
        // Stuff our content into the statusMap, i.e., _serialize() to an
        // output archiver wrapped around the statusMap.
        Oarchive_xmlrpc_c oar(statusMap);
        oar << *this;
        // Finally, return a value_struct constructed from the map
        return(xmlrpc_c::value_struct(statusMap));
    }

};

#endif /* XMLRPCSERIALIZABLE_H_ */
