/*
    ------------------------------------------------------------------

    This file is part of the Tracking plugin for the Open Ephys GUI
    Written by:

    Alessio Buccino     alessiob@ifi.uio.no
    Mikkel Lepperod
    Svenn-Arne Dragly

    Center for Integrated Neuroplasticity CINPLA
    Department of Biosciences
    University of Oslo
    Norway

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRACKINGNODE_H
#define TRACKINGNODE_H

#include <ProcessorHeaders.h>
#include "TrackingMessage.h"

#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/IpEndpointName.h"
#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"

#include <stdio.h>
#include <queue>
#include <utility>

#define BUFFER_SIZE 4096
#define MAX_SOURCES 10
#define DEF_PORT 27020
#define DEF_ADDRESS "/red"
#define DEF_COLOR "red"

/**
    This helper class allows stores input tracking data in a circular queue.
*/
class TrackingQueue
{
public:

    TrackingQueue();
    ~TrackingQueue();

    void push (const TrackingData &message);
    TrackingData *pop();

    bool isEmpty();
    void clear();

private:
    TrackingData m_buffer[BUFFER_SIZE];
    int m_head;
    int m_tail;
};

/**
    This helper class is an OSC server running its own thread to keep data transmission
    continuous.
*/

class TrackingNode;

class TrackingServer: public osc::OscPacketListener,
    public Thread
{
public:
    TrackingServer ();
    TrackingServer (int port, String address);
    ~TrackingServer();

    void run();
    void stop();

    void addProcessor (TrackingNode* processor);
    void removeProcessor (TrackingNode* processor);

protected:
    virtual void ProcessMessage (const osc::ReceivedMessage& m, const IpEndpointName&);

private:
    TrackingServer (TrackingServer const&);
    void operator= (TrackingServer const&);

    int m_incomingPort;
    String m_address;

    UdpListeningReceiveSocket m_listeningSocket;
    std::vector<TrackingNode*> m_processors;
};


/**
    This source processor allows you to pipe tracking data via OSC signals from Bonsai tracker.

    @see TrackingNodeEditor
*/
class TrackingNode : public GenericProcessor
{
public:
    /** The class constructor, used to initialize any members. */
    TrackingNode();

    /** The class destructor, used to deallocate memory */
    ~TrackingNode();

    AudioProcessorEditor* createEditor();

    void updateSettings() override;

    void receiveMessage (int port, String address, const TrackingData &message);
    int getTrackingModuleIndex(int port, String address);
    void addSource (int port, String address, String color);
    void removeSource (int i);
    int getNSources();
    bool isPortUsed(int port);

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Adds all the new serial data that is available to the event data buffer.
     */
    void process (AudioSampleBuffer&) override;

    /**
        This should only be run by the ProcessorGraph, before acquisition will be started.

        It tries to open the serial port previsouly specified by the setDevice and setBaudrate setters.

        Returns true on success, false if port could not be opened.
    */
    bool isReady() override;

    void saveCustomParametersToXml(XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

    void setAddress (int i, String address);
    String getAddress(int i);
    void setPort (int i,int port);
    int getPort(int i);
    void setColor (int i, String color);
    String getColor(int i);

private:

    struct TrackingModule
    {
        String address;
        int port;
        String color;
        TrackingQueue* messageQueue;
        TrackingServer* server;
    };

    int64 m_startingRecTimeMillis;
    int64 m_startingAcqTimeMillis;

    CriticalSection lock;

    bool m_positionIsUpdated;
    bool m_isRecordingTimeLogged;
    bool m_isAcquisitionTimeLogged;   
    int m_received_msg;

    Array<TrackingModule> trackingModules;
    Array<const EventChannel*> moduleEventChannels;
    int lastNumInputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackingNode);

};


#endif  // TRACKINGNODE_H
