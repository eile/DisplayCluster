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

#ifndef CONTENTWINDOWCONTROLLER_H
#define CONTENTWINDOWCONTROLLER_H

#include "types.h"

#include <QtCore/QRectF>

enum SizeState
{
    SIZE_1TO1,
    SIZE_FULLSCREEN,
    SIZE_NORMALIZED
};

/**
 * Controller for moving and resizing windows.
 */
class ContentWindowController
{
public:
    ContentWindowController( ContentWindow& contentWindow,
                             const DisplayGroup& displayGroup );

    /** Resize the window to the desired state. */
    void adjustSize( const SizeState state );

    /**
     * Constrain the position of the window.
     * @param minArea The minimum area of the window that must remain visible,
     *        given in pixels. If empty, the total window area is used instead.
     */
    void constrainPosition( const QSizeF& minArea = QSizeF( )) const;

    /** Is the size in a valid range. */
    bool isValidSize( const QSizeF& size ) const;

    /** Toggle between fullscreen and 'normalized' by keeping the position
     *  and size after leaving fullscreen */
    void toggleFullscreen();

    SizeState getSizeState() const;

private:
    void clampSize( QSizeF& size ) const;
    QRectF getCenteredCoordinates( const QSizeF& size ) const;

    ContentWindow& contentWindow_;
    const DisplayGroup& displayGroup_;

    SizeState sizeState_;
};

#endif // CONTENTWINDOWCONTROLLER_H
