/*********************************************************************/
/* Copyright (c) 2013, The University of Texas at Austin.            */
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

#ifndef DOCK_H
#define DOCK_H

#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QHash>
#include <QtGui/QImage>
#include <dcStream.h>

class PictureFlow;


class ImageStreamer : public QObject
{
    Q_OBJECT

    DcSocket* dcSocket;

public:
    ImageStreamer();
    ~ImageStreamer();

public slots:
    void connect();
    void disconnect();
    void send( const QImage& image );
};

class ImageLoader : public QObject
{
    Q_OBJECT

    PictureFlow* flow_;

public:
    ImageLoader( PictureFlow* flow );
    ~ImageLoader();

public slots:
    void loadImage( const QString& fileName, const int index );
};


class Dock : public QObject
{
    Q_OBJECT

public:
    Dock();
    ~Dock();
    PictureFlow* getFlow() const;

    void open();
    void close();

    void onItem();

    void setPos( const QPointF& pos ) { pos_ = pos; }
    QPointF getPos() const { return pos_; }

Q_SIGNALS:
    void started();
    void finished();
    void renderPreview( const QString& fileName, const int index );

private:
    void changeDirectory( const QString& dir );
    QThread streamThread_;
    QThread loadThread_;
    PictureFlow* flow_;
    ImageStreamer* streamer_;
    ImageLoader* loader_;
    QDir currentDir_;
    QPointF pos_;
    QStringList filters_;
    QHash< QString, int > slideIndex_;

    void addSlide_( const QString& fileName, const QString& shortName,
                    const bool isDir, const QColor& bgcolor1,
                    const QColor& bgcolor2 );

};

#endif