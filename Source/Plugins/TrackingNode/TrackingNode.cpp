/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "TrackingNode.h"
#include "TrackingNodeEditor.h"

//preallocate memory for msg
#define BUFFER_MSG_SIZE 256


TrackingNode::TrackingNode()
    : GenericProcessor("Tracking Port")
    , m_startingRecTimeMillis(0)
    , m_startingAcqTimeMillis(0)
    , eventId(0)
	, m_positionIsUpdated(false)
	, m_port(27020)
    , m_isRecordingTimeLogged(false)
    , m_isAcquisitionTimeLogged(false)
    , m_msgInfo(false)
    , m_msgSent(false)
    , m_msgToSend(0)
    //debug
    , m_cleared_queues(0)
    , m_received_msg(0)
    , countin1sec(0)
    , m_prevTime(0)
    , m_currentTime(0)
    , m_timePassed(0.0)
    , m_msgInBufferIn1sec(0)
    , m_prevTimeBuf(0)
    , m_currentTimeBuf(0)
    , m_timePassedBuf(0.0)
{
    setProcessorType (PROCESSOR_TYPE_SOURCE);
    sendSampleCount = false;
	try {
        TrackingServer::getInstance(m_port)->addProcessor(this);
	} catch(std::runtime_error) {
		DBG("Unable to bind port");
	}
//    memset(m_buffer, 0, BUFFER_SIZE);
}

TrackingNode::~TrackingNode()
{
    TrackingServer::getInstance(m_port)->removeProcessor(this);
    TrackingServer::getInstance(0, true);
}

AudioProcessorEditor* TrackingNode::createEditor()
{
    editor = new TrackingNodeEditor(this, true);
    return editor;
}

//Since the data needs a meximum buffer size but the actual number of read bytes might be less, let's
//add that info as a metadata field.
void TrackingNode::createEventChannels()
{
    //It's going to be raw binary data, so let's make it uint8
    EventChannel* chan = new EventChannel(EventChannel::UINT8_ARRAY, 1, 24, CoreServices::getGlobalSampleRate(), this);
    chan->setName("Tracking data");
    chan->setDescription("Tracking data received from Bonsai. x, y, width, height");
    chan->setIdentifier("external.tracking.rawData");
    eventChannelArray.add(chan);
}

void TrackingNode::setAddress(String address)
{
    m_address = address;
}

String TrackingNode::address()
{
    return m_address;
}

void TrackingNode::setPort(int port)
{
	try{
    TrackingServer::getInstance(m_port)->removeProcessor(this);
    m_port = port;
    TrackingServer::getInstance(port)->addProcessor(this);
	} catch(std::runtime_error){
		DBG("Unable to bind port");
	}
}

int TrackingNode::port()
{
    return m_port;
}

void TrackingNode::process(AudioSampleBuffer&)
{    
//    checkForEvents(events);
    int inBufferNow = msgQueue.getInBuffer();
    int64 timestamp = CoreServices::getGlobalTimestamp();
    setTimestampAndSamples(timestamp, 0);

    if(m_positionIsUpdated && inBufferNow > 0) {

//        lock.enter();
        for (int i=0; i < inBufferNow; i++)
        {

			int64 tsptr = 0;
            char* msg = msgQueue.dequeueMsg(&tsptr);

            if (msg != nullptr)
            {
				// since the event saving messes timestamps up append it to the message
                uint8_t msg_with_ts[BUFFER_MSG_SIZE];
                float position[4];

				memcpy(msg_with_ts, &tsptr, sizeof(tsptr));
                memcpy(&msg_with_ts[sizeof(tsptr)], msg, msgQueue.getMessageSize());

                //setTimestamp(events, tsptr);
                const EventChannel* chan = getEventChannel(getEventChannelIndex(0, getNodeId()));
                BinaryEventPtr event = BinaryEvent::createBinaryEvent(chan, timestamp,
                                                                      msg_with_ts, 24);
                addEvent(chan, event, 0);
                countin1sec++;
            }
            else
                std::cout << "Queue is empty!" << std::endl;

        }
//        lock.exit();
        m_positionIsUpdated = false;
    }

}

void TrackingNode::receiveMessage(std::vector<float> message)
{
    if (CoreServices::getRecordingStatus())
    {
        if (!m_isRecordingTimeLogged)
        {
            m_received_msg = 0;
            m_startingRecTimeMillis =  Time::currentTimeMillis();
            m_isRecordingTimeLogged = true;
            std::cout << "Starting Rec Ts: " << m_startingRecTimeMillis << std::endl;
     		CoreServices::sendStatusMessage("Clearing Queue in rectimelog " + String(m_startingRecTimeMillis));
            msgQueue.clear();
        }

		//m_positionIsUpdated = true;
		//m_message = message;

		//// fill buffer with new messages (assume all messages has the same size)
		//if (!m_msgInfo)
		//{
		//	msgQueue.setMsgInfo(sizeof(float)* m_message.size());
		//	m_msgInfo = true;
		//}

		//int64 ts;
		//ts = Time::currentTimeMillis() - m_startingRecTimeMillis;
		//msgQueue.enqueueMsg((char*)&(m_message[0]), ts);

		//bool correct = msgQueue.checkQueuesConsistency();
		//m_received_msg++;

		//if (!correct)
		//{
		//	msgQueue.clear();
		//	m_cleared_queues++;
		//	String mess = "Cleared input queue " + String(m_cleared_queues) + " times. Received: " + String(m_received_msg);
		//	CoreServices::sendStatusMessage(mess);
		//}
    }
	else
	{
		m_isRecordingTimeLogged = false;
        //msgQueue.clear();
		//CoreServices::sendStatusMessage("Clearing Queue in stop recording");
	}
        

	if (CoreServices::getAcquisitionStatus()) // && !CoreServices::getRecordingStatus())
    {
        if (!m_isAcquisitionTimeLogged)
        {
            m_startingAcqTimeMillis = Time::currentTimeMillis();
            m_isAcquisitionTimeLogged = true;
            std::cout << "Starting Acq Ts: " << m_startingAcqTimeMillis << std::endl;
            msgQueue.clear();
			CoreServices::sendStatusMessage("Clearing Queue in acqtimelog " + String(m_startingAcqTimeMillis));
        }

        m_positionIsUpdated = true;
        m_message = message;

        // fill buffer with new messages (assume all messages has the same size)
        if (!m_msgInfo)
        {
            msgQueue.setMsgInfo(sizeof(float) * m_message.size());
            m_msgInfo = true;
        }

        int64 ts;       
		if (CoreServices::getRecordingStatus())
			ts = Time::currentTimeMillis() - m_startingRecTimeMillis;
		else
			ts = Time::currentTimeMillis() - m_startingAcqTimeMillis;

        msgQueue.enqueueMsg((char*) &(m_message[0]), ts);

        bool correct = msgQueue.checkQueuesConsistency();
        m_received_msg++;

        if (!correct)
        {
            msgQueue.clear();
            m_cleared_queues++;
            String mess = "Cleared input queue " + String(m_cleared_queues) + " times. Received: " + String(m_received_msg);
            CoreServices::sendStatusMessage(mess);
        }

    }
	else
	{
		m_isAcquisitionTimeLogged = false;
        //msgQueue.clear();
		//CoreServices::sendStatusMessage("Clearing Queue in stop acq");
	}

}

bool TrackingNode::isReady()
{
    return true;
}

// Class TrackingQueue methods
TrackingQueue::TrackingQueue()
    : m_msgHead(-1)
    , m_msgTail(-1)
    , m_byteCount(0)
    , m_bufferLength(0)
    , m_msgInBuffer(0)
{
    memset(m_buffer, 0, BUFFER_SIZE);
}

TrackingQueue::~TrackingQueue(){}

void TrackingQueue::enqueueMsg(char* message, int64 ts)
{
    m_msgHead = (m_msgHead + 1) % m_bufferLength;
    memcpy(&m_buffer[m_msgHead*m_byteCount], message, m_byteCount);
    enqueueTimestamp(ts);
}

char* TrackingQueue::dequeueMsg(int64* ts)
{
    if (!isEmpty())
    {
        m_msgTail = (m_msgTail + 1) % m_bufferLength;
        int64 ts_ret = dequeueTimeStamp();
        (*ts) = ts_ret;
        checkQueuesConsistency();
        return &(m_buffer[m_msgTail*m_byteCount]);
    }
    else
        return nullptr;

}

void TrackingQueue::enqueueTimestamp(int64 ts)
{
    m_timestamps.push(ts);
}

int64 TrackingQueue::dequeueTimeStamp()
{
    int64 ts = m_timestamps.front();
    m_timestamps.pop();
    return ts;
}

bool TrackingQueue::isEmpty()
{
    return m_msgHead == m_msgTail;
}

void TrackingQueue::clear()
{
    m_msgHead = -1;
    m_msgTail = -1;
    memset(m_buffer, 0, BUFFER_SIZE);
    // empty queue
    std::queue<int64> empty;
    std::swap(m_timestamps, empty);

}

int TrackingQueue::getHead()
{
    return m_msgHead;
}

int TrackingQueue::getTail()
{
    return m_msgTail;
}

int TrackingQueue::getInBuffer()
{
    int inBufferNow = -1;
    if (!isEmpty())
        inBufferNow = (m_msgHead >= m_msgTail)? m_msgHead - m_msgTail : m_bufferLength - m_msgTail + m_msgHead;
    else
        inBufferNow = 0;
    return inBufferNow;
}

bool TrackingQueue::checkQueuesConsistency()
{
    if (getInBuffer() != m_timestamps.size())
    {
        std::cout << "Queuing error! buffer len: " << getInBuffer() << " tsqueue len: " << m_timestamps.size() << std::endl;
        return false;
    }
    else
        return true;

}

int TrackingQueue::getBufferSize()
{
    return m_bufferLength;
}

int TrackingQueue::getMessageSize()
{
    return m_byteCount;
}

void TrackingQueue::setMsgInfo(int msgSize)
{
    m_byteCount = msgSize;
    m_bufferLength = int(std::floor((float(BUFFER_SIZE)/float(m_byteCount))));

}

