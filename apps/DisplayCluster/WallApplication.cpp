/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
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

#include "WallApplication.h"

#include "log.h"
#include "CommandLineParameters.h"

#include "MPIChannel.h"
#include "WallFromMasterChannel.h"
#include "WallToMasterChannel.h"
#include "WallToWallChannel.h"

#include "configuration/WallConfiguration.h"

#include "RenderContext.h"

#include <stdexcept>

#include <boost/bind.hpp>

WallApplication::WallApplication( int& argc_, char** argv_,
                                  MPIChannelPtr worldChannel,
                                  MPIChannelPtr wallChannel )
    : QApplication( argc_, argv_ )
    , wallChannel_( new WallToWallChannel( wallChannel ))
{
    CommandLineParameters options( argc_, argv_ );
    if( options.getHelp( ))
        options.showSyntax();

    if ( !createConfig( options.getConfigFilename(), worldChannel->getRank( )))
        throw std::runtime_error(" WallApplication: initialization failed." );

    initRenderContext();
    initMPIConnection( worldChannel );
    startRendering();
}

WallApplication::~WallApplication()
{
    mpiReceiveThread_.quit();
    mpiReceiveThread_.wait();

    mpiSendThread_.quit();
    mpiSendThread_.wait();
}

bool WallApplication::createConfig( const QString& filename, const int rank )
{
    try
    {
        config_.reset( new WallConfiguration( filename, rank ));
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Could not load configuration. '%s'", e.what( ));
        return false;
    }
    return true;
}

void WallApplication::initRenderContext()
{
    connect( this, SIGNAL( lastWindowClosed( )), this, SLOT( quit( )));

    try
    {
        renderContext_.reset( new RenderContext( *config_ ));
    }
    catch( const std::runtime_error& e )
    {
        put_flog( LOG_FATAL, "Error creating the RenderContext: '%s'",
                  e.what( ));
        throw std::runtime_error( "WallApplication: initialization failed." );
    }

    renderController_.reset( new RenderController( renderContext_ ));
}

void WallApplication::initMPIConnection( MPIChannelPtr worldChannel )
{
    fromMasterChannel_.reset( new WallFromMasterChannel( worldChannel ));
    toMasterChannel_.reset( new WallToMasterChannel( worldChannel ));

    fromMasterChannel_->moveToThread( &mpiReceiveThread_ );
    toMasterChannel_->moveToThread( &mpiSendThread_ );

    connect( fromMasterChannel_.get(), SIGNAL( receivedQuit( )),
             renderController_.get(), SLOT( updateQuit( )));

    connect( fromMasterChannel_.get(), SIGNAL( received( DisplayGroupPtr )),
             renderController_.get(), SLOT( updateDisplayGroup( DisplayGroupPtr )));

    connect( fromMasterChannel_.get(), SIGNAL( received( OptionsPtr )),
             renderController_.get(), SLOT( updateOptions( OptionsPtr )));

    connect( fromMasterChannel_.get(), SIGNAL( received( MarkersPtr )),
             renderController_.get(), SLOT( updateMarkers( MarkersPtr )));

    connect( fromMasterChannel_.get(),
             SIGNAL( received( deflect::FramePtr )),
             &renderController_->getPixelStreamUpdater(),
             SLOT( updatePixelStream( deflect::FramePtr )));

    if( wallChannel_->getRank() == 0 )
    {
        connect( &renderController_->getPixelStreamUpdater(),
                 SIGNAL( requestFrame( QString )),
                 toMasterChannel_.get(), SLOT( sendRequestFrame( QString )));
    }

    connect( fromMasterChannel_.get(), SIGNAL( receivedQuit( )),
             toMasterChannel_.get(), SLOT( sendQuit( )));

    connect( &mpiReceiveThread_, SIGNAL( started( )),
             fromMasterChannel_.get(), SLOT( processMessages( )));

    mpiReceiveThread_.start();
    mpiSendThread_.start();
}

void WallApplication::startRendering()
{
    // setup connection so renderFrame() will be called continuously.
    // Must be a queued connection to avoid infinite recursion.
    connect( this, SIGNAL( frameFinished( )),
             this, SLOT( renderFrame( )), Qt::QueuedConnection );
    renderFrame();
}

void WallApplication::renderFrame()
{
    wallChannel_->synchronizeClock();

    renderController_->preRenderUpdate( *wallChannel_ );

    renderContext_->updateGLWindows();
    wallChannel_->globalBarrier();
    renderContext_->swapBuffers();

    renderController_->postRenderUpdate( *wallChannel_ );

    if( renderController_->quitRendering( ))
        quit();

    emit( frameFinished( ));
}
