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

#ifndef FFMPEGMOVIE_H
#define FFMPEGMOVIE_H

// required for FFMPEG includes below, specifically for the Linux build
#ifdef __cplusplus
    #ifndef __STDC_CONSTANT_MACROS
        #define __STDC_CONSTANT_MACROS
    #endif

    #ifdef _STDINT_H
        #undef _STDINT_H
    #endif

    #include <stdint.h>
#endif

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/error.h>
    #include <libavutil/mathematics.h>
}

#include "types.h"
#include <deflect/MTQueue.h>

#include <QString>

#include <atomic>
#include <future>
#include <thread>

/**
 * Read and play movies using the FFMPEG library.
 */
class FFMPEGMovie
{
public:
    /**
     * Constructor.
     * @param uri: the movie file to open.
     */
    FFMPEGMovie( const QString& uri );

    /** Destructor */
    ~FFMPEGMovie();

    /** Is the movie valid. */
    bool isValid() const;

    /** Get the frame width. */
    unsigned int getWidth() const;

    /** Get the frame height. */
    unsigned int getHeight() const;

    /** Get the current time position in seconds. */
    double getPosition() const;

    /** True when the EOF was reached and no more frames can be obtained. */
    bool isAtEOF() const;

    /** Get the movie duration in seconds. May be unavailable for some movies. */
    double getDuration() const;

    /** Get the duration of a frame in seconds. */
    double getFrameDuration() const;

    /** Start decoding the movie from the given position. */
    void startDecoding();

    /** Stop decoding the movie. */
    void stopDecoding();

    /** Check if the movie is currently decoding. */
    bool isDecoding() const;

    /**
     * Get a frame at the given position in seconds.
     *
     * If the movie is decoding, any pending future returned by a previous call
     * to this method will be invalidated. If the movie is not decoding, the
     * user is responsible to wait on the completion of the returned future
     * before calling this method again.
     */
    std::future<PicturePtr> getFrame( double posInSeconds );

private:
    AVFormatContext* _avFormatContext;
    std::unique_ptr<FFMPEGVideoStream> _videoStream;

    double _ptsPosition;
    double _streamPosition;
    bool _isValid;
    std::atomic<bool> _isAtEOF;

    deflect::MTQueue<PicturePtr> _queue;
    std::promise<PicturePtr> _promise;

    std::thread _decodeThread;
    std::atomic<bool> _stopDecoding;

    std::thread _consumeThread;
    std::atomic<bool> _stopConsuming;

    std::mutex _seekMutex;
    bool _seek;
    double _seekPosition;
    std::condition_variable _seekRequested;
    std::condition_variable _seekFinished;

    std::mutex _targetMutex;
    double _targetTimestamp;
    bool _targetChangedSent;
    std::condition_variable _targetChanged;

    /** Init the global FFMPEG context. */
    static void initGlobalState();

    bool _open( const QString& uri );
    bool _createAvFormatContext( const QString& uri );
    void _releaseAvFormatContext();

    void _decode();
    void _decodeOneFrame();

    double _getPtsDelta() const;
    void _consume();
    bool _seekTo( double timePosInSeconds );

    bool _readVideoFrame();
    bool _seekFileTo( double timePosInSeconds );
    PicturePtr _grabSingleFrame( const double posInSeconds );
};

#endif // FFMPEGMOVIE_H
