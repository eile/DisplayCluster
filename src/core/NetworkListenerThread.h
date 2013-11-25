/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#ifndef NETWORK_LISTENER_THREAD_H
#define NETWORK_LISTENER_THREAD_H

#include "MessageHeader.h"
#include "InteractionState.h"
#include "PixelStreamSegment.h"

#include <QtCore>
#include <QtNetwork/QTcpSocket>
#include <QQueue>

#include "DisplayGroupInterface.h" // TODO REMOVE??

using dc::InteractionState;
using dc::PixelStreamSegment;

class PixelStreamDispatcher;
class DisplayGroupManager;

class NetworkListenerThread : public QObject
{
    Q_OBJECT

public:

    NetworkListenerThread(PixelStreamDispatcher& pixelStreamDispatcher, DisplayGroupManager &displayGroupManager, int socketDescriptor);
    ~NetworkListenerThread();

public slots:

    void setInteractionState(InteractionState interactionState);
    void pixelStreamerClosed(QString uri);

signals:

    void finished();

    void receivedAddPixelStreamSource(QString uri, int sourceIndex);
    void receivedPixelStreamSegement(QString uri, int SourceIndex, PixelStreamSegment segment);
    void receivedPixelStreamFinishFrame(QString uri, int SourceIndex);
    void receivedRemovePixelStreamSource(QString uri, int sourceIndex);

private slots:

    void initialize();
    void process();
    void socketReceiveMessage();

private:

    int socketDescriptor_;
    QTcpSocket * tcpSocket_;

    boost::shared_ptr<DisplayGroupInterface> displayGroupInterface_;

    QString pixelStreamUri_;

    QString interactionName_;
    bool interactionBound_;
    bool interactionExclusive_;

    QQueue<InteractionState> interactionStates_;

    // Event receivers
    PixelStreamDispatcher& pixelStreamDispatcher_;
    DisplayGroupManager& displayGroupManager_;

    MessageHeader receiveMessageHeader();
    QByteArray receiveMessageBody(int size);

    void handleMessage(MessageHeader messageHeader, QByteArray byteArray);
    void handlePixelStreamMessage(const QString& uri, const QByteArray& byteArray);
    bool bindInteraction();

    void sendBindReply( bool successful );
    void send(const InteractionState &interactionState);
    void sendQuit();
    bool send(const MessageHeader& messageHeader);
};

#endif
