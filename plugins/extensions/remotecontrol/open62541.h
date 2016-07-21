/* THIS IS A SINGLE-FILE DISTRIBUTION CONCATENATED FROM THE OPEN62541 SOURCES 
 * visit http://open62541.org/ for information about this software
 * Git-Revision: v0.2.0-RC1
 */
 
 /*
 * Copyright (C) 2015 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


 #ifndef OPEN62541_H_
#define OPEN62541_H_

#ifdef __cplusplus
extern "C" {
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_config.h" ***********************************/

/*
 * Copyright (C) 2013-2015 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500
#endif
#ifndef _DEFAULT_SOURCE
# define _DEFAULT_SOURCE
#endif

#define UA_LOGLEVEL 300
/* #undef UA_ENABLE_MULTITHREADING */
#define UA_ENABLE_METHODCALLS
/* #undef UA_ENABLE_SUBSCRIPTIONS */
/* #undef UA_ENABLE_TYPENAMES */
/* #undef UA_ENABLE_EMBEDDED_LIBC */
/* #undef UA_ENABLE_GENERATE_NAMESPACE0 */
/* #undef UA_ENABLE_EXTERNAL_NAMESPACES */
#define UA_ENABLE_NODEMANAGEMENT

/* #undef UA_ENABLE_NONSTANDARD_UDP */
/* #undef UA_ENABLE_NONSTANDARD_STATELESS */

/**
 * Function Export
 * --------------- */
#ifdef _WIN32
# ifdef UA_DYNAMIC_LINKING
#  ifdef __GNUC__
#   define UA_EXPORT __attribute__ ((dllexport))
#  else
#   define UA_EXPORT __declspec(dllexport)
#  endif
# else
#  ifdef __GNUC__
#   define UA_EXPORT __attribute__ ((dllexport))
#  else
#   define UA_EXPORT __declspec(dllimport)
#  endif
# endif
#else
# if __GNUC__ || __clang__
#  define UA_EXPORT __attribute__ ((visibility ("default")))
# else
#  define UA_EXPORT
# endif
#endif

/**
 * Inline Functions
 * ---------------- */
#ifdef _MSC_VER
# define UA_INLINE __inline
#else
# define UA_INLINE inline
#endif

/**
 * Non-aliasing pointers
 * -------------------- */
#ifdef _MSC_VER
# define UA_RESTRICT __restrict
#elif defined(__GNUC__)
# define UA_RESTRICT __restrict__
#else
# define UA_RESTRICT restrict
#endif

/**
 * Function attributes
 * ------------------- */
#ifdef __GNUC__
# define UA_FUNC_ATTR_MALLOC __attribute__((malloc))
# define UA_FUNC_ATTR_PURE __attribute__ ((pure))
# define UA_FUNC_ATTR_CONST __attribute__((const))
# define UA_FUNC_ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define UA_FUNC_ATTR_MALLOC
# define UA_FUNC_ATTR_PURE
# define UA_FUNC_ATTR_CONST
# define UA_FUNC_ATTR_WARN_UNUSED_RESULT
#endif

/**
 * Integer Endianness
 * ------------------ */
#if defined(_WIN32) || (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
                        (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) /* little endian detected */
# define UA_BINARY_OVERLAYABLE_INTEGER true
#elif defined(__ANDROID__) /* Andoid */
# include <endian.h>
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define UA_BINARY_OVERLAYABLE_INTEGER true
# endif
#elif defined(__linux__) /* Linux */
# include <endian.h>
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define UA_BINARY_OVERLAYABLE_INTEGER true
# endif
# if __FLOAT_BYTE_ORDER == __LITTLE_ENDIAN
#  define UA_BINARY_OVERLAYABLE_FLOAT true
# endif
#elif defined(__OpenBSD__) /* OpenBSD */
# include <sys/endian.h>
# if BYTE_ORDER == LITTLE_ENDIAN
#  define UA_BINARY_OVERLAYABLE_INTEGER true
# endif
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__) /* Other BSD */
# include <sys/endian.h>
# if _BYTE_ORDER == _LITTLE_ENDIAN
#  define UA_BINARY_OVERLAYABLE_INTEGER true
# endif
#elif defined(__APPLE__) /* Apple (MacOS, iOS) */
# include <libkern/OSByteOrder.h>
# if defined(__LITTLE_ENDIAN__)
#  define UA_BINARY_OVERLAYABLE_INTEGER true
# endif
#elif defined(__QNX__) || defined(__QNXNTO__) /* QNX */
# include <gulliver.h>
# if defined(__LITTLEENDIAN__)
#  define UA_BINARY_OVERLAYABLE_INTEGER true
# endif
#endif

/**
 * Float Endianness
 * ---------------- */
#if defined(_WIN32)
# define UA_BINARY_OVERLAYABLE_FLOAT true
#elif defined(__FLOAT_WORD_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
    (__FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__) /* Defined only in GCC */
# define UA_BINARY_OVERLAYABLE_FLOAT true
#endif

/**
 * Binary Encoding Overlays
 * ------------------------
 * Some data types have the same layout in memory as on the binary data stream.
 * This can be used to speed up the decoding. If we could not detect
 * little-endianness of integers and floats, this is not possible for types that
 * contain integers or floats respectively. See the definition of the
 * overlayable flag defined in `UA_DataType`. */

/* Demote error to a warning on clang. There is no robust way to detect float
 * endianness here. On x86/x86-64, floats are always in the right IEEE 754
 * format. So floats are overlayable is the architecture is little-endian. */
#if defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic warning "-W#warnings"
#endif

#ifndef UA_BINARY_OVERLAYABLE_INTEGER
# define UA_BINARY_OVERLAYABLE_INTEGER false
# warning Slow Integer Encoding
#endif
#ifndef UA_BINARY_OVERLAYABLE_FLOAT
# define UA_BINARY_OVERLAYABLE_FLOAT false
# warning Slow Float Encoding
#endif

#if defined(__clang__)
# pragma GCC diagnostic pop
#endif

/**
 * Embed unavailable libc functions
 * -------------------------------- */
#include <stddef.h>
#ifdef UA_ENABLE_EMBEDDED_LIBC
  void *memcpy(void *UA_RESTRICT dest, const void *UA_RESTRICT src, size_t n);
  void *memset(void *dest, int c, size_t n);
  size_t strlen(const char *s);
  int memcmp(const void *vl, const void *vr, size_t n);
#else
# include <string.h>
#endif

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_constants.h" ***********************************/

/*
 * Copyright (C) 2013-2015 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Standard-Defined Constants
 * ==========================
 * This section contains numerical and string constants that are defined in the
 * OPC UA standard.
 *
 * Attribute Id
 * ------------
 * Every node in an OPC UA information model contains attributes depending on
 * the node type. Possible attributes are as follows: */
typedef enum {
    UA_ATTRIBUTEID_NODEID                  = 1,
    UA_ATTRIBUTEID_NODECLASS               = 2,
    UA_ATTRIBUTEID_BROWSENAME              = 3,
    UA_ATTRIBUTEID_DISPLAYNAME             = 4,
    UA_ATTRIBUTEID_DESCRIPTION             = 5,
    UA_ATTRIBUTEID_WRITEMASK               = 6,
    UA_ATTRIBUTEID_USERWRITEMASK           = 7,
    UA_ATTRIBUTEID_ISABSTRACT              = 8,
    UA_ATTRIBUTEID_SYMMETRIC               = 9,
    UA_ATTRIBUTEID_INVERSENAME             = 10,
    UA_ATTRIBUTEID_CONTAINSNOLOOPS         = 11,
    UA_ATTRIBUTEID_EVENTNOTIFIER           = 12,
    UA_ATTRIBUTEID_VALUE                   = 13,
    UA_ATTRIBUTEID_DATATYPE                = 14,
    UA_ATTRIBUTEID_VALUERANK               = 15,
    UA_ATTRIBUTEID_ARRAYDIMENSIONS         = 16,
    UA_ATTRIBUTEID_ACCESSLEVEL             = 17,
    UA_ATTRIBUTEID_USERACCESSLEVEL         = 18,
    UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL = 19,
    UA_ATTRIBUTEID_HISTORIZING             = 20,
    UA_ATTRIBUTEID_EXECUTABLE              = 21,
    UA_ATTRIBUTEID_USEREXECUTABLE          = 22
} UA_AttributeId;

/**
 * Access Level Masks
 * ------------------
 * The access level to a node is given by the following constants that are XORed
 * for the overall access level. */
#define UA_ACCESSLEVELMASK_READ 0x01
#define UA_ACCESSLEVELMASK_WRITE 0x02
#define UA_ACCESSLEVELMASK_HISTORYREAD 0x4
#define UA_ACCESSLEVELMASK_HISTORYWRITE 0x08
#define UA_ACCESSLEVELMASK_SEMANTICCHANGE 0x10

/**
* Encoding Offsets
* ----------------
* Subtract from the typeid of the encoding nodeids to get to the type
* definition. */
#define UA_ENCODINGOFFSET_XML 1
#define UA_ENCODINGOFFSET_BINARY 2
    
/**
 * .. _statuscodes:
 *
 * StatusCodes
 * -----------
 * StatusCodes are extensively used in the OPC UA protocol and in the open62541
 * API. They are represented by the :ref:`statuscode` data type. The following
 * definitions are autogenerated from the ``Opc.Ua.StatusCodes.csv`` file provided
 * with the OPC UA standard. */
#define UA_STATUSCODE_GOOD 0x00
#define UA_STATUSCODE_BADUNEXPECTEDERROR 0x80010000 // An unexpected error occurred.
#define UA_STATUSCODE_BADINTERNALERROR 0x80020000 // An internal error occurred as a result of a programming or configuration error.
#define UA_STATUSCODE_BADOUTOFMEMORY 0x80030000 // Not enough memory to complete the operation.
#define UA_STATUSCODE_BADRESOURCEUNAVAILABLE 0x80040000 // An operating system resource is not available.
#define UA_STATUSCODE_BADCOMMUNICATIONERROR 0x80050000 // A low level communication error occurred.
#define UA_STATUSCODE_BADENCODINGERROR 0x80060000 // Encoding halted because of invalid data in the objects being serialized.
#define UA_STATUSCODE_BADDECODINGERROR 0x80070000 // Decoding halted because of invalid data in the stream.
#define UA_STATUSCODE_BADENCODINGLIMITSEXCEEDED 0x80080000 // The message encoding/decoding limits imposed by the stack have been exceeded.
#define UA_STATUSCODE_BADREQUESTTOOLARGE 0x80b80000 // The request message size exceeds limits set by the server.
#define UA_STATUSCODE_BADRESPONSETOOLARGE 0x80b90000 // The response message size exceeds limits set by the client.
#define UA_STATUSCODE_BADUNKNOWNRESPONSE 0x80090000 // An unrecognized response was received from the server.
#define UA_STATUSCODE_BADTIMEOUT 0x800a0000 // The operation timed out.
#define UA_STATUSCODE_BADSERVICEUNSUPPORTED 0x800b0000 // The server does not support the requested service.
#define UA_STATUSCODE_BADSHUTDOWN 0x800c0000 // The operation was cancelled because the application is shutting down.
#define UA_STATUSCODE_BADSERVERNOTCONNECTED 0x800d0000 // The operation could not complete because the client is not connected to the server.
#define UA_STATUSCODE_BADSERVERHALTED 0x800e0000 // The server has stopped and cannot process any requests.
#define UA_STATUSCODE_BADNOTHINGTODO 0x800f0000 // There was nothing to do because the client passed a list of operations with no elements.
#define UA_STATUSCODE_BADTOOMANYOPERATIONS 0x80100000 // The request could not be processed because it specified too many operations.
#define UA_STATUSCODE_BADTOOMANYMONITOREDITEMS 0x80db0000 // The request could not be processed because there are too many monitored items in the subscription.
#define UA_STATUSCODE_BADDATATYPEIDUNKNOWN 0x80110000 // The extension object cannot be (de)serialized because the data type id is not recognized.
#define UA_STATUSCODE_BADCERTIFICATEINVALID 0x80120000 // The certificate provided as a parameter is not valid.
#define UA_STATUSCODE_BADSECURITYCHECKSFAILED 0x80130000 // An error occurred verifying security.
#define UA_STATUSCODE_BADCERTIFICATETIMEINVALID 0x80140000 // The Certificate has expired or is not yet valid.
#define UA_STATUSCODE_BADCERTIFICATEISSUERTIMEINVALID 0x80150000 // An Issuer Certificate has expired or is not yet valid.
#define UA_STATUSCODE_BADCERTIFICATEHOSTNAMEINVALID 0x80160000 // The HostName used to connect to a Server does not match a HostName in the Certificate.
#define UA_STATUSCODE_BADCERTIFICATEURIINVALID 0x80170000 // The URI specified in the ApplicationDescription does not match the URI in the Certificate.
#define UA_STATUSCODE_BADCERTIFICATEUSENOTALLOWED 0x80180000 // The Certificate may not be used for the requested operation.
#define UA_STATUSCODE_BADCERTIFICATEISSUERUSENOTALLOWED 0x80190000 // The Issuer Certificate may not be used for the requested operation.
#define UA_STATUSCODE_BADCERTIFICATEUNTRUSTED 0x801a0000 // The Certificate is not trusted.
#define UA_STATUSCODE_BADCERTIFICATEREVOCATIONUNKNOWN 0x801b0000 // It was not possible to determine if the Certificate has been revoked.
#define UA_STATUSCODE_BADCERTIFICATEISSUERREVOCATIONUNKNOWN 0x801c0000 // It was not possible to determine if the Issuer Certificate has been revoked.
#define UA_STATUSCODE_BADCERTIFICATEREVOKED 0x801d0000 // The Certificate has been revoked.
#define UA_STATUSCODE_BADCERTIFICATEISSUERREVOKED 0x801e0000 // The Issuer Certificate has been revoked.
#define UA_STATUSCODE_BADUSERACCESSDENIED 0x801f0000 // User does not have permission to perform the requested operation.
#define UA_STATUSCODE_BADIDENTITYTOKENINVALID 0x80200000 // The user identity token is not valid.
#define UA_STATUSCODE_BADIDENTITYTOKENREJECTED 0x80210000 // The user identity token is valid but the server has rejected it.
#define UA_STATUSCODE_BADSECURECHANNELIDINVALID 0x80220000 // The specified secure channel is no longer valid.
#define UA_STATUSCODE_BADINVALIDTIMESTAMP 0x80230000 // The timestamp is outside the range allowed by the server.
#define UA_STATUSCODE_BADNONCEINVALID 0x80240000 // The nonce does appear to be not a random value or it is not the correct length.
#define UA_STATUSCODE_BADSESSIONIDINVALID 0x80250000 // The session id is not valid.
#define UA_STATUSCODE_BADSESSIONCLOSED 0x80260000 // The session was closed by the client.
#define UA_STATUSCODE_BADSESSIONNOTACTIVATED 0x80270000 // The session cannot be used because ActivateSession has not been called.
#define UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID 0x80280000 // The subscription id is not valid.
#define UA_STATUSCODE_BADREQUESTHEADERINVALID 0x802a0000 // The header for the request is missing or invalid.
#define UA_STATUSCODE_BADTIMESTAMPSTORETURNINVALID 0x802b0000 // The timestamps to return parameter is invalid.
#define UA_STATUSCODE_BADREQUESTCANCELLEDBYCLIENT 0x802c0000 // The request was cancelled by the client.
#define UA_STATUSCODE_GOODSUBSCRIPTIONTRANSFERRED 0x002d0000 // The subscription was transferred to another session.
#define UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY 0x002e0000 // The processing will complete asynchronously.
#define UA_STATUSCODE_GOODOVERLOAD 0x002f0000 // Sampling has slowed down due to resource limitations.
#define UA_STATUSCODE_GOODCLAMPED 0x00300000 // The value written was accepted but was clamped.
#define UA_STATUSCODE_BADNOCOMMUNICATION 0x80310000 // Communication with the data source is defined, but not established and there is no last known value available.
#define UA_STATUSCODE_BADWAITINGFORINITIALDATA 0x80320000 // Waiting for the server to obtain values from the underlying data source.
#define UA_STATUSCODE_BADNODEIDINVALID 0x80330000 // The syntax of the node id is not valid.
#define UA_STATUSCODE_BADNODEIDUNKNOWN 0x80340000 // The node id refers to a node that does not exist in the server address space.
#define UA_STATUSCODE_BADATTRIBUTEIDINVALID 0x80350000 // The attribute is not supported for the specified Node.
#define UA_STATUSCODE_BADINDEXRANGEINVALID 0x80360000 // The syntax of the index range parameter is invalid.
#define UA_STATUSCODE_BADINDEXRANGENODATA 0x80370000 // No data exists within the range of indexes specified.
#define UA_STATUSCODE_BADDATAENCODINGINVALID 0x80380000 // The data encoding is invalid.
#define UA_STATUSCODE_BADDATAENCODINGUNSUPPORTED 0x80390000 // The server does not support the requested data encoding for the node.
#define UA_STATUSCODE_BADNOTREADABLE 0x803a0000 // The access level does not allow reading or subscribing to the Node.
#define UA_STATUSCODE_BADNOTWRITABLE 0x803b0000 // The access level does not allow writing to the Node.
#define UA_STATUSCODE_BADOUTOFRANGE 0x803c0000 // The value was out of range.
#define UA_STATUSCODE_BADNOTSUPPORTED 0x803d0000 // The requested operation is not supported.
#define UA_STATUSCODE_BADNOTFOUND 0x803e0000 // A requested item was not found or a search operation ended without success.
#define UA_STATUSCODE_BADOBJECTDELETED 0x803f0000 // The object cannot be used because it has been deleted.
#define UA_STATUSCODE_BADNOTIMPLEMENTED 0x80400000 // Requested operation is not implemented.
#define UA_STATUSCODE_BADMONITORINGMODEINVALID 0x80410000 // The monitoring mode is invalid.
#define UA_STATUSCODE_BADMONITOREDITEMIDINVALID 0x80420000 // The monitoring item id does not refer to a valid monitored item.
#define UA_STATUSCODE_BADMONITOREDITEMFILTERINVALID 0x80430000 // The monitored item filter parameter is not valid.
#define UA_STATUSCODE_BADMONITOREDITEMFILTERUNSUPPORTED 0x80440000 // The server does not support the requested monitored item filter.
#define UA_STATUSCODE_BADFILTERNOTALLOWED 0x80450000 // A monitoring filter cannot be used in combination with the attribute specified.
#define UA_STATUSCODE_BADSTRUCTUREMISSING 0x80460000 // A mandatory structured parameter was missing or null.
#define UA_STATUSCODE_BADEVENTFILTERINVALID 0x80470000 // The event filter is not valid.
#define UA_STATUSCODE_BADCONTENTFILTERINVALID 0x80480000 // The content filter is not valid.
#define UA_STATUSCODE_BADFILTEROPERATORINVALID 0x80c10000 // An unrecognized operator was provided in a filter.
#define UA_STATUSCODE_BADFILTEROPERATORUNSUPPORTED 0x80c20000 // A valid operator was provided but the server does not provide support for this filter operator.
#define UA_STATUSCODE_BADFILTEROPERANDCOUNTMISMATCH 0x80c30000 // The number of operands provided for the filter operator was less then expected for the operand provided.
#define UA_STATUSCODE_BADFILTEROPERANDINVALID 0x80490000 // The operand used in a content filter is not valid.
#define UA_STATUSCODE_BADFILTERELEMENTINVALID 0x80c40000 // The referenced element is not a valid element in the content filter.
#define UA_STATUSCODE_BADFILTERLITERALINVALID 0x80c50000 // The referenced literal is not a valid value.
#define UA_STATUSCODE_BADCONTINUATIONPOINTINVALID 0x804a0000 // The continuation point provide is longer valid.
#define UA_STATUSCODE_BADNOCONTINUATIONPOINTS 0x804b0000 // The operation could not be processed because all continuation points have been allocated.
#define UA_STATUSCODE_BADREFERENCETYPEIDINVALID 0x804c0000 // The operation could not be processed because all continuation points have been allocated.
#define UA_STATUSCODE_BADBROWSEDIRECTIONINVALID 0x804d0000 // The browse direction is not valid.
#define UA_STATUSCODE_BADNODENOTINVIEW 0x804e0000 // The node is not part of the view.
#define UA_STATUSCODE_BADSERVERURIINVALID 0x804f0000 // The ServerUri is not a valid URI.
#define UA_STATUSCODE_BADSERVERNAMEMISSING 0x80500000 // No ServerName was specified.
#define UA_STATUSCODE_BADDISCOVERYURLMISSING 0x80510000 // No DiscoveryUrl was specified.
#define UA_STATUSCODE_BADSEMPAHOREFILEMISSING 0x80520000 // The semaphore file specified by the client is not valid.
#define UA_STATUSCODE_BADREQUESTTYPEINVALID 0x80530000 // The security token request type is not valid.
#define UA_STATUSCODE_BADSECURITYMODEREJECTED 0x80540000 // The security mode does not meet the requirements set by the Server.
#define UA_STATUSCODE_BADSECURITYPOLICYREJECTED 0x80550000 // The security policy does not meet the requirements set by the Server.
#define UA_STATUSCODE_BADTOOMANYSESSIONS 0x80560000 // The server has reached its maximum number of sessions.
#define UA_STATUSCODE_BADUSERSIGNATUREINVALID 0x80570000 // The user token signature is missing or invalid.
#define UA_STATUSCODE_BADAPPLICATIONSIGNATUREINVALID 0x80580000 // The signature generated with the client certificate is missing or invalid.
#define UA_STATUSCODE_BADNOVALIDCERTIFICATES 0x80590000 // The client did not provide at least one software certificate that is valid and meets the profile requirements for the server.
#define UA_STATUSCODE_BADIDENTITYCHANGENOTSUPPORTED 0x80c60000 // The Server does not support changing the user identity assigned to the session.
#define UA_STATUSCODE_BADREQUESTCANCELLEDBYREQUEST 0x805a0000 // The request was canceled by the client with the Cancel service.
#define UA_STATUSCODE_BADPARENTNODEIDINVALID 0x805b0000 // The parent node id does not to refer to a valid node.
#define UA_STATUSCODE_BADREFERENCENOTALLOWED 0x805c0000 // The reference could not be created because it violates constraints imposed by the data model.
#define UA_STATUSCODE_BADNODEIDREJECTED 0x805d0000 // The requested node id was reject because it was either invalid or server does not allow node ids to be specified by the client.
#define UA_STATUSCODE_BADNODEIDEXISTS 0x805e0000 // The requested node id is already used by another node.
#define UA_STATUSCODE_BADNODECLASSINVALID 0x805f0000 // The node class is not valid.
#define UA_STATUSCODE_BADBROWSENAMEINVALID 0x80600000 // The browse name is invalid.
#define UA_STATUSCODE_BADBROWSENAMEDUPLICATED 0x80610000 // The browse name is not unique among nodes that share the same relationship with the parent.
#define UA_STATUSCODE_BADNODEATTRIBUTESINVALID 0x80620000 // The node attributes are not valid for the node class.
#define UA_STATUSCODE_BADTYPEDEFINITIONINVALID 0x80630000 // The type definition node id does not reference an appropriate type node.
#define UA_STATUSCODE_BADSOURCENODEIDINVALID 0x80640000 // The source node id does not reference a valid node.
#define UA_STATUSCODE_BADTARGETNODEIDINVALID 0x80650000 // The target node id does not reference a valid node.
#define UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED 0x80660000 // The reference type between the nodes is already defined.
#define UA_STATUSCODE_BADINVALIDSELFREFERENCE 0x80670000 // The server does not allow this type of self reference on this node.
#define UA_STATUSCODE_BADREFERENCELOCALONLY 0x80680000 // The reference type is not valid for a reference to a remote server.
#define UA_STATUSCODE_BADNODELETERIGHTS 0x80690000 // The server will not allow the node to be deleted.
#define UA_STATUSCODE_UNCERTAINREFERENCENOTDELETED 0x40bc0000 // The server was not able to delete all target references.
#define UA_STATUSCODE_BADSERVERINDEXINVALID 0x806a0000 // The server index is not valid.
#define UA_STATUSCODE_BADVIEWIDUNKNOWN 0x806b0000 // The view id does not refer to a valid view node.
#define UA_STATUSCODE_BADVIEWTIMESTAMPINVALID 0x80c90000 // The view timestamp is not available or not supported.
#define UA_STATUSCODE_BADVIEWPARAMETERMISMATCH 0x80ca0000 // The view parameters are not consistent with each other.
#define UA_STATUSCODE_BADVIEWVERSIONINVALID 0x80cb0000 // The view version is not available or not supported.
#define UA_STATUSCODE_UNCERTAINNOTALLNODESAVAILABLE 0x40c00000 // The list of references may not be complete because the underlying system is not available.
#define UA_STATUSCODE_GOODRESULTSMAYBEINCOMPLETE 0x00ba0000 // The server should have followed a reference to a node in a remote server but did not. The result set may be incomplete.
#define UA_STATUSCODE_BADNOTTYPEDEFINITION 0x80c80000 // The provided Nodeid was not a type definition nodeid.
#define UA_STATUSCODE_UNCERTAINREFERENCEOUTOFSERVER 0x406c0000 // One of the references to follow in the relative path references to a node in the address space in another server.
#define UA_STATUSCODE_BADTOOMANYMATCHES 0x806d0000 // The requested operation has too many matches to return.
#define UA_STATUSCODE_BADQUERYTOOCOMPLEX 0x806e0000 // The requested operation requires too many resources in the server.
#define UA_STATUSCODE_BADNOMATCH 0x806f0000 // The requested operation has no match to return.
#define UA_STATUSCODE_BADMAXAGEINVALID 0x80700000 // The max age parameter is invalid.
#define UA_STATUSCODE_BADHISTORYOPERATIONINVALID 0x80710000 // The history details parameter is not valid.
#define UA_STATUSCODE_BADHISTORYOPERATIONUNSUPPORTED 0x80720000 // The server does not support the requested operation.
#define UA_STATUSCODE_BADINVALIDTIMESTAMPARGUMENT 0x80bd0000 // The defined timestamp to return was invalid.
#define UA_STATUSCODE_BADWRITENOTSUPPORTED 0x80730000 // The server not does support writing the combination of value status and timestamps provided.
#define UA_STATUSCODE_BADTYPEMISMATCH 0x80740000 // The value supplied for the attribute is not of the same type as the attribute's value.
#define UA_STATUSCODE_BADMETHODINVALID 0x80750000 // The method id does not refer to a method for the specified object.
#define UA_STATUSCODE_BADARGUMENTSMISSING 0x80760000 // The client did not specify all of the input arguments for the method.
#define UA_STATUSCODE_BADTOOMANYSUBSCRIPTIONS 0x80770000 // The server has reached its  maximum number of subscriptions.
#define UA_STATUSCODE_BADTOOMANYPUBLISHREQUESTS 0x80780000 // The server has reached the maximum number of queued publish requests.
#define UA_STATUSCODE_BADNOSUBSCRIPTION 0x80790000 // There is no subscription available for this session.
#define UA_STATUSCODE_BADSEQUENCENUMBERUNKNOWN 0x807a0000 // The sequence number is unknown to the server.
#define UA_STATUSCODE_BADMESSAGENOTAVAILABLE 0x807b0000 // The requested notification message is no longer available.
#define UA_STATUSCODE_BADINSUFFICIENTCLIENTPROFILE 0x807c0000 // The Client of the current Session does not support one or more Profiles that are necessary for the Subscription.
#define UA_STATUSCODE_BADSTATENOTACTIVE 0x80bf0000 // The sub-state machine is not currently active.
#define UA_STATUSCODE_BADTCPSERVERTOOBUSY 0x807d0000 // The server cannot process the request because it is too busy.
#define UA_STATUSCODE_BADTCPMESSAGETYPEINVALID 0x807e0000 // The type of the message specified in the header invalid.
#define UA_STATUSCODE_BADTCPSECURECHANNELUNKNOWN 0x807f0000 // The SecureChannelId and/or TokenId are not currently in use.
#define UA_STATUSCODE_BADTCPMESSAGETOOLARGE 0x80800000 // The size of the message specified in the header is too large.
#define UA_STATUSCODE_BADTCPNOTENOUGHRESOURCES 0x80810000 // There are not enough resources to process the request.
#define UA_STATUSCODE_BADTCPINTERNALERROR 0x80820000 // An internal error occurred.
#define UA_STATUSCODE_BADTCPENDPOINTURLINVALID 0x80830000 // The Server does not recognize the QueryString specified.
#define UA_STATUSCODE_BADREQUESTINTERRUPTED 0x80840000 // The request could not be sent because of a network interruption.
#define UA_STATUSCODE_BADREQUESTTIMEOUT 0x80850000 // Timeout occurred while processing the request.
#define UA_STATUSCODE_BADSECURECHANNELCLOSED 0x80860000 // The secure channel has been closed.
#define UA_STATUSCODE_BADSECURECHANNELTOKENUNKNOWN 0x80870000 // The token has expired or is not recognized.
#define UA_STATUSCODE_BADSEQUENCENUMBERINVALID 0x80880000 // The sequence number is not valid.
#define UA_STATUSCODE_BADPROTOCOLVERSIONUNSUPPORTED 0x80be0000 // The applications do not have compatible protocol versions.
#define UA_STATUSCODE_BADCONFIGURATIONERROR 0x80890000 // There is a problem with the configuration that affects the usefulness of the value.
#define UA_STATUSCODE_BADNOTCONNECTED 0x808a0000 // The variable should receive its value from another variable but has never been configured to do so.
#define UA_STATUSCODE_BADDEVICEFAILURE 0x808b0000 // There has been a failure in the device/data source that generates the value that has affected the value.
#define UA_STATUSCODE_BADSENSORFAILURE 0x808c0000 // There has been a failure in the sensor from which the value is derived by the device/data source.
#define UA_STATUSCODE_BADOUTOFSERVICE 0x808d0000 // The source of the data is not operational.
#define UA_STATUSCODE_BADDEADBANDFILTERINVALID 0x808e0000 // The deadband filter is not valid.
#define UA_STATUSCODE_UNCERTAINNOCOMMUNICATIONLASTUSABLEVALUE 0x408f0000 // Communication to the data source has failed. The variable value is the last value that had a good quality.
#define UA_STATUSCODE_UNCERTAINLASTUSABLEVALUE 0x40900000 // Whatever was updating this value has stopped doing so.
#define UA_STATUSCODE_UNCERTAINSUBSTITUTEVALUE 0x40910000 // The value is an operational value that was manually overwritten.
#define UA_STATUSCODE_UNCERTAININITIALVALUE 0x40920000 // The value is an initial value for a variable that normally receives its value from another variable.
#define UA_STATUSCODE_UNCERTAINSENSORNOTACCURATE 0x40930000 // The value is at one of the sensor limits.
#define UA_STATUSCODE_UNCERTAINENGINEERINGUNITSEXCEEDED 0x40940000 // The value is outside of the range of values defined for this parameter.
#define UA_STATUSCODE_UNCERTAINSUBNORMAL 0x40950000 // The value is derived from multiple sources and has less than the required number of Good sources.
#define UA_STATUSCODE_GOODLOCALOVERRIDE 0x00960000 // The value has been overridden.
#define UA_STATUSCODE_BADREFRESHINPROGRESS 0x80970000 // This Condition refresh failed a Condition refresh operation is already in progress.
#define UA_STATUSCODE_BADCONDITIONALREADYDISABLED 0x80980000 // This condition has already been disabled.
#define UA_STATUSCODE_BADCONDITIONALREADYENABLED 0x80cc0000 // This condition has already been enabled.
#define UA_STATUSCODE_BADCONDITIONDISABLED 0x80990000 // Property not available this condition is disabled.
#define UA_STATUSCODE_BADEVENTIDUNKNOWN 0x809a0000 // The specified event id is not recognized.
#define UA_STATUSCODE_BADEVENTNOTACKNOWLEDGEABLE 0x80bb0000 // The event cannot be acknowledged.
#define UA_STATUSCODE_BADDIALOGNOTACTIVE 0x80cd0000 // The dialog condition is not active.
#define UA_STATUSCODE_BADDIALOGRESPONSEINVALID 0x80ce0000 // The response is not valid for the dialog.
#define UA_STATUSCODE_BADCONDITIONBRANCHALREADYACKED 0x80cf0000 // The condition branch has already been acknowledged.
#define UA_STATUSCODE_BADCONDITIONBRANCHALREADYCONFIRMED 0x80d00000 // The condition branch has already been confirmed.
#define UA_STATUSCODE_BADCONDITIONALREADYSHELVED 0x80d10000 // The condition has already been shelved.
#define UA_STATUSCODE_BADCONDITIONNOTSHELVED 0x80d20000 // The condition is not currently shelved.
#define UA_STATUSCODE_BADSHELVINGTIMEOUTOFRANGE 0x80d30000 // The shelving time not within an acceptable range.
#define UA_STATUSCODE_BADNODATA 0x809b0000 // No data exists for the requested time range or event filter.
#define UA_STATUSCODE_BADBOUNDNOTFOUND 0x80d70000 // No data found to provide upper or lower bound value.
#define UA_STATUSCODE_BADBOUNDNOTSUPPORTED 0x80d80000 // The server cannot retrieve a bound for the variable.
#define UA_STATUSCODE_BADDATALOST 0x809d0000 // Data is missing due to collection started/stopped/lost.
#define UA_STATUSCODE_BADDATAUNAVAILABLE 0x809e0000 // Expected data is unavailable for the requested time range due to an un-mounted volume, an off-line archive or tape or similar reason for temporary unavailability.
#define UA_STATUSCODE_BADENTRYEXISTS 0x809f0000 // The data or event was not successfully inserted because a matching entry exists.
#define UA_STATUSCODE_BADNOENTRYEXISTS 0x80a00000 // The data or event was not successfully updated because no matching entry exists.
#define UA_STATUSCODE_BADTIMESTAMPNOTSUPPORTED 0x80a10000 // The client requested history using a timestamp format the server does not support (i.e requested ServerTimestamp when server only supports SourceTimestamp).
#define UA_STATUSCODE_GOODENTRYINSERTED 0x00a20000 // The data or event was successfully inserted into the historical database.
#define UA_STATUSCODE_GOODENTRYREPLACED 0x00a30000 // The data or event field was successfully replaced in the historical database.
#define UA_STATUSCODE_UNCERTAINDATASUBNORMAL 0x40a40000 // The value is derived from multiple values and has less than the required number of Good values.
#define UA_STATUSCODE_GOODNODATA 0x00a50000 // No data exists for the requested time range or event filter.
#define UA_STATUSCODE_GOODMOREDATA 0x00a60000 // The data or event field was successfully replaced in the historical database.
#define UA_STATUSCODE_BADAGGREGATELISTMISMATCH 0x80d40000 // The requested number of Aggregates does not match the requested number of NodeIds.
#define UA_STATUSCODE_BADAGGREGATENOTSUPPORTED 0x80d50000 // The requested Aggregate is not support by the server.
#define UA_STATUSCODE_BADAGGREGATEINVALIDINPUTS 0x80d60000 // The aggregate value could not be derived due to invalid data inputs.
#define UA_STATUSCODE_BADAGGREGATECONFIGURATIONREJECTED 0x80da0000 // The aggregate configuration is not valid for specified node.
#define UA_STATUSCODE_GOODDATAIGNORED 0x00d90000 // The request specifies fields which are not valid for the EventType or cannot be saved by the historian.
#define UA_STATUSCODE_GOODCOMMUNICATIONEVENT 0x00a70000 // The communication layer has raised an event.
#define UA_STATUSCODE_GOODSHUTDOWNEVENT 0x00a80000 // The system is shutting down.
#define UA_STATUSCODE_GOODCALLAGAIN 0x00a90000 // The operation is not finished and needs to be called again.
#define UA_STATUSCODE_GOODNONCRITICALTIMEOUT 0x00aa0000 // A non-critical timeout occurred.
#define UA_STATUSCODE_BADINVALIDARGUMENT 0x80ab0000 // One or more arguments are invalid.
#define UA_STATUSCODE_BADCONNECTIONREJECTED 0x80ac0000 // Could not establish a network connection to remote server.
#define UA_STATUSCODE_BADDISCONNECT 0x80ad0000 // The server has disconnected from the client.
#define UA_STATUSCODE_BADCONNECTIONCLOSED 0x80ae0000 // The network connection has been closed.
#define UA_STATUSCODE_BADINVALIDSTATE 0x80af0000 // The operation cannot be completed because the object is closed uninitialized or in some other invalid state.
#define UA_STATUSCODE_BADENDOFSTREAM 0x80b00000 // Cannot move beyond end of the stream.
#define UA_STATUSCODE_BADNODATAAVAILABLE 0x80b10000 // No data is currently available for reading from a non-blocking stream.
#define UA_STATUSCODE_BADWAITINGFORRESPONSE 0x80b20000 // The asynchronous operation is waiting for a response.
#define UA_STATUSCODE_BADOPERATIONABANDONED 0x80b30000 // The asynchronous operation was abandoned by the caller.
#define UA_STATUSCODE_BADEXPECTEDSTREAMTOBLOCK 0x80b40000 // The stream did not return all data requested (possibly because it is a non-blocking stream).
#define UA_STATUSCODE_BADWOULDBLOCK 0x80b50000 // Non blocking behaviour is required and the operation would block.
#define UA_STATUSCODE_BADSYNTAXERROR 0x80b60000 // A value had an invalid syntax.
#define UA_STATUSCODE_BADMAXCONNECTIONSREACHED 0x80b70000 // The operation could not be finished because all available connections are in use.

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_types.h" ***********************************/

/*
 * Copyright (C) 2013-2015 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

/**
 * Data Types
 * ==========
 *
 * In open62541, all data types share the same basic API for creation, copying
 * and deletion. The header ua_types.h defines the builtin types. In addition,
 * we auto-generate ua_types_generated.h with additional types as well as the
 * following function definitions for all (builtin and generated) data types
 * ``T``.
 *
 * ``void T_init(T *ptr)``
 *   Initialize the data type. This is synonymous with zeroing out the memory,
 *   i.e. ``memset(dataptr, 0, sizeof(T))``.
 * ``T* T_new()``
 *   Allocate and return the memory for the data type. The memory is already initialized.
 * ``UA_StatusCode T_copy(const T *src, T *dst)``
 *   Copy the content of the data type. Returns ``UA_STATUSCODE_GOOD`` or
 *   ``UA_STATUSCODE_BADOUTOFMEMORY``.
 * ``void T_deleteMembers(T *ptr)``
 *   Delete the dynamically allocated content of the data type, but not the data type itself.
 * ``void T_delete(T *ptr)``
 *   Delete the content of the data type and the memory for the data type itself.
 *
 * OPC UA defines 25 builtin data types. All other data types are combinations
 * of the 25 builtin data types. */

#define UA_BUILTIN_TYPES_COUNT 25U

/**
 * Builtin Types Part 1
 * --------------------
 *
 * Boolean
 * ^^^^^^^
 * A two-state logical value (true or false). */
typedef bool UA_Boolean;
#define UA_TRUE true
#define UA_FALSE false

/**
 * SByte
 * ^^^^^
 * An integer value between -128 and 127. */
typedef int8_t UA_SByte;
#define UA_SBYTE_MAX 127
#define UA_SBYTE_MIN (-128)

/**
 * Byte
 * ^^^^
 * An integer value between 0 and 256. */
typedef uint8_t UA_Byte;
#define UA_BYTE_MAX 256
#define UA_BYTE_MIN 0

/**
 * Int16
 * ^^^^^
 * An integer value between -32 768 and 32 767. */
typedef int16_t UA_Int16;
#define UA_INT16_MAX 32767
#define UA_INT16_MIN (-32768)

/**
 * UInt16
 * ^^^^^^
 * An integer value between 0 and 65 535. */
typedef uint16_t UA_UInt16;
#define UA_UINT16_MAX 65535
#define UA_UINT16_MIN 0

/**
 * Int32
 * ^^^^^
 * An integer value between -2 147 483 648 and 2 147 483 647. */
typedef int32_t UA_Int32;
#define UA_INT32_MAX 2147483647
#define UA_INT32_MIN (-2147483648)

/**
 * UInt32
 * ^^^^^^
 * An integer value between 0 and 4 294 967 295. */
typedef uint32_t UA_UInt32;
#define UA_UINT32_MAX 4294967295
#define UA_UINT32_MIN 0

/**
 * Int64
 * ^^^^^
 * An integer value between -10 223 372 036 854 775 808 and 9 223 372 036 854 775 807. */
typedef int64_t UA_Int64;
#define UA_INT64_MAX (int64_t)9223372036854775807
#define UA_INT64_MIN ((int64_t)-9223372036854775808)

/**
 * UInt64
 * ^^^^^^
 * An integer value between 0 and 18 446 744 073 709 551 615. */
typedef uint64_t UA_UInt64;
#define UA_UINT64_MAX (int64_t)18446744073709551615
#define UA_UINT64_MIN (int64_t)0

/**
 * Float
 * ^^^^^
 * An IEEE single precision (32 bit) floating point value. */
typedef float UA_Float;

/**
 * Double
 * ^^^^^^
 * An IEEE double precision (64 bit) floating point value. */
typedef double UA_Double;

/**
 * .. _statuscode:
 *
 * StatusCode
 * ^^^^^^^^^^
 * A numeric identifier for a error or condition that is associated with a value or an
 * operation. See the section :ref:`statuscodes` for the meaning of a specific code. */
typedef uint32_t UA_StatusCode;

/**
 * Array handling
 * --------------
 * In OPC UA, arrays can have a length of zero or more with the usual meaning.
 * In addition, arrays can be undefined. Then, they don't even have a length. In
 * the binary encoding, this is indicated by an array of length -1.
 *
 * In open62541 however, we use ``size_t`` for array lengths. An undefined array
 * has length 0 and the data pointer is NULL. An array of length 0 also has
 * length 0 but points to a sentinel memory address. */
#define UA_EMPTY_ARRAY_SENTINEL ((void*)0x01)

/** Forward Declaration of UA_DataType. See Section `Generic Type Handling`_
    for details. */
struct UA_DataType;
typedef struct UA_DataType UA_DataType; 

/** The following functions are used for handling arrays of any data type. */

/* Allocates and initializes an array of variables of a specific type
 *
 * @param size The requested array length
 * @param type The datatype description
 * @return Returns the memory location of the variable or (void*)0 if no memory
           could be allocated */
void UA_EXPORT * UA_Array_new(size_t size, const UA_DataType *type) UA_FUNC_ATTR_MALLOC;

/* Allocates and copies an array
 *
 * @param src The memory location of the source array
 * @param size The size of the array
 * @param dst The location of the pointer to the new array
 * @param type The datatype of the array members
 * @return Returns UA_STATUSCODE_GOOD or UA_STATUSCODE_BADOUTOFMEMORY */
UA_StatusCode UA_EXPORT UA_Array_copy(const void *src, size_t size, void **dst, const UA_DataType *type) UA_FUNC_ATTR_WARN_UNUSED_RESULT;

/* Deletes an array.
 *
 * @param p The memory location of the array
 * @param size The size of the array
 * @param type The datatype of the array members */
void UA_EXPORT UA_Array_delete(void *p, size_t size, const UA_DataType *type);

/**
 * Builtin Types, Part 2
 * ---------------------
 *
 * String
 * ^^^^^^
 * A sequence of Unicode characters. Strings are just an array of UA_Byte. */
typedef struct {
    size_t length; /* The length of the string */
    UA_Byte *data; /* The content (not null-terminated) */
} UA_String;

/* Copies the content on the heap. Returns a null-string when alloc fails */
UA_String UA_EXPORT UA_String_fromChars(char const src[]) UA_FUNC_ATTR_WARN_UNUSED_RESULT;

UA_Boolean UA_EXPORT UA_String_equal(const UA_String *s1, const UA_String *s2);

UA_EXPORT extern const UA_String UA_STRING_NULL;

/**
 * ``UA_STRING`` returns a string pointing to the preallocated char-array.
 * ``UA_STRING_ALLOC`` is shorthand for ``UA_String_fromChars`` and makes a copy
 * of the char-array. */
static UA_INLINE UA_String
UA_STRING(char *chars) {
    UA_String str; str.length = strlen(chars);
    str.data = (UA_Byte*)chars; return str;
}

#define UA_STRING_ALLOC(CHARS) UA_String_fromChars(CHARS)

/**
 * DateTime
 * ^^^^^^^^
 * An instance in time. A DateTime value is encoded as a 64-bit signed integer
 * which represents the number of 100 nanosecond intervals since January 1, 1601
 * (UTC). */
typedef int64_t UA_DateTime;

/* Multiply to convert units for time difference computations */
#define UA_USEC_TO_DATETIME 10LL
#define UA_MSEC_TO_DATETIME (UA_USEC_TO_DATETIME * 1000LL)
#define UA_SEC_TO_DATETIME (UA_MSEC_TO_DATETIME * 1000LL)

/* Datetime of 1 Jan 1970 00:00 UTC */
#define UA_DATETIME_UNIX_EPOCH (11644473600LL * UA_SEC_TO_DATETIME)

/* The current time */
UA_DateTime UA_EXPORT UA_DateTime_now(void);

/* CPU clock invariant to system time changes. Use only for time diffs, not current time */
UA_DateTime UA_EXPORT UA_DateTime_nowMonotonic(void);

typedef struct UA_DateTimeStruct {
    UA_UInt16 nanoSec;
    UA_UInt16 microSec;
    UA_UInt16 milliSec;
    UA_UInt16 sec;
    UA_UInt16 min;
    UA_UInt16 hour;
    UA_UInt16 day;
    UA_UInt16 month;
    UA_UInt16 year;
} UA_DateTimeStruct;

UA_DateTimeStruct UA_EXPORT UA_DateTime_toStruct(UA_DateTime t);

UA_String UA_EXPORT UA_DateTime_toString(UA_DateTime t);

/**
 * Guid
 * ^^^^
 * A 16 byte value that can be used as a globally unique identifier. */
typedef struct {
    UA_UInt32 data1;
    UA_UInt16 data2;
    UA_UInt16 data3;
    UA_Byte   data4[8];
} UA_Guid;

UA_Boolean UA_EXPORT UA_Guid_equal(const UA_Guid *g1, const UA_Guid *g2);

/**
 * ByteString
 * ^^^^^^^^^^
 * A sequence of octets. */
typedef UA_String UA_ByteString;

static UA_INLINE UA_Boolean
UA_ByteString_equal(const UA_ByteString *string1, const UA_ByteString *string2) {
    return UA_String_equal((const UA_String*)string1, (const UA_String*)string2); }

/* Allocates memory of size length for the bytestring. The content is not set to zero. */
UA_StatusCode UA_EXPORT UA_ByteString_allocBuffer(UA_ByteString *bs, size_t length);

UA_EXPORT extern const UA_ByteString UA_BYTESTRING_NULL;

static UA_INLINE UA_ByteString
UA_BYTESTRING(char *chars) {
    UA_ByteString str; str.length = strlen(chars);
    str.data = (UA_Byte*)chars; return str;
}

static UA_INLINE UA_ByteString
UA_BYTESTRING_ALLOC(const char *chars) {
    UA_String str = UA_String_fromChars(chars); UA_ByteString bstr;
    bstr.length = str.length; bstr.data = str.data; return bstr;
}

/**
 * XmlElement
 * ^^^^^^^^^^
 * An XML element. */
typedef UA_String UA_XmlElement;

/**
 * NodeId
 * ^^^^^^
 * An identifier for a node in the address space of an OPC UA Server. */
enum UA_NodeIdType {
    UA_NODEIDTYPE_NUMERIC    = 0, /* In the binary encoding, this can also become 1 or 2
                                     (2byte and 4byte encoding of small numeric nodeids) */
    UA_NODEIDTYPE_STRING     = 3,
    UA_NODEIDTYPE_GUID       = 4,
    UA_NODEIDTYPE_BYTESTRING = 5
};

typedef struct {
    UA_UInt16 namespaceIndex;
    enum UA_NodeIdType identifierType;
    union {
        UA_UInt32     numeric;
        UA_String     string;
        UA_Guid       guid;
        UA_ByteString byteString;
    } identifier;
} UA_NodeId;

UA_EXPORT extern const UA_NodeId UA_NODEID_NULL;

static UA_INLINE UA_Boolean
UA_NodeId_isNull(const UA_NodeId *p) {
    return (p->namespaceIndex == 0 &&
            p->identifierType == UA_NODEIDTYPE_NUMERIC &&
            p->identifier.numeric == 0);
}

UA_Boolean UA_EXPORT UA_NodeId_equal(const UA_NodeId *n1, const UA_NodeId *n2);

/** The following functions are shorthand for creating NodeIds. */
static UA_INLINE UA_NodeId
UA_NODEID_NUMERIC(UA_UInt16 nsIndex, UA_UInt32 identifier) {
    UA_NodeId id; id.namespaceIndex = nsIndex;
    id.identifierType = UA_NODEIDTYPE_NUMERIC;
    id.identifier.numeric = identifier; return id;
}

static UA_INLINE UA_NodeId
UA_NODEID_STRING(UA_UInt16 nsIndex, char *chars) {
    UA_NodeId id; id.namespaceIndex = nsIndex;
    id.identifierType = UA_NODEIDTYPE_STRING;
    id.identifier.string = UA_STRING(chars); return id;
}

static UA_INLINE UA_NodeId
UA_NODEID_STRING_ALLOC(UA_UInt16 nsIndex, const char *chars) {
    UA_NodeId id; id.namespaceIndex = nsIndex;
    id.identifierType = UA_NODEIDTYPE_STRING;
    id.identifier.string = UA_STRING_ALLOC(chars); return id;
}

static UA_INLINE UA_NodeId
UA_NODEID_GUID(UA_UInt16 nsIndex, UA_Guid guid) {
    UA_NodeId id; id.namespaceIndex = nsIndex;
    id.identifierType = UA_NODEIDTYPE_GUID;
    id.identifier.guid = guid; return id;
}

static UA_INLINE UA_NodeId
UA_NODEID_BYTESTRING(UA_UInt16 nsIndex, char *chars) {
    UA_NodeId id; id.namespaceIndex = nsIndex;
    id.identifierType = UA_NODEIDTYPE_BYTESTRING;
    id.identifier.byteString = UA_BYTESTRING(chars); return id;
}

static UA_INLINE UA_NodeId
UA_NODEID_BYTESTRING_ALLOC(UA_UInt16 nsIndex, const char *chars) {
    UA_NodeId id; id.namespaceIndex = nsIndex;
    id.identifierType = UA_NODEIDTYPE_BYTESTRING;
    id.identifier.byteString = UA_BYTESTRING_ALLOC(chars); return id;
}

/**
 * ExpandedNodeId
 * ^^^^^^^^^^^^^^
 * A NodeId that allows the namespace URI to be specified instead of an index. */
typedef struct {
    UA_NodeId nodeId;
    UA_String namespaceUri;
    UA_UInt32 serverIndex;
} UA_ExpandedNodeId;

/** The following functions are shorthand for creating ExpandedNodeIds. */
static UA_INLINE UA_ExpandedNodeId
UA_EXPANDEDNODEID_NUMERIC(UA_UInt16 nsIndex, UA_UInt32 identifier) {
    UA_ExpandedNodeId id; id.nodeId = UA_NODEID_NUMERIC(nsIndex, identifier);
    id.serverIndex = 0; id.namespaceUri = UA_STRING_NULL; return id;
}

static UA_INLINE UA_ExpandedNodeId
UA_EXPANDEDNODEID_STRING(UA_UInt16 nsIndex, char *chars) {
    UA_ExpandedNodeId id; id.nodeId = UA_NODEID_STRING(nsIndex, chars);
    id.serverIndex = 0; id.namespaceUri = UA_STRING_NULL; return id;
}

static UA_INLINE UA_ExpandedNodeId
UA_EXPANDEDNODEID_STRING_ALLOC(UA_UInt16 nsIndex, const char *chars) {
    UA_ExpandedNodeId id; id.nodeId = UA_NODEID_STRING_ALLOC(nsIndex, chars);
    id.serverIndex = 0; id.namespaceUri = UA_STRING_NULL; return id;
}

static UA_INLINE UA_ExpandedNodeId
UA_EXPANDEDNODEID_STRING_GUID(UA_UInt16 nsIndex, UA_Guid guid) {
    UA_ExpandedNodeId id; id.nodeId = UA_NODEID_GUID(nsIndex, guid);
    id.serverIndex = 0; id.namespaceUri = UA_STRING_NULL; return id;
}

static UA_INLINE UA_ExpandedNodeId
UA_EXPANDEDNODEID_BYTESTRING(UA_UInt16 nsIndex, char *chars) {
    UA_ExpandedNodeId id; id.nodeId = UA_NODEID_BYTESTRING(nsIndex, chars);
    id.serverIndex = 0; id.namespaceUri = UA_STRING_NULL; return id;
}

static UA_INLINE UA_ExpandedNodeId
UA_EXPANDEDNODEID_BYTESTRING_ALLOC(UA_UInt16 nsIndex, const char *chars) {
    UA_ExpandedNodeId id; id.nodeId = UA_NODEID_BYTESTRING_ALLOC(nsIndex, chars);
    id.serverIndex = 0; id.namespaceUri = UA_STRING_NULL; return id;
}

/**
 * QualifiedName
 * ^^^^^^^^^^^^^
 * A name qualified by a namespace. */
typedef struct {
    UA_UInt16 namespaceIndex;
    UA_String name;
} UA_QualifiedName;

static UA_INLINE UA_QualifiedName
UA_QUALIFIEDNAME(UA_UInt16 nsIndex, char *chars) {
    UA_QualifiedName qn; qn.namespaceIndex = nsIndex;
    qn.name = UA_STRING(chars); return qn;
}

static UA_INLINE UA_QualifiedName
UA_QUALIFIEDNAME_ALLOC(UA_UInt16 nsIndex, const char *chars) {
    UA_QualifiedName qn; qn.namespaceIndex = nsIndex;
    qn.name = UA_STRING_ALLOC(chars); return qn;
}

/**
 * LocalizedText
 * ^^^^^^^^^^^^^
 * Human readable text with an optional locale identifier. */
typedef struct {
    UA_String locale;
    UA_String text;
} UA_LocalizedText;

static UA_INLINE UA_LocalizedText
UA_LOCALIZEDTEXT(char *locale, char *text) {
    UA_LocalizedText lt; lt.locale = UA_STRING(locale);
    lt.text = UA_STRING(text); return lt;
}

static UA_INLINE UA_LocalizedText
UA_LOCALIZEDTEXT_ALLOC(const char *locale, const char *text) {
    UA_LocalizedText lt; lt.locale = UA_STRING_ALLOC(locale);
    lt.text = UA_STRING_ALLOC(text); return lt;
}

/**
 * ExtensionObject
 * ^^^^^^^^^^^^^^^
 * ExtensionObjects may contain scalars of any data type. Even those that are
 * unknown to the receiver. See the Section `Generic Type Handling`_ on how
 * types are described. An ExtensionObject always contains the NodeId of the
 * Data Type. If the data cannot be decoded, we keep the encoded string and the
 * NodeId. */
typedef struct {
    enum {
        UA_EXTENSIONOBJECT_ENCODED_NOBODY     = 0,
        UA_EXTENSIONOBJECT_ENCODED_BYTESTRING = 1,
        UA_EXTENSIONOBJECT_ENCODED_XML        = 2,
        UA_EXTENSIONOBJECT_DECODED            = 3,
        UA_EXTENSIONOBJECT_DECODED_NODELETE   = 4 /* Don't delete the decoded content
                                                     at the lifecycle end */
    } encoding;
    union {
        struct {
            UA_NodeId typeId;   /* The nodeid of the datatype */
            UA_ByteString body; /* The bytestring of the encoded data */
        } encoded;
        struct {
            const UA_DataType *type;
            void *data;
        } decoded;
    } content;
} UA_ExtensionObject;

/**
 * Variant
 * ^^^^^^^
 * Variants may contain data of any type. See the Section `Generic Type
 * Handling`_ on how types are described. If the data is not of one of the 25
 * builtin types, it will be encoded as an `ExtensionObject`_ on the wire. (The
 * standard says that a variant is a union of the built-in types. open62541
 * generalizes this to any data type by transparently de- and encoding
 * ExtensionObjects in the background. If the decoding fails, the variant
 * contains the original ExtensionObject.)
 *
 * Variants can contain a single scalar or an array. For details on the handling
 * of arrays, see the Section `Array Handling`_. Array variants can have an
 * additional dimensionality (matrix, 3-tensor, ...) defined in an array of
 * dimension sizes. Higher rank dimensions are serialized first.
 *
 * The differentiation between variants containing a scalar, an array or no data
 * is as follows:
 *
 * - arrayLength == 0 && data == NULL: no existing data
 * - arrayLength == 0 && data == UA_EMPTY_ARRAY_SENTINEL: array of length 0
 * - arrayLength == 0 && data > UA_EMPTY_ARRAY_SENTINEL: scalar value
 * - arrayLength > 0: array of the given length */
typedef struct {
    const UA_DataType *type; /* The data type description */
    enum {
        UA_VARIANT_DATA,          /* The data has the same lifecycle as the variant */
        UA_VARIANT_DATA_NODELETE, /* The data is "borrowed" by the variant and shall not be
                                     deleted at the end of the variant's lifecycle. */
    } storageType;
    size_t arrayLength;  // The number of elements in the data array
    void *data; // Points to the scalar or array data
    size_t arrayDimensionsSize; // The number of dimensions the data-array has
    UA_Int32 *arrayDimensions; // The length of each dimension of the data-array
} UA_Variant;

/* Returns true if the variant contains a scalar value. Note that empty variants contain
 * an array of length -1 (undefined).
 *
 * @param v The variant
 * @return Does the variant contain a scalar value. */
static UA_INLINE UA_Boolean
UA_Variant_isScalar(const UA_Variant *v) {
    return (v->arrayLength == 0 && v->data > UA_EMPTY_ARRAY_SENTINEL);
}
    
/* Set the variant to a scalar value that already resides in memory. The value takes on
 * the lifecycle of the variant and is deleted with it.
 *
 * @param v The variant
 * @param p A pointer to the value data
 * @param type The datatype of the value in question */
void UA_EXPORT UA_Variant_setScalar(UA_Variant *v, void * UA_RESTRICT p, const UA_DataType *type);

/* Set the variant to a scalar value that is copied from an existing variable.
 * @param v The variant
 * @param p A pointer to the value data
 * @param type The datatype of the value
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT UA_Variant_setScalarCopy(UA_Variant *v, const void *p, const UA_DataType *type);

/* Set the variant to an array that already resides in memory. The array takes on the
 * lifecycle of the variant and is deleted with it.
 *
 * @param v The variant
 * @param array A pointer to the array data
 * @param arraySize The size of the array
 * @param type The datatype of the array */
void UA_EXPORT
UA_Variant_setArray(UA_Variant *v, void * UA_RESTRICT array,
                    size_t arraySize, const UA_DataType *type);

/* Set the variant to an array that is copied from an existing array.
 *
 * @param v The variant
 * @param array A pointer to the array data
 * @param arraySize The size of the array
 * @param type The datatype of the array
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT
UA_Variant_setArrayCopy(UA_Variant *v, const void *array,
                        size_t arraySize, const UA_DataType *type);

/**
 * NumericRanges are used to indicate subsets of a (multidimensional) variant
 * array. NumericRange has no official type structure in the standard. On the
 * wire, it only exists as an encoded string, such as "1:2,0:3,5". The colon
 * separates min/max index and the comma separates dimensions. A single value
 * indicates a range with a single element (min==max). */
typedef struct {
    size_t dimensionsSize;
    struct UA_NumericRangeDimension {
        UA_UInt32 min;
        UA_UInt32 max;
    } *dimensions;
} UA_NumericRange;

/* Copy the variant, but use only a subset of the (multidimensional) array into a variant.
 * Returns an error code if the variant is not an array or if the indicated range does not
 * fit.
 *
 * @param src The source variant
 * @param dst The target variant
 * @param range The range of the copied data
 * @return Returns UA_STATUSCODE_GOOD or an error code */
UA_StatusCode UA_EXPORT
UA_Variant_copyRange(const UA_Variant *src, UA_Variant *dst, const UA_NumericRange range);

/* Insert a range of data into an existing variant. The data array can't be reused afterwards if it
 * contains types without a fixed size (e.g. strings) since the members are moved into the variant
 * and take on its lifecycle.
 *
 * @param v The variant
 * @param dataArray The data array. The type must match the variant
 * @param dataArraySize The length of the data array. This is checked to match the range size.
 * @param range The range of where the new data is inserted
 * @return Returns UA_STATUSCODE_GOOD or an error code */
UA_StatusCode UA_EXPORT
UA_Variant_setRange(UA_Variant *v, void * UA_RESTRICT array,
                    size_t arraySize, const UA_NumericRange range);

/* Deep-copy a range of data into an existing variant.
 *
 * @param v The variant
 * @param dataArray The data array. The type must match the variant
 * @param dataArraySize The length of the data array. This is checked to match the range size.
 * @param range The range of where the new data is inserted
 * @return Returns UA_STATUSCODE_GOOD or an error code */
UA_StatusCode UA_EXPORT
UA_Variant_setRangeCopy(UA_Variant *v, const void *array,
                        size_t arraySize, const UA_NumericRange range);

/**
 * DataValue
 * ^^^^^^^^^
 * A data value with an associated status code and timestamps. */
typedef struct {
    UA_Boolean    hasValue             : 1;
    UA_Boolean    hasStatus            : 1;
    UA_Boolean    hasSourceTimestamp   : 1;
    UA_Boolean    hasServerTimestamp   : 1;
    UA_Boolean    hasSourcePicoseconds : 1;
    UA_Boolean    hasServerPicoseconds : 1;
    UA_Variant    value;
    UA_StatusCode status;
    UA_DateTime   sourceTimestamp;
    UA_UInt16     sourcePicoseconds;
    UA_DateTime   serverTimestamp;
    UA_UInt16     serverPicoseconds;
} UA_DataValue;

/**
 * DiagnosticInfo
 * ^^^^^^^^^^^^^^
 * A structure that contains detailed error and diagnostic information
 * associated with a StatusCode. */
typedef struct UA_DiagnosticInfo {
    UA_Boolean    hasSymbolicId          : 1;
    UA_Boolean    hasNamespaceUri        : 1;
    UA_Boolean    hasLocalizedText       : 1;
    UA_Boolean    hasLocale              : 1;
    UA_Boolean    hasAdditionalInfo      : 1;
    UA_Boolean    hasInnerStatusCode     : 1;
    UA_Boolean    hasInnerDiagnosticInfo : 1;
    UA_Int32      symbolicId;
    UA_Int32      namespaceUri;
    UA_Int32      localizedText;
    UA_Int32      locale;
    UA_String     additionalInfo;
    UA_StatusCode innerStatusCode;
    struct UA_DiagnosticInfo *innerDiagnosticInfo;
} UA_DiagnosticInfo;

/**
 * Generic Type Handling
 * ---------------------
 * The builtin types can be combined to data structures. All information about a
 * (structured) data type is stored in a ``UA_DataType``. The array ``UA_TYPES``
 * contains the description of all standard-defined types and is used for
 * handling of generic types. */
typedef struct {
#ifdef UA_ENABLE_TYPENAMES
    const char *memberName;
#endif
    UA_UInt16 memberTypeIndex;    /* Index of the member in the array of data types */
    UA_Byte   padding;            /* How much padding is there before this member element?
                                     For arrays this is the padding before the size_t
                                     lenght member. (No padding between size_t and the
                                     following ptr.) */
    UA_Boolean namespaceZero : 1; /* The type of the member is defined in namespace zero.
                                     In this implementation, types from custom namespace
                                     may contain members from the same namespace or ns0
                                     only.*/
    UA_Boolean isArray       : 1; /* The member is an array */
} UA_DataTypeMember;
    
struct UA_DataType {
#ifdef UA_ENABLE_TYPENAMES
    const char *typeName;
#endif
    UA_NodeId  typeId;           /* The nodeid of the type */
    UA_UInt16  memSize;          /* Size of the struct in memory */
    UA_UInt16  typeIndex;        /* Index of the type in the datatypetable */
    UA_Byte    membersSize;      /* How many members does the type have? */
    UA_Boolean builtin      : 1; /* The type is "builtin" and has dedicated de- and
                                    encoding functions */
    UA_Boolean fixedSize    : 1; /* The type (and its members) contains no pointers */
    UA_Boolean overlayable  : 1; /* The type has the identical memory layout in
                                    memory and on the binary stream. */
    UA_DataTypeMember *members;
};

/** The following functions are used for generic handling of data types. */

/* Allocates and initializes a variable of type dataType
 *
 * @param type The datatype description
 * @return Returns the memory location of the variable or (void*)0 if no memory is available */
void UA_EXPORT * UA_new(const UA_DataType *type) UA_FUNC_ATTR_MALLOC;

/* Initializes a variable to default values
 *
 * @param p The memory location of the variable
 * @param type The datatype description */
static UA_INLINE void
UA_init(void *p, const UA_DataType *type) {
    memset(p, 0, type->memSize);
}

/* Copies the content of two variables. If copying fails (e.g. because no memory was
 * available for an array), then dst is emptied and initialized to prevent memory leaks.
 *
 * @param src The memory location of the source variable
 * @param dst The memory location of the destination variable
 * @param type The datatype description
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT UA_copy(const void *src, void *dst, const UA_DataType *type);

/* Deletes the dynamically allocated content of a variable (e.g. resets all arrays to
 * undefined arrays). Afterwards, the variable can be safely deleted without causing
 * memory leaks. But the variable is not initialized and may contain old data that is not
 * memory-relevant.
 *
 * @param p The memory location of the variable
 * @param type The datatype description of the variable */
void UA_EXPORT UA_deleteMembers(void *p, const UA_DataType *type);

/* Frees a variable and all of its content.
 *
 * @param p The memory location of the variable
 * @param type The datatype description of the variable */
void UA_EXPORT UA_delete(void *p, const UA_DataType *type);

/**
 * Random Number Generator
 * -----------------------
 * If UA_ENABLE_MULTITHREADING is defined, then the seed is stored in thread local
 * storage. The seed is initialized for every thread in the server/client. */
void UA_EXPORT UA_random_seed(UA_UInt64 seed);
UA_UInt32 UA_EXPORT UA_UInt32_random(void); /* do not use for cryptographic entropy */
UA_Guid UA_EXPORT UA_Guid_random(void); /* do not use for cryptographic entropy */

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_nodeids.h" ***********************************/

/**********************************************************
 * /home/travis/build/open62541/open62541/build/src_generated/ua_nodeids.hgen -- do not modify
 **********************************************************
 * Generated from /home/travis/build/open62541/open62541/tools/schema/NodeIds.csv with script /home/travis/build/open62541/open62541/tools/generate_nodeids.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03
 **********************************************************/
 

#define UA_NS0ID_BOOLEAN 1 // DataType
#define UA_NS0ID_SBYTE 2 // DataType
#define UA_NS0ID_BYTE 3 // DataType
#define UA_NS0ID_INT16 4 // DataType
#define UA_NS0ID_UINT16 5 // DataType
#define UA_NS0ID_INT32 6 // DataType
#define UA_NS0ID_UINT32 7 // DataType
#define UA_NS0ID_INT64 8 // DataType
#define UA_NS0ID_UINT64 9 // DataType
#define UA_NS0ID_FLOAT 10 // DataType
#define UA_NS0ID_DOUBLE 11 // DataType
#define UA_NS0ID_STRING 12 // DataType
#define UA_NS0ID_DATETIME 13 // DataType
#define UA_NS0ID_GUID 14 // DataType
#define UA_NS0ID_BYTESTRING 15 // DataType
#define UA_NS0ID_XMLELEMENT 16 // DataType
#define UA_NS0ID_NODEID 17 // DataType
#define UA_NS0ID_EXPANDEDNODEID 18 // DataType
#define UA_NS0ID_STATUSCODE 19 // DataType
#define UA_NS0ID_QUALIFIEDNAME 20 // DataType
#define UA_NS0ID_LOCALIZEDTEXT 21 // DataType
#define UA_NS0ID_STRUCTURE 22 // DataType
#define UA_NS0ID_DATAVALUE 23 // DataType
#define UA_NS0ID_BASEDATATYPE 24 // DataType
#define UA_NS0ID_DIAGNOSTICINFO 25 // DataType
#define UA_NS0ID_NUMBER 26 // DataType
#define UA_NS0ID_INTEGER 27 // DataType
#define UA_NS0ID_UINTEGER 28 // DataType
#define UA_NS0ID_ENUMERATION 29 // DataType
#define UA_NS0ID_IMAGE 30 // DataType
#define UA_NS0ID_REFERENCES 31 // ReferenceType
#define UA_NS0ID_NONHIERARCHICALREFERENCES 32 // ReferenceType
#define UA_NS0ID_HIERARCHICALREFERENCES 33 // ReferenceType
#define UA_NS0ID_HASCHILD 34 // ReferenceType
#define UA_NS0ID_ORGANIZES 35 // ReferenceType
#define UA_NS0ID_HASEVENTSOURCE 36 // ReferenceType
#define UA_NS0ID_HASMODELLINGRULE 37 // ReferenceType
#define UA_NS0ID_HASENCODING 38 // ReferenceType
#define UA_NS0ID_HASDESCRIPTION 39 // ReferenceType
#define UA_NS0ID_HASTYPEDEFINITION 40 // ReferenceType
#define UA_NS0ID_GENERATESEVENT 41 // ReferenceType
#define UA_NS0ID_AGGREGATES 44 // ReferenceType
#define UA_NS0ID_HASSUBTYPE 45 // ReferenceType
#define UA_NS0ID_HASPROPERTY 46 // ReferenceType
#define UA_NS0ID_HASCOMPONENT 47 // ReferenceType
#define UA_NS0ID_HASNOTIFIER 48 // ReferenceType
#define UA_NS0ID_HASORDEREDCOMPONENT 49 // ReferenceType
#define UA_NS0ID_FROMSTATE 51 // ReferenceType
#define UA_NS0ID_TOSTATE 52 // ReferenceType
#define UA_NS0ID_HASCAUSE 53 // ReferenceType
#define UA_NS0ID_HASEFFECT 54 // ReferenceType
#define UA_NS0ID_HASHISTORICALCONFIGURATION 56 // ReferenceType
#define UA_NS0ID_BASEOBJECTTYPE 58 // ObjectType
#define UA_NS0ID_FOLDERTYPE 61 // ObjectType
#define UA_NS0ID_BASEVARIABLETYPE 62 // VariableType
#define UA_NS0ID_BASEDATAVARIABLETYPE 63 // VariableType
#define UA_NS0ID_PROPERTYTYPE 68 // VariableType
#define UA_NS0ID_DATATYPEDESCRIPTIONTYPE 69 // VariableType
#define UA_NS0ID_DATATYPEDICTIONARYTYPE 72 // VariableType
#define UA_NS0ID_DATATYPESYSTEMTYPE 75 // ObjectType
#define UA_NS0ID_DATATYPEENCODINGTYPE 76 // ObjectType
#define UA_NS0ID_MODELLINGRULETYPE 77 // ObjectType
#define UA_NS0ID_MODELLINGRULE_MANDATORY 78 // Object
#define UA_NS0ID_MODELLINGRULE_MANDATORYSHARED 79 // Object
#define UA_NS0ID_MODELLINGRULE_OPTIONAL 80 // Object
#define UA_NS0ID_MODELLINGRULE_EXPOSESITSARRAY 83 // Object
#define UA_NS0ID_ROOTFOLDER 84 // Object
#define UA_NS0ID_OBJECTSFOLDER 85 // Object
#define UA_NS0ID_TYPESFOLDER 86 // Object
#define UA_NS0ID_VIEWSFOLDER 87 // Object
#define UA_NS0ID_OBJECTTYPESFOLDER 88 // Object
#define UA_NS0ID_VARIABLETYPESFOLDER 89 // Object
#define UA_NS0ID_DATATYPESFOLDER 90 // Object
#define UA_NS0ID_REFERENCETYPESFOLDER 91 // Object
#define UA_NS0ID_XMLSCHEMA_TYPESYSTEM 92 // Object
#define UA_NS0ID_OPCBINARYSCHEMA_TYPESYSTEM 93 // Object
#define UA_NS0ID_MODELLINGRULE_MANDATORY_NAMINGRULE 112 // Variable
#define UA_NS0ID_MODELLINGRULE_OPTIONAL_NAMINGRULE 113 // Variable
#define UA_NS0ID_MODELLINGRULE_EXPOSESITSARRAY_NAMINGRULE 114 // Variable
#define UA_NS0ID_MODELLINGRULE_MANDATORYSHARED_NAMINGRULE 116 // Variable
#define UA_NS0ID_HASSUBSTATEMACHINE 117 // ReferenceType
#define UA_NS0ID_NAMINGRULETYPE 120 // DataType
#define UA_NS0ID_IDTYPE 256 // DataType
#define UA_NS0ID_NODECLASS 257 // DataType
#define UA_NS0ID_NODE 258 // DataType
#define UA_NS0ID_OBJECTNODE 261 // DataType
#define UA_NS0ID_OBJECTTYPENODE 264 // DataType
#define UA_NS0ID_VARIABLENODE 267 // DataType
#define UA_NS0ID_VARIABLETYPENODE 270 // DataType
#define UA_NS0ID_REFERENCETYPENODE 273 // DataType
#define UA_NS0ID_METHODNODE 276 // DataType
#define UA_NS0ID_VIEWNODE 279 // DataType
#define UA_NS0ID_DATATYPENODE 282 // DataType
#define UA_NS0ID_REFERENCENODE 285 // DataType
#define UA_NS0ID_INTEGERID 288 // DataType
#define UA_NS0ID_COUNTER 289 // DataType
#define UA_NS0ID_DURATION 290 // DataType
#define UA_NS0ID_NUMERICRANGE 291 // DataType
#define UA_NS0ID_TIME 292 // DataType
#define UA_NS0ID_DATE 293 // DataType
#define UA_NS0ID_UTCTIME 294 // DataType
#define UA_NS0ID_LOCALEID 295 // DataType
#define UA_NS0ID_ARGUMENT 296 // DataType
#define UA_NS0ID_STATUSRESULT 299 // DataType
#define UA_NS0ID_MESSAGESECURITYMODE 302 // DataType
#define UA_NS0ID_USERTOKENTYPE 303 // DataType
#define UA_NS0ID_USERTOKENPOLICY 304 // DataType
#define UA_NS0ID_APPLICATIONTYPE 307 // DataType
#define UA_NS0ID_APPLICATIONDESCRIPTION 308 // DataType
#define UA_NS0ID_APPLICATIONINSTANCECERTIFICATE 311 // DataType
#define UA_NS0ID_ENDPOINTDESCRIPTION 312 // DataType
#define UA_NS0ID_SECURITYTOKENREQUESTTYPE 315 // DataType
#define UA_NS0ID_USERIDENTITYTOKEN 316 // DataType
#define UA_NS0ID_ANONYMOUSIDENTITYTOKEN 319 // DataType
#define UA_NS0ID_USERNAMEIDENTITYTOKEN 322 // DataType
#define UA_NS0ID_X509IDENTITYTOKEN 325 // DataType
#define UA_NS0ID_ENDPOINTCONFIGURATION 331 // DataType
#define UA_NS0ID_COMPLIANCELEVEL 334 // DataType
#define UA_NS0ID_SUPPORTEDPROFILE 335 // DataType
#define UA_NS0ID_BUILDINFO 338 // DataType
#define UA_NS0ID_SOFTWARECERTIFICATE 341 // DataType
#define UA_NS0ID_SIGNEDSOFTWARECERTIFICATE 344 // DataType
#define UA_NS0ID_ATTRIBUTEWRITEMASK 347 // DataType
#define UA_NS0ID_NODEATTRIBUTESMASK 348 // DataType
#define UA_NS0ID_NODEATTRIBUTES 349 // DataType
#define UA_NS0ID_OBJECTATTRIBUTES 352 // DataType
#define UA_NS0ID_VARIABLEATTRIBUTES 355 // DataType
#define UA_NS0ID_METHODATTRIBUTES 358 // DataType
#define UA_NS0ID_OBJECTTYPEATTRIBUTES 361 // DataType
#define UA_NS0ID_VARIABLETYPEATTRIBUTES 364 // DataType
#define UA_NS0ID_REFERENCETYPEATTRIBUTES 367 // DataType
#define UA_NS0ID_DATATYPEATTRIBUTES 370 // DataType
#define UA_NS0ID_VIEWATTRIBUTES 373 // DataType
#define UA_NS0ID_ADDNODESITEM 376 // DataType
#define UA_NS0ID_ADDREFERENCESITEM 379 // DataType
#define UA_NS0ID_DELETENODESITEM 382 // DataType
#define UA_NS0ID_DELETEREFERENCESITEM 385 // DataType
#define UA_NS0ID_SESSIONAUTHENTICATIONTOKEN 388 // DataType
#define UA_NS0ID_REQUESTHEADER 389 // DataType
#define UA_NS0ID_RESPONSEHEADER 392 // DataType
#define UA_NS0ID_SERVICEFAULT 395 // DataType
#define UA_NS0ID_FINDSERVERSREQUEST 420 // DataType
#define UA_NS0ID_FINDSERVERSRESPONSE 423 // DataType
#define UA_NS0ID_GETENDPOINTSREQUEST 426 // DataType
#define UA_NS0ID_GETENDPOINTSRESPONSE 429 // DataType
#define UA_NS0ID_REGISTEREDSERVER 432 // DataType
#define UA_NS0ID_REGISTERSERVERREQUEST 435 // DataType
#define UA_NS0ID_REGISTERSERVERRESPONSE 438 // DataType
#define UA_NS0ID_CHANNELSECURITYTOKEN 441 // DataType
#define UA_NS0ID_OPENSECURECHANNELREQUEST 444 // DataType
#define UA_NS0ID_OPENSECURECHANNELRESPONSE 447 // DataType
#define UA_NS0ID_CLOSESECURECHANNELREQUEST 450 // DataType
#define UA_NS0ID_CLOSESECURECHANNELRESPONSE 453 // DataType
#define UA_NS0ID_SIGNATUREDATA 456 // DataType
#define UA_NS0ID_CREATESESSIONREQUEST 459 // DataType
#define UA_NS0ID_CREATESESSIONRESPONSE 462 // DataType
#define UA_NS0ID_ACTIVATESESSIONREQUEST 465 // DataType
#define UA_NS0ID_ACTIVATESESSIONRESPONSE 468 // DataType
#define UA_NS0ID_CLOSESESSIONREQUEST 471 // DataType
#define UA_NS0ID_CLOSESESSIONRESPONSE 474 // DataType
#define UA_NS0ID_CANCELREQUEST 477 // DataType
#define UA_NS0ID_CANCELRESPONSE 480 // DataType
#define UA_NS0ID_ADDNODESRESULT 483 // DataType
#define UA_NS0ID_ADDNODESREQUEST 486 // DataType
#define UA_NS0ID_ADDNODESRESPONSE 489 // DataType
#define UA_NS0ID_ADDREFERENCESREQUEST 492 // DataType
#define UA_NS0ID_ADDREFERENCESRESPONSE 495 // DataType
#define UA_NS0ID_DELETENODESREQUEST 498 // DataType
#define UA_NS0ID_DELETENODESRESPONSE 501 // DataType
#define UA_NS0ID_DELETEREFERENCESREQUEST 504 // DataType
#define UA_NS0ID_DELETEREFERENCESRESPONSE 507 // DataType
#define UA_NS0ID_BROWSEDIRECTION 510 // DataType
#define UA_NS0ID_VIEWDESCRIPTION 511 // DataType
#define UA_NS0ID_BROWSEDESCRIPTION 514 // DataType
#define UA_NS0ID_BROWSERESULTMASK 517 // DataType
#define UA_NS0ID_REFERENCEDESCRIPTION 518 // DataType
#define UA_NS0ID_CONTINUATIONPOINT 521 // DataType
#define UA_NS0ID_BROWSERESULT 522 // DataType
#define UA_NS0ID_BROWSEREQUEST 525 // DataType
#define UA_NS0ID_BROWSERESPONSE 528 // DataType
#define UA_NS0ID_BROWSENEXTREQUEST 531 // DataType
#define UA_NS0ID_BROWSENEXTRESPONSE 534 // DataType
#define UA_NS0ID_RELATIVEPATHELEMENT 537 // DataType
#define UA_NS0ID_RELATIVEPATH 540 // DataType
#define UA_NS0ID_BROWSEPATH 543 // DataType
#define UA_NS0ID_BROWSEPATHTARGET 546 // DataType
#define UA_NS0ID_BROWSEPATHRESULT 549 // DataType
#define UA_NS0ID_TRANSLATEBROWSEPATHSTONODEIDSREQUEST 552 // DataType
#define UA_NS0ID_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE 555 // DataType
#define UA_NS0ID_REGISTERNODESREQUEST 558 // DataType
#define UA_NS0ID_REGISTERNODESRESPONSE 561 // DataType
#define UA_NS0ID_UNREGISTERNODESREQUEST 564 // DataType
#define UA_NS0ID_UNREGISTERNODESRESPONSE 567 // DataType
#define UA_NS0ID_QUERYDATADESCRIPTION 570 // DataType
#define UA_NS0ID_NODETYPEDESCRIPTION 573 // DataType
#define UA_NS0ID_FILTEROPERATOR 576 // DataType
#define UA_NS0ID_QUERYDATASET 577 // DataType
#define UA_NS0ID_NODEREFERENCE 580 // DataType
#define UA_NS0ID_CONTENTFILTERELEMENT 583 // DataType
#define UA_NS0ID_CONTENTFILTER 586 // DataType
#define UA_NS0ID_FILTEROPERAND 589 // DataType
#define UA_NS0ID_ELEMENTOPERAND 592 // DataType
#define UA_NS0ID_LITERALOPERAND 595 // DataType
#define UA_NS0ID_ATTRIBUTEOPERAND 598 // DataType
#define UA_NS0ID_SIMPLEATTRIBUTEOPERAND 601 // DataType
#define UA_NS0ID_CONTENTFILTERELEMENTRESULT 604 // DataType
#define UA_NS0ID_CONTENTFILTERRESULT 607 // DataType
#define UA_NS0ID_PARSINGRESULT 610 // DataType
#define UA_NS0ID_QUERYFIRSTREQUEST 613 // DataType
#define UA_NS0ID_QUERYFIRSTRESPONSE 616 // DataType
#define UA_NS0ID_QUERYNEXTREQUEST 619 // DataType
#define UA_NS0ID_QUERYNEXTRESPONSE 622 // DataType
#define UA_NS0ID_TIMESTAMPSTORETURN 625 // DataType
#define UA_NS0ID_READVALUEID 626 // DataType
#define UA_NS0ID_READREQUEST 629 // DataType
#define UA_NS0ID_READRESPONSE 632 // DataType
#define UA_NS0ID_HISTORYREADVALUEID 635 // DataType
#define UA_NS0ID_HISTORYREADRESULT 638 // DataType
#define UA_NS0ID_HISTORYREADDETAILS 641 // DataType
#define UA_NS0ID_READEVENTDETAILS 644 // DataType
#define UA_NS0ID_READRAWMODIFIEDDETAILS 647 // DataType
#define UA_NS0ID_READPROCESSEDDETAILS 650 // DataType
#define UA_NS0ID_READATTIMEDETAILS 653 // DataType
#define UA_NS0ID_HISTORYDATA 656 // DataType
#define UA_NS0ID_HISTORYEVENT 659 // DataType
#define UA_NS0ID_HISTORYREADREQUEST 662 // DataType
#define UA_NS0ID_HISTORYREADRESPONSE 665 // DataType
#define UA_NS0ID_WRITEVALUE 668 // DataType
#define UA_NS0ID_WRITEREQUEST 671 // DataType
#define UA_NS0ID_WRITERESPONSE 674 // DataType
#define UA_NS0ID_HISTORYUPDATEDETAILS 677 // DataType
#define UA_NS0ID_UPDATEDATADETAILS 680 // DataType
#define UA_NS0ID_UPDATEEVENTDETAILS 683 // DataType
#define UA_NS0ID_DELETERAWMODIFIEDDETAILS 686 // DataType
#define UA_NS0ID_DELETEATTIMEDETAILS 689 // DataType
#define UA_NS0ID_DELETEEVENTDETAILS 692 // DataType
#define UA_NS0ID_HISTORYUPDATERESULT 695 // DataType
#define UA_NS0ID_HISTORYUPDATEREQUEST 698 // DataType
#define UA_NS0ID_HISTORYUPDATERESPONSE 701 // DataType
#define UA_NS0ID_CALLMETHODREQUEST 704 // DataType
#define UA_NS0ID_CALLMETHODRESULT 707 // DataType
#define UA_NS0ID_CALLREQUEST 710 // DataType
#define UA_NS0ID_CALLRESPONSE 713 // DataType
#define UA_NS0ID_MONITORINGMODE 716 // DataType
#define UA_NS0ID_DATACHANGETRIGGER 717 // DataType
#define UA_NS0ID_DEADBANDTYPE 718 // DataType
#define UA_NS0ID_MONITORINGFILTER 719 // DataType
#define UA_NS0ID_DATACHANGEFILTER 722 // DataType
#define UA_NS0ID_EVENTFILTER 725 // DataType
#define UA_NS0ID_AGGREGATEFILTER 728 // DataType
#define UA_NS0ID_MONITORINGFILTERRESULT 731 // DataType
#define UA_NS0ID_EVENTFILTERRESULT 734 // DataType
#define UA_NS0ID_AGGREGATEFILTERRESULT 737 // DataType
#define UA_NS0ID_MONITORINGPARAMETERS 740 // DataType
#define UA_NS0ID_MONITOREDITEMCREATEREQUEST 743 // DataType
#define UA_NS0ID_MONITOREDITEMCREATERESULT 746 // DataType
#define UA_NS0ID_CREATEMONITOREDITEMSREQUEST 749 // DataType
#define UA_NS0ID_CREATEMONITOREDITEMSRESPONSE 752 // DataType
#define UA_NS0ID_MONITOREDITEMMODIFYREQUEST 755 // DataType
#define UA_NS0ID_MONITOREDITEMMODIFYRESULT 758 // DataType
#define UA_NS0ID_MODIFYMONITOREDITEMSREQUEST 761 // DataType
#define UA_NS0ID_MODIFYMONITOREDITEMSRESPONSE 764 // DataType
#define UA_NS0ID_SETMONITORINGMODEREQUEST 767 // DataType
#define UA_NS0ID_SETMONITORINGMODERESPONSE 770 // DataType
#define UA_NS0ID_SETTRIGGERINGREQUEST 773 // DataType
#define UA_NS0ID_SETTRIGGERINGRESPONSE 776 // DataType
#define UA_NS0ID_DELETEMONITOREDITEMSREQUEST 779 // DataType
#define UA_NS0ID_DELETEMONITOREDITEMSRESPONSE 782 // DataType
#define UA_NS0ID_CREATESUBSCRIPTIONREQUEST 785 // DataType
#define UA_NS0ID_CREATESUBSCRIPTIONRESPONSE 788 // DataType
#define UA_NS0ID_MODIFYSUBSCRIPTIONREQUEST 791 // DataType
#define UA_NS0ID_MODIFYSUBSCRIPTIONRESPONSE 794 // DataType
#define UA_NS0ID_SETPUBLISHINGMODEREQUEST 797 // DataType
#define UA_NS0ID_SETPUBLISHINGMODERESPONSE 800 // DataType
#define UA_NS0ID_NOTIFICATIONMESSAGE 803 // DataType
#define UA_NS0ID_MONITOREDITEMNOTIFICATION 806 // DataType
#define UA_NS0ID_DATACHANGENOTIFICATION 809 // DataType
#define UA_NS0ID_STATUSCHANGENOTIFICATION 818 // DataType
#define UA_NS0ID_SUBSCRIPTIONACKNOWLEDGEMENT 821 // DataType
#define UA_NS0ID_PUBLISHREQUEST 824 // DataType
#define UA_NS0ID_PUBLISHRESPONSE 827 // DataType
#define UA_NS0ID_REPUBLISHREQUEST 830 // DataType
#define UA_NS0ID_REPUBLISHRESPONSE 833 // DataType
#define UA_NS0ID_TRANSFERRESULT 836 // DataType
#define UA_NS0ID_TRANSFERSUBSCRIPTIONSREQUEST 839 // DataType
#define UA_NS0ID_TRANSFERSUBSCRIPTIONSRESPONSE 842 // DataType
#define UA_NS0ID_DELETESUBSCRIPTIONSREQUEST 845 // DataType
#define UA_NS0ID_DELETESUBSCRIPTIONSRESPONSE 848 // DataType
#define UA_NS0ID_REDUNDANCYSUPPORT 851 // DataType
#define UA_NS0ID_SERVERSTATE 852 // DataType
#define UA_NS0ID_REDUNDANTSERVERDATATYPE 853 // DataType
#define UA_NS0ID_SAMPLINGINTERVALDIAGNOSTICSDATATYPE 856 // DataType
#define UA_NS0ID_SERVERDIAGNOSTICSSUMMARYDATATYPE 859 // DataType
#define UA_NS0ID_SERVERSTATUSDATATYPE 862 // DataType
#define UA_NS0ID_SESSIONDIAGNOSTICSDATATYPE 865 // DataType
#define UA_NS0ID_SESSIONSECURITYDIAGNOSTICSDATATYPE 868 // DataType
#define UA_NS0ID_SERVICECOUNTERDATATYPE 871 // DataType
#define UA_NS0ID_SUBSCRIPTIONDIAGNOSTICSDATATYPE 874 // DataType
#define UA_NS0ID_MODELCHANGESTRUCTUREDATATYPE 877 // DataType
#define UA_NS0ID_RANGE 884 // DataType
#define UA_NS0ID_EUINFORMATION 887 // DataType
#define UA_NS0ID_EXCEPTIONDEVIATIONFORMAT 890 // DataType
#define UA_NS0ID_ANNOTATION 891 // DataType
#define UA_NS0ID_PROGRAMDIAGNOSTICDATATYPE 894 // DataType
#define UA_NS0ID_SEMANTICCHANGESTRUCTUREDATATYPE 897 // DataType
#define UA_NS0ID_EVENTNOTIFICATIONLIST 914 // DataType
#define UA_NS0ID_EVENTFIELDLIST 917 // DataType
#define UA_NS0ID_HISTORYEVENTFIELDLIST 920 // DataType
#define UA_NS0ID_HISTORYUPDATEEVENTRESULT 929 // DataType
#define UA_NS0ID_ISSUEDIDENTITYTOKEN 938 // DataType
#define UA_NS0ID_NOTIFICATIONDATA 945 // DataType
#define UA_NS0ID_AGGREGATECONFIGURATION 948 // DataType
#define UA_NS0ID_IMAGEBMP 2000 // DataType
#define UA_NS0ID_IMAGEGIF 2001 // DataType
#define UA_NS0ID_IMAGEJPG 2002 // DataType
#define UA_NS0ID_IMAGEPNG 2003 // DataType
#define UA_NS0ID_SERVERTYPE 2004 // ObjectType
#define UA_NS0ID_SERVERCAPABILITIESTYPE 2013 // ObjectType
#define UA_NS0ID_SERVERDIAGNOSTICSTYPE 2020 // ObjectType
#define UA_NS0ID_SESSIONSDIAGNOSTICSSUMMARYTYPE 2026 // ObjectType
#define UA_NS0ID_SESSIONDIAGNOSTICSOBJECTTYPE 2029 // ObjectType
#define UA_NS0ID_VENDORSERVERINFOTYPE 2033 // ObjectType
#define UA_NS0ID_SERVERREDUNDANCYTYPE 2034 // ObjectType
#define UA_NS0ID_TRANSPARENTREDUNDANCYTYPE 2036 // ObjectType
#define UA_NS0ID_NONTRANSPARENTREDUNDANCYTYPE 2039 // ObjectType
#define UA_NS0ID_BASEEVENTTYPE 2041 // ObjectType
#define UA_NS0ID_AUDITEVENTTYPE 2052 // ObjectType
#define UA_NS0ID_AUDITSECURITYEVENTTYPE 2058 // ObjectType
#define UA_NS0ID_AUDITCHANNELEVENTTYPE 2059 // ObjectType
#define UA_NS0ID_AUDITOPENSECURECHANNELEVENTTYPE 2060 // ObjectType
#define UA_NS0ID_AUDITSESSIONEVENTTYPE 2069 // ObjectType
#define UA_NS0ID_AUDITCREATESESSIONEVENTTYPE 2071 // ObjectType
#define UA_NS0ID_AUDITACTIVATESESSIONEVENTTYPE 2075 // ObjectType
#define UA_NS0ID_AUDITCANCELEVENTTYPE 2078 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEEVENTTYPE 2080 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEDATAMISMATCHEVENTTYPE 2082 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEEXPIREDEVENTTYPE 2085 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEINVALIDEVENTTYPE 2086 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEUNTRUSTEDEVENTTYPE 2087 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEREVOKEDEVENTTYPE 2088 // ObjectType
#define UA_NS0ID_AUDITCERTIFICATEMISMATCHEVENTTYPE 2089 // ObjectType
#define UA_NS0ID_AUDITNODEMANAGEMENTEVENTTYPE 2090 // ObjectType
#define UA_NS0ID_AUDITADDNODESEVENTTYPE 2091 // ObjectType
#define UA_NS0ID_AUDITDELETENODESEVENTTYPE 2093 // ObjectType
#define UA_NS0ID_AUDITADDREFERENCESEVENTTYPE 2095 // ObjectType
#define UA_NS0ID_AUDITDELETEREFERENCESEVENTTYPE 2097 // ObjectType
#define UA_NS0ID_AUDITUPDATEEVENTTYPE 2099 // ObjectType
#define UA_NS0ID_AUDITWRITEUPDATEEVENTTYPE 2100 // ObjectType
#define UA_NS0ID_AUDITHISTORYUPDATEEVENTTYPE 2104 // ObjectType
#define UA_NS0ID_AUDITUPDATEMETHODEVENTTYPE 2127 // ObjectType
#define UA_NS0ID_SYSTEMEVENTTYPE 2130 // ObjectType
#define UA_NS0ID_DEVICEFAILUREEVENTTYPE 2131 // ObjectType
#define UA_NS0ID_BASEMODELCHANGEEVENTTYPE 2132 // ObjectType
#define UA_NS0ID_GENERALMODELCHANGEEVENTTYPE 2133 // ObjectType
#define UA_NS0ID_SERVERVENDORCAPABILITYTYPE 2137 // VariableType
#define UA_NS0ID_SERVERSTATUSTYPE 2138 // VariableType
#define UA_NS0ID_SERVERDIAGNOSTICSSUMMARYTYPE 2150 // VariableType
#define UA_NS0ID_SAMPLINGINTERVALDIAGNOSTICSARRAYTYPE 2164 // VariableType
#define UA_NS0ID_SAMPLINGINTERVALDIAGNOSTICSTYPE 2165 // VariableType
#define UA_NS0ID_SUBSCRIPTIONDIAGNOSTICSARRAYTYPE 2171 // VariableType
#define UA_NS0ID_SUBSCRIPTIONDIAGNOSTICSTYPE 2172 // VariableType
#define UA_NS0ID_SESSIONDIAGNOSTICSARRAYTYPE 2196 // VariableType
#define UA_NS0ID_SESSIONDIAGNOSTICSVARIABLETYPE 2197 // VariableType
#define UA_NS0ID_SESSIONSECURITYDIAGNOSTICSARRAYTYPE 2243 // VariableType
#define UA_NS0ID_SESSIONSECURITYDIAGNOSTICSTYPE 2244 // VariableType
#define UA_NS0ID_SERVER 2253 // Object
#define UA_NS0ID_SERVER_SERVERARRAY 2254 // Variable
#define UA_NS0ID_SERVER_NAMESPACEARRAY 2255 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS 2256 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_STARTTIME 2257 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME 2258 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_STATE 2259 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO 2260 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTNAME 2261 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTURI 2262 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_MANUFACTURERNAME 2263 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_SOFTWAREVERSION 2264 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDNUMBER 2265 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDDATE 2266 // Variable
#define UA_NS0ID_SERVER_SERVICELEVEL 2267 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES 2268 // Object
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_SERVERPROFILEARRAY 2269 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_LOCALEIDARRAY 2271 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MINSUPPORTEDSAMPLERATE 2272 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS 2274 // Object
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY 2275 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_SERVERVIEWCOUNT 2276 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_CURRENTSESSIONCOUNT 2277 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_CUMULATEDSESSIONCOUNT 2278 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_SECURITYREJECTEDSESSIONCOUNT 2279 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_SESSIONTIMEOUTCOUNT 2281 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_SESSIONABORTCOUNT 2282 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_PUBLISHINGINTERVALCOUNT 2284 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_CURRENTSUBSCRIPTIONCOUNT 2285 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_CUMULATEDSUBSCRIPTIONCOUNT 2286 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_SECURITYREJECTEDREQUESTSCOUNT 2287 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_REJECTEDREQUESTSCOUNT 2288 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SAMPLINGINTERVALDIAGNOSTICSARRAY 2289 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SUBSCRIPTIONDIAGNOSTICSARRAY 2290 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_ENABLEDFLAG 2294 // Variable
#define UA_NS0ID_SERVER_VENDORSERVERINFO 2295 // Object
#define UA_NS0ID_SERVER_SERVERREDUNDANCY 2296 // Object
#define UA_NS0ID_STATEMACHINETYPE 2299 // ObjectType
#define UA_NS0ID_STATETYPE 2307 // ObjectType
#define UA_NS0ID_INITIALSTATETYPE 2309 // ObjectType
#define UA_NS0ID_TRANSITIONTYPE 2310 // ObjectType
#define UA_NS0ID_TRANSITIONEVENTTYPE 2311 // ObjectType
#define UA_NS0ID_AUDITUPDATESTATEEVENTTYPE 2315 // ObjectType
#define UA_NS0ID_HISTORICALDATACONFIGURATIONTYPE 2318 // ObjectType
#define UA_NS0ID_HISTORYSERVERCAPABILITIESTYPE 2330 // ObjectType
#define UA_NS0ID_AGGREGATEFUNCTIONTYPE 2340 // ObjectType
#define UA_NS0ID_AGGREGATEFUNCTION_INTERPOLATIVE 2341 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_AVERAGE 2342 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_TIMEAVERAGE 2343 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_TOTAL 2344 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MINIMUM 2346 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MAXIMUM 2347 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MINIMUMACTUALTIME 2348 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MAXIMUMACTUALTIME 2349 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_RANGE 2350 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_ANNOTATIONCOUNT 2351 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_COUNT 2352 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_NUMBEROFTRANSITIONS 2355 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_START 2357 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_END 2358 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_DELTA 2359 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_DURATIONGOOD 2360 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_DURATIONBAD 2361 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_PERCENTGOOD 2362 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_PERCENTBAD 2363 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_WORSTQUALITY 2364 // Object
#define UA_NS0ID_DATAITEMTYPE 2365 // VariableType
#define UA_NS0ID_ANALOGITEMTYPE 2368 // VariableType
#define UA_NS0ID_DISCRETEITEMTYPE 2372 // VariableType
#define UA_NS0ID_TWOSTATEDISCRETETYPE 2373 // VariableType
#define UA_NS0ID_MULTISTATEDISCRETETYPE 2376 // VariableType
#define UA_NS0ID_PROGRAMTRANSITIONEVENTTYPE 2378 // ObjectType
#define UA_NS0ID_PROGRAMDIAGNOSTICTYPE 2380 // VariableType
#define UA_NS0ID_PROGRAMSTATEMACHINETYPE 2391 // ObjectType
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXBROWSECONTINUATIONPOINTS 2735 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXQUERYCONTINUATIONPOINTS 2736 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXHISTORYCONTINUATIONPOINTS 2737 // Variable
#define UA_NS0ID_SEMANTICCHANGEEVENTTYPE 2738 // ObjectType
#define UA_NS0ID_AUDITURLMISMATCHEVENTTYPE 2748 // ObjectType
#define UA_NS0ID_STATEVARIABLETYPE 2755 // VariableType
#define UA_NS0ID_FINITESTATEVARIABLETYPE 2760 // VariableType
#define UA_NS0ID_TRANSITIONVARIABLETYPE 2762 // VariableType
#define UA_NS0ID_FINITETRANSITIONVARIABLETYPE 2767 // VariableType
#define UA_NS0ID_FINITESTATEMACHINETYPE 2771 // ObjectType
#define UA_NS0ID_CONDITIONTYPE 2782 // ObjectType
#define UA_NS0ID_REFRESHSTARTEVENTTYPE 2787 // ObjectType
#define UA_NS0ID_REFRESHENDEVENTTYPE 2788 // ObjectType
#define UA_NS0ID_REFRESHREQUIREDEVENTTYPE 2789 // ObjectType
#define UA_NS0ID_AUDITCONDITIONEVENTTYPE 2790 // ObjectType
#define UA_NS0ID_AUDITCONDITIONENABLEEVENTTYPE 2803 // ObjectType
#define UA_NS0ID_AUDITCONDITIONCOMMENTEVENTTYPE 2829 // ObjectType
#define UA_NS0ID_DIALOGCONDITIONTYPE 2830 // ObjectType
#define UA_NS0ID_ACKNOWLEDGEABLECONDITIONTYPE 2881 // ObjectType
#define UA_NS0ID_ALARMCONDITIONTYPE 2915 // ObjectType
#define UA_NS0ID_SHELVEDSTATEMACHINETYPE 2929 // ObjectType
#define UA_NS0ID_LIMITALARMTYPE 2955 // ObjectType
#define UA_NS0ID_SERVER_SERVERSTATUS_SECONDSTILLSHUTDOWN 2992 // Variable
#define UA_NS0ID_SERVER_SERVERSTATUS_SHUTDOWNREASON 2993 // Variable
#define UA_NS0ID_SERVER_AUDITING 2994 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MODELLINGRULES 2996 // Object
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_AGGREGATEFUNCTIONS 2997 // Object
#define UA_NS0ID_AUDITHISTORYEVENTUPDATEEVENTTYPE 2999 // ObjectType
#define UA_NS0ID_AUDITHISTORYVALUEUPDATEEVENTTYPE 3006 // ObjectType
#define UA_NS0ID_AUDITHISTORYDELETEEVENTTYPE 3012 // ObjectType
#define UA_NS0ID_AUDITHISTORYRAWMODIFYDELETEEVENTTYPE 3014 // ObjectType
#define UA_NS0ID_AUDITHISTORYATTIMEDELETEEVENTTYPE 3019 // ObjectType
#define UA_NS0ID_AUDITHISTORYEVENTDELETEEVENTTYPE 3022 // ObjectType
#define UA_NS0ID_EVENTQUEUEOVERFLOWEVENTTYPE 3035 // ObjectType
#define UA_NS0ID_EVENTTYPESFOLDER 3048 // Object
#define UA_NS0ID_BUILDINFOTYPE 3051 // VariableType
#define UA_NS0ID_DEFAULTBINARY 3062 // Object
#define UA_NS0ID_DEFAULTXML 3063 // Object
#define UA_NS0ID_ALWAYSGENERATESEVENT 3065 // ReferenceType
#define UA_NS0ID_ICON 3067 // Variable
#define UA_NS0ID_NODEVERSION 3068 // Variable
#define UA_NS0ID_LOCALTIME 3069 // Variable
#define UA_NS0ID_ALLOWNULLS 3070 // Variable
#define UA_NS0ID_ENUMVALUES 3071 // Variable
#define UA_NS0ID_INPUTARGUMENTS 3072 // Variable
#define UA_NS0ID_OUTPUTARGUMENTS 3073 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_SOFTWARECERTIFICATES 3704 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SERVERDIAGNOSTICSSUMMARY_REJECTEDSESSIONCOUNT 3705 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SESSIONSDIAGNOSTICSSUMMARY 3706 // Object
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SESSIONSDIAGNOSTICSSUMMARY_SESSIONDIAGNOSTICSARRAY 3707 // Variable
#define UA_NS0ID_SERVER_SERVERDIAGNOSTICS_SESSIONSDIAGNOSTICSSUMMARY_SESSIONSECURITYDIAGNOSTICSARRAY 3708 // Variable
#define UA_NS0ID_SERVER_SERVERREDUNDANCY_REDUNDANCYSUPPORT 3709 // Variable
#define UA_NS0ID_PROGRAMTRANSITIONAUDITEVENTTYPE 3806 // ObjectType
#define UA_NS0ID_ADDCOMMENTMETHODTYPE 3863 // Method
#define UA_NS0ID_TIMEDSHELVEMETHODTYPE 6102 // Method
#define UA_NS0ID_ENUMVALUETYPE 7594 // DataType
#define UA_NS0ID_MESSAGESECURITYMODE_ENUMSTRINGS 7595 // Variable
#define UA_NS0ID_COMPLIANCELEVEL_ENUMSTRINGS 7599 // Variable
#define UA_NS0ID_BROWSEDIRECTION_ENUMSTRINGS 7603 // Variable
#define UA_NS0ID_FILTEROPERATOR_ENUMSTRINGS 7605 // Variable
#define UA_NS0ID_TIMESTAMPSTORETURN_ENUMSTRINGS 7606 // Variable
#define UA_NS0ID_MONITORINGMODE_ENUMSTRINGS 7608 // Variable
#define UA_NS0ID_DATACHANGETRIGGER_ENUMSTRINGS 7609 // Variable
#define UA_NS0ID_REDUNDANCYSUPPORT_ENUMSTRINGS 7611 // Variable
#define UA_NS0ID_SERVERSTATE_ENUMSTRINGS 7612 // Variable
#define UA_NS0ID_EXCEPTIONDEVIATIONFORMAT_ENUMSTRINGS 7614 // Variable
#define UA_NS0ID_TIMEZONEDATATYPE 8912 // DataType
#define UA_NS0ID_LOCKTYPE 8921 // ObjectType
#define UA_NS0ID_SERVERLOCK 8924 // Object
#define UA_NS0ID_SERVERLOCK_LOCK 8925 // Method
#define UA_NS0ID_SERVERLOCK_UNLOCK 8926 // Method
#define UA_NS0ID_AUDITCONDITIONRESPONDEVENTTYPE 8927 // ObjectType
#define UA_NS0ID_AUDITCONDITIONACKNOWLEDGEEVENTTYPE 8944 // ObjectType
#define UA_NS0ID_AUDITCONDITIONCONFIRMEVENTTYPE 8961 // ObjectType
#define UA_NS0ID_TWOSTATEVARIABLETYPE 8995 // VariableType
#define UA_NS0ID_CONDITIONVARIABLETYPE 9002 // VariableType
#define UA_NS0ID_HASTRUESUBSTATE 9004 // ReferenceType
#define UA_NS0ID_HASFALSESUBSTATE 9005 // ReferenceType
#define UA_NS0ID_HASCONDITION 9006 // ReferenceType
#define UA_NS0ID_CONDITIONREFRESHMETHODTYPE 9007 // Method
#define UA_NS0ID_DIALOGRESPONSEMETHODTYPE 9031 // Method
#define UA_NS0ID_EXCLUSIVELIMITSTATEMACHINETYPE 9318 // ObjectType
#define UA_NS0ID_EXCLUSIVELIMITALARMTYPE 9341 // ObjectType
#define UA_NS0ID_EXCLUSIVELEVELALARMTYPE 9482 // ObjectType
#define UA_NS0ID_EXCLUSIVERATEOFCHANGEALARMTYPE 9623 // ObjectType
#define UA_NS0ID_EXCLUSIVEDEVIATIONALARMTYPE 9764 // ObjectType
#define UA_NS0ID_NONEXCLUSIVELIMITALARMTYPE 9906 // ObjectType
#define UA_NS0ID_NONEXCLUSIVELEVELALARMTYPE 10060 // ObjectType
#define UA_NS0ID_NONEXCLUSIVERATEOFCHANGEALARMTYPE 10214 // ObjectType
#define UA_NS0ID_NONEXCLUSIVEDEVIATIONALARMTYPE 10368 // ObjectType
#define UA_NS0ID_DISCRETEALARMTYPE 10523 // ObjectType
#define UA_NS0ID_OFFNORMALALARMTYPE 10637 // ObjectType
#define UA_NS0ID_TRIPALARMTYPE 10751 // ObjectType
#define UA_NS0ID_AUDITCONDITIONSHELVINGEVENTTYPE 11093 // ObjectType
#define UA_NS0ID_BASECONDITIONCLASSTYPE 11163 // ObjectType
#define UA_NS0ID_PROCESSCONDITIONCLASSTYPE 11164 // ObjectType
#define UA_NS0ID_MAINTENANCECONDITIONCLASSTYPE 11165 // ObjectType
#define UA_NS0ID_SYSTEMCONDITIONCLASSTYPE 11166 // ObjectType
#define UA_NS0ID_AGGREGATECONFIGURATIONTYPE 11187 // ObjectType
#define UA_NS0ID_HISTORYSERVERCAPABILITIES 11192 // Object
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_ACCESSHISTORYDATACAPABILITY 11193 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_INSERTDATACAPABILITY 11196 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_REPLACEDATACAPABILITY 11197 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_UPDATEDATACAPABILITY 11198 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_DELETERAWCAPABILITY 11199 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_DELETEATTIMECAPABILITY 11200 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_AGGREGATEFUNCTIONS 11201 // Object
#define UA_NS0ID_HACONFIGURATION 11202 // Object
#define UA_NS0ID_HACONFIGURATION_AGGREGATECONFIGURATION 11203 // Object
#define UA_NS0ID_HACONFIGURATION_AGGREGATECONFIGURATION_TREATUNCERTAINASBAD 11204 // Variable
#define UA_NS0ID_HACONFIGURATION_AGGREGATECONFIGURATION_PERCENTDATABAD 11205 // Variable
#define UA_NS0ID_HACONFIGURATION_AGGREGATECONFIGURATION_PERCENTDATAGOOD 11206 // Variable
#define UA_NS0ID_HACONFIGURATION_AGGREGATECONFIGURATION_USESLOPEDEXTRAPOLATION 11207 // Variable
#define UA_NS0ID_HACONFIGURATION_STEPPED 11208 // Variable
#define UA_NS0ID_HACONFIGURATION_DEFINITION 11209 // Variable
#define UA_NS0ID_HACONFIGURATION_MAXTIMEINTERVAL 11210 // Variable
#define UA_NS0ID_HACONFIGURATION_MINTIMEINTERVAL 11211 // Variable
#define UA_NS0ID_HACONFIGURATION_EXCEPTIONDEVIATION 11212 // Variable
#define UA_NS0ID_HACONFIGURATION_EXCEPTIONDEVIATIONFORMAT 11213 // Variable
#define UA_NS0ID_ANNOTATIONS 11214 // Variable
#define UA_NS0ID_HISTORICALEVENTFILTER 11215 // Variable
#define UA_NS0ID_MODIFICATIONINFO 11216 // DataType
#define UA_NS0ID_HISTORYMODIFIEDDATA 11217 // DataType
#define UA_NS0ID_HISTORYUPDATETYPE 11234 // DataType
#define UA_NS0ID_MULTISTATEVALUEDISCRETETYPE 11238 // VariableType
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_ACCESSHISTORYEVENTSCAPABILITY 11242 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_MAXRETURNDATAVALUES 11273 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_MAXRETURNEVENTVALUES 11274 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_INSERTANNOTATIONCAPABILITY 11275 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_INSERTEVENTCAPABILITY 11281 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_REPLACEEVENTCAPABILITY 11282 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_UPDATEEVENTCAPABILITY 11283 // Variable
#define UA_NS0ID_AGGREGATEFUNCTION_TIMEAVERAGE2 11285 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MINIMUM2 11286 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MAXIMUM2 11287 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_RANGE2 11288 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_WORSTQUALITY2 11292 // Object
#define UA_NS0ID_PERFORMUPDATETYPE 11293 // DataType
#define UA_NS0ID_UPDATESTRUCTUREDATADETAILS 11295 // DataType
#define UA_NS0ID_AGGREGATEFUNCTION_TOTAL2 11304 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MINIMUMACTUALTIME2 11305 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_MAXIMUMACTUALTIME2 11306 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_DURATIONINSTATEZERO 11307 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_DURATIONINSTATENONZERO 11308 // Object
#define UA_NS0ID_SERVER_SERVERREDUNDANCY_CURRENTSERVERID 11312 // Variable
#define UA_NS0ID_SERVER_SERVERREDUNDANCY_REDUNDANTSERVERARRAY 11313 // Variable
#define UA_NS0ID_SERVER_SERVERREDUNDANCY_SERVERURIARRAY 11314 // Variable
#define UA_NS0ID_AGGREGATEFUNCTION_STANDARDDEVIATIONSAMPLE 11426 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_STANDARDDEVIATIONPOPULATION 11427 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_VARIANCESAMPLE 11428 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_VARIANCEPOPULATION 11429 // Object
#define UA_NS0ID_ENUMSTRINGS 11432 // Variable
#define UA_NS0ID_VALUEASTEXT 11433 // Variable
#define UA_NS0ID_PROGRESSEVENTTYPE 11436 // ObjectType
#define UA_NS0ID_SYSTEMSTATUSCHANGEEVENTTYPE 11446 // ObjectType
#define UA_NS0ID_OPTIONSETTYPE 11487 // VariableType
#define UA_NS0ID_SERVER_GETMONITOREDITEMS 11492 // Method
#define UA_NS0ID_SERVER_GETMONITOREDITEMS_INPUTARGUMENTS 11493 // Variable
#define UA_NS0ID_SERVER_GETMONITOREDITEMS_OUTPUTARGUMENTS 11494 // Variable
#define UA_NS0ID_GETMONITOREDITEMSMETHODTYPE 11495 // Method
#define UA_NS0ID_MAXSTRINGLENGTH 11498 // Variable
#define UA_NS0ID_HISTORYSERVERCAPABILITIES_DELETEEVENTCAPABILITY 11502 // Variable
#define UA_NS0ID_HACONFIGURATION_STARTOFARCHIVE 11503 // Variable
#define UA_NS0ID_HACONFIGURATION_STARTOFONLINEARCHIVE 11504 // Variable
#define UA_NS0ID_AGGREGATEFUNCTION_STARTBOUND 11505 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_ENDBOUND 11506 // Object
#define UA_NS0ID_AGGREGATEFUNCTION_DELTABOUNDS 11507 // Object
#define UA_NS0ID_MODELLINGRULE_OPTIONALPLACEHOLDER 11508 // Object
#define UA_NS0ID_MODELLINGRULE_OPTIONALPLACEHOLDER_NAMINGRULE 11509 // Variable
#define UA_NS0ID_MODELLINGRULE_MANDATORYPLACEHOLDER 11510 // Object
#define UA_NS0ID_MODELLINGRULE_MANDATORYPLACEHOLDER_NAMINGRULE 11511 // Variable
#define UA_NS0ID_MAXARRAYLENGTH 11512 // Variable
#define UA_NS0ID_ENGINEERINGUNITS 11513 // Variable
#define UA_NS0ID_OPERATIONLIMITSTYPE 11564 // ObjectType
#define UA_NS0ID_FILETYPE 11575 // ObjectType
#define UA_NS0ID_ADDRESSSPACEFILETYPE 11595 // ObjectType
#define UA_NS0ID_NAMESPACEMETADATATYPE 11616 // ObjectType
#define UA_NS0ID_NAMESPACESTYPE 11645 // ObjectType
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXARRAYLENGTH 11702 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXSTRINGLENGTH 11703 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS 11704 // Object
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERREAD 11705 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERWRITE 11707 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERMETHODCALL 11709 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERBROWSE 11710 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERREGISTERNODES 11711 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERTRANSLATEBROWSEPATHSTONODEIDS 11712 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERNODEMANAGEMENT 11713 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXMONITOREDITEMSPERCALL 11714 // Variable
#define UA_NS0ID_SERVER_NAMESPACES 11715 // Object
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE 11716 // Object
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_SIZE 11717 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_WRITEABLE 11718 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_USERWRITEABLE 11719 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_OPENCOUNT 11720 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_OPEN 11721 // Method
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_OPEN_INPUTARGUMENTS 11722 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_OPEN_OUTPUTARGUMENTS 11723 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_CLOSE 11724 // Method
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_CLOSE_INPUTARGUMENTS 11725 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_READ 11726 // Method
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_READ_INPUTARGUMENTS 11727 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_READ_OUTPUTARGUMENTS 11728 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_WRITE 11729 // Method
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_WRITE_INPUTARGUMENTS 11730 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_GETPOSITION 11731 // Method
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_GETPOSITION_INPUTARGUMENTS 11732 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_GETPOSITION_OUTPUTARGUMENTS 11733 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_SETPOSITION 11734 // Method
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_SETPOSITION_INPUTARGUMENTS 11735 // Variable
#define UA_NS0ID_SERVER_NAMESPACES_ADDRESSSPACEFILE_EXPORTNAMESPACE 11736 // Method
#define UA_NS0ID_BITFIELDMASKDATATYPE 11737 // DataType
#define UA_NS0ID_OPENMETHODTYPE 11738 // Method
#define UA_NS0ID_CLOSEMETHODTYPE 11741 // Method
#define UA_NS0ID_READMETHODTYPE 11743 // Method
#define UA_NS0ID_WRITEMETHODTYPE 11746 // Method
#define UA_NS0ID_GETPOSITIONMETHODTYPE 11748 // Method
#define UA_NS0ID_SETPOSITIONMETHODTYPE 11751 // Method
#define UA_NS0ID_SYSTEMOFFNORMALALARMTYPE 11753 // ObjectType
#define UA_NS0ID_AUDITPROGRAMTRANSITIONEVENTTYPE 11856 // ObjectType
#define UA_NS0ID_HACONFIGURATION_AGGREGATEFUNCTIONS 11877 // Object
#define UA_NS0ID_NODECLASS_ENUMVALUES 11878 // Variable
#define UA_NS0ID_INSTANCENODE 11879 // DataType
#define UA_NS0ID_TYPENODE 11880 // DataType
#define UA_NS0ID_NODEATTRIBUTESMASK_ENUMVALUES 11881 // Variable
#define UA_NS0ID_ATTRIBUTEWRITEMASK_ENUMVALUES 11882 // Variable
#define UA_NS0ID_BROWSERESULTMASK_ENUMVALUES 11883 // Variable
#define UA_NS0ID_OPENFILEMODE 11939 // DataType
#define UA_NS0ID_OPENFILEMODE_ENUMVALUES 11940 // Variable
#define UA_NS0ID_MODELCHANGESTRUCTUREVERBMASK 11941 // DataType
#define UA_NS0ID_MODELCHANGESTRUCTUREVERBMASK_ENUMVALUES 11942 // Variable
#define UA_NS0ID_ENDPOINTURLLISTDATATYPE 11943 // DataType
#define UA_NS0ID_NETWORKGROUPDATATYPE 11944 // DataType
#define UA_NS0ID_NONTRANSPARENTNETWORKREDUNDANCYTYPE 11945 // ObjectType
#define UA_NS0ID_ARRAYITEMTYPE 12021 // VariableType
#define UA_NS0ID_YARRAYITEMTYPE 12029 // VariableType
#define UA_NS0ID_XYARRAYITEMTYPE 12038 // VariableType
#define UA_NS0ID_IMAGEITEMTYPE 12047 // VariableType
#define UA_NS0ID_CUBEITEMTYPE 12057 // VariableType
#define UA_NS0ID_NDIMENSIONARRAYITEMTYPE 12068 // VariableType
#define UA_NS0ID_AXISSCALEENUMERATION 12077 // DataType
#define UA_NS0ID_AXISSCALEENUMERATION_ENUMSTRINGS 12078 // Variable
#define UA_NS0ID_AXISINFORMATION 12079 // DataType
#define UA_NS0ID_XVTYPE 12080 // DataType
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERHISTORYREADDATA 12165 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERHISTORYREADEVENTS 12166 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERHISTORYUPDATEDATA 12167 // Variable
#define UA_NS0ID_SERVER_SERVERCAPABILITIES_OPERATIONLIMITS_MAXNODESPERHISTORYUPDATEEVENTS 12168 // Variable
#define UA_NS0ID_VIEWVERSION 12170 // Variable
#define UA_NS0ID_COMPLEXNUMBERTYPE 12171 // DataType
#define UA_NS0ID_DOUBLECOMPLEXNUMBERTYPE 12172 // DataType
#define UA_NS0ID_HASMODELPARENT 50 // ReferenceType


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_types_generated.h" ***********************************/

/* Generated from Opc.Ua.Types.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03 */


#ifdef __cplusplus
extern "C" {
#endif

#ifdef UA_INTERNAL
#endif

/**
 * Additional Data Type Definitions
 * ================================
 */

#define UA_TYPES_COUNT 158
extern UA_EXPORT const UA_DataType UA_TYPES[UA_TYPES_COUNT];

/**
 * Boolean
 * -------
 */
#define UA_TYPES_BOOLEAN 0
static UA_INLINE void UA_Boolean_init(UA_Boolean *p) { memset(p, 0, sizeof(UA_Boolean)); }
static UA_INLINE UA_Boolean * UA_Boolean_new(void) { return (UA_Boolean*) UA_new(&UA_TYPES[UA_TYPES_BOOLEAN]); }
static UA_INLINE UA_StatusCode UA_Boolean_copy(const UA_Boolean *src, UA_Boolean *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Boolean_deleteMembers(UA_Boolean *p) { }
static UA_INLINE void UA_Boolean_delete(UA_Boolean *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BOOLEAN]); }

/**
 * SByte
 * -----
 */
#define UA_TYPES_SBYTE 1
static UA_INLINE void UA_SByte_init(UA_SByte *p) { memset(p, 0, sizeof(UA_SByte)); }
static UA_INLINE UA_SByte * UA_SByte_new(void) { return (UA_SByte*) UA_new(&UA_TYPES[UA_TYPES_SBYTE]); }
static UA_INLINE UA_StatusCode UA_SByte_copy(const UA_SByte *src, UA_SByte *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_SByte_deleteMembers(UA_SByte *p) { }
static UA_INLINE void UA_SByte_delete(UA_SByte *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SBYTE]); }

/**
 * Byte
 * ----
 */
#define UA_TYPES_BYTE 2
static UA_INLINE void UA_Byte_init(UA_Byte *p) { memset(p, 0, sizeof(UA_Byte)); }
static UA_INLINE UA_Byte * UA_Byte_new(void) { return (UA_Byte*) UA_new(&UA_TYPES[UA_TYPES_BYTE]); }
static UA_INLINE UA_StatusCode UA_Byte_copy(const UA_Byte *src, UA_Byte *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Byte_deleteMembers(UA_Byte *p) { }
static UA_INLINE void UA_Byte_delete(UA_Byte *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BYTE]); }

/**
 * Int16
 * -----
 */
#define UA_TYPES_INT16 3
static UA_INLINE void UA_Int16_init(UA_Int16 *p) { memset(p, 0, sizeof(UA_Int16)); }
static UA_INLINE UA_Int16 * UA_Int16_new(void) { return (UA_Int16*) UA_new(&UA_TYPES[UA_TYPES_INT16]); }
static UA_INLINE UA_StatusCode UA_Int16_copy(const UA_Int16 *src, UA_Int16 *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Int16_deleteMembers(UA_Int16 *p) { }
static UA_INLINE void UA_Int16_delete(UA_Int16 *p) { UA_delete(p, &UA_TYPES[UA_TYPES_INT16]); }

/**
 * UInt16
 * ------
 */
#define UA_TYPES_UINT16 4
static UA_INLINE void UA_UInt16_init(UA_UInt16 *p) { memset(p, 0, sizeof(UA_UInt16)); }
static UA_INLINE UA_UInt16 * UA_UInt16_new(void) { return (UA_UInt16*) UA_new(&UA_TYPES[UA_TYPES_UINT16]); }
static UA_INLINE UA_StatusCode UA_UInt16_copy(const UA_UInt16 *src, UA_UInt16 *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_UInt16_deleteMembers(UA_UInt16 *p) { }
static UA_INLINE void UA_UInt16_delete(UA_UInt16 *p) { UA_delete(p, &UA_TYPES[UA_TYPES_UINT16]); }

/**
 * Int32
 * -----
 */
#define UA_TYPES_INT32 5
static UA_INLINE void UA_Int32_init(UA_Int32 *p) { memset(p, 0, sizeof(UA_Int32)); }
static UA_INLINE UA_Int32 * UA_Int32_new(void) { return (UA_Int32*) UA_new(&UA_TYPES[UA_TYPES_INT32]); }
static UA_INLINE UA_StatusCode UA_Int32_copy(const UA_Int32 *src, UA_Int32 *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Int32_deleteMembers(UA_Int32 *p) { }
static UA_INLINE void UA_Int32_delete(UA_Int32 *p) { UA_delete(p, &UA_TYPES[UA_TYPES_INT32]); }

/**
 * UInt32
 * ------
 */
#define UA_TYPES_UINT32 6
static UA_INLINE void UA_UInt32_init(UA_UInt32 *p) { memset(p, 0, sizeof(UA_UInt32)); }
static UA_INLINE UA_UInt32 * UA_UInt32_new(void) { return (UA_UInt32*) UA_new(&UA_TYPES[UA_TYPES_UINT32]); }
static UA_INLINE UA_StatusCode UA_UInt32_copy(const UA_UInt32 *src, UA_UInt32 *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_UInt32_deleteMembers(UA_UInt32 *p) { }
static UA_INLINE void UA_UInt32_delete(UA_UInt32 *p) { UA_delete(p, &UA_TYPES[UA_TYPES_UINT32]); }

/**
 * Int64
 * -----
 */
#define UA_TYPES_INT64 7
static UA_INLINE void UA_Int64_init(UA_Int64 *p) { memset(p, 0, sizeof(UA_Int64)); }
static UA_INLINE UA_Int64 * UA_Int64_new(void) { return (UA_Int64*) UA_new(&UA_TYPES[UA_TYPES_INT64]); }
static UA_INLINE UA_StatusCode UA_Int64_copy(const UA_Int64 *src, UA_Int64 *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Int64_deleteMembers(UA_Int64 *p) { }
static UA_INLINE void UA_Int64_delete(UA_Int64 *p) { UA_delete(p, &UA_TYPES[UA_TYPES_INT64]); }

/**
 * UInt64
 * ------
 */
#define UA_TYPES_UINT64 8
static UA_INLINE void UA_UInt64_init(UA_UInt64 *p) { memset(p, 0, sizeof(UA_UInt64)); }
static UA_INLINE UA_UInt64 * UA_UInt64_new(void) { return (UA_UInt64*) UA_new(&UA_TYPES[UA_TYPES_UINT64]); }
static UA_INLINE UA_StatusCode UA_UInt64_copy(const UA_UInt64 *src, UA_UInt64 *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_UInt64_deleteMembers(UA_UInt64 *p) { }
static UA_INLINE void UA_UInt64_delete(UA_UInt64 *p) { UA_delete(p, &UA_TYPES[UA_TYPES_UINT64]); }

/**
 * Float
 * -----
 */
#define UA_TYPES_FLOAT 9
static UA_INLINE void UA_Float_init(UA_Float *p) { memset(p, 0, sizeof(UA_Float)); }
static UA_INLINE UA_Float * UA_Float_new(void) { return (UA_Float*) UA_new(&UA_TYPES[UA_TYPES_FLOAT]); }
static UA_INLINE UA_StatusCode UA_Float_copy(const UA_Float *src, UA_Float *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Float_deleteMembers(UA_Float *p) { }
static UA_INLINE void UA_Float_delete(UA_Float *p) { UA_delete(p, &UA_TYPES[UA_TYPES_FLOAT]); }

/**
 * Double
 * ------
 */
#define UA_TYPES_DOUBLE 10
static UA_INLINE void UA_Double_init(UA_Double *p) { memset(p, 0, sizeof(UA_Double)); }
static UA_INLINE UA_Double * UA_Double_new(void) { return (UA_Double*) UA_new(&UA_TYPES[UA_TYPES_DOUBLE]); }
static UA_INLINE UA_StatusCode UA_Double_copy(const UA_Double *src, UA_Double *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Double_deleteMembers(UA_Double *p) { }
static UA_INLINE void UA_Double_delete(UA_Double *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DOUBLE]); }

/**
 * String
 * ------
 */
#define UA_TYPES_STRING 11
static UA_INLINE void UA_String_init(UA_String *p) { memset(p, 0, sizeof(UA_String)); }
static UA_INLINE UA_String * UA_String_new(void) { return (UA_String*) UA_new(&UA_TYPES[UA_TYPES_STRING]); }
static UA_INLINE UA_StatusCode UA_String_copy(const UA_String *src, UA_String *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_STRING]); }
static UA_INLINE void UA_String_deleteMembers(UA_String *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_STRING]); }
static UA_INLINE void UA_String_delete(UA_String *p) { UA_delete(p, &UA_TYPES[UA_TYPES_STRING]); }

/**
 * DateTime
 * --------
 */
#define UA_TYPES_DATETIME 12
static UA_INLINE void UA_DateTime_init(UA_DateTime *p) { memset(p, 0, sizeof(UA_DateTime)); }
static UA_INLINE UA_DateTime * UA_DateTime_new(void) { return (UA_DateTime*) UA_new(&UA_TYPES[UA_TYPES_DATETIME]); }
static UA_INLINE UA_StatusCode UA_DateTime_copy(const UA_DateTime *src, UA_DateTime *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_DateTime_deleteMembers(UA_DateTime *p) { }
static UA_INLINE void UA_DateTime_delete(UA_DateTime *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DATETIME]); }

/**
 * Guid
 * ----
 */
#define UA_TYPES_GUID 13
static UA_INLINE void UA_Guid_init(UA_Guid *p) { memset(p, 0, sizeof(UA_Guid)); }
static UA_INLINE UA_Guid * UA_Guid_new(void) { return (UA_Guid*) UA_new(&UA_TYPES[UA_TYPES_GUID]); }
static UA_INLINE UA_StatusCode UA_Guid_copy(const UA_Guid *src, UA_Guid *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_Guid_deleteMembers(UA_Guid *p) { }
static UA_INLINE void UA_Guid_delete(UA_Guid *p) { UA_delete(p, &UA_TYPES[UA_TYPES_GUID]); }

/**
 * ByteString
 * ----------
 */
#define UA_TYPES_BYTESTRING 14
static UA_INLINE void UA_ByteString_init(UA_ByteString *p) { memset(p, 0, sizeof(UA_ByteString)); }
static UA_INLINE UA_ByteString * UA_ByteString_new(void) { return (UA_ByteString*) UA_new(&UA_TYPES[UA_TYPES_BYTESTRING]); }
static UA_INLINE UA_StatusCode UA_ByteString_copy(const UA_ByteString *src, UA_ByteString *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BYTESTRING]); }
static UA_INLINE void UA_ByteString_deleteMembers(UA_ByteString *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BYTESTRING]); }
static UA_INLINE void UA_ByteString_delete(UA_ByteString *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BYTESTRING]); }

/**
 * XmlElement
 * ----------
 */
#define UA_TYPES_XMLELEMENT 15
static UA_INLINE void UA_XmlElement_init(UA_XmlElement *p) { memset(p, 0, sizeof(UA_XmlElement)); }
static UA_INLINE UA_XmlElement * UA_XmlElement_new(void) { return (UA_XmlElement*) UA_new(&UA_TYPES[UA_TYPES_XMLELEMENT]); }
static UA_INLINE UA_StatusCode UA_XmlElement_copy(const UA_XmlElement *src, UA_XmlElement *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_XMLELEMENT]); }
static UA_INLINE void UA_XmlElement_deleteMembers(UA_XmlElement *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_XMLELEMENT]); }
static UA_INLINE void UA_XmlElement_delete(UA_XmlElement *p) { UA_delete(p, &UA_TYPES[UA_TYPES_XMLELEMENT]); }

/**
 * NodeId
 * ------
 */
#define UA_TYPES_NODEID 16
static UA_INLINE void UA_NodeId_init(UA_NodeId *p) { memset(p, 0, sizeof(UA_NodeId)); }
static UA_INLINE UA_NodeId * UA_NodeId_new(void) { return (UA_NodeId*) UA_new(&UA_TYPES[UA_TYPES_NODEID]); }
static UA_INLINE UA_StatusCode UA_NodeId_copy(const UA_NodeId *src, UA_NodeId *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NODEID]); }
static UA_INLINE void UA_NodeId_deleteMembers(UA_NodeId *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_NODEID]); }
static UA_INLINE void UA_NodeId_delete(UA_NodeId *p) { UA_delete(p, &UA_TYPES[UA_TYPES_NODEID]); }

/**
 * ExpandedNodeId
 * --------------
 */
#define UA_TYPES_EXPANDEDNODEID 17
static UA_INLINE void UA_ExpandedNodeId_init(UA_ExpandedNodeId *p) { memset(p, 0, sizeof(UA_ExpandedNodeId)); }
static UA_INLINE UA_ExpandedNodeId * UA_ExpandedNodeId_new(void) { return (UA_ExpandedNodeId*) UA_new(&UA_TYPES[UA_TYPES_EXPANDEDNODEID]); }
static UA_INLINE UA_StatusCode UA_ExpandedNodeId_copy(const UA_ExpandedNodeId *src, UA_ExpandedNodeId *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]); }
static UA_INLINE void UA_ExpandedNodeId_deleteMembers(UA_ExpandedNodeId *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]); }
static UA_INLINE void UA_ExpandedNodeId_delete(UA_ExpandedNodeId *p) { UA_delete(p, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]); }

/**
 * StatusCode
 * ----------
 */
#define UA_TYPES_STATUSCODE 18
static UA_INLINE void UA_StatusCode_init(UA_StatusCode *p) { memset(p, 0, sizeof(UA_StatusCode)); }
static UA_INLINE UA_StatusCode * UA_StatusCode_new(void) { return (UA_StatusCode*) UA_new(&UA_TYPES[UA_TYPES_STATUSCODE]); }
static UA_INLINE UA_StatusCode UA_StatusCode_copy(const UA_StatusCode *src, UA_StatusCode *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_StatusCode_deleteMembers(UA_StatusCode *p) { }
static UA_INLINE void UA_StatusCode_delete(UA_StatusCode *p) { UA_delete(p, &UA_TYPES[UA_TYPES_STATUSCODE]); }

/**
 * QualifiedName
 * -------------
 */
#define UA_TYPES_QUALIFIEDNAME 19
static UA_INLINE void UA_QualifiedName_init(UA_QualifiedName *p) { memset(p, 0, sizeof(UA_QualifiedName)); }
static UA_INLINE UA_QualifiedName * UA_QualifiedName_new(void) { return (UA_QualifiedName*) UA_new(&UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }
static UA_INLINE UA_StatusCode UA_QualifiedName_copy(const UA_QualifiedName *src, UA_QualifiedName *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }
static UA_INLINE void UA_QualifiedName_deleteMembers(UA_QualifiedName *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }
static UA_INLINE void UA_QualifiedName_delete(UA_QualifiedName *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }

/**
 * LocalizedText
 * -------------
 */
#define UA_TYPES_LOCALIZEDTEXT 20
static UA_INLINE void UA_LocalizedText_init(UA_LocalizedText *p) { memset(p, 0, sizeof(UA_LocalizedText)); }
static UA_INLINE UA_LocalizedText * UA_LocalizedText_new(void) { return (UA_LocalizedText*) UA_new(&UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }
static UA_INLINE UA_StatusCode UA_LocalizedText_copy(const UA_LocalizedText *src, UA_LocalizedText *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }
static UA_INLINE void UA_LocalizedText_deleteMembers(UA_LocalizedText *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }
static UA_INLINE void UA_LocalizedText_delete(UA_LocalizedText *p) { UA_delete(p, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

/**
 * ExtensionObject
 * ---------------
 */
#define UA_TYPES_EXTENSIONOBJECT 21
static UA_INLINE void UA_ExtensionObject_init(UA_ExtensionObject *p) { memset(p, 0, sizeof(UA_ExtensionObject)); }
static UA_INLINE UA_ExtensionObject * UA_ExtensionObject_new(void) { return (UA_ExtensionObject*) UA_new(&UA_TYPES[UA_TYPES_EXTENSIONOBJECT]); }
static UA_INLINE UA_StatusCode UA_ExtensionObject_copy(const UA_ExtensionObject *src, UA_ExtensionObject *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]); }
static UA_INLINE void UA_ExtensionObject_deleteMembers(UA_ExtensionObject *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]); }
static UA_INLINE void UA_ExtensionObject_delete(UA_ExtensionObject *p) { UA_delete(p, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]); }

/**
 * DataValue
 * ---------
 */
#define UA_TYPES_DATAVALUE 22
static UA_INLINE void UA_DataValue_init(UA_DataValue *p) { memset(p, 0, sizeof(UA_DataValue)); }
static UA_INLINE UA_DataValue * UA_DataValue_new(void) { return (UA_DataValue*) UA_new(&UA_TYPES[UA_TYPES_DATAVALUE]); }
static UA_INLINE UA_StatusCode UA_DataValue_copy(const UA_DataValue *src, UA_DataValue *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DATAVALUE]); }
static UA_INLINE void UA_DataValue_deleteMembers(UA_DataValue *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DATAVALUE]); }
static UA_INLINE void UA_DataValue_delete(UA_DataValue *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DATAVALUE]); }

/**
 * Variant
 * -------
 */
#define UA_TYPES_VARIANT 23
static UA_INLINE void UA_Variant_init(UA_Variant *p) { memset(p, 0, sizeof(UA_Variant)); }
static UA_INLINE UA_Variant * UA_Variant_new(void) { return (UA_Variant*) UA_new(&UA_TYPES[UA_TYPES_VARIANT]); }
static UA_INLINE UA_StatusCode UA_Variant_copy(const UA_Variant *src, UA_Variant *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VARIANT]); }
static UA_INLINE void UA_Variant_deleteMembers(UA_Variant *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_VARIANT]); }
static UA_INLINE void UA_Variant_delete(UA_Variant *p) { UA_delete(p, &UA_TYPES[UA_TYPES_VARIANT]); }

/**
 * DiagnosticInfo
 * --------------
 */
#define UA_TYPES_DIAGNOSTICINFO 24
static UA_INLINE void UA_DiagnosticInfo_init(UA_DiagnosticInfo *p) { memset(p, 0, sizeof(UA_DiagnosticInfo)); }
static UA_INLINE UA_DiagnosticInfo * UA_DiagnosticInfo_new(void) { return (UA_DiagnosticInfo*) UA_new(&UA_TYPES[UA_TYPES_DIAGNOSTICINFO]); }
static UA_INLINE UA_StatusCode UA_DiagnosticInfo_copy(const UA_DiagnosticInfo *src, UA_DiagnosticInfo *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]); }
static UA_INLINE void UA_DiagnosticInfo_deleteMembers(UA_DiagnosticInfo *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]); }
static UA_INLINE void UA_DiagnosticInfo_delete(UA_DiagnosticInfo *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]); }

/**
 * SignedSoftwareCertificate
 * -------------------------
 * A software certificate with a digital signature. */
typedef struct {
    UA_ByteString certificateData;
    UA_ByteString signature;
} UA_SignedSoftwareCertificate;

#define UA_TYPES_SIGNEDSOFTWARECERTIFICATE 25
static UA_INLINE void UA_SignedSoftwareCertificate_init(UA_SignedSoftwareCertificate *p) { memset(p, 0, sizeof(UA_SignedSoftwareCertificate)); }
static UA_INLINE UA_SignedSoftwareCertificate * UA_SignedSoftwareCertificate_new(void) { return (UA_SignedSoftwareCertificate*) UA_new(&UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]); }
static UA_INLINE UA_StatusCode UA_SignedSoftwareCertificate_copy(const UA_SignedSoftwareCertificate *src, UA_SignedSoftwareCertificate *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]); }
static UA_INLINE void UA_SignedSoftwareCertificate_deleteMembers(UA_SignedSoftwareCertificate *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]); }
static UA_INLINE void UA_SignedSoftwareCertificate_delete(UA_SignedSoftwareCertificate *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]); }

/**
 * BrowsePathTarget
 * ----------------
 * The target of the translated path. */
typedef struct {
    UA_ExpandedNodeId targetId;
    UA_UInt32 remainingPathIndex;
} UA_BrowsePathTarget;

#define UA_TYPES_BROWSEPATHTARGET 26
static UA_INLINE void UA_BrowsePathTarget_init(UA_BrowsePathTarget *p) { memset(p, 0, sizeof(UA_BrowsePathTarget)); }
static UA_INLINE UA_BrowsePathTarget * UA_BrowsePathTarget_new(void) { return (UA_BrowsePathTarget*) UA_new(&UA_TYPES[UA_TYPES_BROWSEPATHTARGET]); }
static UA_INLINE UA_StatusCode UA_BrowsePathTarget_copy(const UA_BrowsePathTarget *src, UA_BrowsePathTarget *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]); }
static UA_INLINE void UA_BrowsePathTarget_deleteMembers(UA_BrowsePathTarget *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]); }
static UA_INLINE void UA_BrowsePathTarget_delete(UA_BrowsePathTarget *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]); }

/**
 * ViewAttributes
 * --------------
 * The attributes for a view node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Boolean containsNoLoops;
    UA_Byte eventNotifier;
} UA_ViewAttributes;

#define UA_TYPES_VIEWATTRIBUTES 27
static UA_INLINE void UA_ViewAttributes_init(UA_ViewAttributes *p) { memset(p, 0, sizeof(UA_ViewAttributes)); }
static UA_INLINE UA_ViewAttributes * UA_ViewAttributes_new(void) { return (UA_ViewAttributes*) UA_new(&UA_TYPES[UA_TYPES_VIEWATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_ViewAttributes_copy(const UA_ViewAttributes *src, UA_ViewAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]); }
static UA_INLINE void UA_ViewAttributes_deleteMembers(UA_ViewAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]); }
static UA_INLINE void UA_ViewAttributes_delete(UA_ViewAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]); }

/**
 * BrowseResultMask
 * ----------------
 * A bit mask which specifies what should be returned in a browse response. */
typedef enum { 
    UA_BROWSERESULTMASK_NONE = 0,
    UA_BROWSERESULTMASK_REFERENCETYPEID = 1,
    UA_BROWSERESULTMASK_ISFORWARD = 2,
    UA_BROWSERESULTMASK_NODECLASS = 4,
    UA_BROWSERESULTMASK_BROWSENAME = 8,
    UA_BROWSERESULTMASK_DISPLAYNAME = 16,
    UA_BROWSERESULTMASK_TYPEDEFINITION = 32,
    UA_BROWSERESULTMASK_ALL = 63,
    UA_BROWSERESULTMASK_REFERENCETYPEINFO = 3,
    UA_BROWSERESULTMASK_TARGETINFO = 60
} UA_BrowseResultMask;

#define UA_TYPES_BROWSERESULTMASK 28
static UA_INLINE void UA_BrowseResultMask_init(UA_BrowseResultMask *p) { memset(p, 0, sizeof(UA_BrowseResultMask)); }
static UA_INLINE UA_BrowseResultMask * UA_BrowseResultMask_new(void) { return (UA_BrowseResultMask*) UA_new(&UA_TYPES[UA_TYPES_BROWSERESULTMASK]); }
static UA_INLINE UA_StatusCode UA_BrowseResultMask_copy(const UA_BrowseResultMask *src, UA_BrowseResultMask *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_BrowseResultMask_deleteMembers(UA_BrowseResultMask *p) { }
static UA_INLINE void UA_BrowseResultMask_delete(UA_BrowseResultMask *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSERESULTMASK]); }

/**
 * RequestHeader
 * -------------
 * The header passed with every server request. */
typedef struct {
    UA_NodeId authenticationToken;
    UA_DateTime timestamp;
    UA_UInt32 requestHandle;
    UA_UInt32 returnDiagnostics;
    UA_String auditEntryId;
    UA_UInt32 timeoutHint;
    UA_ExtensionObject additionalHeader;
} UA_RequestHeader;

#define UA_TYPES_REQUESTHEADER 29
static UA_INLINE void UA_RequestHeader_init(UA_RequestHeader *p) { memset(p, 0, sizeof(UA_RequestHeader)); }
static UA_INLINE UA_RequestHeader * UA_RequestHeader_new(void) { return (UA_RequestHeader*) UA_new(&UA_TYPES[UA_TYPES_REQUESTHEADER]); }
static UA_INLINE UA_StatusCode UA_RequestHeader_copy(const UA_RequestHeader *src, UA_RequestHeader *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REQUESTHEADER]); }
static UA_INLINE void UA_RequestHeader_deleteMembers(UA_RequestHeader *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REQUESTHEADER]); }
static UA_INLINE void UA_RequestHeader_delete(UA_RequestHeader *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REQUESTHEADER]); }

/**
 * MonitoredItemModifyResult
 * -------------------------
 */
typedef struct {
    UA_StatusCode statusCode;
    UA_Double revisedSamplingInterval;
    UA_UInt32 revisedQueueSize;
    UA_ExtensionObject filterResult;
} UA_MonitoredItemModifyResult;

#define UA_TYPES_MONITOREDITEMMODIFYRESULT 30
static UA_INLINE void UA_MonitoredItemModifyResult_init(UA_MonitoredItemModifyResult *p) { memset(p, 0, sizeof(UA_MonitoredItemModifyResult)); }
static UA_INLINE UA_MonitoredItemModifyResult * UA_MonitoredItemModifyResult_new(void) { return (UA_MonitoredItemModifyResult*) UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]); }
static UA_INLINE UA_StatusCode UA_MonitoredItemModifyResult_copy(const UA_MonitoredItemModifyResult *src, UA_MonitoredItemModifyResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]); }
static UA_INLINE void UA_MonitoredItemModifyResult_deleteMembers(UA_MonitoredItemModifyResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]); }
static UA_INLINE void UA_MonitoredItemModifyResult_delete(UA_MonitoredItemModifyResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]); }

/**
 * ViewDescription
 * ---------------
 * The view to browse. */
typedef struct {
    UA_NodeId viewId;
    UA_DateTime timestamp;
    UA_UInt32 viewVersion;
} UA_ViewDescription;

#define UA_TYPES_VIEWDESCRIPTION 31
static UA_INLINE void UA_ViewDescription_init(UA_ViewDescription *p) { memset(p, 0, sizeof(UA_ViewDescription)); }
static UA_INLINE UA_ViewDescription * UA_ViewDescription_new(void) { return (UA_ViewDescription*) UA_new(&UA_TYPES[UA_TYPES_VIEWDESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_ViewDescription_copy(const UA_ViewDescription *src, UA_ViewDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]); }
static UA_INLINE void UA_ViewDescription_deleteMembers(UA_ViewDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]); }
static UA_INLINE void UA_ViewDescription_delete(UA_ViewDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]); }

/**
 * CloseSecureChannelRequest
 * -------------------------
 * Closes a secure channel. */
typedef struct {
    UA_RequestHeader requestHeader;
} UA_CloseSecureChannelRequest;

#define UA_TYPES_CLOSESECURECHANNELREQUEST 32
static UA_INLINE void UA_CloseSecureChannelRequest_init(UA_CloseSecureChannelRequest *p) { memset(p, 0, sizeof(UA_CloseSecureChannelRequest)); }
static UA_INLINE UA_CloseSecureChannelRequest * UA_CloseSecureChannelRequest_new(void) { return (UA_CloseSecureChannelRequest*) UA_new(&UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]); }
static UA_INLINE UA_StatusCode UA_CloseSecureChannelRequest_copy(const UA_CloseSecureChannelRequest *src, UA_CloseSecureChannelRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]); }
static UA_INLINE void UA_CloseSecureChannelRequest_deleteMembers(UA_CloseSecureChannelRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]); }
static UA_INLINE void UA_CloseSecureChannelRequest_delete(UA_CloseSecureChannelRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]); }

/**
 * AddNodesResult
 * --------------
 * A result of an add node operation. */
typedef struct {
    UA_StatusCode statusCode;
    UA_NodeId addedNodeId;
} UA_AddNodesResult;

#define UA_TYPES_ADDNODESRESULT 33
static UA_INLINE void UA_AddNodesResult_init(UA_AddNodesResult *p) { memset(p, 0, sizeof(UA_AddNodesResult)); }
static UA_INLINE UA_AddNodesResult * UA_AddNodesResult_new(void) { return (UA_AddNodesResult*) UA_new(&UA_TYPES[UA_TYPES_ADDNODESRESULT]); }
static UA_INLINE UA_StatusCode UA_AddNodesResult_copy(const UA_AddNodesResult *src, UA_AddNodesResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESRESULT]); }
static UA_INLINE void UA_AddNodesResult_deleteMembers(UA_AddNodesResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDNODESRESULT]); }
static UA_INLINE void UA_AddNodesResult_delete(UA_AddNodesResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESRESULT]); }

/**
 * VariableAttributes
 * ------------------
 * The attributes for a variable node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Variant value;
    UA_NodeId dataType;
    UA_Int32 valueRank;
    size_t arrayDimensionsSize;
    UA_UInt32 *arrayDimensions;
    UA_Byte accessLevel;
    UA_Byte userAccessLevel;
    UA_Double minimumSamplingInterval;
    UA_Boolean historizing;
} UA_VariableAttributes;

#define UA_TYPES_VARIABLEATTRIBUTES 34
static UA_INLINE void UA_VariableAttributes_init(UA_VariableAttributes *p) { memset(p, 0, sizeof(UA_VariableAttributes)); }
static UA_INLINE UA_VariableAttributes * UA_VariableAttributes_new(void) { return (UA_VariableAttributes*) UA_new(&UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_VariableAttributes_copy(const UA_VariableAttributes *src, UA_VariableAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]); }
static UA_INLINE void UA_VariableAttributes_deleteMembers(UA_VariableAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]); }
static UA_INLINE void UA_VariableAttributes_delete(UA_VariableAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]); }

/**
 * NotificationMessage
 * -------------------
 */
typedef struct {
    UA_UInt32 sequenceNumber;
    UA_DateTime publishTime;
    size_t notificationDataSize;
    UA_ExtensionObject *notificationData;
} UA_NotificationMessage;

#define UA_TYPES_NOTIFICATIONMESSAGE 35
static UA_INLINE void UA_NotificationMessage_init(UA_NotificationMessage *p) { memset(p, 0, sizeof(UA_NotificationMessage)); }
static UA_INLINE UA_NotificationMessage * UA_NotificationMessage_new(void) { return (UA_NotificationMessage*) UA_new(&UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]); }
static UA_INLINE UA_StatusCode UA_NotificationMessage_copy(const UA_NotificationMessage *src, UA_NotificationMessage *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]); }
static UA_INLINE void UA_NotificationMessage_deleteMembers(UA_NotificationMessage *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]); }
static UA_INLINE void UA_NotificationMessage_delete(UA_NotificationMessage *p) { UA_delete(p, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]); }

/**
 * NodeAttributesMask
 * ------------------
 * The bits used to specify default attributes for a new node. */
typedef enum { 
    UA_NODEATTRIBUTESMASK_NONE = 0,
    UA_NODEATTRIBUTESMASK_ACCESSLEVEL = 1,
    UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS = 2,
    UA_NODEATTRIBUTESMASK_BROWSENAME = 4,
    UA_NODEATTRIBUTESMASK_CONTAINSNOLOOPS = 8,
    UA_NODEATTRIBUTESMASK_DATATYPE = 16,
    UA_NODEATTRIBUTESMASK_DESCRIPTION = 32,
    UA_NODEATTRIBUTESMASK_DISPLAYNAME = 64,
    UA_NODEATTRIBUTESMASK_EVENTNOTIFIER = 128,
    UA_NODEATTRIBUTESMASK_EXECUTABLE = 256,
    UA_NODEATTRIBUTESMASK_HISTORIZING = 512,
    UA_NODEATTRIBUTESMASK_INVERSENAME = 1024,
    UA_NODEATTRIBUTESMASK_ISABSTRACT = 2048,
    UA_NODEATTRIBUTESMASK_MINIMUMSAMPLINGINTERVAL = 4096,
    UA_NODEATTRIBUTESMASK_NODECLASS = 8192,
    UA_NODEATTRIBUTESMASK_NODEID = 16384,
    UA_NODEATTRIBUTESMASK_SYMMETRIC = 32768,
    UA_NODEATTRIBUTESMASK_USERACCESSLEVEL = 65536,
    UA_NODEATTRIBUTESMASK_USEREXECUTABLE = 131072,
    UA_NODEATTRIBUTESMASK_USERWRITEMASK = 262144,
    UA_NODEATTRIBUTESMASK_VALUERANK = 524288,
    UA_NODEATTRIBUTESMASK_WRITEMASK = 1048576,
    UA_NODEATTRIBUTESMASK_VALUE = 2097152,
    UA_NODEATTRIBUTESMASK_ALL = 4194303,
    UA_NODEATTRIBUTESMASK_BASENODE = 1335396,
    UA_NODEATTRIBUTESMASK_OBJECT = 1335524,
    UA_NODEATTRIBUTESMASK_OBJECTTYPEORDATATYPE = 1337444,
    UA_NODEATTRIBUTESMASK_VARIABLE = 4026999,
    UA_NODEATTRIBUTESMASK_VARIABLETYPE = 3958902,
    UA_NODEATTRIBUTESMASK_METHOD = 1466724,
    UA_NODEATTRIBUTESMASK_REFERENCETYPE = 1371236,
    UA_NODEATTRIBUTESMASK_VIEW = 1335532
} UA_NodeAttributesMask;

#define UA_TYPES_NODEATTRIBUTESMASK 36
static UA_INLINE void UA_NodeAttributesMask_init(UA_NodeAttributesMask *p) { memset(p, 0, sizeof(UA_NodeAttributesMask)); }
static UA_INLINE UA_NodeAttributesMask * UA_NodeAttributesMask_new(void) { return (UA_NodeAttributesMask*) UA_new(&UA_TYPES[UA_TYPES_NODEATTRIBUTESMASK]); }
static UA_INLINE UA_StatusCode UA_NodeAttributesMask_copy(const UA_NodeAttributesMask *src, UA_NodeAttributesMask *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_NodeAttributesMask_deleteMembers(UA_NodeAttributesMask *p) { }
static UA_INLINE void UA_NodeAttributesMask_delete(UA_NodeAttributesMask *p) { UA_delete(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTESMASK]); }

/**
 * MonitoringMode
 * --------------
 */
typedef enum { 
    UA_MONITORINGMODE_DISABLED = 0,
    UA_MONITORINGMODE_SAMPLING = 1,
    UA_MONITORINGMODE_REPORTING = 2
} UA_MonitoringMode;

#define UA_TYPES_MONITORINGMODE 37
static UA_INLINE void UA_MonitoringMode_init(UA_MonitoringMode *p) { memset(p, 0, sizeof(UA_MonitoringMode)); }
static UA_INLINE UA_MonitoringMode * UA_MonitoringMode_new(void) { return (UA_MonitoringMode*) UA_new(&UA_TYPES[UA_TYPES_MONITORINGMODE]); }
static UA_INLINE UA_StatusCode UA_MonitoringMode_copy(const UA_MonitoringMode *src, UA_MonitoringMode *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_MonitoringMode_deleteMembers(UA_MonitoringMode *p) { }
static UA_INLINE void UA_MonitoringMode_delete(UA_MonitoringMode *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITORINGMODE]); }

/**
 * CallMethodResult
 * ----------------
 */
typedef struct {
    UA_StatusCode statusCode;
    size_t inputArgumentResultsSize;
    UA_StatusCode *inputArgumentResults;
    size_t inputArgumentDiagnosticInfosSize;
    UA_DiagnosticInfo *inputArgumentDiagnosticInfos;
    size_t outputArgumentsSize;
    UA_Variant *outputArguments;
} UA_CallMethodResult;

#define UA_TYPES_CALLMETHODRESULT 38
static UA_INLINE void UA_CallMethodResult_init(UA_CallMethodResult *p) { memset(p, 0, sizeof(UA_CallMethodResult)); }
static UA_INLINE UA_CallMethodResult * UA_CallMethodResult_new(void) { return (UA_CallMethodResult*) UA_new(&UA_TYPES[UA_TYPES_CALLMETHODRESULT]); }
static UA_INLINE UA_StatusCode UA_CallMethodResult_copy(const UA_CallMethodResult *src, UA_CallMethodResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]); }
static UA_INLINE void UA_CallMethodResult_deleteMembers(UA_CallMethodResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]); }
static UA_INLINE void UA_CallMethodResult_delete(UA_CallMethodResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]); }

/**
 * ParsingResult
 * -------------
 */
typedef struct {
    UA_StatusCode statusCode;
    size_t dataStatusCodesSize;
    UA_StatusCode *dataStatusCodes;
    size_t dataDiagnosticInfosSize;
    UA_DiagnosticInfo *dataDiagnosticInfos;
} UA_ParsingResult;

#define UA_TYPES_PARSINGRESULT 39
static UA_INLINE void UA_ParsingResult_init(UA_ParsingResult *p) { memset(p, 0, sizeof(UA_ParsingResult)); }
static UA_INLINE UA_ParsingResult * UA_ParsingResult_new(void) { return (UA_ParsingResult*) UA_new(&UA_TYPES[UA_TYPES_PARSINGRESULT]); }
static UA_INLINE UA_StatusCode UA_ParsingResult_copy(const UA_ParsingResult *src, UA_ParsingResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_PARSINGRESULT]); }
static UA_INLINE void UA_ParsingResult_deleteMembers(UA_ParsingResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_PARSINGRESULT]); }
static UA_INLINE void UA_ParsingResult_delete(UA_ParsingResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_PARSINGRESULT]); }

/**
 * RelativePathElement
 * -------------------
 * An element in a relative path. */
typedef struct {
    UA_NodeId referenceTypeId;
    UA_Boolean isInverse;
    UA_Boolean includeSubtypes;
    UA_QualifiedName targetName;
} UA_RelativePathElement;

#define UA_TYPES_RELATIVEPATHELEMENT 40
static UA_INLINE void UA_RelativePathElement_init(UA_RelativePathElement *p) { memset(p, 0, sizeof(UA_RelativePathElement)); }
static UA_INLINE UA_RelativePathElement * UA_RelativePathElement_new(void) { return (UA_RelativePathElement*) UA_new(&UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]); }
static UA_INLINE UA_StatusCode UA_RelativePathElement_copy(const UA_RelativePathElement *src, UA_RelativePathElement *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]); }
static UA_INLINE void UA_RelativePathElement_deleteMembers(UA_RelativePathElement *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]); }
static UA_INLINE void UA_RelativePathElement_delete(UA_RelativePathElement *p) { UA_delete(p, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]); }

/**
 * BrowseDirection
 * ---------------
 * The directions of the references to return. */
typedef enum { 
    UA_BROWSEDIRECTION_FORWARD = 0,
    UA_BROWSEDIRECTION_INVERSE = 1,
    UA_BROWSEDIRECTION_BOTH = 2
} UA_BrowseDirection;

#define UA_TYPES_BROWSEDIRECTION 41
static UA_INLINE void UA_BrowseDirection_init(UA_BrowseDirection *p) { memset(p, 0, sizeof(UA_BrowseDirection)); }
static UA_INLINE UA_BrowseDirection * UA_BrowseDirection_new(void) { return (UA_BrowseDirection*) UA_new(&UA_TYPES[UA_TYPES_BROWSEDIRECTION]); }
static UA_INLINE UA_StatusCode UA_BrowseDirection_copy(const UA_BrowseDirection *src, UA_BrowseDirection *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_BrowseDirection_deleteMembers(UA_BrowseDirection *p) { }
static UA_INLINE void UA_BrowseDirection_delete(UA_BrowseDirection *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEDIRECTION]); }

/**
 * CallMethodRequest
 * -----------------
 */
typedef struct {
    UA_NodeId objectId;
    UA_NodeId methodId;
    size_t inputArgumentsSize;
    UA_Variant *inputArguments;
} UA_CallMethodRequest;

#define UA_TYPES_CALLMETHODREQUEST 42
static UA_INLINE void UA_CallMethodRequest_init(UA_CallMethodRequest *p) { memset(p, 0, sizeof(UA_CallMethodRequest)); }
static UA_INLINE UA_CallMethodRequest * UA_CallMethodRequest_new(void) { return (UA_CallMethodRequest*) UA_new(&UA_TYPES[UA_TYPES_CALLMETHODREQUEST]); }
static UA_INLINE UA_StatusCode UA_CallMethodRequest_copy(const UA_CallMethodRequest *src, UA_CallMethodRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]); }
static UA_INLINE void UA_CallMethodRequest_deleteMembers(UA_CallMethodRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]); }
static UA_INLINE void UA_CallMethodRequest_delete(UA_CallMethodRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]); }

/**
 * ServerState
 * -----------
 */
typedef enum { 
    UA_SERVERSTATE_RUNNING = 0,
    UA_SERVERSTATE_FAILED = 1,
    UA_SERVERSTATE_NOCONFIGURATION = 2,
    UA_SERVERSTATE_SUSPENDED = 3,
    UA_SERVERSTATE_SHUTDOWN = 4,
    UA_SERVERSTATE_TEST = 5,
    UA_SERVERSTATE_COMMUNICATIONFAULT = 6,
    UA_SERVERSTATE_UNKNOWN = 7
} UA_ServerState;

#define UA_TYPES_SERVERSTATE 43
static UA_INLINE void UA_ServerState_init(UA_ServerState *p) { memset(p, 0, sizeof(UA_ServerState)); }
static UA_INLINE UA_ServerState * UA_ServerState_new(void) { return (UA_ServerState*) UA_new(&UA_TYPES[UA_TYPES_SERVERSTATE]); }
static UA_INLINE UA_StatusCode UA_ServerState_copy(const UA_ServerState *src, UA_ServerState *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_ServerState_deleteMembers(UA_ServerState *p) { }
static UA_INLINE void UA_ServerState_delete(UA_ServerState *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SERVERSTATE]); }

/**
 * UnregisterNodesRequest
 * ----------------------
 * Unregisters one or more previously registered nodes. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t nodesToUnregisterSize;
    UA_NodeId *nodesToUnregister;
} UA_UnregisterNodesRequest;

#define UA_TYPES_UNREGISTERNODESREQUEST 44
static UA_INLINE void UA_UnregisterNodesRequest_init(UA_UnregisterNodesRequest *p) { memset(p, 0, sizeof(UA_UnregisterNodesRequest)); }
static UA_INLINE UA_UnregisterNodesRequest * UA_UnregisterNodesRequest_new(void) { return (UA_UnregisterNodesRequest*) UA_new(&UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]); }
static UA_INLINE UA_StatusCode UA_UnregisterNodesRequest_copy(const UA_UnregisterNodesRequest *src, UA_UnregisterNodesRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]); }
static UA_INLINE void UA_UnregisterNodesRequest_deleteMembers(UA_UnregisterNodesRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]); }
static UA_INLINE void UA_UnregisterNodesRequest_delete(UA_UnregisterNodesRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]); }

/**
 * ContentFilterElementResult
 * --------------------------
 */
typedef struct {
    UA_StatusCode statusCode;
    size_t operandStatusCodesSize;
    UA_StatusCode *operandStatusCodes;
    size_t operandDiagnosticInfosSize;
    UA_DiagnosticInfo *operandDiagnosticInfos;
} UA_ContentFilterElementResult;

#define UA_TYPES_CONTENTFILTERELEMENTRESULT 45
static UA_INLINE void UA_ContentFilterElementResult_init(UA_ContentFilterElementResult *p) { memset(p, 0, sizeof(UA_ContentFilterElementResult)); }
static UA_INLINE UA_ContentFilterElementResult * UA_ContentFilterElementResult_new(void) { return (UA_ContentFilterElementResult*) UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]); }
static UA_INLINE UA_StatusCode UA_ContentFilterElementResult_copy(const UA_ContentFilterElementResult *src, UA_ContentFilterElementResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]); }
static UA_INLINE void UA_ContentFilterElementResult_deleteMembers(UA_ContentFilterElementResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]); }
static UA_INLINE void UA_ContentFilterElementResult_delete(UA_ContentFilterElementResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]); }

/**
 * QueryDataSet
 * ------------
 */
typedef struct {
    UA_ExpandedNodeId nodeId;
    UA_ExpandedNodeId typeDefinitionNode;
    size_t valuesSize;
    UA_Variant *values;
} UA_QueryDataSet;

#define UA_TYPES_QUERYDATASET 46
static UA_INLINE void UA_QueryDataSet_init(UA_QueryDataSet *p) { memset(p, 0, sizeof(UA_QueryDataSet)); }
static UA_INLINE UA_QueryDataSet * UA_QueryDataSet_new(void) { return (UA_QueryDataSet*) UA_new(&UA_TYPES[UA_TYPES_QUERYDATASET]); }
static UA_INLINE UA_StatusCode UA_QueryDataSet_copy(const UA_QueryDataSet *src, UA_QueryDataSet *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUERYDATASET]); }
static UA_INLINE void UA_QueryDataSet_deleteMembers(UA_QueryDataSet *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUERYDATASET]); }
static UA_INLINE void UA_QueryDataSet_delete(UA_QueryDataSet *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUERYDATASET]); }

/**
 * SetPublishingModeRequest
 * ------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_Boolean publishingEnabled;
    size_t subscriptionIdsSize;
    UA_UInt32 *subscriptionIds;
} UA_SetPublishingModeRequest;

#define UA_TYPES_SETPUBLISHINGMODEREQUEST 47
static UA_INLINE void UA_SetPublishingModeRequest_init(UA_SetPublishingModeRequest *p) { memset(p, 0, sizeof(UA_SetPublishingModeRequest)); }
static UA_INLINE UA_SetPublishingModeRequest * UA_SetPublishingModeRequest_new(void) { return (UA_SetPublishingModeRequest*) UA_new(&UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]); }
static UA_INLINE UA_StatusCode UA_SetPublishingModeRequest_copy(const UA_SetPublishingModeRequest *src, UA_SetPublishingModeRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]); }
static UA_INLINE void UA_SetPublishingModeRequest_deleteMembers(UA_SetPublishingModeRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]); }
static UA_INLINE void UA_SetPublishingModeRequest_delete(UA_SetPublishingModeRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]); }

/**
 * TimestampsToReturn
 * ------------------
 */
typedef enum { 
    UA_TIMESTAMPSTORETURN_SOURCE = 0,
    UA_TIMESTAMPSTORETURN_SERVER = 1,
    UA_TIMESTAMPSTORETURN_BOTH = 2,
    UA_TIMESTAMPSTORETURN_NEITHER = 3
} UA_TimestampsToReturn;

#define UA_TYPES_TIMESTAMPSTORETURN 48
static UA_INLINE void UA_TimestampsToReturn_init(UA_TimestampsToReturn *p) { memset(p, 0, sizeof(UA_TimestampsToReturn)); }
static UA_INLINE UA_TimestampsToReturn * UA_TimestampsToReturn_new(void) { return (UA_TimestampsToReturn*) UA_new(&UA_TYPES[UA_TYPES_TIMESTAMPSTORETURN]); }
static UA_INLINE UA_StatusCode UA_TimestampsToReturn_copy(const UA_TimestampsToReturn *src, UA_TimestampsToReturn *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_TimestampsToReturn_deleteMembers(UA_TimestampsToReturn *p) { }
static UA_INLINE void UA_TimestampsToReturn_delete(UA_TimestampsToReturn *p) { UA_delete(p, &UA_TYPES[UA_TYPES_TIMESTAMPSTORETURN]); }

/**
 * CallRequest
 * -----------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t methodsToCallSize;
    UA_CallMethodRequest *methodsToCall;
} UA_CallRequest;

#define UA_TYPES_CALLREQUEST 49
static UA_INLINE void UA_CallRequest_init(UA_CallRequest *p) { memset(p, 0, sizeof(UA_CallRequest)); }
static UA_INLINE UA_CallRequest * UA_CallRequest_new(void) { return (UA_CallRequest*) UA_new(&UA_TYPES[UA_TYPES_CALLREQUEST]); }
static UA_INLINE UA_StatusCode UA_CallRequest_copy(const UA_CallRequest *src, UA_CallRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLREQUEST]); }
static UA_INLINE void UA_CallRequest_deleteMembers(UA_CallRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CALLREQUEST]); }
static UA_INLINE void UA_CallRequest_delete(UA_CallRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CALLREQUEST]); }

/**
 * MethodAttributes
 * ----------------
 * The attributes for a method node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Boolean executable;
    UA_Boolean userExecutable;
} UA_MethodAttributes;

#define UA_TYPES_METHODATTRIBUTES 50
static UA_INLINE void UA_MethodAttributes_init(UA_MethodAttributes *p) { memset(p, 0, sizeof(UA_MethodAttributes)); }
static UA_INLINE UA_MethodAttributes * UA_MethodAttributes_new(void) { return (UA_MethodAttributes*) UA_new(&UA_TYPES[UA_TYPES_METHODATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_MethodAttributes_copy(const UA_MethodAttributes *src, UA_MethodAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]); }
static UA_INLINE void UA_MethodAttributes_deleteMembers(UA_MethodAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]); }
static UA_INLINE void UA_MethodAttributes_delete(UA_MethodAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]); }

/**
 * DeleteReferencesItem
 * --------------------
 * A request to delete a node from the server address space. */
typedef struct {
    UA_NodeId sourceNodeId;
    UA_NodeId referenceTypeId;
    UA_Boolean isForward;
    UA_ExpandedNodeId targetNodeId;
    UA_Boolean deleteBidirectional;
} UA_DeleteReferencesItem;

#define UA_TYPES_DELETEREFERENCESITEM 51
static UA_INLINE void UA_DeleteReferencesItem_init(UA_DeleteReferencesItem *p) { memset(p, 0, sizeof(UA_DeleteReferencesItem)); }
static UA_INLINE UA_DeleteReferencesItem * UA_DeleteReferencesItem_new(void) { return (UA_DeleteReferencesItem*) UA_new(&UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]); }
static UA_INLINE UA_StatusCode UA_DeleteReferencesItem_copy(const UA_DeleteReferencesItem *src, UA_DeleteReferencesItem *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]); }
static UA_INLINE void UA_DeleteReferencesItem_deleteMembers(UA_DeleteReferencesItem *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]); }
static UA_INLINE void UA_DeleteReferencesItem_delete(UA_DeleteReferencesItem *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]); }

/**
 * WriteValue
 * ----------
 */
typedef struct {
    UA_NodeId nodeId;
    UA_UInt32 attributeId;
    UA_String indexRange;
    UA_DataValue value;
} UA_WriteValue;

#define UA_TYPES_WRITEVALUE 52
static UA_INLINE void UA_WriteValue_init(UA_WriteValue *p) { memset(p, 0, sizeof(UA_WriteValue)); }
static UA_INLINE UA_WriteValue * UA_WriteValue_new(void) { return (UA_WriteValue*) UA_new(&UA_TYPES[UA_TYPES_WRITEVALUE]); }
static UA_INLINE UA_StatusCode UA_WriteValue_copy(const UA_WriteValue *src, UA_WriteValue *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_WRITEVALUE]); }
static UA_INLINE void UA_WriteValue_deleteMembers(UA_WriteValue *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_WRITEVALUE]); }
static UA_INLINE void UA_WriteValue_delete(UA_WriteValue *p) { UA_delete(p, &UA_TYPES[UA_TYPES_WRITEVALUE]); }

/**
 * MonitoredItemCreateResult
 * -------------------------
 */
typedef struct {
    UA_StatusCode statusCode;
    UA_UInt32 monitoredItemId;
    UA_Double revisedSamplingInterval;
    UA_UInt32 revisedQueueSize;
    UA_ExtensionObject filterResult;
} UA_MonitoredItemCreateResult;

#define UA_TYPES_MONITOREDITEMCREATERESULT 53
static UA_INLINE void UA_MonitoredItemCreateResult_init(UA_MonitoredItemCreateResult *p) { memset(p, 0, sizeof(UA_MonitoredItemCreateResult)); }
static UA_INLINE UA_MonitoredItemCreateResult * UA_MonitoredItemCreateResult_new(void) { return (UA_MonitoredItemCreateResult*) UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]); }
static UA_INLINE UA_StatusCode UA_MonitoredItemCreateResult_copy(const UA_MonitoredItemCreateResult *src, UA_MonitoredItemCreateResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]); }
static UA_INLINE void UA_MonitoredItemCreateResult_deleteMembers(UA_MonitoredItemCreateResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]); }
static UA_INLINE void UA_MonitoredItemCreateResult_delete(UA_MonitoredItemCreateResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]); }

/**
 * MessageSecurityMode
 * -------------------
 * The type of security to use on a message. */
typedef enum { 
    UA_MESSAGESECURITYMODE_INVALID = 0,
    UA_MESSAGESECURITYMODE_NONE = 1,
    UA_MESSAGESECURITYMODE_SIGN = 2,
    UA_MESSAGESECURITYMODE_SIGNANDENCRYPT = 3
} UA_MessageSecurityMode;

#define UA_TYPES_MESSAGESECURITYMODE 54
static UA_INLINE void UA_MessageSecurityMode_init(UA_MessageSecurityMode *p) { memset(p, 0, sizeof(UA_MessageSecurityMode)); }
static UA_INLINE UA_MessageSecurityMode * UA_MessageSecurityMode_new(void) { return (UA_MessageSecurityMode*) UA_new(&UA_TYPES[UA_TYPES_MESSAGESECURITYMODE]); }
static UA_INLINE UA_StatusCode UA_MessageSecurityMode_copy(const UA_MessageSecurityMode *src, UA_MessageSecurityMode *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_MessageSecurityMode_deleteMembers(UA_MessageSecurityMode *p) { }
static UA_INLINE void UA_MessageSecurityMode_delete(UA_MessageSecurityMode *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MESSAGESECURITYMODE]); }

/**
 * MonitoringParameters
 * --------------------
 */
typedef struct {
    UA_UInt32 clientHandle;
    UA_Double samplingInterval;
    UA_ExtensionObject filter;
    UA_UInt32 queueSize;
    UA_Boolean discardOldest;
} UA_MonitoringParameters;

#define UA_TYPES_MONITORINGPARAMETERS 55
static UA_INLINE void UA_MonitoringParameters_init(UA_MonitoringParameters *p) { memset(p, 0, sizeof(UA_MonitoringParameters)); }
static UA_INLINE UA_MonitoringParameters * UA_MonitoringParameters_new(void) { return (UA_MonitoringParameters*) UA_new(&UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]); }
static UA_INLINE UA_StatusCode UA_MonitoringParameters_copy(const UA_MonitoringParameters *src, UA_MonitoringParameters *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]); }
static UA_INLINE void UA_MonitoringParameters_deleteMembers(UA_MonitoringParameters *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]); }
static UA_INLINE void UA_MonitoringParameters_delete(UA_MonitoringParameters *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]); }

/**
 * SignatureData
 * -------------
 * A digital signature. */
typedef struct {
    UA_String algorithm;
    UA_ByteString signature;
} UA_SignatureData;

#define UA_TYPES_SIGNATUREDATA 56
static UA_INLINE void UA_SignatureData_init(UA_SignatureData *p) { memset(p, 0, sizeof(UA_SignatureData)); }
static UA_INLINE UA_SignatureData * UA_SignatureData_new(void) { return (UA_SignatureData*) UA_new(&UA_TYPES[UA_TYPES_SIGNATUREDATA]); }
static UA_INLINE UA_StatusCode UA_SignatureData_copy(const UA_SignatureData *src, UA_SignatureData *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SIGNATUREDATA]); }
static UA_INLINE void UA_SignatureData_deleteMembers(UA_SignatureData *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_SIGNATUREDATA]); }
static UA_INLINE void UA_SignatureData_delete(UA_SignatureData *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SIGNATUREDATA]); }

/**
 * ReferenceNode
 * -------------
 * Specifies a reference which belongs to a node. */
typedef struct {
    UA_NodeId referenceTypeId;
    UA_Boolean isInverse;
    UA_ExpandedNodeId targetId;
} UA_ReferenceNode;

#define UA_TYPES_REFERENCENODE 57
static UA_INLINE void UA_ReferenceNode_init(UA_ReferenceNode *p) { memset(p, 0, sizeof(UA_ReferenceNode)); }
static UA_INLINE UA_ReferenceNode * UA_ReferenceNode_new(void) { return (UA_ReferenceNode*) UA_new(&UA_TYPES[UA_TYPES_REFERENCENODE]); }
static UA_INLINE UA_StatusCode UA_ReferenceNode_copy(const UA_ReferenceNode *src, UA_ReferenceNode *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REFERENCENODE]); }
static UA_INLINE void UA_ReferenceNode_deleteMembers(UA_ReferenceNode *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REFERENCENODE]); }
static UA_INLINE void UA_ReferenceNode_delete(UA_ReferenceNode *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REFERENCENODE]); }

/**
 * Argument
 * --------
 * An argument for a method. */
typedef struct {
    UA_String name;
    UA_NodeId dataType;
    UA_Int32 valueRank;
    size_t arrayDimensionsSize;
    UA_UInt32 *arrayDimensions;
    UA_LocalizedText description;
} UA_Argument;

#define UA_TYPES_ARGUMENT 58
static UA_INLINE void UA_Argument_init(UA_Argument *p) { memset(p, 0, sizeof(UA_Argument)); }
static UA_INLINE UA_Argument * UA_Argument_new(void) { return (UA_Argument*) UA_new(&UA_TYPES[UA_TYPES_ARGUMENT]); }
static UA_INLINE UA_StatusCode UA_Argument_copy(const UA_Argument *src, UA_Argument *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ARGUMENT]); }
static UA_INLINE void UA_Argument_deleteMembers(UA_Argument *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ARGUMENT]); }
static UA_INLINE void UA_Argument_delete(UA_Argument *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ARGUMENT]); }

/**
 * UserIdentityToken
 * -----------------
 * A base type for a user identity token. */
typedef struct {
    UA_String policyId;
} UA_UserIdentityToken;

#define UA_TYPES_USERIDENTITYTOKEN 59
static UA_INLINE void UA_UserIdentityToken_init(UA_UserIdentityToken *p) { memset(p, 0, sizeof(UA_UserIdentityToken)); }
static UA_INLINE UA_UserIdentityToken * UA_UserIdentityToken_new(void) { return (UA_UserIdentityToken*) UA_new(&UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]); }
static UA_INLINE UA_StatusCode UA_UserIdentityToken_copy(const UA_UserIdentityToken *src, UA_UserIdentityToken *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]); }
static UA_INLINE void UA_UserIdentityToken_deleteMembers(UA_UserIdentityToken *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]); }
static UA_INLINE void UA_UserIdentityToken_delete(UA_UserIdentityToken *p) { UA_delete(p, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]); }

/**
 * ObjectTypeAttributes
 * --------------------
 * The attributes for an object type node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Boolean isAbstract;
} UA_ObjectTypeAttributes;

#define UA_TYPES_OBJECTTYPEATTRIBUTES 60
static UA_INLINE void UA_ObjectTypeAttributes_init(UA_ObjectTypeAttributes *p) { memset(p, 0, sizeof(UA_ObjectTypeAttributes)); }
static UA_INLINE UA_ObjectTypeAttributes * UA_ObjectTypeAttributes_new(void) { return (UA_ObjectTypeAttributes*) UA_new(&UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_ObjectTypeAttributes_copy(const UA_ObjectTypeAttributes *src, UA_ObjectTypeAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]); }
static UA_INLINE void UA_ObjectTypeAttributes_deleteMembers(UA_ObjectTypeAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]); }
static UA_INLINE void UA_ObjectTypeAttributes_delete(UA_ObjectTypeAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]); }

/**
 * SecurityTokenRequestType
 * ------------------------
 * Indicates whether a token if being created or renewed. */
typedef enum { 
    UA_SECURITYTOKENREQUESTTYPE_ISSUE = 0,
    UA_SECURITYTOKENREQUESTTYPE_RENEW = 1
} UA_SecurityTokenRequestType;

#define UA_TYPES_SECURITYTOKENREQUESTTYPE 61
static UA_INLINE void UA_SecurityTokenRequestType_init(UA_SecurityTokenRequestType *p) { memset(p, 0, sizeof(UA_SecurityTokenRequestType)); }
static UA_INLINE UA_SecurityTokenRequestType * UA_SecurityTokenRequestType_new(void) { return (UA_SecurityTokenRequestType*) UA_new(&UA_TYPES[UA_TYPES_SECURITYTOKENREQUESTTYPE]); }
static UA_INLINE UA_StatusCode UA_SecurityTokenRequestType_copy(const UA_SecurityTokenRequestType *src, UA_SecurityTokenRequestType *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_SecurityTokenRequestType_deleteMembers(UA_SecurityTokenRequestType *p) { }
static UA_INLINE void UA_SecurityTokenRequestType_delete(UA_SecurityTokenRequestType *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SECURITYTOKENREQUESTTYPE]); }

/**
 * BuildInfo
 * ---------
 */
typedef struct {
    UA_String productUri;
    UA_String manufacturerName;
    UA_String productName;
    UA_String softwareVersion;
    UA_String buildNumber;
    UA_DateTime buildDate;
} UA_BuildInfo;

#define UA_TYPES_BUILDINFO 62
static UA_INLINE void UA_BuildInfo_init(UA_BuildInfo *p) { memset(p, 0, sizeof(UA_BuildInfo)); }
static UA_INLINE UA_BuildInfo * UA_BuildInfo_new(void) { return (UA_BuildInfo*) UA_new(&UA_TYPES[UA_TYPES_BUILDINFO]); }
static UA_INLINE UA_StatusCode UA_BuildInfo_copy(const UA_BuildInfo *src, UA_BuildInfo *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BUILDINFO]); }
static UA_INLINE void UA_BuildInfo_deleteMembers(UA_BuildInfo *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BUILDINFO]); }
static UA_INLINE void UA_BuildInfo_delete(UA_BuildInfo *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BUILDINFO]); }

/**
 * NodeClass
 * ---------
 * A mask specifying the class of the node. */
typedef enum { 
    UA_NODECLASS_UNSPECIFIED = 0,
    UA_NODECLASS_OBJECT = 1,
    UA_NODECLASS_VARIABLE = 2,
    UA_NODECLASS_METHOD = 4,
    UA_NODECLASS_OBJECTTYPE = 8,
    UA_NODECLASS_VARIABLETYPE = 16,
    UA_NODECLASS_REFERENCETYPE = 32,
    UA_NODECLASS_DATATYPE = 64,
    UA_NODECLASS_VIEW = 128
} UA_NodeClass;

#define UA_TYPES_NODECLASS 63
static UA_INLINE void UA_NodeClass_init(UA_NodeClass *p) { memset(p, 0, sizeof(UA_NodeClass)); }
static UA_INLINE UA_NodeClass * UA_NodeClass_new(void) { return (UA_NodeClass*) UA_new(&UA_TYPES[UA_TYPES_NODECLASS]); }
static UA_INLINE UA_StatusCode UA_NodeClass_copy(const UA_NodeClass *src, UA_NodeClass *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_NodeClass_deleteMembers(UA_NodeClass *p) { }
static UA_INLINE void UA_NodeClass_delete(UA_NodeClass *p) { UA_delete(p, &UA_TYPES[UA_TYPES_NODECLASS]); }

/**
 * ChannelSecurityToken
 * --------------------
 * The token that identifies a set of keys for an active secure channel. */
typedef struct {
    UA_UInt32 channelId;
    UA_UInt32 tokenId;
    UA_DateTime createdAt;
    UA_UInt32 revisedLifetime;
} UA_ChannelSecurityToken;

#define UA_TYPES_CHANNELSECURITYTOKEN 64
static UA_INLINE void UA_ChannelSecurityToken_init(UA_ChannelSecurityToken *p) { memset(p, 0, sizeof(UA_ChannelSecurityToken)); }
static UA_INLINE UA_ChannelSecurityToken * UA_ChannelSecurityToken_new(void) { return (UA_ChannelSecurityToken*) UA_new(&UA_TYPES[UA_TYPES_CHANNELSECURITYTOKEN]); }
static UA_INLINE UA_StatusCode UA_ChannelSecurityToken_copy(const UA_ChannelSecurityToken *src, UA_ChannelSecurityToken *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_ChannelSecurityToken_deleteMembers(UA_ChannelSecurityToken *p) { }
static UA_INLINE void UA_ChannelSecurityToken_delete(UA_ChannelSecurityToken *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CHANNELSECURITYTOKEN]); }

/**
 * MonitoredItemNotification
 * -------------------------
 */
typedef struct {
    UA_UInt32 clientHandle;
    UA_DataValue value;
} UA_MonitoredItemNotification;

#define UA_TYPES_MONITOREDITEMNOTIFICATION 65
static UA_INLINE void UA_MonitoredItemNotification_init(UA_MonitoredItemNotification *p) { memset(p, 0, sizeof(UA_MonitoredItemNotification)); }
static UA_INLINE UA_MonitoredItemNotification * UA_MonitoredItemNotification_new(void) { return (UA_MonitoredItemNotification*) UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]); }
static UA_INLINE UA_StatusCode UA_MonitoredItemNotification_copy(const UA_MonitoredItemNotification *src, UA_MonitoredItemNotification *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]); }
static UA_INLINE void UA_MonitoredItemNotification_deleteMembers(UA_MonitoredItemNotification *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]); }
static UA_INLINE void UA_MonitoredItemNotification_delete(UA_MonitoredItemNotification *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]); }

/**
 * DeleteNodesItem
 * ---------------
 * A request to delete a node to the server address space. */
typedef struct {
    UA_NodeId nodeId;
    UA_Boolean deleteTargetReferences;
} UA_DeleteNodesItem;

#define UA_TYPES_DELETENODESITEM 66
static UA_INLINE void UA_DeleteNodesItem_init(UA_DeleteNodesItem *p) { memset(p, 0, sizeof(UA_DeleteNodesItem)); }
static UA_INLINE UA_DeleteNodesItem * UA_DeleteNodesItem_new(void) { return (UA_DeleteNodesItem*) UA_new(&UA_TYPES[UA_TYPES_DELETENODESITEM]); }
static UA_INLINE UA_StatusCode UA_DeleteNodesItem_copy(const UA_DeleteNodesItem *src, UA_DeleteNodesItem *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETENODESITEM]); }
static UA_INLINE void UA_DeleteNodesItem_deleteMembers(UA_DeleteNodesItem *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETENODESITEM]); }
static UA_INLINE void UA_DeleteNodesItem_delete(UA_DeleteNodesItem *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETENODESITEM]); }

/**
 * SubscriptionAcknowledgement
 * ---------------------------
 */
typedef struct {
    UA_UInt32 subscriptionId;
    UA_UInt32 sequenceNumber;
} UA_SubscriptionAcknowledgement;

#define UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT 67
static UA_INLINE void UA_SubscriptionAcknowledgement_init(UA_SubscriptionAcknowledgement *p) { memset(p, 0, sizeof(UA_SubscriptionAcknowledgement)); }
static UA_INLINE UA_SubscriptionAcknowledgement * UA_SubscriptionAcknowledgement_new(void) { return (UA_SubscriptionAcknowledgement*) UA_new(&UA_TYPES[UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT]); }
static UA_INLINE UA_StatusCode UA_SubscriptionAcknowledgement_copy(const UA_SubscriptionAcknowledgement *src, UA_SubscriptionAcknowledgement *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_SubscriptionAcknowledgement_deleteMembers(UA_SubscriptionAcknowledgement *p) { }
static UA_INLINE void UA_SubscriptionAcknowledgement_delete(UA_SubscriptionAcknowledgement *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT]); }

/**
 * ReadValueId
 * -----------
 */
typedef struct {
    UA_NodeId nodeId;
    UA_UInt32 attributeId;
    UA_String indexRange;
    UA_QualifiedName dataEncoding;
} UA_ReadValueId;

#define UA_TYPES_READVALUEID 68
static UA_INLINE void UA_ReadValueId_init(UA_ReadValueId *p) { memset(p, 0, sizeof(UA_ReadValueId)); }
static UA_INLINE UA_ReadValueId * UA_ReadValueId_new(void) { return (UA_ReadValueId*) UA_new(&UA_TYPES[UA_TYPES_READVALUEID]); }
static UA_INLINE UA_StatusCode UA_ReadValueId_copy(const UA_ReadValueId *src, UA_ReadValueId *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_READVALUEID]); }
static UA_INLINE void UA_ReadValueId_deleteMembers(UA_ReadValueId *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_READVALUEID]); }
static UA_INLINE void UA_ReadValueId_delete(UA_ReadValueId *p) { UA_delete(p, &UA_TYPES[UA_TYPES_READVALUEID]); }

/**
 * AnonymousIdentityToken
 * ----------------------
 * A token representing an anonymous user. */
typedef struct {
    UA_String policyId;
} UA_AnonymousIdentityToken;

#define UA_TYPES_ANONYMOUSIDENTITYTOKEN 69
static UA_INLINE void UA_AnonymousIdentityToken_init(UA_AnonymousIdentityToken *p) { memset(p, 0, sizeof(UA_AnonymousIdentityToken)); }
static UA_INLINE UA_AnonymousIdentityToken * UA_AnonymousIdentityToken_new(void) { return (UA_AnonymousIdentityToken*) UA_new(&UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]); }
static UA_INLINE UA_StatusCode UA_AnonymousIdentityToken_copy(const UA_AnonymousIdentityToken *src, UA_AnonymousIdentityToken *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]); }
static UA_INLINE void UA_AnonymousIdentityToken_deleteMembers(UA_AnonymousIdentityToken *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]); }
static UA_INLINE void UA_AnonymousIdentityToken_delete(UA_AnonymousIdentityToken *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]); }

/**
 * DataTypeAttributes
 * ------------------
 * The attributes for a data type node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Boolean isAbstract;
} UA_DataTypeAttributes;

#define UA_TYPES_DATATYPEATTRIBUTES 70
static UA_INLINE void UA_DataTypeAttributes_init(UA_DataTypeAttributes *p) { memset(p, 0, sizeof(UA_DataTypeAttributes)); }
static UA_INLINE UA_DataTypeAttributes * UA_DataTypeAttributes_new(void) { return (UA_DataTypeAttributes*) UA_new(&UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_DataTypeAttributes_copy(const UA_DataTypeAttributes *src, UA_DataTypeAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]); }
static UA_INLINE void UA_DataTypeAttributes_deleteMembers(UA_DataTypeAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]); }
static UA_INLINE void UA_DataTypeAttributes_delete(UA_DataTypeAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]); }

/**
 * ResponseHeader
 * --------------
 * The header passed with every server response. */
typedef struct {
    UA_DateTime timestamp;
    UA_UInt32 requestHandle;
    UA_StatusCode serviceResult;
    UA_DiagnosticInfo serviceDiagnostics;
    size_t stringTableSize;
    UA_String *stringTable;
    UA_ExtensionObject additionalHeader;
} UA_ResponseHeader;

#define UA_TYPES_RESPONSEHEADER 71
static UA_INLINE void UA_ResponseHeader_init(UA_ResponseHeader *p) { memset(p, 0, sizeof(UA_ResponseHeader)); }
static UA_INLINE UA_ResponseHeader * UA_ResponseHeader_new(void) { return (UA_ResponseHeader*) UA_new(&UA_TYPES[UA_TYPES_RESPONSEHEADER]); }
static UA_INLINE UA_StatusCode UA_ResponseHeader_copy(const UA_ResponseHeader *src, UA_ResponseHeader *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_RESPONSEHEADER]); }
static UA_INLINE void UA_ResponseHeader_deleteMembers(UA_ResponseHeader *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_RESPONSEHEADER]); }
static UA_INLINE void UA_ResponseHeader_delete(UA_ResponseHeader *p) { UA_delete(p, &UA_TYPES[UA_TYPES_RESPONSEHEADER]); }

/**
 * DeleteSubscriptionsRequest
 * --------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t subscriptionIdsSize;
    UA_UInt32 *subscriptionIds;
} UA_DeleteSubscriptionsRequest;

#define UA_TYPES_DELETESUBSCRIPTIONSREQUEST 72
static UA_INLINE void UA_DeleteSubscriptionsRequest_init(UA_DeleteSubscriptionsRequest *p) { memset(p, 0, sizeof(UA_DeleteSubscriptionsRequest)); }
static UA_INLINE UA_DeleteSubscriptionsRequest * UA_DeleteSubscriptionsRequest_new(void) { return (UA_DeleteSubscriptionsRequest*) UA_new(&UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]); }
static UA_INLINE UA_StatusCode UA_DeleteSubscriptionsRequest_copy(const UA_DeleteSubscriptionsRequest *src, UA_DeleteSubscriptionsRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]); }
static UA_INLINE void UA_DeleteSubscriptionsRequest_deleteMembers(UA_DeleteSubscriptionsRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]); }
static UA_INLINE void UA_DeleteSubscriptionsRequest_delete(UA_DeleteSubscriptionsRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]); }

/**
 * DataChangeNotification
 * ----------------------
 */
typedef struct {
    size_t monitoredItemsSize;
    UA_MonitoredItemNotification *monitoredItems;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_DataChangeNotification;

#define UA_TYPES_DATACHANGENOTIFICATION 73
static UA_INLINE void UA_DataChangeNotification_init(UA_DataChangeNotification *p) { memset(p, 0, sizeof(UA_DataChangeNotification)); }
static UA_INLINE UA_DataChangeNotification * UA_DataChangeNotification_new(void) { return (UA_DataChangeNotification*) UA_new(&UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]); }
static UA_INLINE UA_StatusCode UA_DataChangeNotification_copy(const UA_DataChangeNotification *src, UA_DataChangeNotification *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]); }
static UA_INLINE void UA_DataChangeNotification_deleteMembers(UA_DataChangeNotification *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]); }
static UA_INLINE void UA_DataChangeNotification_delete(UA_DataChangeNotification *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]); }

/**
 * DeleteMonitoredItemsResponse
 * ----------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_DeleteMonitoredItemsResponse;

#define UA_TYPES_DELETEMONITOREDITEMSRESPONSE 74
static UA_INLINE void UA_DeleteMonitoredItemsResponse_init(UA_DeleteMonitoredItemsResponse *p) { memset(p, 0, sizeof(UA_DeleteMonitoredItemsResponse)); }
static UA_INLINE UA_DeleteMonitoredItemsResponse * UA_DeleteMonitoredItemsResponse_new(void) { return (UA_DeleteMonitoredItemsResponse*) UA_new(&UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_DeleteMonitoredItemsResponse_copy(const UA_DeleteMonitoredItemsResponse *src, UA_DeleteMonitoredItemsResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]); }
static UA_INLINE void UA_DeleteMonitoredItemsResponse_deleteMembers(UA_DeleteMonitoredItemsResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]); }
static UA_INLINE void UA_DeleteMonitoredItemsResponse_delete(UA_DeleteMonitoredItemsResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]); }

/**
 * RelativePath
 * ------------
 * A relative path constructed from reference types and browse names. */
typedef struct {
    size_t elementsSize;
    UA_RelativePathElement *elements;
} UA_RelativePath;

#define UA_TYPES_RELATIVEPATH 75
static UA_INLINE void UA_RelativePath_init(UA_RelativePath *p) { memset(p, 0, sizeof(UA_RelativePath)); }
static UA_INLINE UA_RelativePath * UA_RelativePath_new(void) { return (UA_RelativePath*) UA_new(&UA_TYPES[UA_TYPES_RELATIVEPATH]); }
static UA_INLINE UA_StatusCode UA_RelativePath_copy(const UA_RelativePath *src, UA_RelativePath *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_RELATIVEPATH]); }
static UA_INLINE void UA_RelativePath_deleteMembers(UA_RelativePath *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_RELATIVEPATH]); }
static UA_INLINE void UA_RelativePath_delete(UA_RelativePath *p) { UA_delete(p, &UA_TYPES[UA_TYPES_RELATIVEPATH]); }

/**
 * RegisterNodesRequest
 * --------------------
 * Registers one or more nodes for repeated use within a session. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t nodesToRegisterSize;
    UA_NodeId *nodesToRegister;
} UA_RegisterNodesRequest;

#define UA_TYPES_REGISTERNODESREQUEST 76
static UA_INLINE void UA_RegisterNodesRequest_init(UA_RegisterNodesRequest *p) { memset(p, 0, sizeof(UA_RegisterNodesRequest)); }
static UA_INLINE UA_RegisterNodesRequest * UA_RegisterNodesRequest_new(void) { return (UA_RegisterNodesRequest*) UA_new(&UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]); }
static UA_INLINE UA_StatusCode UA_RegisterNodesRequest_copy(const UA_RegisterNodesRequest *src, UA_RegisterNodesRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]); }
static UA_INLINE void UA_RegisterNodesRequest_deleteMembers(UA_RegisterNodesRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]); }
static UA_INLINE void UA_RegisterNodesRequest_delete(UA_RegisterNodesRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]); }

/**
 * DeleteNodesRequest
 * ------------------
 * Delete one or more nodes from the server address space. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t nodesToDeleteSize;
    UA_DeleteNodesItem *nodesToDelete;
} UA_DeleteNodesRequest;

#define UA_TYPES_DELETENODESREQUEST 77
static UA_INLINE void UA_DeleteNodesRequest_init(UA_DeleteNodesRequest *p) { memset(p, 0, sizeof(UA_DeleteNodesRequest)); }
static UA_INLINE UA_DeleteNodesRequest * UA_DeleteNodesRequest_new(void) { return (UA_DeleteNodesRequest*) UA_new(&UA_TYPES[UA_TYPES_DELETENODESREQUEST]); }
static UA_INLINE UA_StatusCode UA_DeleteNodesRequest_copy(const UA_DeleteNodesRequest *src, UA_DeleteNodesRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]); }
static UA_INLINE void UA_DeleteNodesRequest_deleteMembers(UA_DeleteNodesRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]); }
static UA_INLINE void UA_DeleteNodesRequest_delete(UA_DeleteNodesRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]); }

/**
 * PublishResponse
 * ---------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_UInt32 subscriptionId;
    size_t availableSequenceNumbersSize;
    UA_UInt32 *availableSequenceNumbers;
    UA_Boolean moreNotifications;
    UA_NotificationMessage notificationMessage;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_PublishResponse;

#define UA_TYPES_PUBLISHRESPONSE 78
static UA_INLINE void UA_PublishResponse_init(UA_PublishResponse *p) { memset(p, 0, sizeof(UA_PublishResponse)); }
static UA_INLINE UA_PublishResponse * UA_PublishResponse_new(void) { return (UA_PublishResponse*) UA_new(&UA_TYPES[UA_TYPES_PUBLISHRESPONSE]); }
static UA_INLINE UA_StatusCode UA_PublishResponse_copy(const UA_PublishResponse *src, UA_PublishResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]); }
static UA_INLINE void UA_PublishResponse_deleteMembers(UA_PublishResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]); }
static UA_INLINE void UA_PublishResponse_delete(UA_PublishResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]); }

/**
 * MonitoredItemModifyRequest
 * --------------------------
 */
typedef struct {
    UA_UInt32 monitoredItemId;
    UA_MonitoringParameters requestedParameters;
} UA_MonitoredItemModifyRequest;

#define UA_TYPES_MONITOREDITEMMODIFYREQUEST 79
static UA_INLINE void UA_MonitoredItemModifyRequest_init(UA_MonitoredItemModifyRequest *p) { memset(p, 0, sizeof(UA_MonitoredItemModifyRequest)); }
static UA_INLINE UA_MonitoredItemModifyRequest * UA_MonitoredItemModifyRequest_new(void) { return (UA_MonitoredItemModifyRequest*) UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]); }
static UA_INLINE UA_StatusCode UA_MonitoredItemModifyRequest_copy(const UA_MonitoredItemModifyRequest *src, UA_MonitoredItemModifyRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]); }
static UA_INLINE void UA_MonitoredItemModifyRequest_deleteMembers(UA_MonitoredItemModifyRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]); }
static UA_INLINE void UA_MonitoredItemModifyRequest_delete(UA_MonitoredItemModifyRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]); }

/**
 * UserNameIdentityToken
 * ---------------------
 * A token representing a user identified by a user name and password. */
typedef struct {
    UA_String policyId;
    UA_String userName;
    UA_ByteString password;
    UA_String encryptionAlgorithm;
} UA_UserNameIdentityToken;

#define UA_TYPES_USERNAMEIDENTITYTOKEN 80
static UA_INLINE void UA_UserNameIdentityToken_init(UA_UserNameIdentityToken *p) { memset(p, 0, sizeof(UA_UserNameIdentityToken)); }
static UA_INLINE UA_UserNameIdentityToken * UA_UserNameIdentityToken_new(void) { return (UA_UserNameIdentityToken*) UA_new(&UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]); }
static UA_INLINE UA_StatusCode UA_UserNameIdentityToken_copy(const UA_UserNameIdentityToken *src, UA_UserNameIdentityToken *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]); }
static UA_INLINE void UA_UserNameIdentityToken_deleteMembers(UA_UserNameIdentityToken *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]); }
static UA_INLINE void UA_UserNameIdentityToken_delete(UA_UserNameIdentityToken *p) { UA_delete(p, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]); }

/**
 * IdType
 * ------
 * The type of identifier used in a node id. */
typedef enum { 
    UA_IDTYPE_NUMERIC = 0,
    UA_IDTYPE_STRING = 1,
    UA_IDTYPE_GUID = 2,
    UA_IDTYPE_OPAQUE = 3
} UA_IdType;

#define UA_TYPES_IDTYPE 81
static UA_INLINE void UA_IdType_init(UA_IdType *p) { memset(p, 0, sizeof(UA_IdType)); }
static UA_INLINE UA_IdType * UA_IdType_new(void) { return (UA_IdType*) UA_new(&UA_TYPES[UA_TYPES_IDTYPE]); }
static UA_INLINE UA_StatusCode UA_IdType_copy(const UA_IdType *src, UA_IdType *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_IdType_deleteMembers(UA_IdType *p) { }
static UA_INLINE void UA_IdType_delete(UA_IdType *p) { UA_delete(p, &UA_TYPES[UA_TYPES_IDTYPE]); }

/**
 * UserTokenType
 * -------------
 * The possible user token types. */
typedef enum { 
    UA_USERTOKENTYPE_ANONYMOUS = 0,
    UA_USERTOKENTYPE_USERNAME = 1,
    UA_USERTOKENTYPE_CERTIFICATE = 2,
    UA_USERTOKENTYPE_ISSUEDTOKEN = 3
} UA_UserTokenType;

#define UA_TYPES_USERTOKENTYPE 82
static UA_INLINE void UA_UserTokenType_init(UA_UserTokenType *p) { memset(p, 0, sizeof(UA_UserTokenType)); }
static UA_INLINE UA_UserTokenType * UA_UserTokenType_new(void) { return (UA_UserTokenType*) UA_new(&UA_TYPES[UA_TYPES_USERTOKENTYPE]); }
static UA_INLINE UA_StatusCode UA_UserTokenType_copy(const UA_UserTokenType *src, UA_UserTokenType *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_UserTokenType_deleteMembers(UA_UserTokenType *p) { }
static UA_INLINE void UA_UserTokenType_delete(UA_UserTokenType *p) { UA_delete(p, &UA_TYPES[UA_TYPES_USERTOKENTYPE]); }

/**
 * NodeAttributes
 * --------------
 * The base attributes for all nodes. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
} UA_NodeAttributes;

#define UA_TYPES_NODEATTRIBUTES 83
static UA_INLINE void UA_NodeAttributes_init(UA_NodeAttributes *p) { memset(p, 0, sizeof(UA_NodeAttributes)); }
static UA_INLINE UA_NodeAttributes * UA_NodeAttributes_new(void) { return (UA_NodeAttributes*) UA_new(&UA_TYPES[UA_TYPES_NODEATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_NodeAttributes_copy(const UA_NodeAttributes *src, UA_NodeAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]); }
static UA_INLINE void UA_NodeAttributes_deleteMembers(UA_NodeAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]); }
static UA_INLINE void UA_NodeAttributes_delete(UA_NodeAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]); }

/**
 * ActivateSessionRequest
 * ----------------------
 * Activates a session with the server. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_SignatureData clientSignature;
    size_t clientSoftwareCertificatesSize;
    UA_SignedSoftwareCertificate *clientSoftwareCertificates;
    size_t localeIdsSize;
    UA_String *localeIds;
    UA_ExtensionObject userIdentityToken;
    UA_SignatureData userTokenSignature;
} UA_ActivateSessionRequest;

#define UA_TYPES_ACTIVATESESSIONREQUEST 84
static UA_INLINE void UA_ActivateSessionRequest_init(UA_ActivateSessionRequest *p) { memset(p, 0, sizeof(UA_ActivateSessionRequest)); }
static UA_INLINE UA_ActivateSessionRequest * UA_ActivateSessionRequest_new(void) { return (UA_ActivateSessionRequest*) UA_new(&UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]); }
static UA_INLINE UA_StatusCode UA_ActivateSessionRequest_copy(const UA_ActivateSessionRequest *src, UA_ActivateSessionRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]); }
static UA_INLINE void UA_ActivateSessionRequest_deleteMembers(UA_ActivateSessionRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]); }
static UA_INLINE void UA_ActivateSessionRequest_delete(UA_ActivateSessionRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]); }

/**
 * OpenSecureChannelResponse
 * -------------------------
 * Creates a secure channel with a server. */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_UInt32 serverProtocolVersion;
    UA_ChannelSecurityToken securityToken;
    UA_ByteString serverNonce;
} UA_OpenSecureChannelResponse;

#define UA_TYPES_OPENSECURECHANNELRESPONSE 85
static UA_INLINE void UA_OpenSecureChannelResponse_init(UA_OpenSecureChannelResponse *p) { memset(p, 0, sizeof(UA_OpenSecureChannelResponse)); }
static UA_INLINE UA_OpenSecureChannelResponse * UA_OpenSecureChannelResponse_new(void) { return (UA_OpenSecureChannelResponse*) UA_new(&UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]); }
static UA_INLINE UA_StatusCode UA_OpenSecureChannelResponse_copy(const UA_OpenSecureChannelResponse *src, UA_OpenSecureChannelResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]); }
static UA_INLINE void UA_OpenSecureChannelResponse_deleteMembers(UA_OpenSecureChannelResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]); }
static UA_INLINE void UA_OpenSecureChannelResponse_delete(UA_OpenSecureChannelResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]); }

/**
 * ApplicationType
 * ---------------
 * The types of applications. */
typedef enum { 
    UA_APPLICATIONTYPE_SERVER = 0,
    UA_APPLICATIONTYPE_CLIENT = 1,
    UA_APPLICATIONTYPE_CLIENTANDSERVER = 2,
    UA_APPLICATIONTYPE_DISCOVERYSERVER = 3
} UA_ApplicationType;

#define UA_TYPES_APPLICATIONTYPE 86
static UA_INLINE void UA_ApplicationType_init(UA_ApplicationType *p) { memset(p, 0, sizeof(UA_ApplicationType)); }
static UA_INLINE UA_ApplicationType * UA_ApplicationType_new(void) { return (UA_ApplicationType*) UA_new(&UA_TYPES[UA_TYPES_APPLICATIONTYPE]); }
static UA_INLINE UA_StatusCode UA_ApplicationType_copy(const UA_ApplicationType *src, UA_ApplicationType *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_ApplicationType_deleteMembers(UA_ApplicationType *p) { }
static UA_INLINE void UA_ApplicationType_delete(UA_ApplicationType *p) { UA_delete(p, &UA_TYPES[UA_TYPES_APPLICATIONTYPE]); }

/**
 * QueryNextResponse
 * -----------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t queryDataSetsSize;
    UA_QueryDataSet *queryDataSets;
    UA_ByteString revisedContinuationPoint;
} UA_QueryNextResponse;

#define UA_TYPES_QUERYNEXTRESPONSE 87
static UA_INLINE void UA_QueryNextResponse_init(UA_QueryNextResponse *p) { memset(p, 0, sizeof(UA_QueryNextResponse)); }
static UA_INLINE UA_QueryNextResponse * UA_QueryNextResponse_new(void) { return (UA_QueryNextResponse*) UA_new(&UA_TYPES[UA_TYPES_QUERYNEXTRESPONSE]); }
static UA_INLINE UA_StatusCode UA_QueryNextResponse_copy(const UA_QueryNextResponse *src, UA_QueryNextResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUERYNEXTRESPONSE]); }
static UA_INLINE void UA_QueryNextResponse_deleteMembers(UA_QueryNextResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUERYNEXTRESPONSE]); }
static UA_INLINE void UA_QueryNextResponse_delete(UA_QueryNextResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUERYNEXTRESPONSE]); }

/**
 * ActivateSessionResponse
 * -----------------------
 * Activates a session with the server. */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_ByteString serverNonce;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_ActivateSessionResponse;

#define UA_TYPES_ACTIVATESESSIONRESPONSE 88
static UA_INLINE void UA_ActivateSessionResponse_init(UA_ActivateSessionResponse *p) { memset(p, 0, sizeof(UA_ActivateSessionResponse)); }
static UA_INLINE UA_ActivateSessionResponse * UA_ActivateSessionResponse_new(void) { return (UA_ActivateSessionResponse*) UA_new(&UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]); }
static UA_INLINE UA_StatusCode UA_ActivateSessionResponse_copy(const UA_ActivateSessionResponse *src, UA_ActivateSessionResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]); }
static UA_INLINE void UA_ActivateSessionResponse_deleteMembers(UA_ActivateSessionResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]); }
static UA_INLINE void UA_ActivateSessionResponse_delete(UA_ActivateSessionResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]); }

/**
 * FilterOperator
 * --------------
 */
typedef enum { 
    UA_FILTEROPERATOR_EQUALS = 0,
    UA_FILTEROPERATOR_ISNULL = 1,
    UA_FILTEROPERATOR_GREATERTHAN = 2,
    UA_FILTEROPERATOR_LESSTHAN = 3,
    UA_FILTEROPERATOR_GREATERTHANOREQUAL = 4,
    UA_FILTEROPERATOR_LESSTHANOREQUAL = 5,
    UA_FILTEROPERATOR_LIKE = 6,
    UA_FILTEROPERATOR_NOT = 7,
    UA_FILTEROPERATOR_BETWEEN = 8,
    UA_FILTEROPERATOR_INLIST = 9,
    UA_FILTEROPERATOR_AND = 10,
    UA_FILTEROPERATOR_OR = 11,
    UA_FILTEROPERATOR_CAST = 12,
    UA_FILTEROPERATOR_INVIEW = 13,
    UA_FILTEROPERATOR_OFTYPE = 14,
    UA_FILTEROPERATOR_RELATEDTO = 15,
    UA_FILTEROPERATOR_BITWISEAND = 16,
    UA_FILTEROPERATOR_BITWISEOR = 17
} UA_FilterOperator;

#define UA_TYPES_FILTEROPERATOR 89
static UA_INLINE void UA_FilterOperator_init(UA_FilterOperator *p) { memset(p, 0, sizeof(UA_FilterOperator)); }
static UA_INLINE UA_FilterOperator * UA_FilterOperator_new(void) { return (UA_FilterOperator*) UA_new(&UA_TYPES[UA_TYPES_FILTEROPERATOR]); }
static UA_INLINE UA_StatusCode UA_FilterOperator_copy(const UA_FilterOperator *src, UA_FilterOperator *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_FilterOperator_deleteMembers(UA_FilterOperator *p) { }
static UA_INLINE void UA_FilterOperator_delete(UA_FilterOperator *p) { UA_delete(p, &UA_TYPES[UA_TYPES_FILTEROPERATOR]); }

/**
 * QueryNextRequest
 * ----------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_Boolean releaseContinuationPoint;
    UA_ByteString continuationPoint;
} UA_QueryNextRequest;

#define UA_TYPES_QUERYNEXTREQUEST 90
static UA_INLINE void UA_QueryNextRequest_init(UA_QueryNextRequest *p) { memset(p, 0, sizeof(UA_QueryNextRequest)); }
static UA_INLINE UA_QueryNextRequest * UA_QueryNextRequest_new(void) { return (UA_QueryNextRequest*) UA_new(&UA_TYPES[UA_TYPES_QUERYNEXTREQUEST]); }
static UA_INLINE UA_StatusCode UA_QueryNextRequest_copy(const UA_QueryNextRequest *src, UA_QueryNextRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUERYNEXTREQUEST]); }
static UA_INLINE void UA_QueryNextRequest_deleteMembers(UA_QueryNextRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUERYNEXTREQUEST]); }
static UA_INLINE void UA_QueryNextRequest_delete(UA_QueryNextRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUERYNEXTREQUEST]); }

/**
 * BrowseNextRequest
 * -----------------
 * Continues one or more browse operations. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_Boolean releaseContinuationPoints;
    size_t continuationPointsSize;
    UA_ByteString *continuationPoints;
} UA_BrowseNextRequest;

#define UA_TYPES_BROWSENEXTREQUEST 91
static UA_INLINE void UA_BrowseNextRequest_init(UA_BrowseNextRequest *p) { memset(p, 0, sizeof(UA_BrowseNextRequest)); }
static UA_INLINE UA_BrowseNextRequest * UA_BrowseNextRequest_new(void) { return (UA_BrowseNextRequest*) UA_new(&UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]); }
static UA_INLINE UA_StatusCode UA_BrowseNextRequest_copy(const UA_BrowseNextRequest *src, UA_BrowseNextRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]); }
static UA_INLINE void UA_BrowseNextRequest_deleteMembers(UA_BrowseNextRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]); }
static UA_INLINE void UA_BrowseNextRequest_delete(UA_BrowseNextRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]); }

/**
 * CreateSubscriptionRequest
 * -------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_Double requestedPublishingInterval;
    UA_UInt32 requestedLifetimeCount;
    UA_UInt32 requestedMaxKeepAliveCount;
    UA_UInt32 maxNotificationsPerPublish;
    UA_Boolean publishingEnabled;
    UA_Byte priority;
} UA_CreateSubscriptionRequest;

#define UA_TYPES_CREATESUBSCRIPTIONREQUEST 92
static UA_INLINE void UA_CreateSubscriptionRequest_init(UA_CreateSubscriptionRequest *p) { memset(p, 0, sizeof(UA_CreateSubscriptionRequest)); }
static UA_INLINE UA_CreateSubscriptionRequest * UA_CreateSubscriptionRequest_new(void) { return (UA_CreateSubscriptionRequest*) UA_new(&UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]); }
static UA_INLINE UA_StatusCode UA_CreateSubscriptionRequest_copy(const UA_CreateSubscriptionRequest *src, UA_CreateSubscriptionRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]); }
static UA_INLINE void UA_CreateSubscriptionRequest_deleteMembers(UA_CreateSubscriptionRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]); }
static UA_INLINE void UA_CreateSubscriptionRequest_delete(UA_CreateSubscriptionRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]); }

/**
 * VariableTypeAttributes
 * ----------------------
 * The attributes for a variable type node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Variant value;
    UA_NodeId dataType;
    UA_Int32 valueRank;
    size_t arrayDimensionsSize;
    UA_UInt32 *arrayDimensions;
    UA_Boolean isAbstract;
} UA_VariableTypeAttributes;

#define UA_TYPES_VARIABLETYPEATTRIBUTES 93
static UA_INLINE void UA_VariableTypeAttributes_init(UA_VariableTypeAttributes *p) { memset(p, 0, sizeof(UA_VariableTypeAttributes)); }
static UA_INLINE UA_VariableTypeAttributes * UA_VariableTypeAttributes_new(void) { return (UA_VariableTypeAttributes*) UA_new(&UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_VariableTypeAttributes_copy(const UA_VariableTypeAttributes *src, UA_VariableTypeAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]); }
static UA_INLINE void UA_VariableTypeAttributes_deleteMembers(UA_VariableTypeAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]); }
static UA_INLINE void UA_VariableTypeAttributes_delete(UA_VariableTypeAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]); }

/**
 * BrowsePathResult
 * ----------------
 * The result of a translate opearation. */
typedef struct {
    UA_StatusCode statusCode;
    size_t targetsSize;
    UA_BrowsePathTarget *targets;
} UA_BrowsePathResult;

#define UA_TYPES_BROWSEPATHRESULT 94
static UA_INLINE void UA_BrowsePathResult_init(UA_BrowsePathResult *p) { memset(p, 0, sizeof(UA_BrowsePathResult)); }
static UA_INLINE UA_BrowsePathResult * UA_BrowsePathResult_new(void) { return (UA_BrowsePathResult*) UA_new(&UA_TYPES[UA_TYPES_BROWSEPATHRESULT]); }
static UA_INLINE UA_StatusCode UA_BrowsePathResult_copy(const UA_BrowsePathResult *src, UA_BrowsePathResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]); }
static UA_INLINE void UA_BrowsePathResult_deleteMembers(UA_BrowsePathResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]); }
static UA_INLINE void UA_BrowsePathResult_delete(UA_BrowsePathResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]); }

/**
 * ModifySubscriptionResponse
 * --------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_Double revisedPublishingInterval;
    UA_UInt32 revisedLifetimeCount;
    UA_UInt32 revisedMaxKeepAliveCount;
} UA_ModifySubscriptionResponse;

#define UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE 95
static UA_INLINE void UA_ModifySubscriptionResponse_init(UA_ModifySubscriptionResponse *p) { memset(p, 0, sizeof(UA_ModifySubscriptionResponse)); }
static UA_INLINE UA_ModifySubscriptionResponse * UA_ModifySubscriptionResponse_new(void) { return (UA_ModifySubscriptionResponse*) UA_new(&UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]); }
static UA_INLINE UA_StatusCode UA_ModifySubscriptionResponse_copy(const UA_ModifySubscriptionResponse *src, UA_ModifySubscriptionResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]); }
static UA_INLINE void UA_ModifySubscriptionResponse_deleteMembers(UA_ModifySubscriptionResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]); }
static UA_INLINE void UA_ModifySubscriptionResponse_delete(UA_ModifySubscriptionResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]); }

/**
 * RegisterNodesResponse
 * ---------------------
 * Registers one or more nodes for repeated use within a session. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t registeredNodeIdsSize;
    UA_NodeId *registeredNodeIds;
} UA_RegisterNodesResponse;

#define UA_TYPES_REGISTERNODESRESPONSE 96
static UA_INLINE void UA_RegisterNodesResponse_init(UA_RegisterNodesResponse *p) { memset(p, 0, sizeof(UA_RegisterNodesResponse)); }
static UA_INLINE UA_RegisterNodesResponse * UA_RegisterNodesResponse_new(void) { return (UA_RegisterNodesResponse*) UA_new(&UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]); }
static UA_INLINE UA_StatusCode UA_RegisterNodesResponse_copy(const UA_RegisterNodesResponse *src, UA_RegisterNodesResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]); }
static UA_INLINE void UA_RegisterNodesResponse_deleteMembers(UA_RegisterNodesResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]); }
static UA_INLINE void UA_RegisterNodesResponse_delete(UA_RegisterNodesResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]); }

/**
 * CloseSessionRequest
 * -------------------
 * Closes a session with the server. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_Boolean deleteSubscriptions;
} UA_CloseSessionRequest;

#define UA_TYPES_CLOSESESSIONREQUEST 97
static UA_INLINE void UA_CloseSessionRequest_init(UA_CloseSessionRequest *p) { memset(p, 0, sizeof(UA_CloseSessionRequest)); }
static UA_INLINE UA_CloseSessionRequest * UA_CloseSessionRequest_new(void) { return (UA_CloseSessionRequest*) UA_new(&UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]); }
static UA_INLINE UA_StatusCode UA_CloseSessionRequest_copy(const UA_CloseSessionRequest *src, UA_CloseSessionRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]); }
static UA_INLINE void UA_CloseSessionRequest_deleteMembers(UA_CloseSessionRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]); }
static UA_INLINE void UA_CloseSessionRequest_delete(UA_CloseSessionRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]); }

/**
 * ModifySubscriptionRequest
 * -------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_UInt32 subscriptionId;
    UA_Double requestedPublishingInterval;
    UA_UInt32 requestedLifetimeCount;
    UA_UInt32 requestedMaxKeepAliveCount;
    UA_UInt32 maxNotificationsPerPublish;
    UA_Byte priority;
} UA_ModifySubscriptionRequest;

#define UA_TYPES_MODIFYSUBSCRIPTIONREQUEST 98
static UA_INLINE void UA_ModifySubscriptionRequest_init(UA_ModifySubscriptionRequest *p) { memset(p, 0, sizeof(UA_ModifySubscriptionRequest)); }
static UA_INLINE UA_ModifySubscriptionRequest * UA_ModifySubscriptionRequest_new(void) { return (UA_ModifySubscriptionRequest*) UA_new(&UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]); }
static UA_INLINE UA_StatusCode UA_ModifySubscriptionRequest_copy(const UA_ModifySubscriptionRequest *src, UA_ModifySubscriptionRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]); }
static UA_INLINE void UA_ModifySubscriptionRequest_deleteMembers(UA_ModifySubscriptionRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]); }
static UA_INLINE void UA_ModifySubscriptionRequest_delete(UA_ModifySubscriptionRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]); }

/**
 * UserTokenPolicy
 * ---------------
 * Describes a user token that can be used with a server. */
typedef struct {
    UA_String policyId;
    UA_UserTokenType tokenType;
    UA_String issuedTokenType;
    UA_String issuerEndpointUrl;
    UA_String securityPolicyUri;
} UA_UserTokenPolicy;

#define UA_TYPES_USERTOKENPOLICY 99
static UA_INLINE void UA_UserTokenPolicy_init(UA_UserTokenPolicy *p) { memset(p, 0, sizeof(UA_UserTokenPolicy)); }
static UA_INLINE UA_UserTokenPolicy * UA_UserTokenPolicy_new(void) { return (UA_UserTokenPolicy*) UA_new(&UA_TYPES[UA_TYPES_USERTOKENPOLICY]); }
static UA_INLINE UA_StatusCode UA_UserTokenPolicy_copy(const UA_UserTokenPolicy *src, UA_UserTokenPolicy *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]); }
static UA_INLINE void UA_UserTokenPolicy_deleteMembers(UA_UserTokenPolicy *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]); }
static UA_INLINE void UA_UserTokenPolicy_delete(UA_UserTokenPolicy *p) { UA_delete(p, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]); }

/**
 * DeleteMonitoredItemsRequest
 * ---------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_UInt32 subscriptionId;
    size_t monitoredItemIdsSize;
    UA_UInt32 *monitoredItemIds;
} UA_DeleteMonitoredItemsRequest;

#define UA_TYPES_DELETEMONITOREDITEMSREQUEST 100
static UA_INLINE void UA_DeleteMonitoredItemsRequest_init(UA_DeleteMonitoredItemsRequest *p) { memset(p, 0, sizeof(UA_DeleteMonitoredItemsRequest)); }
static UA_INLINE UA_DeleteMonitoredItemsRequest * UA_DeleteMonitoredItemsRequest_new(void) { return (UA_DeleteMonitoredItemsRequest*) UA_new(&UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]); }
static UA_INLINE UA_StatusCode UA_DeleteMonitoredItemsRequest_copy(const UA_DeleteMonitoredItemsRequest *src, UA_DeleteMonitoredItemsRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]); }
static UA_INLINE void UA_DeleteMonitoredItemsRequest_deleteMembers(UA_DeleteMonitoredItemsRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]); }
static UA_INLINE void UA_DeleteMonitoredItemsRequest_delete(UA_DeleteMonitoredItemsRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]); }

/**
 * ReferenceTypeAttributes
 * -----------------------
 * The attributes for a reference type node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Boolean isAbstract;
    UA_Boolean symmetric;
    UA_LocalizedText inverseName;
} UA_ReferenceTypeAttributes;

#define UA_TYPES_REFERENCETYPEATTRIBUTES 101
static UA_INLINE void UA_ReferenceTypeAttributes_init(UA_ReferenceTypeAttributes *p) { memset(p, 0, sizeof(UA_ReferenceTypeAttributes)); }
static UA_INLINE UA_ReferenceTypeAttributes * UA_ReferenceTypeAttributes_new(void) { return (UA_ReferenceTypeAttributes*) UA_new(&UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_ReferenceTypeAttributes_copy(const UA_ReferenceTypeAttributes *src, UA_ReferenceTypeAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]); }
static UA_INLINE void UA_ReferenceTypeAttributes_deleteMembers(UA_ReferenceTypeAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]); }
static UA_INLINE void UA_ReferenceTypeAttributes_delete(UA_ReferenceTypeAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]); }

/**
 * BrowsePath
 * ----------
 * A request to translate a path into a node id. */
typedef struct {
    UA_NodeId startingNode;
    UA_RelativePath relativePath;
} UA_BrowsePath;

#define UA_TYPES_BROWSEPATH 102
static UA_INLINE void UA_BrowsePath_init(UA_BrowsePath *p) { memset(p, 0, sizeof(UA_BrowsePath)); }
static UA_INLINE UA_BrowsePath * UA_BrowsePath_new(void) { return (UA_BrowsePath*) UA_new(&UA_TYPES[UA_TYPES_BROWSEPATH]); }
static UA_INLINE UA_StatusCode UA_BrowsePath_copy(const UA_BrowsePath *src, UA_BrowsePath *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEPATH]); }
static UA_INLINE void UA_BrowsePath_deleteMembers(UA_BrowsePath *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSEPATH]); }
static UA_INLINE void UA_BrowsePath_delete(UA_BrowsePath *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEPATH]); }

/**
 * UnregisterNodesResponse
 * -----------------------
 * Unregisters one or more previously registered nodes. */
typedef struct {
    UA_ResponseHeader responseHeader;
} UA_UnregisterNodesResponse;

#define UA_TYPES_UNREGISTERNODESRESPONSE 103
static UA_INLINE void UA_UnregisterNodesResponse_init(UA_UnregisterNodesResponse *p) { memset(p, 0, sizeof(UA_UnregisterNodesResponse)); }
static UA_INLINE UA_UnregisterNodesResponse * UA_UnregisterNodesResponse_new(void) { return (UA_UnregisterNodesResponse*) UA_new(&UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]); }
static UA_INLINE UA_StatusCode UA_UnregisterNodesResponse_copy(const UA_UnregisterNodesResponse *src, UA_UnregisterNodesResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]); }
static UA_INLINE void UA_UnregisterNodesResponse_deleteMembers(UA_UnregisterNodesResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]); }
static UA_INLINE void UA_UnregisterNodesResponse_delete(UA_UnregisterNodesResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]); }

/**
 * WriteRequest
 * ------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t nodesToWriteSize;
    UA_WriteValue *nodesToWrite;
} UA_WriteRequest;

#define UA_TYPES_WRITEREQUEST 104
static UA_INLINE void UA_WriteRequest_init(UA_WriteRequest *p) { memset(p, 0, sizeof(UA_WriteRequest)); }
static UA_INLINE UA_WriteRequest * UA_WriteRequest_new(void) { return (UA_WriteRequest*) UA_new(&UA_TYPES[UA_TYPES_WRITEREQUEST]); }
static UA_INLINE UA_StatusCode UA_WriteRequest_copy(const UA_WriteRequest *src, UA_WriteRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_WRITEREQUEST]); }
static UA_INLINE void UA_WriteRequest_deleteMembers(UA_WriteRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_WRITEREQUEST]); }
static UA_INLINE void UA_WriteRequest_delete(UA_WriteRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_WRITEREQUEST]); }

/**
 * ObjectAttributes
 * ----------------
 * The attributes for an object node. */
typedef struct {
    UA_UInt32 specifiedAttributes;
    UA_LocalizedText displayName;
    UA_LocalizedText description;
    UA_UInt32 writeMask;
    UA_UInt32 userWriteMask;
    UA_Byte eventNotifier;
} UA_ObjectAttributes;

#define UA_TYPES_OBJECTATTRIBUTES 105
static UA_INLINE void UA_ObjectAttributes_init(UA_ObjectAttributes *p) { memset(p, 0, sizeof(UA_ObjectAttributes)); }
static UA_INLINE UA_ObjectAttributes * UA_ObjectAttributes_new(void) { return (UA_ObjectAttributes*) UA_new(&UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]); }
static UA_INLINE UA_StatusCode UA_ObjectAttributes_copy(const UA_ObjectAttributes *src, UA_ObjectAttributes *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]); }
static UA_INLINE void UA_ObjectAttributes_deleteMembers(UA_ObjectAttributes *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]); }
static UA_INLINE void UA_ObjectAttributes_delete(UA_ObjectAttributes *p) { UA_delete(p, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]); }

/**
 * BrowseDescription
 * -----------------
 * A request to browse the the references from a node. */
typedef struct {
    UA_NodeId nodeId;
    UA_BrowseDirection browseDirection;
    UA_NodeId referenceTypeId;
    UA_Boolean includeSubtypes;
    UA_UInt32 nodeClassMask;
    UA_UInt32 resultMask;
} UA_BrowseDescription;

#define UA_TYPES_BROWSEDESCRIPTION 106
static UA_INLINE void UA_BrowseDescription_init(UA_BrowseDescription *p) { memset(p, 0, sizeof(UA_BrowseDescription)); }
static UA_INLINE UA_BrowseDescription * UA_BrowseDescription_new(void) { return (UA_BrowseDescription*) UA_new(&UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_BrowseDescription_copy(const UA_BrowseDescription *src, UA_BrowseDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]); }
static UA_INLINE void UA_BrowseDescription_deleteMembers(UA_BrowseDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]); }
static UA_INLINE void UA_BrowseDescription_delete(UA_BrowseDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]); }

/**
 * RepublishRequest
 * ----------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_UInt32 subscriptionId;
    UA_UInt32 retransmitSequenceNumber;
} UA_RepublishRequest;

#define UA_TYPES_REPUBLISHREQUEST 107
static UA_INLINE void UA_RepublishRequest_init(UA_RepublishRequest *p) { memset(p, 0, sizeof(UA_RepublishRequest)); }
static UA_INLINE UA_RepublishRequest * UA_RepublishRequest_new(void) { return (UA_RepublishRequest*) UA_new(&UA_TYPES[UA_TYPES_REPUBLISHREQUEST]); }
static UA_INLINE UA_StatusCode UA_RepublishRequest_copy(const UA_RepublishRequest *src, UA_RepublishRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]); }
static UA_INLINE void UA_RepublishRequest_deleteMembers(UA_RepublishRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]); }
static UA_INLINE void UA_RepublishRequest_delete(UA_RepublishRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]); }

/**
 * GetEndpointsRequest
 * -------------------
 * Gets the endpoints used by the server. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_String endpointUrl;
    size_t localeIdsSize;
    UA_String *localeIds;
    size_t profileUrisSize;
    UA_String *profileUris;
} UA_GetEndpointsRequest;

#define UA_TYPES_GETENDPOINTSREQUEST 108
static UA_INLINE void UA_GetEndpointsRequest_init(UA_GetEndpointsRequest *p) { memset(p, 0, sizeof(UA_GetEndpointsRequest)); }
static UA_INLINE UA_GetEndpointsRequest * UA_GetEndpointsRequest_new(void) { return (UA_GetEndpointsRequest*) UA_new(&UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]); }
static UA_INLINE UA_StatusCode UA_GetEndpointsRequest_copy(const UA_GetEndpointsRequest *src, UA_GetEndpointsRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]); }
static UA_INLINE void UA_GetEndpointsRequest_deleteMembers(UA_GetEndpointsRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]); }
static UA_INLINE void UA_GetEndpointsRequest_delete(UA_GetEndpointsRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]); }

/**
 * PublishRequest
 * --------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t subscriptionAcknowledgementsSize;
    UA_SubscriptionAcknowledgement *subscriptionAcknowledgements;
} UA_PublishRequest;

#define UA_TYPES_PUBLISHREQUEST 109
static UA_INLINE void UA_PublishRequest_init(UA_PublishRequest *p) { memset(p, 0, sizeof(UA_PublishRequest)); }
static UA_INLINE UA_PublishRequest * UA_PublishRequest_new(void) { return (UA_PublishRequest*) UA_new(&UA_TYPES[UA_TYPES_PUBLISHREQUEST]); }
static UA_INLINE UA_StatusCode UA_PublishRequest_copy(const UA_PublishRequest *src, UA_PublishRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]); }
static UA_INLINE void UA_PublishRequest_deleteMembers(UA_PublishRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]); }
static UA_INLINE void UA_PublishRequest_delete(UA_PublishRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]); }

/**
 * AddNodesResponse
 * ----------------
 * Adds one or more nodes to the server address space. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_AddNodesResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_AddNodesResponse;

#define UA_TYPES_ADDNODESRESPONSE 110
static UA_INLINE void UA_AddNodesResponse_init(UA_AddNodesResponse *p) { memset(p, 0, sizeof(UA_AddNodesResponse)); }
static UA_INLINE UA_AddNodesResponse * UA_AddNodesResponse_new(void) { return (UA_AddNodesResponse*) UA_new(&UA_TYPES[UA_TYPES_ADDNODESRESPONSE]); }
static UA_INLINE UA_StatusCode UA_AddNodesResponse_copy(const UA_AddNodesResponse *src, UA_AddNodesResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]); }
static UA_INLINE void UA_AddNodesResponse_deleteMembers(UA_AddNodesResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]); }
static UA_INLINE void UA_AddNodesResponse_delete(UA_AddNodesResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]); }

/**
 * CloseSecureChannelResponse
 * --------------------------
 * Closes a secure channel. */
typedef struct {
    UA_ResponseHeader responseHeader;
} UA_CloseSecureChannelResponse;

#define UA_TYPES_CLOSESECURECHANNELRESPONSE 111
static UA_INLINE void UA_CloseSecureChannelResponse_init(UA_CloseSecureChannelResponse *p) { memset(p, 0, sizeof(UA_CloseSecureChannelResponse)); }
static UA_INLINE UA_CloseSecureChannelResponse * UA_CloseSecureChannelResponse_new(void) { return (UA_CloseSecureChannelResponse*) UA_new(&UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]); }
static UA_INLINE UA_StatusCode UA_CloseSecureChannelResponse_copy(const UA_CloseSecureChannelResponse *src, UA_CloseSecureChannelResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]); }
static UA_INLINE void UA_CloseSecureChannelResponse_deleteMembers(UA_CloseSecureChannelResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]); }
static UA_INLINE void UA_CloseSecureChannelResponse_delete(UA_CloseSecureChannelResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]); }

/**
 * ModifyMonitoredItemsRequest
 * ---------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_UInt32 subscriptionId;
    UA_TimestampsToReturn timestampsToReturn;
    size_t itemsToModifySize;
    UA_MonitoredItemModifyRequest *itemsToModify;
} UA_ModifyMonitoredItemsRequest;

#define UA_TYPES_MODIFYMONITOREDITEMSREQUEST 112
static UA_INLINE void UA_ModifyMonitoredItemsRequest_init(UA_ModifyMonitoredItemsRequest *p) { memset(p, 0, sizeof(UA_ModifyMonitoredItemsRequest)); }
static UA_INLINE UA_ModifyMonitoredItemsRequest * UA_ModifyMonitoredItemsRequest_new(void) { return (UA_ModifyMonitoredItemsRequest*) UA_new(&UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]); }
static UA_INLINE UA_StatusCode UA_ModifyMonitoredItemsRequest_copy(const UA_ModifyMonitoredItemsRequest *src, UA_ModifyMonitoredItemsRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]); }
static UA_INLINE void UA_ModifyMonitoredItemsRequest_deleteMembers(UA_ModifyMonitoredItemsRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]); }
static UA_INLINE void UA_ModifyMonitoredItemsRequest_delete(UA_ModifyMonitoredItemsRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]); }

/**
 * FindServersRequest
 * ------------------
 * Finds the servers known to the discovery server. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_String endpointUrl;
    size_t localeIdsSize;
    UA_String *localeIds;
    size_t serverUrisSize;
    UA_String *serverUris;
} UA_FindServersRequest;

#define UA_TYPES_FINDSERVERSREQUEST 113
static UA_INLINE void UA_FindServersRequest_init(UA_FindServersRequest *p) { memset(p, 0, sizeof(UA_FindServersRequest)); }
static UA_INLINE UA_FindServersRequest * UA_FindServersRequest_new(void) { return (UA_FindServersRequest*) UA_new(&UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]); }
static UA_INLINE UA_StatusCode UA_FindServersRequest_copy(const UA_FindServersRequest *src, UA_FindServersRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]); }
static UA_INLINE void UA_FindServersRequest_deleteMembers(UA_FindServersRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]); }
static UA_INLINE void UA_FindServersRequest_delete(UA_FindServersRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]); }

/**
 * ReferenceDescription
 * --------------------
 * The description of a reference. */
typedef struct {
    UA_NodeId referenceTypeId;
    UA_Boolean isForward;
    UA_ExpandedNodeId nodeId;
    UA_QualifiedName browseName;
    UA_LocalizedText displayName;
    UA_NodeClass nodeClass;
    UA_ExpandedNodeId typeDefinition;
} UA_ReferenceDescription;

#define UA_TYPES_REFERENCEDESCRIPTION 114
static UA_INLINE void UA_ReferenceDescription_init(UA_ReferenceDescription *p) { memset(p, 0, sizeof(UA_ReferenceDescription)); }
static UA_INLINE UA_ReferenceDescription * UA_ReferenceDescription_new(void) { return (UA_ReferenceDescription*) UA_new(&UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_ReferenceDescription_copy(const UA_ReferenceDescription *src, UA_ReferenceDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]); }
static UA_INLINE void UA_ReferenceDescription_deleteMembers(UA_ReferenceDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]); }
static UA_INLINE void UA_ReferenceDescription_delete(UA_ReferenceDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]); }

/**
 * SetPublishingModeResponse
 * -------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_SetPublishingModeResponse;

#define UA_TYPES_SETPUBLISHINGMODERESPONSE 115
static UA_INLINE void UA_SetPublishingModeResponse_init(UA_SetPublishingModeResponse *p) { memset(p, 0, sizeof(UA_SetPublishingModeResponse)); }
static UA_INLINE UA_SetPublishingModeResponse * UA_SetPublishingModeResponse_new(void) { return (UA_SetPublishingModeResponse*) UA_new(&UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]); }
static UA_INLINE UA_StatusCode UA_SetPublishingModeResponse_copy(const UA_SetPublishingModeResponse *src, UA_SetPublishingModeResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]); }
static UA_INLINE void UA_SetPublishingModeResponse_deleteMembers(UA_SetPublishingModeResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]); }
static UA_INLINE void UA_SetPublishingModeResponse_delete(UA_SetPublishingModeResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]); }

/**
 * ContentFilterResult
 * -------------------
 */
typedef struct {
    size_t elementResultsSize;
    UA_ContentFilterElementResult *elementResults;
    size_t elementDiagnosticInfosSize;
    UA_DiagnosticInfo *elementDiagnosticInfos;
} UA_ContentFilterResult;

#define UA_TYPES_CONTENTFILTERRESULT 116
static UA_INLINE void UA_ContentFilterResult_init(UA_ContentFilterResult *p) { memset(p, 0, sizeof(UA_ContentFilterResult)); }
static UA_INLINE UA_ContentFilterResult * UA_ContentFilterResult_new(void) { return (UA_ContentFilterResult*) UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]); }
static UA_INLINE UA_StatusCode UA_ContentFilterResult_copy(const UA_ContentFilterResult *src, UA_ContentFilterResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]); }
static UA_INLINE void UA_ContentFilterResult_deleteMembers(UA_ContentFilterResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]); }
static UA_INLINE void UA_ContentFilterResult_delete(UA_ContentFilterResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]); }

/**
 * AddReferencesItem
 * -----------------
 * A request to add a reference to the server address space. */
typedef struct {
    UA_NodeId sourceNodeId;
    UA_NodeId referenceTypeId;
    UA_Boolean isForward;
    UA_String targetServerUri;
    UA_ExpandedNodeId targetNodeId;
    UA_NodeClass targetNodeClass;
} UA_AddReferencesItem;

#define UA_TYPES_ADDREFERENCESITEM 117
static UA_INLINE void UA_AddReferencesItem_init(UA_AddReferencesItem *p) { memset(p, 0, sizeof(UA_AddReferencesItem)); }
static UA_INLINE UA_AddReferencesItem * UA_AddReferencesItem_new(void) { return (UA_AddReferencesItem*) UA_new(&UA_TYPES[UA_TYPES_ADDREFERENCESITEM]); }
static UA_INLINE UA_StatusCode UA_AddReferencesItem_copy(const UA_AddReferencesItem *src, UA_AddReferencesItem *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]); }
static UA_INLINE void UA_AddReferencesItem_deleteMembers(UA_AddReferencesItem *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]); }
static UA_INLINE void UA_AddReferencesItem_delete(UA_AddReferencesItem *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]); }

/**
 * QueryDataDescription
 * --------------------
 */
typedef struct {
    UA_RelativePath relativePath;
    UA_UInt32 attributeId;
    UA_String indexRange;
} UA_QueryDataDescription;

#define UA_TYPES_QUERYDATADESCRIPTION 118
static UA_INLINE void UA_QueryDataDescription_init(UA_QueryDataDescription *p) { memset(p, 0, sizeof(UA_QueryDataDescription)); }
static UA_INLINE UA_QueryDataDescription * UA_QueryDataDescription_new(void) { return (UA_QueryDataDescription*) UA_new(&UA_TYPES[UA_TYPES_QUERYDATADESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_QueryDataDescription_copy(const UA_QueryDataDescription *src, UA_QueryDataDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUERYDATADESCRIPTION]); }
static UA_INLINE void UA_QueryDataDescription_deleteMembers(UA_QueryDataDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUERYDATADESCRIPTION]); }
static UA_INLINE void UA_QueryDataDescription_delete(UA_QueryDataDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUERYDATADESCRIPTION]); }

/**
 * CreateSubscriptionResponse
 * --------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_UInt32 subscriptionId;
    UA_Double revisedPublishingInterval;
    UA_UInt32 revisedLifetimeCount;
    UA_UInt32 revisedMaxKeepAliveCount;
} UA_CreateSubscriptionResponse;

#define UA_TYPES_CREATESUBSCRIPTIONRESPONSE 119
static UA_INLINE void UA_CreateSubscriptionResponse_init(UA_CreateSubscriptionResponse *p) { memset(p, 0, sizeof(UA_CreateSubscriptionResponse)); }
static UA_INLINE UA_CreateSubscriptionResponse * UA_CreateSubscriptionResponse_new(void) { return (UA_CreateSubscriptionResponse*) UA_new(&UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]); }
static UA_INLINE UA_StatusCode UA_CreateSubscriptionResponse_copy(const UA_CreateSubscriptionResponse *src, UA_CreateSubscriptionResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]); }
static UA_INLINE void UA_CreateSubscriptionResponse_deleteMembers(UA_CreateSubscriptionResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]); }
static UA_INLINE void UA_CreateSubscriptionResponse_delete(UA_CreateSubscriptionResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]); }

/**
 * DeleteSubscriptionsResponse
 * ---------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_DeleteSubscriptionsResponse;

#define UA_TYPES_DELETESUBSCRIPTIONSRESPONSE 120
static UA_INLINE void UA_DeleteSubscriptionsResponse_init(UA_DeleteSubscriptionsResponse *p) { memset(p, 0, sizeof(UA_DeleteSubscriptionsResponse)); }
static UA_INLINE UA_DeleteSubscriptionsResponse * UA_DeleteSubscriptionsResponse_new(void) { return (UA_DeleteSubscriptionsResponse*) UA_new(&UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_DeleteSubscriptionsResponse_copy(const UA_DeleteSubscriptionsResponse *src, UA_DeleteSubscriptionsResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]); }
static UA_INLINE void UA_DeleteSubscriptionsResponse_deleteMembers(UA_DeleteSubscriptionsResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]); }
static UA_INLINE void UA_DeleteSubscriptionsResponse_delete(UA_DeleteSubscriptionsResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]); }

/**
 * WriteResponse
 * -------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_WriteResponse;

#define UA_TYPES_WRITERESPONSE 121
static UA_INLINE void UA_WriteResponse_init(UA_WriteResponse *p) { memset(p, 0, sizeof(UA_WriteResponse)); }
static UA_INLINE UA_WriteResponse * UA_WriteResponse_new(void) { return (UA_WriteResponse*) UA_new(&UA_TYPES[UA_TYPES_WRITERESPONSE]); }
static UA_INLINE UA_StatusCode UA_WriteResponse_copy(const UA_WriteResponse *src, UA_WriteResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_WRITERESPONSE]); }
static UA_INLINE void UA_WriteResponse_deleteMembers(UA_WriteResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_WRITERESPONSE]); }
static UA_INLINE void UA_WriteResponse_delete(UA_WriteResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_WRITERESPONSE]); }

/**
 * DeleteReferencesResponse
 * ------------------------
 * Delete one or more references from the server address space. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_DeleteReferencesResponse;

#define UA_TYPES_DELETEREFERENCESRESPONSE 122
static UA_INLINE void UA_DeleteReferencesResponse_init(UA_DeleteReferencesResponse *p) { memset(p, 0, sizeof(UA_DeleteReferencesResponse)); }
static UA_INLINE UA_DeleteReferencesResponse * UA_DeleteReferencesResponse_new(void) { return (UA_DeleteReferencesResponse*) UA_new(&UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]); }
static UA_INLINE UA_StatusCode UA_DeleteReferencesResponse_copy(const UA_DeleteReferencesResponse *src, UA_DeleteReferencesResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]); }
static UA_INLINE void UA_DeleteReferencesResponse_deleteMembers(UA_DeleteReferencesResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]); }
static UA_INLINE void UA_DeleteReferencesResponse_delete(UA_DeleteReferencesResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]); }

/**
 * CreateMonitoredItemsResponse
 * ----------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_MonitoredItemCreateResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_CreateMonitoredItemsResponse;

#define UA_TYPES_CREATEMONITOREDITEMSRESPONSE 123
static UA_INLINE void UA_CreateMonitoredItemsResponse_init(UA_CreateMonitoredItemsResponse *p) { memset(p, 0, sizeof(UA_CreateMonitoredItemsResponse)); }
static UA_INLINE UA_CreateMonitoredItemsResponse * UA_CreateMonitoredItemsResponse_new(void) { return (UA_CreateMonitoredItemsResponse*) UA_new(&UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_CreateMonitoredItemsResponse_copy(const UA_CreateMonitoredItemsResponse *src, UA_CreateMonitoredItemsResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]); }
static UA_INLINE void UA_CreateMonitoredItemsResponse_deleteMembers(UA_CreateMonitoredItemsResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]); }
static UA_INLINE void UA_CreateMonitoredItemsResponse_delete(UA_CreateMonitoredItemsResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]); }

/**
 * CallResponse
 * ------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_CallMethodResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_CallResponse;

#define UA_TYPES_CALLRESPONSE 124
static UA_INLINE void UA_CallResponse_init(UA_CallResponse *p) { memset(p, 0, sizeof(UA_CallResponse)); }
static UA_INLINE UA_CallResponse * UA_CallResponse_new(void) { return (UA_CallResponse*) UA_new(&UA_TYPES[UA_TYPES_CALLRESPONSE]); }
static UA_INLINE UA_StatusCode UA_CallResponse_copy(const UA_CallResponse *src, UA_CallResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CALLRESPONSE]); }
static UA_INLINE void UA_CallResponse_deleteMembers(UA_CallResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CALLRESPONSE]); }
static UA_INLINE void UA_CallResponse_delete(UA_CallResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CALLRESPONSE]); }

/**
 * DeleteNodesResponse
 * -------------------
 * Delete one or more nodes from the server address space. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_DeleteNodesResponse;

#define UA_TYPES_DELETENODESRESPONSE 125
static UA_INLINE void UA_DeleteNodesResponse_init(UA_DeleteNodesResponse *p) { memset(p, 0, sizeof(UA_DeleteNodesResponse)); }
static UA_INLINE UA_DeleteNodesResponse * UA_DeleteNodesResponse_new(void) { return (UA_DeleteNodesResponse*) UA_new(&UA_TYPES[UA_TYPES_DELETENODESRESPONSE]); }
static UA_INLINE UA_StatusCode UA_DeleteNodesResponse_copy(const UA_DeleteNodesResponse *src, UA_DeleteNodesResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]); }
static UA_INLINE void UA_DeleteNodesResponse_deleteMembers(UA_DeleteNodesResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]); }
static UA_INLINE void UA_DeleteNodesResponse_delete(UA_DeleteNodesResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]); }

/**
 * RepublishResponse
 * -----------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_NotificationMessage notificationMessage;
} UA_RepublishResponse;

#define UA_TYPES_REPUBLISHRESPONSE 126
static UA_INLINE void UA_RepublishResponse_init(UA_RepublishResponse *p) { memset(p, 0, sizeof(UA_RepublishResponse)); }
static UA_INLINE UA_RepublishResponse * UA_RepublishResponse_new(void) { return (UA_RepublishResponse*) UA_new(&UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]); }
static UA_INLINE UA_StatusCode UA_RepublishResponse_copy(const UA_RepublishResponse *src, UA_RepublishResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]); }
static UA_INLINE void UA_RepublishResponse_deleteMembers(UA_RepublishResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]); }
static UA_INLINE void UA_RepublishResponse_delete(UA_RepublishResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]); }

/**
 * MonitoredItemCreateRequest
 * --------------------------
 */
typedef struct {
    UA_ReadValueId itemToMonitor;
    UA_MonitoringMode monitoringMode;
    UA_MonitoringParameters requestedParameters;
} UA_MonitoredItemCreateRequest;

#define UA_TYPES_MONITOREDITEMCREATEREQUEST 127
static UA_INLINE void UA_MonitoredItemCreateRequest_init(UA_MonitoredItemCreateRequest *p) { memset(p, 0, sizeof(UA_MonitoredItemCreateRequest)); }
static UA_INLINE UA_MonitoredItemCreateRequest * UA_MonitoredItemCreateRequest_new(void) { return (UA_MonitoredItemCreateRequest*) UA_new(&UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]); }
static UA_INLINE UA_StatusCode UA_MonitoredItemCreateRequest_copy(const UA_MonitoredItemCreateRequest *src, UA_MonitoredItemCreateRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]); }
static UA_INLINE void UA_MonitoredItemCreateRequest_deleteMembers(UA_MonitoredItemCreateRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]); }
static UA_INLINE void UA_MonitoredItemCreateRequest_delete(UA_MonitoredItemCreateRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]); }

/**
 * DeleteReferencesRequest
 * -----------------------
 * Delete one or more references from the server address space. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t referencesToDeleteSize;
    UA_DeleteReferencesItem *referencesToDelete;
} UA_DeleteReferencesRequest;

#define UA_TYPES_DELETEREFERENCESREQUEST 128
static UA_INLINE void UA_DeleteReferencesRequest_init(UA_DeleteReferencesRequest *p) { memset(p, 0, sizeof(UA_DeleteReferencesRequest)); }
static UA_INLINE UA_DeleteReferencesRequest * UA_DeleteReferencesRequest_new(void) { return (UA_DeleteReferencesRequest*) UA_new(&UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]); }
static UA_INLINE UA_StatusCode UA_DeleteReferencesRequest_copy(const UA_DeleteReferencesRequest *src, UA_DeleteReferencesRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]); }
static UA_INLINE void UA_DeleteReferencesRequest_deleteMembers(UA_DeleteReferencesRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]); }
static UA_INLINE void UA_DeleteReferencesRequest_delete(UA_DeleteReferencesRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]); }

/**
 * ModifyMonitoredItemsResponse
 * ----------------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_MonitoredItemModifyResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_ModifyMonitoredItemsResponse;

#define UA_TYPES_MODIFYMONITOREDITEMSRESPONSE 129
static UA_INLINE void UA_ModifyMonitoredItemsResponse_init(UA_ModifyMonitoredItemsResponse *p) { memset(p, 0, sizeof(UA_ModifyMonitoredItemsResponse)); }
static UA_INLINE UA_ModifyMonitoredItemsResponse * UA_ModifyMonitoredItemsResponse_new(void) { return (UA_ModifyMonitoredItemsResponse*) UA_new(&UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_ModifyMonitoredItemsResponse_copy(const UA_ModifyMonitoredItemsResponse *src, UA_ModifyMonitoredItemsResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]); }
static UA_INLINE void UA_ModifyMonitoredItemsResponse_deleteMembers(UA_ModifyMonitoredItemsResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]); }
static UA_INLINE void UA_ModifyMonitoredItemsResponse_delete(UA_ModifyMonitoredItemsResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]); }

/**
 * ReadResponse
 * ------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_DataValue *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_ReadResponse;

#define UA_TYPES_READRESPONSE 130
static UA_INLINE void UA_ReadResponse_init(UA_ReadResponse *p) { memset(p, 0, sizeof(UA_ReadResponse)); }
static UA_INLINE UA_ReadResponse * UA_ReadResponse_new(void) { return (UA_ReadResponse*) UA_new(&UA_TYPES[UA_TYPES_READRESPONSE]); }
static UA_INLINE UA_StatusCode UA_ReadResponse_copy(const UA_ReadResponse *src, UA_ReadResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_READRESPONSE]); }
static UA_INLINE void UA_ReadResponse_deleteMembers(UA_ReadResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_READRESPONSE]); }
static UA_INLINE void UA_ReadResponse_delete(UA_ReadResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_READRESPONSE]); }

/**
 * AddReferencesRequest
 * --------------------
 * Adds one or more references to the server address space. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t referencesToAddSize;
    UA_AddReferencesItem *referencesToAdd;
} UA_AddReferencesRequest;

#define UA_TYPES_ADDREFERENCESREQUEST 131
static UA_INLINE void UA_AddReferencesRequest_init(UA_AddReferencesRequest *p) { memset(p, 0, sizeof(UA_AddReferencesRequest)); }
static UA_INLINE UA_AddReferencesRequest * UA_AddReferencesRequest_new(void) { return (UA_AddReferencesRequest*) UA_new(&UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]); }
static UA_INLINE UA_StatusCode UA_AddReferencesRequest_copy(const UA_AddReferencesRequest *src, UA_AddReferencesRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]); }
static UA_INLINE void UA_AddReferencesRequest_deleteMembers(UA_AddReferencesRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]); }
static UA_INLINE void UA_AddReferencesRequest_delete(UA_AddReferencesRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]); }

/**
 * ReadRequest
 * -----------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_Double maxAge;
    UA_TimestampsToReturn timestampsToReturn;
    size_t nodesToReadSize;
    UA_ReadValueId *nodesToRead;
} UA_ReadRequest;

#define UA_TYPES_READREQUEST 132
static UA_INLINE void UA_ReadRequest_init(UA_ReadRequest *p) { memset(p, 0, sizeof(UA_ReadRequest)); }
static UA_INLINE UA_ReadRequest * UA_ReadRequest_new(void) { return (UA_ReadRequest*) UA_new(&UA_TYPES[UA_TYPES_READREQUEST]); }
static UA_INLINE UA_StatusCode UA_ReadRequest_copy(const UA_ReadRequest *src, UA_ReadRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_READREQUEST]); }
static UA_INLINE void UA_ReadRequest_deleteMembers(UA_ReadRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_READREQUEST]); }
static UA_INLINE void UA_ReadRequest_delete(UA_ReadRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_READREQUEST]); }

/**
 * OpenSecureChannelRequest
 * ------------------------
 * Creates a secure channel with a server. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_UInt32 clientProtocolVersion;
    UA_SecurityTokenRequestType requestType;
    UA_MessageSecurityMode securityMode;
    UA_ByteString clientNonce;
    UA_UInt32 requestedLifetime;
} UA_OpenSecureChannelRequest;

#define UA_TYPES_OPENSECURECHANNELREQUEST 133
static UA_INLINE void UA_OpenSecureChannelRequest_init(UA_OpenSecureChannelRequest *p) { memset(p, 0, sizeof(UA_OpenSecureChannelRequest)); }
static UA_INLINE UA_OpenSecureChannelRequest * UA_OpenSecureChannelRequest_new(void) { return (UA_OpenSecureChannelRequest*) UA_new(&UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]); }
static UA_INLINE UA_StatusCode UA_OpenSecureChannelRequest_copy(const UA_OpenSecureChannelRequest *src, UA_OpenSecureChannelRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]); }
static UA_INLINE void UA_OpenSecureChannelRequest_deleteMembers(UA_OpenSecureChannelRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]); }
static UA_INLINE void UA_OpenSecureChannelRequest_delete(UA_OpenSecureChannelRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]); }

/**
 * AddNodesItem
 * ------------
 * A request to add a node to the server address space. */
typedef struct {
    UA_ExpandedNodeId parentNodeId;
    UA_NodeId referenceTypeId;
    UA_ExpandedNodeId requestedNewNodeId;
    UA_QualifiedName browseName;
    UA_NodeClass nodeClass;
    UA_ExtensionObject nodeAttributes;
    UA_ExpandedNodeId typeDefinition;
} UA_AddNodesItem;

#define UA_TYPES_ADDNODESITEM 134
static UA_INLINE void UA_AddNodesItem_init(UA_AddNodesItem *p) { memset(p, 0, sizeof(UA_AddNodesItem)); }
static UA_INLINE UA_AddNodesItem * UA_AddNodesItem_new(void) { return (UA_AddNodesItem*) UA_new(&UA_TYPES[UA_TYPES_ADDNODESITEM]); }
static UA_INLINE UA_StatusCode UA_AddNodesItem_copy(const UA_AddNodesItem *src, UA_AddNodesItem *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESITEM]); }
static UA_INLINE void UA_AddNodesItem_deleteMembers(UA_AddNodesItem *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDNODESITEM]); }
static UA_INLINE void UA_AddNodesItem_delete(UA_AddNodesItem *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESITEM]); }

/**
 * ApplicationDescription
 * ----------------------
 * Describes an application and how to find it. */
typedef struct {
    UA_String applicationUri;
    UA_String productUri;
    UA_LocalizedText applicationName;
    UA_ApplicationType applicationType;
    UA_String gatewayServerUri;
    UA_String discoveryProfileUri;
    size_t discoveryUrlsSize;
    UA_String *discoveryUrls;
} UA_ApplicationDescription;

#define UA_TYPES_APPLICATIONDESCRIPTION 135
static UA_INLINE void UA_ApplicationDescription_init(UA_ApplicationDescription *p) { memset(p, 0, sizeof(UA_ApplicationDescription)); }
static UA_INLINE UA_ApplicationDescription * UA_ApplicationDescription_new(void) { return (UA_ApplicationDescription*) UA_new(&UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_ApplicationDescription_copy(const UA_ApplicationDescription *src, UA_ApplicationDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]); }
static UA_INLINE void UA_ApplicationDescription_deleteMembers(UA_ApplicationDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]); }
static UA_INLINE void UA_ApplicationDescription_delete(UA_ApplicationDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]); }

/**
 * NodeTypeDescription
 * -------------------
 */
typedef struct {
    UA_ExpandedNodeId typeDefinitionNode;
    UA_Boolean includeSubTypes;
    size_t dataToReturnSize;
    UA_QueryDataDescription *dataToReturn;
} UA_NodeTypeDescription;

#define UA_TYPES_NODETYPEDESCRIPTION 136
static UA_INLINE void UA_NodeTypeDescription_init(UA_NodeTypeDescription *p) { memset(p, 0, sizeof(UA_NodeTypeDescription)); }
static UA_INLINE UA_NodeTypeDescription * UA_NodeTypeDescription_new(void) { return (UA_NodeTypeDescription*) UA_new(&UA_TYPES[UA_TYPES_NODETYPEDESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_NodeTypeDescription_copy(const UA_NodeTypeDescription *src, UA_NodeTypeDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_NODETYPEDESCRIPTION]); }
static UA_INLINE void UA_NodeTypeDescription_deleteMembers(UA_NodeTypeDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_NODETYPEDESCRIPTION]); }
static UA_INLINE void UA_NodeTypeDescription_delete(UA_NodeTypeDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_NODETYPEDESCRIPTION]); }

/**
 * FindServersResponse
 * -------------------
 * Finds the servers known to the discovery server. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t serversSize;
    UA_ApplicationDescription *servers;
} UA_FindServersResponse;

#define UA_TYPES_FINDSERVERSRESPONSE 137
static UA_INLINE void UA_FindServersResponse_init(UA_FindServersResponse *p) { memset(p, 0, sizeof(UA_FindServersResponse)); }
static UA_INLINE UA_FindServersResponse * UA_FindServersResponse_new(void) { return (UA_FindServersResponse*) UA_new(&UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_FindServersResponse_copy(const UA_FindServersResponse *src, UA_FindServersResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]); }
static UA_INLINE void UA_FindServersResponse_deleteMembers(UA_FindServersResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]); }
static UA_INLINE void UA_FindServersResponse_delete(UA_FindServersResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]); }

/**
 * ServerStatusDataType
 * --------------------
 */
typedef struct {
    UA_DateTime startTime;
    UA_DateTime currentTime;
    UA_ServerState state;
    UA_BuildInfo buildInfo;
    UA_UInt32 secondsTillShutdown;
    UA_LocalizedText shutdownReason;
} UA_ServerStatusDataType;

#define UA_TYPES_SERVERSTATUSDATATYPE 138
static UA_INLINE void UA_ServerStatusDataType_init(UA_ServerStatusDataType *p) { memset(p, 0, sizeof(UA_ServerStatusDataType)); }
static UA_INLINE UA_ServerStatusDataType * UA_ServerStatusDataType_new(void) { return (UA_ServerStatusDataType*) UA_new(&UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]); }
static UA_INLINE UA_StatusCode UA_ServerStatusDataType_copy(const UA_ServerStatusDataType *src, UA_ServerStatusDataType *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]); }
static UA_INLINE void UA_ServerStatusDataType_deleteMembers(UA_ServerStatusDataType *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]); }
static UA_INLINE void UA_ServerStatusDataType_delete(UA_ServerStatusDataType *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]); }

/**
 * AddReferencesResponse
 * ---------------------
 * Adds one or more references to the server address space. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_StatusCode *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_AddReferencesResponse;

#define UA_TYPES_ADDREFERENCESRESPONSE 139
static UA_INLINE void UA_AddReferencesResponse_init(UA_AddReferencesResponse *p) { memset(p, 0, sizeof(UA_AddReferencesResponse)); }
static UA_INLINE UA_AddReferencesResponse * UA_AddReferencesResponse_new(void) { return (UA_AddReferencesResponse*) UA_new(&UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]); }
static UA_INLINE UA_StatusCode UA_AddReferencesResponse_copy(const UA_AddReferencesResponse *src, UA_AddReferencesResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]); }
static UA_INLINE void UA_AddReferencesResponse_deleteMembers(UA_AddReferencesResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]); }
static UA_INLINE void UA_AddReferencesResponse_delete(UA_AddReferencesResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]); }

/**
 * TranslateBrowsePathsToNodeIdsResponse
 * -------------------------------------
 * Translates one or more paths in the server address space. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_BrowsePathResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_TranslateBrowsePathsToNodeIdsResponse;

#define UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE 140
static UA_INLINE void UA_TranslateBrowsePathsToNodeIdsResponse_init(UA_TranslateBrowsePathsToNodeIdsResponse *p) { memset(p, 0, sizeof(UA_TranslateBrowsePathsToNodeIdsResponse)); }
static UA_INLINE UA_TranslateBrowsePathsToNodeIdsResponse * UA_TranslateBrowsePathsToNodeIdsResponse_new(void) { return (UA_TranslateBrowsePathsToNodeIdsResponse*) UA_new(&UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_copy(const UA_TranslateBrowsePathsToNodeIdsResponse *src, UA_TranslateBrowsePathsToNodeIdsResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]); }
static UA_INLINE void UA_TranslateBrowsePathsToNodeIdsResponse_deleteMembers(UA_TranslateBrowsePathsToNodeIdsResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]); }
static UA_INLINE void UA_TranslateBrowsePathsToNodeIdsResponse_delete(UA_TranslateBrowsePathsToNodeIdsResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]); }

/**
 * ContentFilterElement
 * --------------------
 */
typedef struct {
    UA_FilterOperator filterOperator;
    size_t filterOperandsSize;
    UA_ExtensionObject *filterOperands;
} UA_ContentFilterElement;

#define UA_TYPES_CONTENTFILTERELEMENT 141
static UA_INLINE void UA_ContentFilterElement_init(UA_ContentFilterElement *p) { memset(p, 0, sizeof(UA_ContentFilterElement)); }
static UA_INLINE UA_ContentFilterElement * UA_ContentFilterElement_new(void) { return (UA_ContentFilterElement*) UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]); }
static UA_INLINE UA_StatusCode UA_ContentFilterElement_copy(const UA_ContentFilterElement *src, UA_ContentFilterElement *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]); }
static UA_INLINE void UA_ContentFilterElement_deleteMembers(UA_ContentFilterElement *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]); }
static UA_INLINE void UA_ContentFilterElement_delete(UA_ContentFilterElement *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]); }

/**
 * TranslateBrowsePathsToNodeIdsRequest
 * ------------------------------------
 * Translates one or more paths in the server address space. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t browsePathsSize;
    UA_BrowsePath *browsePaths;
} UA_TranslateBrowsePathsToNodeIdsRequest;

#define UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST 142
static UA_INLINE void UA_TranslateBrowsePathsToNodeIdsRequest_init(UA_TranslateBrowsePathsToNodeIdsRequest *p) { memset(p, 0, sizeof(UA_TranslateBrowsePathsToNodeIdsRequest)); }
static UA_INLINE UA_TranslateBrowsePathsToNodeIdsRequest * UA_TranslateBrowsePathsToNodeIdsRequest_new(void) { return (UA_TranslateBrowsePathsToNodeIdsRequest*) UA_new(&UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]); }
static UA_INLINE UA_StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_copy(const UA_TranslateBrowsePathsToNodeIdsRequest *src, UA_TranslateBrowsePathsToNodeIdsRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]); }
static UA_INLINE void UA_TranslateBrowsePathsToNodeIdsRequest_deleteMembers(UA_TranslateBrowsePathsToNodeIdsRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]); }
static UA_INLINE void UA_TranslateBrowsePathsToNodeIdsRequest_delete(UA_TranslateBrowsePathsToNodeIdsRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]); }

/**
 * CloseSessionResponse
 * --------------------
 * Closes a session with the server. */
typedef struct {
    UA_ResponseHeader responseHeader;
} UA_CloseSessionResponse;

#define UA_TYPES_CLOSESESSIONRESPONSE 143
static UA_INLINE void UA_CloseSessionResponse_init(UA_CloseSessionResponse *p) { memset(p, 0, sizeof(UA_CloseSessionResponse)); }
static UA_INLINE UA_CloseSessionResponse * UA_CloseSessionResponse_new(void) { return (UA_CloseSessionResponse*) UA_new(&UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]); }
static UA_INLINE UA_StatusCode UA_CloseSessionResponse_copy(const UA_CloseSessionResponse *src, UA_CloseSessionResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]); }
static UA_INLINE void UA_CloseSessionResponse_deleteMembers(UA_CloseSessionResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]); }
static UA_INLINE void UA_CloseSessionResponse_delete(UA_CloseSessionResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]); }

/**
 * ServiceFault
 * ------------
 * The response returned by all services when there is a service level error. */
typedef struct {
    UA_ResponseHeader responseHeader;
} UA_ServiceFault;

#define UA_TYPES_SERVICEFAULT 144
static UA_INLINE void UA_ServiceFault_init(UA_ServiceFault *p) { memset(p, 0, sizeof(UA_ServiceFault)); }
static UA_INLINE UA_ServiceFault * UA_ServiceFault_new(void) { return (UA_ServiceFault*) UA_new(&UA_TYPES[UA_TYPES_SERVICEFAULT]); }
static UA_INLINE UA_StatusCode UA_ServiceFault_copy(const UA_ServiceFault *src, UA_ServiceFault *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_SERVICEFAULT]); }
static UA_INLINE void UA_ServiceFault_deleteMembers(UA_ServiceFault *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_SERVICEFAULT]); }
static UA_INLINE void UA_ServiceFault_delete(UA_ServiceFault *p) { UA_delete(p, &UA_TYPES[UA_TYPES_SERVICEFAULT]); }

/**
 * CreateMonitoredItemsRequest
 * ---------------------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_UInt32 subscriptionId;
    UA_TimestampsToReturn timestampsToReturn;
    size_t itemsToCreateSize;
    UA_MonitoredItemCreateRequest *itemsToCreate;
} UA_CreateMonitoredItemsRequest;

#define UA_TYPES_CREATEMONITOREDITEMSREQUEST 145
static UA_INLINE void UA_CreateMonitoredItemsRequest_init(UA_CreateMonitoredItemsRequest *p) { memset(p, 0, sizeof(UA_CreateMonitoredItemsRequest)); }
static UA_INLINE UA_CreateMonitoredItemsRequest * UA_CreateMonitoredItemsRequest_new(void) { return (UA_CreateMonitoredItemsRequest*) UA_new(&UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]); }
static UA_INLINE UA_StatusCode UA_CreateMonitoredItemsRequest_copy(const UA_CreateMonitoredItemsRequest *src, UA_CreateMonitoredItemsRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]); }
static UA_INLINE void UA_CreateMonitoredItemsRequest_deleteMembers(UA_CreateMonitoredItemsRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]); }
static UA_INLINE void UA_CreateMonitoredItemsRequest_delete(UA_CreateMonitoredItemsRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]); }

/**
 * ContentFilter
 * -------------
 */
typedef struct {
    size_t elementsSize;
    UA_ContentFilterElement *elements;
} UA_ContentFilter;

#define UA_TYPES_CONTENTFILTER 146
static UA_INLINE void UA_ContentFilter_init(UA_ContentFilter *p) { memset(p, 0, sizeof(UA_ContentFilter)); }
static UA_INLINE UA_ContentFilter * UA_ContentFilter_new(void) { return (UA_ContentFilter*) UA_new(&UA_TYPES[UA_TYPES_CONTENTFILTER]); }
static UA_INLINE UA_StatusCode UA_ContentFilter_copy(const UA_ContentFilter *src, UA_ContentFilter *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CONTENTFILTER]); }
static UA_INLINE void UA_ContentFilter_deleteMembers(UA_ContentFilter *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CONTENTFILTER]); }
static UA_INLINE void UA_ContentFilter_delete(UA_ContentFilter *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CONTENTFILTER]); }

/**
 * QueryFirstResponse
 * ------------------
 */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t queryDataSetsSize;
    UA_QueryDataSet *queryDataSets;
    UA_ByteString continuationPoint;
    size_t parsingResultsSize;
    UA_ParsingResult *parsingResults;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
    UA_ContentFilterResult filterResult;
} UA_QueryFirstResponse;

#define UA_TYPES_QUERYFIRSTRESPONSE 147
static UA_INLINE void UA_QueryFirstResponse_init(UA_QueryFirstResponse *p) { memset(p, 0, sizeof(UA_QueryFirstResponse)); }
static UA_INLINE UA_QueryFirstResponse * UA_QueryFirstResponse_new(void) { return (UA_QueryFirstResponse*) UA_new(&UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]); }
static UA_INLINE UA_StatusCode UA_QueryFirstResponse_copy(const UA_QueryFirstResponse *src, UA_QueryFirstResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]); }
static UA_INLINE void UA_QueryFirstResponse_deleteMembers(UA_QueryFirstResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]); }
static UA_INLINE void UA_QueryFirstResponse_delete(UA_QueryFirstResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]); }

/**
 * AddNodesRequest
 * ---------------
 * Adds one or more nodes to the server address space. */
typedef struct {
    UA_RequestHeader requestHeader;
    size_t nodesToAddSize;
    UA_AddNodesItem *nodesToAdd;
} UA_AddNodesRequest;

#define UA_TYPES_ADDNODESREQUEST 148
static UA_INLINE void UA_AddNodesRequest_init(UA_AddNodesRequest *p) { memset(p, 0, sizeof(UA_AddNodesRequest)); }
static UA_INLINE UA_AddNodesRequest * UA_AddNodesRequest_new(void) { return (UA_AddNodesRequest*) UA_new(&UA_TYPES[UA_TYPES_ADDNODESREQUEST]); }
static UA_INLINE UA_StatusCode UA_AddNodesRequest_copy(const UA_AddNodesRequest *src, UA_AddNodesRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]); }
static UA_INLINE void UA_AddNodesRequest_deleteMembers(UA_AddNodesRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]); }
static UA_INLINE void UA_AddNodesRequest_delete(UA_AddNodesRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]); }

/**
 * BrowseRequest
 * -------------
 * Browse the references for one or more nodes from the server address space. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_ViewDescription view;
    UA_UInt32 requestedMaxReferencesPerNode;
    size_t nodesToBrowseSize;
    UA_BrowseDescription *nodesToBrowse;
} UA_BrowseRequest;

#define UA_TYPES_BROWSEREQUEST 149
static UA_INLINE void UA_BrowseRequest_init(UA_BrowseRequest *p) { memset(p, 0, sizeof(UA_BrowseRequest)); }
static UA_INLINE UA_BrowseRequest * UA_BrowseRequest_new(void) { return (UA_BrowseRequest*) UA_new(&UA_TYPES[UA_TYPES_BROWSEREQUEST]); }
static UA_INLINE UA_StatusCode UA_BrowseRequest_copy(const UA_BrowseRequest *src, UA_BrowseRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSEREQUEST]); }
static UA_INLINE void UA_BrowseRequest_deleteMembers(UA_BrowseRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSEREQUEST]); }
static UA_INLINE void UA_BrowseRequest_delete(UA_BrowseRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSEREQUEST]); }

/**
 * BrowseResult
 * ------------
 * The result of a browse operation. */
typedef struct {
    UA_StatusCode statusCode;
    UA_ByteString continuationPoint;
    size_t referencesSize;
    UA_ReferenceDescription *references;
} UA_BrowseResult;

#define UA_TYPES_BROWSERESULT 150
static UA_INLINE void UA_BrowseResult_init(UA_BrowseResult *p) { memset(p, 0, sizeof(UA_BrowseResult)); }
static UA_INLINE UA_BrowseResult * UA_BrowseResult_new(void) { return (UA_BrowseResult*) UA_new(&UA_TYPES[UA_TYPES_BROWSERESULT]); }
static UA_INLINE UA_StatusCode UA_BrowseResult_copy(const UA_BrowseResult *src, UA_BrowseResult *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSERESULT]); }
static UA_INLINE void UA_BrowseResult_deleteMembers(UA_BrowseResult *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSERESULT]); }
static UA_INLINE void UA_BrowseResult_delete(UA_BrowseResult *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSERESULT]); }

/**
 * CreateSessionRequest
 * --------------------
 * Creates a new session with the server. */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_ApplicationDescription clientDescription;
    UA_String serverUri;
    UA_String endpointUrl;
    UA_String sessionName;
    UA_ByteString clientNonce;
    UA_ByteString clientCertificate;
    UA_Double requestedSessionTimeout;
    UA_UInt32 maxResponseMessageSize;
} UA_CreateSessionRequest;

#define UA_TYPES_CREATESESSIONREQUEST 151
static UA_INLINE void UA_CreateSessionRequest_init(UA_CreateSessionRequest *p) { memset(p, 0, sizeof(UA_CreateSessionRequest)); }
static UA_INLINE UA_CreateSessionRequest * UA_CreateSessionRequest_new(void) { return (UA_CreateSessionRequest*) UA_new(&UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]); }
static UA_INLINE UA_StatusCode UA_CreateSessionRequest_copy(const UA_CreateSessionRequest *src, UA_CreateSessionRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]); }
static UA_INLINE void UA_CreateSessionRequest_deleteMembers(UA_CreateSessionRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]); }
static UA_INLINE void UA_CreateSessionRequest_delete(UA_CreateSessionRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]); }

/**
 * EndpointDescription
 * -------------------
 * The description of a endpoint that can be used to access a server. */
typedef struct {
    UA_String endpointUrl;
    UA_ApplicationDescription server;
    UA_ByteString serverCertificate;
    UA_MessageSecurityMode securityMode;
    UA_String securityPolicyUri;
    size_t userIdentityTokensSize;
    UA_UserTokenPolicy *userIdentityTokens;
    UA_String transportProfileUri;
    UA_Byte securityLevel;
} UA_EndpointDescription;

#define UA_TYPES_ENDPOINTDESCRIPTION 152
static UA_INLINE void UA_EndpointDescription_init(UA_EndpointDescription *p) { memset(p, 0, sizeof(UA_EndpointDescription)); }
static UA_INLINE UA_EndpointDescription * UA_EndpointDescription_new(void) { return (UA_EndpointDescription*) UA_new(&UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]); }
static UA_INLINE UA_StatusCode UA_EndpointDescription_copy(const UA_EndpointDescription *src, UA_EndpointDescription *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]); }
static UA_INLINE void UA_EndpointDescription_deleteMembers(UA_EndpointDescription *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]); }
static UA_INLINE void UA_EndpointDescription_delete(UA_EndpointDescription *p) { UA_delete(p, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]); }

/**
 * GetEndpointsResponse
 * --------------------
 * Gets the endpoints used by the server. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t endpointsSize;
    UA_EndpointDescription *endpoints;
} UA_GetEndpointsResponse;

#define UA_TYPES_GETENDPOINTSRESPONSE 153
static UA_INLINE void UA_GetEndpointsResponse_init(UA_GetEndpointsResponse *p) { memset(p, 0, sizeof(UA_GetEndpointsResponse)); }
static UA_INLINE UA_GetEndpointsResponse * UA_GetEndpointsResponse_new(void) { return (UA_GetEndpointsResponse*) UA_new(&UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]); }
static UA_INLINE UA_StatusCode UA_GetEndpointsResponse_copy(const UA_GetEndpointsResponse *src, UA_GetEndpointsResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]); }
static UA_INLINE void UA_GetEndpointsResponse_deleteMembers(UA_GetEndpointsResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]); }
static UA_INLINE void UA_GetEndpointsResponse_delete(UA_GetEndpointsResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]); }

/**
 * BrowseNextResponse
 * ------------------
 * Continues one or more browse operations. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_BrowseResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_BrowseNextResponse;

#define UA_TYPES_BROWSENEXTRESPONSE 154
static UA_INLINE void UA_BrowseNextResponse_init(UA_BrowseNextResponse *p) { memset(p, 0, sizeof(UA_BrowseNextResponse)); }
static UA_INLINE UA_BrowseNextResponse * UA_BrowseNextResponse_new(void) { return (UA_BrowseNextResponse*) UA_new(&UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]); }
static UA_INLINE UA_StatusCode UA_BrowseNextResponse_copy(const UA_BrowseNextResponse *src, UA_BrowseNextResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]); }
static UA_INLINE void UA_BrowseNextResponse_deleteMembers(UA_BrowseNextResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]); }
static UA_INLINE void UA_BrowseNextResponse_delete(UA_BrowseNextResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]); }

/**
 * BrowseResponse
 * --------------
 * Browse the references for one or more nodes from the server address space. */
typedef struct {
    UA_ResponseHeader responseHeader;
    size_t resultsSize;
    UA_BrowseResult *results;
    size_t diagnosticInfosSize;
    UA_DiagnosticInfo *diagnosticInfos;
} UA_BrowseResponse;

#define UA_TYPES_BROWSERESPONSE 155
static UA_INLINE void UA_BrowseResponse_init(UA_BrowseResponse *p) { memset(p, 0, sizeof(UA_BrowseResponse)); }
static UA_INLINE UA_BrowseResponse * UA_BrowseResponse_new(void) { return (UA_BrowseResponse*) UA_new(&UA_TYPES[UA_TYPES_BROWSERESPONSE]); }
static UA_INLINE UA_StatusCode UA_BrowseResponse_copy(const UA_BrowseResponse *src, UA_BrowseResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_BROWSERESPONSE]); }
static UA_INLINE void UA_BrowseResponse_deleteMembers(UA_BrowseResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_BROWSERESPONSE]); }
static UA_INLINE void UA_BrowseResponse_delete(UA_BrowseResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_BROWSERESPONSE]); }

/**
 * CreateSessionResponse
 * ---------------------
 * Creates a new session with the server. */
typedef struct {
    UA_ResponseHeader responseHeader;
    UA_NodeId sessionId;
    UA_NodeId authenticationToken;
    UA_Double revisedSessionTimeout;
    UA_ByteString serverNonce;
    UA_ByteString serverCertificate;
    size_t serverEndpointsSize;
    UA_EndpointDescription *serverEndpoints;
    size_t serverSoftwareCertificatesSize;
    UA_SignedSoftwareCertificate *serverSoftwareCertificates;
    UA_SignatureData serverSignature;
    UA_UInt32 maxRequestMessageSize;
} UA_CreateSessionResponse;

#define UA_TYPES_CREATESESSIONRESPONSE 156
static UA_INLINE void UA_CreateSessionResponse_init(UA_CreateSessionResponse *p) { memset(p, 0, sizeof(UA_CreateSessionResponse)); }
static UA_INLINE UA_CreateSessionResponse * UA_CreateSessionResponse_new(void) { return (UA_CreateSessionResponse*) UA_new(&UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]); }
static UA_INLINE UA_StatusCode UA_CreateSessionResponse_copy(const UA_CreateSessionResponse *src, UA_CreateSessionResponse *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]); }
static UA_INLINE void UA_CreateSessionResponse_deleteMembers(UA_CreateSessionResponse *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]); }
static UA_INLINE void UA_CreateSessionResponse_delete(UA_CreateSessionResponse *p) { UA_delete(p, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]); }

/**
 * QueryFirstRequest
 * -----------------
 */
typedef struct {
    UA_RequestHeader requestHeader;
    UA_ViewDescription view;
    size_t nodeTypesSize;
    UA_NodeTypeDescription *nodeTypes;
    UA_ContentFilter filter;
    UA_UInt32 maxDataSetsToReturn;
    UA_UInt32 maxReferencesToReturn;
} UA_QueryFirstRequest;

#define UA_TYPES_QUERYFIRSTREQUEST 157
static UA_INLINE void UA_QueryFirstRequest_init(UA_QueryFirstRequest *p) { memset(p, 0, sizeof(UA_QueryFirstRequest)); }
static UA_INLINE UA_QueryFirstRequest * UA_QueryFirstRequest_new(void) { return (UA_QueryFirstRequest*) UA_new(&UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST]); }
static UA_INLINE UA_StatusCode UA_QueryFirstRequest_copy(const UA_QueryFirstRequest *src, UA_QueryFirstRequest *dst) { return UA_copy(src, dst, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST]); }
static UA_INLINE void UA_QueryFirstRequest_deleteMembers(UA_QueryFirstRequest *p) { UA_deleteMembers(p, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST]); }
static UA_INLINE void UA_QueryFirstRequest_delete(UA_QueryFirstRequest *p) { UA_delete(p, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST]); }

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_connection.h" ***********************************/

/*
 * Copyright (C) 2014-2016 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif


/* Forward declarations */
struct UA_Connection;
typedef struct UA_Connection UA_Connection;

struct UA_SecureChannel;
typedef struct UA_SecureChannel UA_SecureChannel;

/**
 * Networking
 * ----------
 * Client-server connection is represented by a `UA_Connection` structure. In
 * order to allow for different operating systems and connection types. For
 * this, `UA_Connection` stores a pointer to user-defined data and
 * function-pointers to interact with the underlying networking implementation.
 *
 * An example networklayer for TCP communication is contained in the plugins
 * folder. The networklayer forwards messages with `UA_Connection` structures to
 * the main open62541 library. The library can then return messages vie TCP
 * without being aware of the underlying transport technology.
 *
 * Connection Config
 * ================= */
typedef struct UA_ConnectionConfig {
    UA_UInt32 protocolVersion;
    UA_UInt32 sendBufferSize;
    UA_UInt32 recvBufferSize;
    UA_UInt32 maxMessageSize;
    UA_UInt32 maxChunkCount;
} UA_ConnectionConfig;

extern const UA_EXPORT UA_ConnectionConfig UA_ConnectionConfig_standard;

/**
 * Connection Structure
 * ==================== */
typedef enum UA_ConnectionState {
    UA_CONNECTION_OPENING,     /* The socket is open, but the HEL/ACK handshake
                                  is not done */
    UA_CONNECTION_ESTABLISHED, /* The socket is open and the connection
                                  configured */
    UA_CONNECTION_CLOSED,      /* The socket has been closed and the connection
                                  will be deleted */
} UA_ConnectionState;

struct UA_Connection {
    UA_ConnectionState state;
    UA_ConnectionConfig localConf;
    UA_ConnectionConfig remoteConf;
    UA_SecureChannel *channel;       /* The securechannel that is attached to
                                        this connection */
    UA_Int32 sockfd;                 /* Most connectivity solutions run on
                                        sockets. Having the socket id here
                                        simplifies the design. */
    void *handle;                    /* A pointer to the networklayer */
    UA_ByteString incompleteMessage; /* A half-received message (TCP is a
                                        streaming protocol) is stored here */

    /* Get a buffer for sending */
    UA_StatusCode (*getSendBuffer)(UA_Connection *connection, size_t length,
                                   UA_ByteString *buf);

    /* Release the send buffer manually */
    void (*releaseSendBuffer)(UA_Connection *connection, UA_ByteString *buf);

    /* Sends a message over the connection. The message buffer is always freed,
     * even if sending fails.
     *
     * @param connection The connection
     * @param buf The message buffer
     * @return Returns an error code or UA_STATUSCODE_GOOD. */
    UA_StatusCode (*send)(UA_Connection *connection, UA_ByteString *buf);

    /* Receive a message from the remote connection
     *
	 * @param connection The connection
	 * @param response The response string. It is allocated by the connection
     *        and needs to be freed with connection->releaseBuffer
     * @param timeout Timeout of the recv operation in milliseconds
     * @return Returns UA_STATUSCODE_BADCOMMUNICATIONERROR if the recv operation
     *         can be repeated, UA_STATUSCODE_GOOD if it succeeded and
     *         UA_STATUSCODE_BADCONNECTIONCLOSED if the connection was
     *         closed. */
    UA_StatusCode (*recv)(UA_Connection *connection, UA_ByteString *response, UA_UInt32 timeout);

    /* Release the buffer of a received message */
    void (*releaseRecvBuffer)(UA_Connection *connection, UA_ByteString *buf);

    /* Close the connection */
    void (*close)(UA_Connection *connection);
};

void UA_EXPORT UA_Connection_init(UA_Connection *connection);
void UA_EXPORT UA_Connection_deleteMembers(UA_Connection *connection);

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_job.h" ***********************************/

 /*
 * Copyright (C) 2014 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */



#ifdef __cplusplus
extern "C" {
#endif

struct UA_Server;
typedef struct UA_Server UA_Server;

typedef void (*UA_ServerCallback)(UA_Server *server, void *data);

/** Jobs describe work that is executed once or repeatedly in the server */
typedef struct {
    enum {
        UA_JOBTYPE_NOTHING, ///< Guess what?
        UA_JOBTYPE_DETACHCONNECTION, ///< Detach the connection from the secure channel (but don't delete it)
        UA_JOBTYPE_BINARYMESSAGE_NETWORKLAYER, ///< The binary message is memory managed by the networklayer
        UA_JOBTYPE_BINARYMESSAGE_ALLOCATED, ///< The binary message was relocated away from the networklayer
        UA_JOBTYPE_METHODCALL, ///< Call the method as soon as possible
        UA_JOBTYPE_METHODCALL_DELAYED, ///< Call the method as soon as all previous jobs have finished
    } type;
    union {
        UA_Connection *closeConnection;
        struct {
            UA_Connection *connection;
            UA_ByteString message;
        } binaryMessage;
        struct {
            void *data;
            UA_ServerCallback method;
        } methodCall;
    } job;
} UA_Job;

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_log.h" ***********************************/

/*
 * Copyright (C) 2014-2016 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Logging
 * -------
 * Servers and clients may contain a logger. Every logger needs to implement the
 * `UA_Logger` signature. An example logger that writes to stdout is provided in
 * the plugins folder.
 *
 * Every log-message consists of a log-level, a log-category and a string
 * message content. The timestamp of the log-message is created within the
 * logger.
 */

typedef enum {
    UA_LOGLEVEL_TRACE,
    UA_LOGLEVEL_DEBUG,
    UA_LOGLEVEL_INFO,
    UA_LOGLEVEL_WARNING,
    UA_LOGLEVEL_ERROR,
    UA_LOGLEVEL_FATAL
} UA_LogLevel;

typedef enum {
    UA_LOGCATEGORY_NETWORK,
    UA_LOGCATEGORY_SECURECHANNEL,
    UA_LOGCATEGORY_SESSION,
    UA_LOGCATEGORY_SERVER,
    UA_LOGCATEGORY_CLIENT,
    UA_LOGCATEGORY_USERLAND
} UA_LogCategory;
    
/**
 * The signature of the logger. The msg string and following varargs are
 * formatted according to the rules of the printf command.
 *
 * Do not use the logger directly but make use of the following macros that take
 * the minimum log-level defined in ua_config.h into account. */
typedef void (*UA_Logger)(UA_LogLevel level, UA_LogCategory category, const char *msg, ...);

#if UA_LOGLEVEL <= 100
#define UA_LOG_TRACE(LOGGER, CATEGORY, ...) do { \
        if(LOGGER) LOGGER(UA_LOGLEVEL_TRACE, CATEGORY, __VA_ARGS__); } while(0)
#else
#define UA_LOG_TRACE(LOGGER, CATEGORY, ...) do {} while(0)
#endif

#if UA_LOGLEVEL <= 200
#define UA_LOG_DEBUG(LOGGER, CATEGORY, ...) do { \
        if(LOGGER) LOGGER(UA_LOGLEVEL_DEBUG, CATEGORY, __VA_ARGS__); } while(0)
#else
#define UA_LOG_DEBUG(LOGGER, CATEGORY, ...) do {} while(0)
#endif

#if UA_LOGLEVEL <= 300
#define UA_LOG_INFO(LOGGER, CATEGORY, ...) do { \
        if(LOGGER) LOGGER(UA_LOGLEVEL_INFO, CATEGORY, __VA_ARGS__); } while(0)
#else
#define UA_LOG_INFO(LOGGER, CATEGORY, ...) do {} while(0)
#endif

#if UA_LOGLEVEL <= 400
#define UA_LOG_WARNING(LOGGER, CATEGORY, ...) do { \
        if(LOGGER) LOGGER(UA_LOGLEVEL_WARNING, CATEGORY, __VA_ARGS__); } while(0)
#else
#define UA_LOG_WARNING(LOGGER, CATEGORY, ...) do {} while(0)
#endif

#if UA_LOGLEVEL <= 500
#define UA_LOG_ERROR(LOGGER, CATEGORY, ...) do { \
        if(LOGGER) LOGGER(UA_LOGLEVEL_ERROR, CATEGORY, __VA_ARGS__); } while(0)
#else
#define UA_LOG_ERROR(LOGGER, CATEGORY, ...) do {} while(0)
#endif

#if UA_LOGLEVEL <= 600
#define UA_LOG_FATAL(LOGGER, CATEGORY, ...) do { \
        if(LOGGER) LOGGER(UA_LOGLEVEL_FATAL, CATEGORY, __VA_ARGS__); } while(0)
#else
#define UA_LOG_FATAL(LOGGER, CATEGORY, ...) do {} while(0)
#endif

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_server.h" ***********************************/

 /*
 * Copyright (C) 2014 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Server
 * ======
 *
 * Network Layer
 * -------------
 * Interface to the binary network layers. The functions in the network layer
 * are never called in parallel but only sequentially from the server's main
 * loop. So the network layer does not need to be thread-safe. */
struct UA_ServerNetworkLayer;
typedef struct UA_ServerNetworkLayer UA_ServerNetworkLayer;    

struct UA_ServerNetworkLayer {
    void *handle; // pointer to internal data
    UA_String discoveryUrl;

    /* Starts listening on the the networklayer.
     *
     * @param nl The network layer
     * @param logger The logger
     * @return Returns UA_STATUSCODE_GOOD or an error code. */
    UA_StatusCode (*start)(UA_ServerNetworkLayer *nl, UA_Logger logger);
    
    /* Gets called from the main server loop and returns the jobs (accumulated
     * messages and close events) for dispatch.
     *
     * @param nl The network layer
     * @param jobs When the returned integer is >0, *jobs points to an array of UA_Job of the
     *             returned size.
     * @param timeout The timeout during which an event must arrive in microseconds
     * @return The size of the jobs array. If the result is negative, an error has occurred. */
    size_t (*getJobs)(UA_ServerNetworkLayer *nl, UA_Job **jobs, UA_UInt16 timeout);

    /* Closes the network connection and returns all the jobs that need to be
     * finished before the network layer can be safely deleted.
     *
     * @param nl The network layer
     * @param jobs When the returned integer is >0, jobs points to an array of UA_Job of the
     *             returned size.
     * @return The size of the jobs array. If the result is negative, an error has occurred. */
    size_t (*stop)(UA_ServerNetworkLayer *nl, UA_Job **jobs);

    /** Deletes the network content. Call only after stopping. */
    void (*deleteMembers)(UA_ServerNetworkLayer *nl);
};

/**
 * Server Configuration
 * --------------------
 * The following structure is passed to a new server for configuration. */
typedef struct {
    UA_String username;
    UA_String password;
} UA_UsernamePasswordLogin;

typedef struct {
    UA_UInt32 min;
    UA_UInt32 max;
} UA_UInt32Range;

typedef struct {
	UA_Double min;
	UA_Double max;
} UA_DoubleRange;

typedef struct {
    UA_UInt16 nThreads; // only if multithreading is enabled
    UA_Logger logger;

    UA_BuildInfo buildInfo;
    UA_ApplicationDescription applicationDescription;
    UA_ByteString serverCertificate;

    /* Networking */
    size_t networkLayersSize;
    UA_ServerNetworkLayer *networkLayers;

    /* Login */
    UA_Boolean enableAnonymousLogin;
    UA_Boolean enableUsernamePasswordLogin;
    size_t usernamePasswordLoginsSize;
    UA_UsernamePasswordLogin* usernamePasswordLogins;

    /* Limits for subscription settings */
	UA_DoubleRange publishingIntervalLimits;
	UA_UInt32Range lifeTimeCountLimits;
	UA_UInt32Range keepAliveCountLimits;
	UA_UInt32 maxNotificationsPerPublish;

	/* Limits for monitoreditem settings */
    UA_DoubleRange samplingIntervalLimits;
	UA_UInt32Range queueSizeLimits;
} UA_ServerConfig;

/**
 * Server Lifecycle
 * ---------------- */
UA_Server UA_EXPORT * UA_Server_new(const UA_ServerConfig config);
void UA_EXPORT UA_Server_delete(UA_Server *server);

/* Runs the main loop of the server. In each iteration, this calls into the
 * networklayers to see if jobs have arrived and checks if repeated jobs need to
 * be triggered.
 *
 * @param server The server object.
 * @param running The loop is run as long as *running is true. Otherwise, the server shuts down.
 * @return Returns the statuscode of the UA_Server_run_shutdown method */
UA_StatusCode UA_EXPORT UA_Server_run(UA_Server *server, volatile UA_Boolean *running);

/* The prologue part of UA_Server_run (no need to use if you call UA_Server_run) */
UA_StatusCode UA_EXPORT UA_Server_run_startup(UA_Server *server);

/* Executes a single iteration of the server's main loop.
 *
 * @param server The server object.
 * @param waitInternal Should we wait for messages in the networklayer?
 *        Otherwise, the timouts for the networklayers are set to zero.
 *        The default max wait time is 50millisec.
 * @return Returns how long we can wait until the next scheduled job (in millisec) */
UA_UInt16 UA_EXPORT UA_Server_run_iterate(UA_Server *server, UA_Boolean waitInternal);

/* The epilogue part of UA_Server_run (no need to use if you call UA_Server_run) */
UA_StatusCode UA_EXPORT UA_Server_run_shutdown(UA_Server *server);

/**
 * Modify a running server
 * ----------------------- */
/* Add a job for cyclic repetition to the server.
 *
 * @param server The server object.
 * @param job The job that shall be added.
 * @param interval The job shall be repeatedly executed with the given interval
 *        (in ms). The interval must be larger than 5ms. The first execution
 *        occurs at now() + interval at the latest.
 * @param jobId Set to the guid of the repeated job. This can be used to cancel
 *        the job later on. If the pointer is null, the guid is not set.
 * @return Upon success, UA_STATUSCODE_GOOD is returned. An error code otherwise. */
UA_StatusCode UA_EXPORT UA_Server_addRepeatedJob(UA_Server *server, UA_Job job,
                                                 UA_UInt32 interval, UA_Guid *jobId);

/* Remove repeated job. The entry will be removed asynchronously during the next
 * iteration of the server main loop.
 *
 * @param server The server object.
 * @param jobId The id of the job that shall be removed.
 * @return Upon sucess, UA_STATUSCODE_GOOD is returned. An error code otherwise. */
UA_StatusCode UA_EXPORT UA_Server_removeRepeatedJob(UA_Server *server, UA_Guid jobId);

/* Add a new namespace to the server. Returns the index of the new namespace */
UA_UInt16 UA_EXPORT UA_Server_addNamespace(UA_Server *server, const char* name);

/**
 * Node Management
 * ---------------
 *
 * Callback Mechanisms
 * ^^^^^^^^^^^^^^^^^^^
 * There are four mechanisms for callbacks from the node-based information model
 * into userspace:
 *
 * - Datasources for variable nodes, where the variable content is managed
 *   externally
 * - Value-callbacks for variable nodes, where userspace is notified when a
 *   read/write occurs
 * - Object lifecycle management, where a user-defined constructor and
 *   destructor is added to an object type
 * - Method callbacks, where a user-defined method is exposed in the information
 *   model
 *
 * Data Source Callback
 * ~~~~~~~~~~~~~~~~~~~~
 * Datasources are the interface to local data providers. It is expected that
 * the read and release callbacks are implemented. The write callback can be set
 * to a null-pointer. */
typedef struct {
    void *handle; /* A custom pointer to reuse the same datasource functions for
                     multiple sources */
    /* Copies the data from the source into the provided value.
     *
     * @param handle An optional pointer to user-defined data for the specific data source
     * @param nodeid Id of the read node
     * @param includeSourceTimeStamp If true, then the datasource is expected to set the source
     *        timestamp in the returned value
     * @param range If not null, then the datasource shall return only a
     *        selection of the (nonscalar) data. Set
     *        UA_STATUSCODE_BADINDEXRANGEINVALID in the value if this does not
     *        apply.
     * @param value The (non-null) DataValue that is returned to the client. The
     *        data source sets the read data, the result status and optionally a
     *        sourcetimestamp.
     * @return Returns a status code for logging. Error codes intended for the
     *         original caller are set in the value. If an error is returned,
     *         then no releasing of the value is done. */
    UA_StatusCode (*read)(void *handle, const UA_NodeId nodeid,
                          UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range,
                          UA_DataValue *value);

    /* Write into a data source. The write member of UA_DataSource can be empty
     * if the operation is unsupported.
     *
     * @param handle An optional pointer to user-defined data for the specific data source
     * @param nodeid Id of the node being written to
     * @param data The data to be written into the data source
     * @param range An optional data range. If the data source is scalar or does
     *        not support writing of ranges, then an error code is returned.
     * @return Returns a status code that is returned to the user
     */
    UA_StatusCode (*write)(void *handle, const UA_NodeId nodeid,
                           const UA_Variant *data, const UA_NumericRange *range);
} UA_DataSource;

UA_StatusCode UA_EXPORT
UA_Server_setVariableNode_dataSource(UA_Server *server, const UA_NodeId nodeId,
                                     const UA_DataSource dataSource);

/**
 * Value Callback
 * ~~~~~~~~~~~~~~
 * Value Callbacks can be attached to variable and variable type nodes. If
 * not-null, they are called before reading and after writing respectively. */
typedef struct {
    void *handle;
    void (*onRead)(void *handle, const UA_NodeId nodeid,
                   const UA_Variant *data, const UA_NumericRange *range);
    void (*onWrite)(void *handle, const UA_NodeId nodeid,
                    const UA_Variant *data, const UA_NumericRange *range);
} UA_ValueCallback;

UA_StatusCode UA_EXPORT
UA_Server_setVariableNode_valueCallback(UA_Server *server, const UA_NodeId nodeId,
                                        const UA_ValueCallback callback);

/**
 * Object Lifecycle Management Callbacks
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Lifecycle management adds constructor and destructor callbacks to
 * object types. */
typedef struct {
    /* Returns the instance handle that is then attached to the node */
    void * (*constructor)(const UA_NodeId instance);
    void (*destructor)(const UA_NodeId instance, void *instanceHandle);
} UA_ObjectLifecycleManagement;

UA_StatusCode UA_EXPORT
UA_Server_setObjectTypeNode_lifecycleManagement(UA_Server *server, UA_NodeId nodeId,
                                                UA_ObjectLifecycleManagement olm);

/**
 * Method Callbacks
 * ~~~~~~~~~~~~~~~~ */
typedef UA_StatusCode (*UA_MethodCallback)(void *methodHandle, const UA_NodeId objectId,
                                           size_t inputSize, const UA_Variant *input,
                                           size_t outputSize, UA_Variant *output);

#ifdef UA_ENABLE_METHODCALLS
UA_StatusCode UA_EXPORT
UA_Server_setMethodNode_callback(UA_Server *server, const UA_NodeId methodNodeId,
                                 UA_MethodCallback method, void *handle);
#endif

/**
 * Node Addition and Deletion
 * ^^^^^^^^^^^^^^^^^^^^^^^^^ */
UA_StatusCode UA_EXPORT
UA_Server_deleteNode(UA_Server *server, const UA_NodeId nodeId, UA_Boolean deleteReferences);
    
/**
 * The instantiation callback is used to track the addition of new nodes. It is
 * also called for all sub-nodes contained in an object or variable type node
 * that is instantiated.
 */
typedef struct UA_InstantiationCallback_s {
  UA_StatusCode (*method)(UA_NodeId objectId, UA_NodeId definitionId, void *handle);
  void *handle;
} UA_InstantiationCallback;

/* Don't use this function. There are typed versions as inline functions. */
UA_StatusCode UA_EXPORT
__UA_Server_addNode(UA_Server *server, const UA_NodeClass nodeClass,
                    const UA_NodeId requestedNewNodeId, const UA_NodeId parentNodeId,
                    const UA_NodeId referenceTypeId, const UA_QualifiedName browseName,
                    const UA_NodeId typeDefinition, const UA_NodeAttributes *attr,
                    const UA_DataType *attributeType,
                    UA_InstantiationCallback *instantiationCallback, UA_NodeId *outNewNodeId);

static UA_INLINE UA_StatusCode
UA_Server_addVariableNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                          const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                          const UA_QualifiedName browseName, const UA_NodeId typeDefinition,
                          const UA_VariableAttributes attr,
                          UA_InstantiationCallback *instantiationCallback,
                          UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_VARIABLE, requestedNewNodeId, parentNodeId,
                               referenceTypeId, browseName, typeDefinition,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_addVariableTypeNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                              const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                              const UA_QualifiedName browseName,
                              const UA_VariableTypeAttributes attr,
                              UA_InstantiationCallback *instantiationCallback,
                              UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_VARIABLETYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_addObjectNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                        const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                        const UA_QualifiedName browseName, const UA_NodeId typeDefinition,
                        const UA_ObjectAttributes attr,
                        UA_InstantiationCallback *instantiationCallback,
                        UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_OBJECT, requestedNewNodeId, parentNodeId,
                               referenceTypeId, browseName, typeDefinition,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_addObjectTypeNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                            const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                            const UA_QualifiedName browseName,
                            const UA_ObjectTypeAttributes attr,
                            UA_InstantiationCallback *instantiationCallback,
                            UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_OBJECTTYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_addViewNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                      const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                      const UA_QualifiedName browseName, const UA_ViewAttributes attr,
                      UA_InstantiationCallback *instantiationCallback,
                      UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_VIEW, requestedNewNodeId, parentNodeId,
                               referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_VIEWATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_addReferenceTypeNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                               const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                               const UA_QualifiedName browseName,
                               const UA_ReferenceTypeAttributes attr,
                               UA_InstantiationCallback *instantiationCallback,
                               UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_REFERENCETYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_addDataTypeNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                          const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                          const UA_QualifiedName browseName, const UA_DataTypeAttributes attr,
                          UA_InstantiationCallback *instantiationCallback,
                          UA_NodeId *outNewNodeId) {
    return __UA_Server_addNode(server, UA_NODECLASS_DATATYPE, requestedNewNodeId, parentNodeId,
                               referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr,
                               &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES],
                               instantiationCallback, outNewNodeId); }

UA_StatusCode UA_EXPORT
UA_Server_addDataSourceVariableNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                                    const UA_NodeId parentNodeId,
                                    const UA_NodeId referenceTypeId,
                                    const UA_QualifiedName browseName,
                                    const UA_NodeId typeDefinition,
                                    const UA_VariableAttributes attr,
                                    const UA_DataSource dataSource, UA_NodeId *outNewNodeId);

#ifdef UA_ENABLE_METHODCALLS
UA_StatusCode UA_EXPORT
UA_Server_addMethodNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                        const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                        const UA_QualifiedName browseName, const UA_MethodAttributes attr,
                        UA_MethodCallback method, void *handle,
                        size_t inputArgumentsSize, const UA_Argument* inputArguments, 
                        size_t outputArgumentsSize, const UA_Argument* outputArguments,
                        UA_NodeId *outNewNodeId);
#endif

/**
 * Write Node Attributes
 * ^^^^^^^^^^^^^^^^^^^^^
 * The following node attributes cannot be written
 *
 * - NodeClass
 * - NodeId
 * - Symmetric
 * - ContainsNoLoop
 *  
 * The following attributes cannot be written from the server, as there is no "user" in the server
 *
 * - UserWriteMask
 * - UserAccessLevel
 * - UserExecutable
 *
 * The following attributes are currently taken from the value variant:
 * TODO: Handle them independent from the variable, ensure that the implicit constraints hold
 *
 * - DataType
 * - ValueRank
 * - ArrayDimensions
 * 
 * - Historizing is currently unsupported */
/* Don't use this function. There are typed versions with no additional overhead. */
UA_StatusCode UA_EXPORT
__UA_Server_write(UA_Server *server, const UA_NodeId *nodeId,
                  const UA_AttributeId attributeId,
                  const UA_DataType *type, const void *value);

static UA_INLINE UA_StatusCode
UA_Server_writeBrowseName(UA_Server *server, const UA_NodeId nodeId,
                          const UA_QualifiedName browseName) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_BROWSENAME,
                             &UA_TYPES[UA_TYPES_QUALIFIEDNAME], &browseName); }

static UA_INLINE UA_StatusCode
UA_Server_writeDisplayName(UA_Server *server, const UA_NodeId nodeId,
                           const UA_LocalizedText displayName) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_DISPLAYNAME,
                             &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &displayName); }

static UA_INLINE UA_StatusCode
UA_Server_writeDescription(UA_Server *server, const UA_NodeId nodeId,
                           const UA_LocalizedText description) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_DESCRIPTION,
                             &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &description); }

static UA_INLINE UA_StatusCode
UA_Server_writeWriteMask(UA_Server *server, const UA_NodeId nodeId,
                         const UA_UInt32 writeMask) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_WRITEMASK,
                             &UA_TYPES[UA_TYPES_UINT32], &writeMask); }

static UA_INLINE UA_StatusCode
UA_Server_writeIsAbstract(UA_Server *server, const UA_NodeId nodeId,
                          const UA_Boolean isAbstract) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_ISABSTRACT,
                             &UA_TYPES[UA_TYPES_BOOLEAN], &isAbstract); }

static UA_INLINE UA_StatusCode
UA_Server_writeInverseName(UA_Server *server, const UA_NodeId nodeId,
                           const UA_LocalizedText inverseName) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_INVERSENAME,
                             &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], &inverseName); }

static UA_INLINE UA_StatusCode
UA_Server_writeEventNotifier(UA_Server *server, const UA_NodeId nodeId,
                             const UA_Byte eventNotifier) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER,
                             &UA_TYPES[UA_TYPES_BYTE], &eventNotifier); }

static UA_INLINE UA_StatusCode
UA_Server_writeValue(UA_Server *server, const UA_NodeId nodeId,
                     const UA_Variant value) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_VALUE,
                             &UA_TYPES[UA_TYPES_VARIANT], &value); }

static UA_INLINE UA_StatusCode
UA_Server_writeAccessLevel(UA_Server *server, const UA_NodeId nodeId,
                           const UA_UInt32 accessLevel) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_ACCESSLEVEL,
                             &UA_TYPES[UA_TYPES_UINT32], &accessLevel); }

static UA_INLINE UA_StatusCode
UA_Server_writeMinimumSamplingInterval(UA_Server *server, const UA_NodeId nodeId,
                                       const UA_Double miniumSamplingInterval) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                             &UA_TYPES[UA_TYPES_DOUBLE], &miniumSamplingInterval); }

static UA_INLINE UA_StatusCode
UA_Server_writeExecutable(UA_Server *server, const UA_NodeId nodeId,
                          const UA_Boolean executable) {
    return __UA_Server_write(server, &nodeId, UA_ATTRIBUTEID_EXECUTABLE,
                             &UA_TYPES[UA_TYPES_BOOLEAN], &executable); }

/**
 * Read Node Attributes
 * ^^^^^^^^^^^^^^^^^^^^
 * The following attributes cannot be read, since the local "admin" user always has
 * full rights.
 *
 * - UserWriteMask
 * - UserAccessLevel
 * - UserExecutable */
/* Don't use this function. There are typed versions for every supported attribute. */
UA_StatusCode UA_EXPORT
__UA_Server_read(UA_Server *server, const UA_NodeId *nodeId,
                 UA_AttributeId attributeId, void *v);
  
static UA_INLINE UA_StatusCode
UA_Server_readNodeId(UA_Server *server, const UA_NodeId nodeId,
                     UA_NodeId *outNodeId) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_NODEID, outNodeId); }

static UA_INLINE UA_StatusCode
UA_Server_readNodeClass(UA_Server *server, const UA_NodeId nodeId,
                        UA_NodeClass *outNodeClass) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_NODECLASS, outNodeClass); }

static UA_INLINE UA_StatusCode
UA_Server_readBrowseName(UA_Server *server, const UA_NodeId nodeId,
                         UA_QualifiedName *outBrowseName) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_BROWSENAME, outBrowseName); }

static UA_INLINE UA_StatusCode
UA_Server_readDisplayName(UA_Server *server, const UA_NodeId nodeId,
                          UA_LocalizedText *outDisplayName) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_DISPLAYNAME, outDisplayName); }

static UA_INLINE UA_StatusCode
UA_Server_readDescription(UA_Server *server, const UA_NodeId nodeId,
                          UA_LocalizedText *outDescription) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_DESCRIPTION, outDescription); }

static UA_INLINE UA_StatusCode
UA_Server_readWriteMask(UA_Server *server, const UA_NodeId nodeId,
                        UA_UInt32 *outWriteMask) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_WRITEMASK, outWriteMask); }

static UA_INLINE UA_StatusCode
UA_Server_readIsAbstract(UA_Server *server, const UA_NodeId nodeId,
                         UA_Boolean *outIsAbstract) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_ISABSTRACT, outIsAbstract); }

static UA_INLINE UA_StatusCode
UA_Server_readSymmetric(UA_Server *server, const UA_NodeId nodeId,
                        UA_Boolean *outSymmetric) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_SYMMETRIC, outSymmetric); }

static UA_INLINE UA_StatusCode
UA_Server_readInverseName(UA_Server *server, const UA_NodeId nodeId,
                          UA_LocalizedText *outInverseName) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_INVERSENAME, outInverseName); }

static UA_INLINE UA_StatusCode
UA_Server_readContainsNoLoop(UA_Server *server, const UA_NodeId nodeId,
                             UA_Boolean *outContainsNoLoops) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS,
                            outContainsNoLoops); }

static UA_INLINE UA_StatusCode
UA_Server_readEventNotifier(UA_Server *server, const UA_NodeId nodeId,
                            UA_Byte *outEventNotifier) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, outEventNotifier); }

static UA_INLINE UA_StatusCode
UA_Server_readValue(UA_Server *server, const UA_NodeId nodeId,
                    UA_Variant *outValue) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_VALUE, outValue); }

static UA_INLINE UA_StatusCode
UA_Server_readDataType(UA_Server *server, const UA_NodeId nodeId,
                       UA_NodeId *outDataType) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_DATATYPE, outDataType); }

static UA_INLINE UA_StatusCode
UA_Server_readValueRank(UA_Server *server, const UA_NodeId nodeId,
                        UA_Int32 *outValueRank) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_VALUERANK, outValueRank); }

/* Returns a variant with an int32 array */
static UA_INLINE UA_StatusCode
UA_Server_readArrayDimensions(UA_Server *server, const UA_NodeId nodeId,
                              UA_Variant *outArrayDimensions) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_ARRAYDIMENSIONS,
                            outArrayDimensions); }

static UA_INLINE UA_StatusCode
UA_Server_readAccessLevel(UA_Server *server, const UA_NodeId nodeId,
                          UA_UInt32 *outAccessLevel) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, outAccessLevel); }

static UA_INLINE UA_StatusCode
UA_Server_readMinimumSamplingInterval(UA_Server *server, const UA_NodeId nodeId,
                                      UA_Double *outMinimumSamplingInterval) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL,
                            outMinimumSamplingInterval); }

static UA_INLINE UA_StatusCode
UA_Server_readHistorizing(UA_Server *server, const UA_NodeId nodeId,
                          UA_Boolean *outHistorizing) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_HISTORIZING, outHistorizing); }

static UA_INLINE UA_StatusCode
UA_Server_readExecutable(UA_Server *server, const UA_NodeId nodeId,
                         UA_Boolean *outExecutable) {
    return __UA_Server_read(server, &nodeId, UA_ATTRIBUTEID_EXECUTABLE, outExecutable); }

/**
 * Reference Management
 * -------------------- */
UA_StatusCode UA_EXPORT
UA_Server_addReference(UA_Server *server, const UA_NodeId sourceId, const UA_NodeId refTypeId,
                       const UA_ExpandedNodeId targetId, UA_Boolean isForward);

UA_StatusCode UA_EXPORT
UA_Server_deleteReference(UA_Server *server, const UA_NodeId sourceNodeId,
                          const UA_NodeId referenceTypeId, UA_Boolean isForward,
                          const UA_ExpandedNodeId targetNodeId, UA_Boolean deleteBidirectional);

/**
 * Browsing
 * -------- */
UA_BrowseResult UA_EXPORT
UA_Server_browse(UA_Server *server, UA_UInt32 maxrefs, const UA_BrowseDescription *descr);

UA_BrowseResult UA_EXPORT
UA_Server_browseNext(UA_Server *server, UA_Boolean releaseContinuationPoint,
                     const UA_ByteString *continuationPoint);

#ifndef HAVE_NODEITER_CALLBACK
#define HAVE_NODEITER_CALLBACK
/* Iterate over all nodes referenced by parentNodeId by calling the callback
 * function for each child node (in ifdef because GCC/CLANG handle include order
 * differently) */
typedef UA_StatusCode (*UA_NodeIteratorCallback)(UA_NodeId childId, UA_Boolean isInverse,
                                                 UA_NodeId referenceTypeId, void *handle);
#endif

UA_StatusCode UA_EXPORT
UA_Server_forEachChildNodeCall(UA_Server *server, UA_NodeId parentNodeId,
                               UA_NodeIteratorCallback callback, void *handle);

/**
 * Method Call
 * ----------- */
#ifdef UA_ENABLE_METHODCALLS
UA_CallMethodResult UA_EXPORT
UA_Server_call(UA_Server *server, const UA_CallMethodRequest *request);
#endif

#ifdef __cplusplus
}
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_server_external_ns.h" ***********************************/

 /*
 * Copyright (C) 2014 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * An external application that manages its own data and data model. To plug in
 * outside data sources, one can use
 *
 * - VariableNodes with a data source (functions that are called for read and write access)
 * - An external nodestore that is mapped to specific namespaces
 *
 * If no external nodestore is defined for a nodeid, it is always looked up in
 * the "local" nodestore of open62541. Namespace Zero is always in the local
 * nodestore.
 *
 * @{
 */

typedef UA_Int32 (*UA_ExternalNodeStore_addNodes)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_AddNodesItem *nodesToAdd, UA_UInt32 *indices,
 UA_UInt32 indicesSize, UA_AddNodesResult* addNodesResults, UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_addReferences)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_AddReferencesItem* referencesToAdd,
 UA_UInt32 *indices,UA_UInt32 indicesSize, UA_StatusCode *addReferencesResults,
 UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_deleteNodes)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_DeleteNodesItem *nodesToDelete, UA_UInt32 *indices,
 UA_UInt32 indicesSize, UA_StatusCode *deleteNodesResults, UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_deleteReferences)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_DeleteReferencesItem *referenceToDelete,
 UA_UInt32 *indices, UA_UInt32 indicesSize, UA_StatusCode deleteReferencesresults,
 UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_readNodes)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_ReadValueId *readValueIds, UA_UInt32 *indices,
 UA_UInt32 indicesSize,UA_DataValue *readNodesResults, UA_Boolean timeStampToReturn,
 UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_writeNodes)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_WriteValue *writeValues, UA_UInt32 *indices,
 UA_UInt32 indicesSize, UA_StatusCode *writeNodesResults, UA_DiagnosticInfo *diagnosticInfo);

typedef UA_Int32 (*UA_ExternalNodeStore_browseNodes)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_BrowseDescription *browseDescriptions,
 UA_UInt32 *indices, UA_UInt32 indicesSize, UA_UInt32 requestedMaxReferencesPerNode,
 UA_BrowseResult *browseResults, UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_translateBrowsePathsToNodeIds)
(void *ensHandle, const UA_RequestHeader *requestHeader, UA_BrowsePath *browsePath, UA_UInt32 *indices,
 UA_UInt32 indicesSize, UA_BrowsePathResult *browsePathResults, UA_DiagnosticInfo *diagnosticInfos);

typedef UA_Int32 (*UA_ExternalNodeStore_delete)(void *ensHandle);

typedef struct UA_ExternalNodeStore {
    void *ensHandle;
	UA_ExternalNodeStore_addNodes addNodes;
	UA_ExternalNodeStore_deleteNodes deleteNodes;
	UA_ExternalNodeStore_writeNodes writeNodes;
	UA_ExternalNodeStore_readNodes readNodes;
	UA_ExternalNodeStore_browseNodes browseNodes;
	UA_ExternalNodeStore_translateBrowsePathsToNodeIds translateBrowsePathsToNodeIds;
	UA_ExternalNodeStore_addReferences addReferences;
	UA_ExternalNodeStore_deleteReferences deleteReferences;
	UA_ExternalNodeStore_delete destroy;
} UA_ExternalNodeStore;

UA_StatusCode UA_EXPORT
UA_Server_addExternalNamespace(UA_Server *server, const UA_String *url,
                               UA_ExternalNodeStore *nodeStore, UA_UInt16 *assignedNamespaceIndex);

#ifdef __cplusplus
}
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_client.h" ***********************************/

/*
 * Copyright (C) 2014 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Client
 * ======
 *
 * Client Configuration
 * -------------------- */
typedef UA_Connection (*UA_ConnectClientConnection)(UA_ConnectionConfig localConf,
                                                    const char *endpointUrl,
                                                    UA_Logger logger);

typedef struct UA_ClientConfig {
    UA_UInt32 timeout; //sync response timeout
    UA_UInt32 secureChannelLifeTime; // lifetime in ms (then the channel needs to be renewed)
    UA_Logger logger;
    UA_ConnectionConfig localConnectionConfig;
    UA_ConnectClientConnection connectionFunc;
} UA_ClientConfig;

/**
 * Client Lifecycle
 * ---------------- */
typedef enum {
     UA_CLIENTSTATE_READY,     /* The client is not connected but initialized and ready to
                                  use. */
     UA_CLIENTSTATE_CONNECTED, /* The client is connected to a server. */
     UA_CLIENTSTATE_FAULTED,   /* An error has occured that might have influenced the
                                  connection state. A successfull service call or renewal
                                  of the secure channel will reset the state to
                                  CONNECTED. */
     UA_CLIENTSTATE_ERRORED    /* A non-recoverable error has occured and the connection
                                  is no longer reliable. The client needs to be
                                  disconnected and reinitialized to recover into a
                                  CONNECTED state. */
} UA_ClientState;

struct UA_Client;
typedef struct UA_Client UA_Client;

/* Create a new client
 *
 * @param config for the new client. You can use UA_ClientConfig_standard which has sane defaults
 * @param logger function pointer to a logger function. See examples/logger_stdout.c for a simple
 *               implementation
 * @return return the new Client object */
UA_Client UA_EXPORT * UA_Client_new(UA_ClientConfig config);

/* Get the client connection status */
UA_ClientState UA_EXPORT UA_Client_getState(UA_Client *client);

/* Reset a client */
void UA_EXPORT UA_Client_reset(UA_Client *client);

/* Delete a client */
void UA_EXPORT UA_Client_delete(UA_Client *client);

/**
 * Manage the Connection
 * --------------------- */
/* Gets a list of endpoints of a server
 *
 * @param client to use
 * @param server url to connect (for example "opc.tcp://localhost:16664")
 * @param endpointDescriptionsSize size of the array of endpoint descriptions
 * @param endpointDescriptions array of endpoint descriptions that is allocated by the function (you need to free manually)
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT
UA_Client_getEndpoints(UA_Client *client, const char *serverUrl,
                       size_t* endpointDescriptionsSize,
                       UA_EndpointDescription** endpointDescriptions);

/* Connect to the selected server
 *
 * @param client to use
 * @param endpointURL to connect (for example "opc.tcp://localhost:16664")
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT
UA_Client_connect(UA_Client *client, const char *endpointUrl);

/* Connect to the selected server with the given username and password
 *
 * @param client to use
 * @param endpointURL to connect (for example "opc.tcp://localhost:16664")
 * @param username
 * @param password
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT
UA_Client_connect_username(UA_Client *client, const char *endpointUrl,
                           const char *username, const char *password);

/* Close a connection to the selected server */
UA_StatusCode UA_EXPORT UA_Client_disconnect(UA_Client *client);

/* Renew the underlying secure channel */
UA_StatusCode UA_EXPORT UA_Client_manuallyRenewSecureChannel(UA_Client *client);

/**
 * Raw Services
 * ------------
 * The raw OPC UA services are exposed to the client. But most of them time, it is better to use the
 * convenience functions from `ua_client_highlevel.h` that wrap the raw services. See the Section
 * :ref:`services` for a detailed description of each service. */
/* Don't use this function. Use the type versions below instead. */
void UA_EXPORT
__UA_Client_Service(UA_Client *client, const void *request, const UA_DataType *requestType,
                    void *response, const UA_DataType *responseType);

/**
 * Attribute Service Set
 * ^^^^^^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_ReadResponse
UA_Client_Service_read(UA_Client *client, const UA_ReadRequest request) {
    UA_ReadResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_READREQUEST],
                        &response, &UA_TYPES[UA_TYPES_READRESPONSE]);
    return response; }

static UA_INLINE UA_WriteResponse
UA_Client_Service_write(UA_Client *client, const UA_WriteRequest request) {
    UA_WriteResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_WRITEREQUEST],
                        &response, &UA_TYPES[UA_TYPES_WRITERESPONSE]);
    return response; }

/**
 * Method Service Set
 * ^^^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_CallResponse
UA_Client_Service_call(UA_Client *client, const UA_CallRequest request) {
    UA_CallResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_CALLREQUEST],
                        &response, &UA_TYPES[UA_TYPES_CALLRESPONSE]);
    return response; }

/**
 * NodeManagement Service Set
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_AddNodesResponse
UA_Client_Service_addNodes(UA_Client *client, const UA_AddNodesRequest request) {
    UA_AddNodesResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_ADDNODESREQUEST],
                        &response, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
    return response; }

static UA_INLINE UA_AddReferencesResponse
UA_Client_Service_addReferences(UA_Client *client, const UA_AddReferencesRequest request) {
    UA_AddReferencesResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_ADDNODESREQUEST],
                        &response, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]);
    return response; }

static UA_INLINE UA_DeleteNodesResponse
UA_Client_Service_deleteNodes(UA_Client *client, const UA_DeleteNodesRequest request) {
    UA_DeleteNodesResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_DELETENODESREQUEST],
                        &response, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
    return response; }

static UA_INLINE UA_DeleteReferencesResponse
UA_Client_Service_deleteReferences(UA_Client *client, const UA_DeleteReferencesRequest request) {
    UA_DeleteReferencesResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_DELETENODESREQUEST],
                        &response, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]);
    return response; }

/**
 * View Service Set
 * ^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_BrowseResponse
UA_Client_Service_browse(UA_Client *client, const UA_BrowseRequest request) {
    UA_BrowseResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_BROWSEREQUEST],
                        &response, &UA_TYPES[UA_TYPES_BROWSERESPONSE]);
    return response; }

static UA_INLINE UA_BrowseNextResponse
UA_Client_Service_browseNext(UA_Client *client, const UA_BrowseNextRequest request) {
    UA_BrowseNextResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST],
                        &response, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
    return response; }

static UA_INLINE UA_TranslateBrowsePathsToNodeIdsResponse
UA_Client_Service_translateBrowsePathsToNodeIds(UA_Client *client,
                                                const UA_TranslateBrowsePathsToNodeIdsRequest request) {
    UA_TranslateBrowsePathsToNodeIdsResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST],
                        &response, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]);
    return response; }

static UA_INLINE UA_RegisterNodesResponse
UA_Client_Service_registerNodes(UA_Client *client, const UA_RegisterNodesRequest request) {
    UA_RegisterNodesResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST],
                        &response, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]);
    return response; }

static UA_INLINE UA_UnregisterNodesResponse
UA_Client_Service_unregisterNodes(UA_Client *client, const UA_UnregisterNodesRequest request) {
    UA_UnregisterNodesResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST],
                        &response, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]);
    return response; }

/**
 * Query Service Set
 * ^^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_QueryFirstResponse
UA_Client_Service_queryFirst(UA_Client *client, const UA_QueryFirstRequest request) {
    UA_QueryFirstResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST],
                        &response, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]);
    return response; }

static UA_INLINE UA_QueryNextResponse
UA_Client_Service_queryNext(UA_Client *client, const UA_QueryNextRequest request) {
    UA_QueryNextResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST],
                        &response, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]);
    return response; }

#ifdef UA_ENABLE_SUBSCRIPTIONS

/**
 * MonitoredItem Service Set
 * ^^^^^^^^^^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_CreateMonitoredItemsResponse
UA_Client_Service_createMonitoredItems(UA_Client *client, const UA_CreateMonitoredItemsRequest request) {
    UA_CreateMonitoredItemsResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST],
                        &response, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]);
    return response; }

static UA_INLINE UA_DeleteMonitoredItemsResponse
UA_Client_Service_deleteMonitoredItems(UA_Client *client, const UA_DeleteMonitoredItemsRequest request) {
    UA_DeleteMonitoredItemsResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST],
                        &response, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]);
    return response; }

/**
 * Subscription Service Set
 * ^^^^^^^^^^^^^^^^^^^^^^^^ */
static UA_INLINE UA_CreateSubscriptionResponse
UA_Client_Service_createSubscription(UA_Client *client, const UA_CreateSubscriptionRequest request) {
    UA_CreateSubscriptionResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST],
                        &response, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]);
    return response; }

static UA_INLINE UA_ModifySubscriptionResponse
UA_Client_Service_modifySubscription(UA_Client *client, const UA_ModifySubscriptionRequest request) {
    UA_ModifySubscriptionResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST],
                        &response, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]);
    return response; }

static UA_INLINE UA_DeleteSubscriptionsResponse
UA_Client_Service_deleteSubscriptions(UA_Client *client, const UA_DeleteSubscriptionsRequest request) {
    UA_DeleteSubscriptionsResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST],
                        &response, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]);
    return response; }

static UA_INLINE UA_PublishResponse
UA_Client_Service_publish(UA_Client *client, const UA_PublishRequest request) {
    UA_PublishResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_PUBLISHREQUEST],
                        &response, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
    return response; }

#endif

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/include/ua_client_highlevel.h" ***********************************/

/*
 * Copyright (C) 2014-2016 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Highlevel Client Functionality
 * ------------------------------
 * The following definitions are convenience functions making use of the
 * standard OPC UA services in the background.
 *
 * Read Attributes
 * ===============
 * The following functions can be used to retrieve a single node attribute. Use
 * the regular service to read several attributes at once. */
/* Don't call this function, use the typed versions */
UA_StatusCode UA_EXPORT
__UA_Client_readAttribute(UA_Client *client, const UA_NodeId *nodeId, UA_AttributeId attributeId,
                          void *out, const UA_DataType *outDataType);

static UA_INLINE UA_StatusCode
UA_Client_readNodeIdAttribute(UA_Client *client, const UA_NodeId nodeId, UA_NodeId *outNodeId) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_NODEID, outNodeId, &UA_TYPES[UA_TYPES_NODEID]); }

static UA_INLINE UA_StatusCode
UA_Client_readNodeClassAttribute(UA_Client *client, const UA_NodeId nodeId, UA_NodeClass *outNodeClass) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_NODECLASS, outNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]); }

static UA_INLINE UA_StatusCode
UA_Client_readBrowseNameAttribute(UA_Client *client, const UA_NodeId nodeId, UA_QualifiedName *outBrowseName) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_BROWSENAME, outBrowseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }

static UA_INLINE UA_StatusCode
UA_Client_readDisplayNameAttribute(UA_Client *client, const UA_NodeId nodeId, UA_LocalizedText *outDisplayName) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_DISPLAYNAME, outDisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

static UA_INLINE UA_StatusCode
UA_Client_readDescriptionAttribute(UA_Client *client, const UA_NodeId nodeId, UA_LocalizedText *outDescription) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_DESCRIPTION, outDescription, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

static UA_INLINE UA_StatusCode
UA_Client_readWriteMaskAttribute(UA_Client *client, const UA_NodeId nodeId, UA_UInt32 *outWriteMask) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_WRITEMASK, outWriteMask, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_readUserWriteMaskAttribute(UA_Client *client, const UA_NodeId nodeId, UA_UInt32 *outUserWriteMask) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_USERWRITEMASK, outUserWriteMask, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_readIsAbstractAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Boolean *outIsAbstract) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_ISABSTRACT, outIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_readSymmetricAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Boolean *outSymmetric) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_SYMMETRIC, outSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_readInverseNameAttribute(UA_Client *client, const UA_NodeId nodeId, UA_LocalizedText *outInverseName) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_INVERSENAME, outInverseName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

static UA_INLINE UA_StatusCode
UA_Client_readContainsNoLoopsAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Boolean *outContainsNoLoops) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, outContainsNoLoops, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_readEventNotifierAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Byte *outEventNotifier) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, outEventNotifier, &UA_TYPES[UA_TYPES_BYTE]); }

static UA_INLINE UA_StatusCode
UA_Client_readValueAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Variant *outValue) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_VALUE, outValue, &UA_TYPES[UA_TYPES_VARIANT]); }

static UA_INLINE UA_StatusCode
UA_Client_readDataTypeAttribute(UA_Client *client, const UA_NodeId nodeId, UA_NodeId *outDataType) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_DATATYPE, outDataType, &UA_TYPES[UA_TYPES_NODEID]); }

static UA_INLINE UA_StatusCode
UA_Client_readValueRankAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Int32 *outValueRank) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_VALUERANK, outValueRank, &UA_TYPES[UA_TYPES_INT32]); }

UA_StatusCode UA_EXPORT
UA_Client_readArrayDimensionsAttribute(UA_Client *client, const UA_NodeId nodeId,
                                       UA_Int32 **outArrayDimensions, size_t *outArrayDimensionsSize);

static UA_INLINE UA_StatusCode
UA_Client_readAccessLevelAttribute(UA_Client *client, const UA_NodeId nodeId, UA_UInt32 *outAccessLevel) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, outAccessLevel, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_readUserAccessLevelAttribute(UA_Client *client, const UA_NodeId nodeId, UA_UInt32 *outUserAccessLevel) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_USERACCESSLEVEL, outUserAccessLevel, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_readMinimumSamplingIntervalAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Double *outMinimumSamplingInterval) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL, outMinimumSamplingInterval, &UA_TYPES[UA_TYPES_DOUBLE]); }

static UA_INLINE UA_StatusCode
UA_Client_readHistorizingAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Boolean *outHistorizing) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_HISTORIZING, outHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_readExecutableAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Boolean *outExecutable) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_EXECUTABLE, outExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_readUserExecutableAttribute(UA_Client *client, const UA_NodeId nodeId, UA_Boolean *outUserExecutable) {
    return __UA_Client_readAttribute(client, &nodeId, UA_ATTRIBUTEID_USEREXECUTABLE, outUserExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]); }

/**
 * Write Attributes
 * ================
 * The following functions can be use to write a single node attribute at a
 * time. Use the regular write service to write several attributes at once. */
/* Don't call this function, use the typed versions */
UA_StatusCode UA_EXPORT
__UA_Client_writeAttribute(UA_Client *client, const UA_NodeId *nodeId,
                           UA_AttributeId attributeId, const void *in,
                           const UA_DataType *inDataType);

static UA_INLINE UA_StatusCode
UA_Client_writeNodeIdAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_NodeId *newNodeId) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_NODEID, newNodeId, &UA_TYPES[UA_TYPES_NODEID]); }

static UA_INLINE UA_StatusCode
UA_Client_writeNodeClassAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_NodeClass *newNodeClass) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_NODECLASS, newNodeClass, &UA_TYPES[UA_TYPES_NODECLASS]); }

static UA_INLINE UA_StatusCode
UA_Client_writeBrowseNameAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_QualifiedName *newBrowseName) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_BROWSENAME, newBrowseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }
    
static UA_INLINE UA_StatusCode
UA_Client_writeDisplayNameAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_LocalizedText *newDisplayName) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_DISPLAYNAME, newDisplayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

static UA_INLINE UA_StatusCode
UA_Client_writeDescriptionAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_LocalizedText *newDescription) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_DESCRIPTION, newDescription, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

static UA_INLINE UA_StatusCode
UA_Client_writeWriteMaskAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_UInt32 *newWriteMask) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_WRITEMASK, newWriteMask, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_writeUserWriteMaskAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_UInt32 *newUserWriteMask) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_USERWRITEMASK, newUserWriteMask, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_writeIsAbstractAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Boolean *newIsAbstract) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_ISABSTRACT, newIsAbstract, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_writeSymmetricAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Boolean *newSymmetric) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_SYMMETRIC, newSymmetric, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_writeInverseNameAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_LocalizedText *newInverseName) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_INVERSENAME, newInverseName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

static UA_INLINE UA_StatusCode
UA_Client_writeContainsNoLoopsAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Boolean *newContainsNoLoops) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_CONTAINSNOLOOPS, newContainsNoLoops, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_writeEventNotifierAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Byte *newEventNotifier) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_EVENTNOTIFIER, newEventNotifier, &UA_TYPES[UA_TYPES_BYTE]); }
    
static UA_INLINE UA_StatusCode
UA_Client_writeValueAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Variant *newValue) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_VALUE, newValue, &UA_TYPES[UA_TYPES_VARIANT]); }
                                     
static UA_INLINE UA_StatusCode
UA_Client_writeDataTypeAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_NodeId *newDataType) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_DATATYPE, newDataType, &UA_TYPES[UA_TYPES_NODEID]); }

static UA_INLINE UA_StatusCode
UA_Client_writeValueRankAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Int32 *newValueRank) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_VALUERANK, newValueRank, &UA_TYPES[UA_TYPES_INT32]); }

UA_StatusCode UA_EXPORT
UA_Client_writeArrayDimensionsAttribute(UA_Client *client, const UA_NodeId nodeId,
                                        const UA_Int32 *newArrayDimensions, size_t newArrayDimensionsSize);

static UA_INLINE UA_StatusCode
UA_Client_writeAccessLevelAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_UInt32 *newAccessLevel) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_ACCESSLEVEL, newAccessLevel, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_writeUserAccessLevelAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_UInt32 *newUserAccessLevel) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_USERACCESSLEVEL, newUserAccessLevel, &UA_TYPES[UA_TYPES_UINT32]); }

static UA_INLINE UA_StatusCode
UA_Client_writeMinimumSamplingIntervalAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Double *newMinimumSamplingInterval) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL, newMinimumSamplingInterval, &UA_TYPES[UA_TYPES_DOUBLE]); }

static UA_INLINE UA_StatusCode
UA_Client_writeHistorizingAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Boolean *newHistorizing) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_HISTORIZING, newHistorizing, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_writeExecutableAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Boolean *newExecutable) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_EXECUTABLE, newExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]); }

static UA_INLINE UA_StatusCode
UA_Client_writeUserExecutableAttribute(UA_Client *client, const UA_NodeId nodeId, const UA_Boolean *newUserExecutable) {
    return __UA_Client_writeAttribute(client, &nodeId, UA_ATTRIBUTEID_USEREXECUTABLE, newUserExecutable, &UA_TYPES[UA_TYPES_BOOLEAN]); }

/**
 * Method Calling
 * ============== */
UA_StatusCode UA_EXPORT
UA_Client_call(UA_Client *client, const UA_NodeId objectId, const UA_NodeId methodId,
               size_t inputSize, const UA_Variant *input, size_t *outputSize, UA_Variant **output);

/**
 * Node Management
 * =============== */
UA_StatusCode UA_EXPORT
UA_Client_addReference(UA_Client *client, const UA_NodeId sourceNodeId, const UA_NodeId referenceTypeId,
                       UA_Boolean isForward, const UA_String targetServerUri,
                       const UA_ExpandedNodeId targetNodeId, UA_NodeClass targetNodeClass);

UA_StatusCode UA_EXPORT
UA_Client_deleteReference(UA_Client *client, const UA_NodeId sourceNodeId, const UA_NodeId referenceTypeId,
                          UA_Boolean isForward, const UA_ExpandedNodeId targetNodeId,
                          UA_Boolean deleteBidirectional);

UA_StatusCode UA_EXPORT
UA_Client_deleteNode(UA_Client *client, const UA_NodeId nodeId, UA_Boolean deleteTargetReferences);
    
/* Don't call this function, use the typed versions */
UA_StatusCode UA_EXPORT
__UA_Client_addNode(UA_Client *client, const UA_NodeClass nodeClass,
                    const UA_NodeId requestedNewNodeId, const UA_NodeId parentNodeId,
                    const UA_NodeId referenceTypeId, const UA_QualifiedName browseName,
                    const UA_NodeId typeDefinition, const UA_NodeAttributes *attr,
                    const UA_DataType *attributeType, UA_NodeId *outNewNodeId);

static UA_INLINE UA_StatusCode
UA_Client_addVariableNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                          const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                          const UA_QualifiedName browseName, const UA_NodeId typeDefinition,
                          const UA_VariableAttributes attr, UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_VARIABLE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, typeDefinition,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addVariableTypeNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                              const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                              const UA_QualifiedName browseName, const UA_VariableTypeAttributes attr,
                              UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_VARIABLETYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addObjectNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                        const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                        const UA_QualifiedName browseName, const UA_NodeId typeDefinition,
                        const UA_ObjectAttributes attr, UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_OBJECT, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, typeDefinition,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addObjectTypeNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                            const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                            const UA_QualifiedName browseName, const UA_ObjectTypeAttributes attr,
                            UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_OBJECTTYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addViewNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                      const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                      const UA_QualifiedName browseName, const UA_ViewAttributes attr,
                      UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_VIEW, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addReferenceTypeNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                               const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                               const UA_QualifiedName browseName, const UA_ReferenceTypeAttributes attr,
                               UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_REFERENCETYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addDataTypeNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                          const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                          const UA_QualifiedName browseName, const UA_DataTypeAttributes attr,
                          UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_DATATYPE, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES],
                               outNewNodeId); }

static UA_INLINE UA_StatusCode
UA_Client_addMethodNode(UA_Client *client, const UA_NodeId requestedNewNodeId,
                          const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                          const UA_QualifiedName browseName, const UA_MethodAttributes attr,
                          UA_NodeId *outNewNodeId) {
    return __UA_Client_addNode(client, UA_NODECLASS_METHOD, requestedNewNodeId,
                               parentNodeId, referenceTypeId, browseName, UA_NODEID_NULL,
                               (const UA_NodeAttributes*)&attr, &UA_TYPES[UA_TYPES_METHODATTRIBUTES],
                               outNewNodeId); }

/**
 * Subscriptions Handling
 * ====================== */
#ifdef UA_ENABLE_SUBSCRIPTIONS

typedef struct {
    UA_Double requestedPublishingInterval;
    UA_UInt32 requestedLifetimeCount;
    UA_UInt32 requestedMaxKeepAliveCount;
    UA_UInt32 maxNotificationsPerPublish;
    UA_Boolean publishingEnabled;
    UA_Byte priority;
} UA_SubscriptionSettings;

extern const UA_EXPORT UA_SubscriptionSettings UA_SubscriptionSettings_standard;
    
UA_StatusCode UA_EXPORT
UA_Client_Subscriptions_new(UA_Client *client, UA_SubscriptionSettings settings,
                            UA_UInt32 *newSubscriptionId);
    
UA_StatusCode UA_EXPORT
UA_Client_Subscriptions_remove(UA_Client *client, UA_UInt32 subscriptionId);

UA_StatusCode UA_EXPORT UA_Client_Subscriptions_manuallySendPublishRequest(UA_Client *client);

typedef void (*UA_MonitoredItemHandlingFunction) (UA_UInt32 handle, UA_DataValue *value, void *context);

UA_StatusCode UA_EXPORT
UA_Client_Subscriptions_addMonitoredItem(UA_Client *client, UA_UInt32 subscriptionId,
                                         UA_NodeId nodeId, UA_UInt32 attributeID,
                                         UA_MonitoredItemHandlingFunction handlingFunction,
                                         void *handlingContext, UA_UInt32 *newMonitoredItemId);

UA_StatusCode UA_EXPORT
UA_Client_Subscriptions_removeMonitoredItem(UA_Client *client, UA_UInt32 subscriptionId,
                                            UA_UInt32 monitoredItemId);

#endif

/**
 * Misc Highlevel Functionality
 * ============================ */
/* Get the namespace-index of a namespace-URI
 *
 * @param client The UA_Client struct for this connection
 * @param namespaceUri The interested namespace URI
 * @param namespaceIndex The namespace index of the URI. The value is unchanged
 *        in case of an error
 * @return Indicates whether the operation succeeded or returns an error code */
UA_StatusCode UA_EXPORT
UA_Client_NamespaceGetIndex(UA_Client *client, UA_String *namespaceUri, UA_UInt16 *namespaceIndex);

#ifndef HAVE_NODEITER_CALLBACK
#define HAVE_NODEITER_CALLBACK
/* Iterate over all nodes referenced by parentNodeId by calling the callback
   function for each child node */                                                        
typedef UA_StatusCode (*UA_NodeIteratorCallback)(UA_NodeId childId, UA_Boolean isInverse,
                                                  UA_NodeId referenceTypeId, void *handle);
#endif
 
UA_StatusCode UA_EXPORT
UA_Client_forEachChildNodeCall(UA_Client *client, UA_NodeId parentNodeId,
                               UA_NodeIteratorCallback callback, void *handle) ;

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/plugins/networklayer_tcp.h" ***********************************/

/*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */


#ifdef __cplusplus
extern "C" {
#endif


UA_ServerNetworkLayer UA_EXPORT
UA_ServerNetworkLayerTCP(UA_ConnectionConfig conf, UA_UInt16 port);

UA_Connection UA_EXPORT
UA_ClientConnectionTCP(UA_ConnectionConfig conf, const char *endpointUrl, UA_Logger logger);

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/plugins/logger_stdout.h" ***********************************/

/*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */



#ifdef __cplusplus
extern "C" {
#endif

UA_EXPORT void Logger_Stdout(UA_LogLevel level, UA_LogCategory category, const char *msg, ...);

#ifdef __cplusplus
}
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/plugins/ua_config_standard.h" ***********************************/

/*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */



#ifdef __cplusplus
extern "C" {
#endif

extern UA_EXPORT const UA_ServerConfig UA_ServerConfig_standard;
extern UA_EXPORT const UA_ClientConfig UA_ClientConfig_standard;

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OPEN62541_H_ */