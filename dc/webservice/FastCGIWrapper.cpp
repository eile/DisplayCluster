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

#include <sstream>
#include <cstdio>

#include "FastCGIWrapper.h"

namespace dcWebservice
{

FastCGIWrapper::FastCGIWrapper()
    : _request(new FCGX_Request())
    , _run(false)
    , _socket(-1)
{}

FastCGIWrapper::~FastCGIWrapper()
{}

bool FastCGIWrapper::init(const unsigned int port, const unsigned int nbOfConnections)
{
    std::stringstream ss;
    ss << ":" << port;
    _socket = FCGX_OpenSocket(ss.str().c_str(), nbOfConnections);
    if(_socket < 0)
        return false;

    _run = true;
    FCGX_Init(); // FastCGI takes care of not initializing itself more than once
    FCGX_InitRequest(_request.get(), _socket, 0);
    return true;
}


bool FastCGIWrapper::accept()
{
    /*
     * Here we use non-blocking io. Read "man select"
     * to understand how this is used
     */
    fd_set read_from;
    struct timeval timeout;

    while(_run)
    {
        // Timeout must be reset every iteration
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Set of file descriptors we are interested in must be reset
        // every iteration
        FD_ZERO(&read_from);
        FD_SET(_socket, &read_from);
        const int retval = select(_socket + 1, &read_from, 0,0, &timeout);
        if( retval == -1)
            break;
        else if(retval)
            return FCGX_Accept_r(_request.get()) >= 0;
    }
    return false;
}

FCGX_Request* FastCGIWrapper::getRequest()
{
    return _request.get();
}

bool FastCGIWrapper::write(const std::string& msg)
{
    FCGX_PutS(msg.c_str(), _request->out);
    FCGX_FFlush(_request->out);
    FCGX_FClose(_request->out);
    FCGX_Finish_r(_request.get());
    return true;
}

bool FastCGIWrapper::stop()
{
    _run = false;
    FCGX_ShutdownPending();
    return true;
}

}