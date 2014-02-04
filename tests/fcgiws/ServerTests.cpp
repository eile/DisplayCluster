/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
/*                     Julio Delgado <julio.delgadomangas@epfl.ch>   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/
#define TESTS 1
#define BOOST_TEST_MODULE RequestBuilderTests
#include <boost/test/unit_test.hpp>
#include "fcgiws/Server.h"
namespace ut = boost::unit_test;

class MockHandler : public fcgiws::Handler
{
public:

    MockHandler() : simulateFailure(false) {}

    virtual fcgiws::ConstResponsePtr handle(const fcgiws::Request& request) const
    {
        if(simulateFailure)
            return fcgiws::ResponsePtr();
        return fcgiws::Response::OK();
    }

    bool simulateFailure;
};

class FakeBuilder : public fcgiws::RequestBuilder
{
public:

    FakeBuilder() : simulateFailure(false) {};

    virtual fcgiws::RequestPtr buildRequest(FCGX_Request& fcgiRequest)
    {
        if(simulateFailure)
            return fcgiws::RequestPtr();
        fcgiws::RequestPtr request(new fcgiws::Request());
        request->resource = "/home/index.html";
        return request;
    }

    bool simulateFailure;
};

class FakeFCGI : public fcgiws::FastCGIWrapper
{
public:
    virtual bool init(const int socket)
    {
        return true;
    }

    virtual bool accept()
    {
        return true;
    }

    virtual FCGX_Request* getRequest()
    {
        return new FCGX_Request();
    }

    bool write(const std::string& msg)
    {
        message = msg;
        return true;
    }

    std::string message;
};

BOOST_AUTO_TEST_CASE( testWhenRequestBuilderFailsReponseIsServerError )
{
    fcgiws::Server server;
    FakeFCGI* fcgi = new FakeFCGI();
    FakeBuilder* builder = new FakeBuilder();

    builder->simulateFailure = true;
    server.setRequestBuilder(builder);
    server.setFastCGIWrapper(fcgi);
    server.fireProcessing();

    BOOST_CHECK_EQUAL(fcgiws::Response::ServerError()->serialize(),
                      fcgi->message);
}

BOOST_AUTO_TEST_CASE( testWhenEmptyMappingsThenReturns404 )
{
    fcgiws::Server server;
    FakeFCGI* fcgi = new FakeFCGI();
    FakeBuilder* builder = new FakeBuilder();

    server.setRequestBuilder(builder);
    server.setFastCGIWrapper(fcgi);
    server.fireProcessing();

    BOOST_CHECK_EQUAL(fcgiws::Response::NotFound()->serialize(),
                      fcgi->message);
}

BOOST_AUTO_TEST_CASE( testWhenMappedURLThenHandlerCalled )
{
    fcgiws::Server server;
    FakeFCGI* fcgi = new FakeFCGI();
    FakeBuilder* builder = new FakeBuilder();
    fcgiws::HandlerPtr handler(new MockHandler());

    server.addHandler("/home/index.html", handler);
    server.setRequestBuilder(builder);
    server.setFastCGIWrapper(fcgi);
    server.fireProcessing();

    BOOST_CHECK_EQUAL(fcgiws::Response::OK()->serialize(),
                      fcgi->message);
}

BOOST_AUTO_TEST_CASE( testWhenUnmappedMappedURLThen404Returned )
{
    fcgiws::Server server;
    FakeFCGI* fcgi = new FakeFCGI();
    FakeBuilder* builder = new FakeBuilder();
    fcgiws::HandlerPtr handler(new MockHandler());

    server.addHandler("/away/index.html", handler);
    server.setRequestBuilder(builder);
    server.setFastCGIWrapper(fcgi);
    server.fireProcessing();

    BOOST_CHECK_EQUAL(fcgiws::Response::NotFound()->serialize(),
                      fcgi->message);
}

BOOST_AUTO_TEST_CASE( testWhenHandlerFailsThenServerError )
{
    fcgiws::Server server;
    FakeFCGI* fcgi = new FakeFCGI();
    FakeBuilder* builder = new FakeBuilder();
    MockHandler* h = new MockHandler();
    fcgiws::HandlerPtr handler(h);

    h->simulateFailure = true;
    builder->simulateFailure = false;
    server.addHandler("/home/index.html", handler);
    server.setRequestBuilder(builder);
    server.setFastCGIWrapper(fcgi);
    server.fireProcessing();

    BOOST_CHECK_EQUAL(fcgiws::Response::ServerError()->serialize(),
                      fcgi->message);
}

